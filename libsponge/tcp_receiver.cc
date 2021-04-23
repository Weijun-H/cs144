#include "tcp_receiver.hh"
#include <iostream>
// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    SEG = seg;
    header = seg.header();
    if ((firstConnection && seg.header().syn)) {
        isn = seg.header().seqno;
        firstConnection = false;
        ACK = isn;
        wantSend.clear();
        haveSend.clear();
        wantSend.insert(isn.raw_value()+_capacity + 1);
    }
//    cout<<"firstConnect: "<<firstConnection<<endl;

    if (firstConnection){
        return;
    } else {
        haveSend.insert(header.seqno.raw_value());
        bool fin = seg.header().fin;
//        if (fin) {
//            firstConnection = true;
//        }
        uint64_t streamIndex = convertToStreamIndex(seg.header().seqno);
        _reassembler.push_substring(seg.payload().copy(), header.syn ? 0 : streamIndex, fin);
//        cout<<"SYN: "<<header.syn<<endl;
//        cout<<"FIN: "<<header.fin<<endl;
//        cout<<"Seqnos: "<<header.seqno<<endl;
//        cout<<"Capability: "<<_capacity<<endl;
//        cout<<"unassembled: "<<_reassembler.unassembled_bytes()<<endl;
        calculateACK();
    }
    cout<<endl;

}

void TCPReceiver::calculateACK() {
    size_t cnt = 0;
    if (header.syn) {
        cnt++;
    }
    if (header.fin) {
        cnt++;
    }

    ACK = header.seqno + SEG.payload().size() + cnt;


    // received Segment has been sent before
    // update wantSend and haveSend
    if (wantSend.find(header.seqno.raw_value()) != wantSend.end()) {
        wantSend.erase(wantSend.find(header.seqno.raw_value()));
        haveSend.erase(haveSend.find(header.seqno.raw_value()));

    }

    // received Segment has not been sent before
    if (haveSend.find(ACK.raw_value()) == haveSend.end()) {
        wantSend.insert(ACK.raw_value());
    }

    // find the next Segment ACK
//    auto index =  wantSend.lower_bound(isn.raw_value());
//    if (index != wantSend.end()) {
//        ACK = WrappingInt32(*index);
//        cout<<"1";
        ACK = WrappingInt32(*wantSend.begin());
        while (ACK.raw_value() > header.seqno.raw_value() && ACK.raw_value() < header.seqno.raw_value() + SEG.payload().size() + cnt) {
            wantSend.erase(wantSend.begin());
            ACK = WrappingInt32(*wantSend.begin());
        }
//
//    cout<<"windows: "<<window_size()<<endl;
//    cout<<"ACK:"<<ACK<<endl;
//
//    cout<<"hasSend: ";
//    for (auto i: haveSend) {
//        cout<<i<<",  ";
//    }
//    cout<<endl;
//
//    cout<<"wantSend: ";
//    for (auto i: wantSend) {
//        cout<<i<<",  ";
//    }
//    cout<<endl<<endl;
}


optional<WrappingInt32> TCPReceiver::ackno() const {
    if (firstConnection) {
        return nullopt;
    }
    return ACK;
}

size_t TCPReceiver::window_size() const {
    size_t window = _capacity - _reassembler.stream_out().buffer_size();
    return window;
}

uint64_t TCPReceiver::convertToStreamIndex(WrappingInt32 seqno) {
    uint64_t absqno = unwrap(seqno, isn, checkPoint);
    checkPoint = absqno;
    return absqno - 1;
}
