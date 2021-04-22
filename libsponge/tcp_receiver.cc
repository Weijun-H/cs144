#include "tcp_receiver.hh"
#include <iostream>
// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    cout<<"ACK:"<<ACK<<endl;
    SEG = seg;
    header = seg.header();
    if (firstConnection && seg.header().syn) {
        isn = seg.header().seqno;
        firstConnection = false;
        ACK = isn;
    }
    bool fin = seg.header().fin;
//    haveSend.insert(header.seqno);
    uint64_t streamIndex = convertToStreamIndex(seg.header().seqno);
    _reassembler.push_substring(seg.payload().copy(), streamIndex, fin);
    ACK = ackno().value();
//    wantSend.insert(ACK);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    int cnt = 0;
    if (firstConnection) {
        return nullopt;
    }

    if (ACK != SEG.header().seqno){
        return ACK;
    } else {
        if (header.syn) {
            cnt++;
        }
        if (header.fin) {
            cnt++;
        }
    }
    return header.seqno + SEG.payload().size() + cnt;
}

size_t TCPReceiver::window_size() const {
//    cout<<"Unassemblered: "<<_reassembler.stream_out().buffer_size()<<endl;
    return _capacity - _reassembler.stream_out().buffer_size();
}

uint64_t TCPReceiver::convertToStreamIndex(WrappingInt32 seqno) {
    uint64_t absqno = unwrap(seqno, isn, checkPoint);
    checkPoint = absqno;
    return absqno - 1;
}
//
//WrappingInt32 TCPReceiver::nextAck() {
//    if (wantSend.empty()){
//        return
//    }
//}