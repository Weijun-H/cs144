#include "wrapping_integers.hh"
#include <math.h>
#include <iostream>
// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint64_t mod = n % static_cast<uint64_t> (pow(2,32));
    cout << "n: "<< n<<"\n";
    return isn + mod;
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t ans = n.raw_value() - isn.raw_value();
    uint64_t temp = ans;
    uint64_t base = static_cast<uint64_t>(pow(2, 32));
//    uint64_t gap = checkpoint > ans ? checkpoint - ans : ans - checkpoint;

    uint64_t baseI = checkpoint / base;

    //uint64_t mod = checkpoint % base;

    if (baseI != 0 ){
        ans = ans + pow2(base, baseI, base);
    } else {
        if (ans == checkpoint){
            return ans;
        }
        ans += base;
    }
    cout << "base: "<< base<<"\n";
    cout << "baseI: "<< baseI<<"\n";

    if (ans > checkpoint) {
        temp = ans - base;
    } else {
        temp = ans;
        ans += base;
    }
    uint64_t gap1 = absUit64(temp, checkpoint);
    uint64_t gap2 = absUit64(ans, checkpoint);

    cout << "gap1: "<< gap1<<"\n" ;
    cout << "gap2: "<< gap2<<"\n" ;
    if (gap1 < gap2){
        cout << "ans1: "<< ans - base<<"\n" ;
        return temp;
    } else {
        cout << "ans2: "<< ans<<"\n" ;
        cout<<endl;
        return ans;
    }

}

uint64_t absUit64(uint64_t a, uint64_t b) {
    return a > b ? a - b : b - a;
}

uint64_t pow2(uint64_t a, uint64_t b, uint64_t base){
    uint64_t temp = a;
    for (size_t i = 0; i < b-1; ++i) {
        temp += base;
    }
    return temp;
}
