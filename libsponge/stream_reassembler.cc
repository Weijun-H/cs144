#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _Unassembled(),  _output(capacity), _capacity(capacity), _firstUnassembled(0), _nUnassembled(0), _eof(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    _eof = eof | _eof;
    if (data.size() > _capacity) {
        _eof = 0;
    }
    if (data.empty() || index + data.size() < _firstUnassembled) { //! Sequence number + data.size
        if (_eof) {
            _output.end_input();
        }
        return;
    }

    size_t firstUnacceptable = _firstUnassembled + (_capacity - _output.buffer_size());

    //! Do with stored data firstly
    set<typeUnassembled>::iterator iter2;
    size_t resIndex = index;
    auto resData = string(data);
    if (resIndex < _firstUnassembled) {
        resData = resData.substr(_firstUnassembled - resIndex);
        resIndex = _firstUnassembled;
    }

    //! Updata firstUnacceptable
    if (resIndex + resData.size() > firstUnacceptable) {
        resData = resData.substr(0, firstUnacceptable - resIndex);
    }

    //! Begin to do with the unread assembled data
    //!         | resData |
    //!       <---|iter|
    iter2=_Unassembled.lower_bound(typeUnassembled(resIndex,resData));
    while(iter2!=_Unassembled.begin()){
        //resIndex > _firstUnassembled
        if(iter2==_Unassembled.end())   //! if _Unassembled is null, return _Unassembled.end
            iter2--;
        if (size_t deleteNum = mergeSubstring(resIndex, resData, iter2)) {  //返回值是删掉重合的bytes数
            _nUnassembled -= deleteNum;
            if(iter2 !=_Unassembled.begin()){
                _Unassembled.erase(iter2--);
            }
            else{
                _Unassembled.erase(iter2);
                break;
            }
        }
        else {
            break;
        }
    }

    //!         ｜resData |
    //!           |iter2｜ --->
    iter2 = _Unassembled.lower_bound(typeUnassembled(resIndex, resData));
    while(iter2 != _Unassembled.end()){
        if (size_t deleteNum = mergeSubstring(resIndex, resData, iter2)) {  //返回值是删掉重合的bytes数
            _Unassembled.erase(iter2++);
            _nUnassembled -= deleteNum;
        } else {
            break;
        }
    }

    //! Start to output to a ByteStream
    if (resIndex <= _firstUnassembled) {
        size_t wSize = _output.write(string(resData.begin() + _firstUnassembled - resIndex, resData.end()));
        if (wSize == resData.size() && eof) {
            _output.end_input();
        }
        _firstUnassembled += wSize;
    } else {
        _Unassembled.insert(typeUnassembled(resIndex, resData));
        _nUnassembled += resData.size();
    }

    if (empty() && _eof) {
        _output.end_input();
    }
    return;


}

int StreamReassembler::mergeSubstring(size_t &index, std::string &data, std::set<typeUnassembled>::iterator iter2) {
    std::string data2 = (*iter2).data;
    size_t startIndex2 = (*iter2).inedx, endIndex2 = startIndex2 + data2.size() - 1;
    size_t startIndex1 = index, endIndex1 = startIndex1 + data.size() - 1;
    if (startIndex2 > endIndex1 + 1 || startIndex1 > endIndex2 + 1){
        return 0;
    }
    index = min(startIndex1, startIndex2);
    size_t deleteNum = data2.size();
    if (startIndex1 <= startIndex2) {
        if (endIndex2 > endIndex1){
            data += std::string(data2.begin() + endIndex1 - startIndex2 + 1, data2.end());
        }
    } else {
        if (endIndex1 > endIndex2) {
            data2 += std::string(data.begin() + endIndex2 - startIndex1 + 1, data.end());
        }
        data.assign(data2);
    }
    return deleteNum;
}

size_t StreamReassembler::unassembled_bytes() const {
    return _nUnassembled;
}

bool StreamReassembler::empty() const {
    return _nUnassembled == 0;
}
