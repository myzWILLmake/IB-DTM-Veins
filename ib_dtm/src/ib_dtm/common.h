#pragma once
#include "ib_dtm/ib_dtm.h"

using namespace std;

namespace ib_dtm {
typedef size_t HashVal;
typedef int RSUIdx;
typedef int VehIdx;

void split(const string& s, vector<string>& tokens, const string& delimiters = " ");

class BeaconMsg {
public:
    int sender;
    simtime_t time;
    bool isMalicious;

    BeaconMsg(){}
    BeaconMsg(int s, simtime_t t, bool m);
    string encode();
    void decode(string input);
};

class Block {
protected:
    map<VehIdx, int> trustOffsets;
    string doEncode();
public:
    HashVal hash;
    HashVal prev;
    // RSUIdx proposer;

    Block();
    void generateHash();
    void addTrustOffset(VehIdx id, int val);
    string encode();
    void decode(string input);
};
}
