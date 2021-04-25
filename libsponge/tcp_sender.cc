#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

#include <iostream>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _nBytes_inflight(0)
    , firstConnect(true)
    , _recv_ackno(0)
    , _window_size(0)
    , _syn_sent(0)
    , _fin_sent(0){}

uint64_t TCPSender::bytes_in_flight() const {
    return _nBytes_inflight;
}

void TCPSender::fill_window() {
    if (_next_seqno == 0) {
        TCPSegment seg = TCPSegment();
        seg.header().syn = true;
        seg.header().seqno = _isn;
        send_non_empty_segment(seg);
        return;
    } else if (_next_seqno == _nBytes_inflight) {
        return;
    }

    //send multiple non-empty segments
    uint16_t win = _window_size;
    if (_window_size == 0) {
        win = 1;  // zero window probing
    }

    uint64_t remaining;
    while ((remaining = static_cast<uint64_t>(win) + (_recv_ackno - _next_seqno))){
        // FIN flag occupies space in window
        TCPSegment newseg;
        if (_stream.eof() && !_fin_sent) {
            newseg.header().fin = 1;
            _fin_sent = 1;
            send_non_empty_segment(newseg);
            return;
        } else if (_stream.eof())
            return;
        else {  // SYN_ACKED
            size_t size = min(static_cast<size_t>(remaining), TCPConfig::MAX_PAYLOAD_SIZE);
            newseg.payload() = Buffer((_stream.read(size)));
            if (newseg.length_in_sequence_space() < win && _stream.eof()) {  // piggy-back FIN
                newseg.header().fin = false;
                _fin_sent = 1;
            }
            if (newseg.length_in_sequence_space() == 0)
                return;
            send_non_empty_segment(newseg);
        }
    }

}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {

    uint64_t abs_ackno = unwrap(ackno, _isn, _recv_ackno);
    if (ackno - next_seqno() > 0){
        return 0;
    }
    _window_size = window_size;
    if (abs_ackno - _recv_ackno <= 0){
        return 1;
    }

    //删掉fully-acknowledged segments
    TCPSegment tempSeg;
    while (!_segments_outstanding.empty()) {
        tempSeg = _segments_outstanding.front();
        if (ackno - tempSeg.header().seqno >= static_cast<int32_t>(tempSeg.length_in_sequence_space())) {
            _nBytes_inflight -= tempSeg.length_in_sequence_space();
            _segments_outstanding.pop();
//            cout<<"Segment Size: "<<segments_out().size()<<endl;
        } else
            break;
    }

    _recv_ackno = ackno.raw_value();

    fill_window();
    cout<<"Segment Size: "<<segments_out().size()<<endl;

    return 1;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { DUMMY_CODE(ms_since_last_tick); }

unsigned int TCPSender::consecutive_retransmissions() const { return {}; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
}

void TCPSender::send_non_empty_segment(TCPSegment &seg) {
    seg.header().seqno = wrap(_next_seqno, _isn);

    _next_seqno += seg.length_in_sequence_space();
    _nBytes_inflight += seg.length_in_sequence_space();
    //std::cerr << "send non empty: " << seg.header().to_string() << "length:" << seg.length_in_sequence_space() << endl << endl;
    _segments_out.push(seg);
    _segments_outstanding.push(seg);

//    // [RFC6298]:(5.1)
//    if (!_timer.open())
//        _timer.start();
}