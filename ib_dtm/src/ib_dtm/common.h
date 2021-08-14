#pragma once
#include "ib_dtm/ib_dtm.h"

using namespace std;

namespace ib_dtm {
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

}
