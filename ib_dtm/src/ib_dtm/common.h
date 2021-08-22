#pragma once
#include "ib_dtm/ib_dtm.h"

using namespace std;

namespace ib_dtm {
typedef size_t HashVal;
typedef int RSUIdx;
typedef int VehIdx;

enum RSUMsgType {
   ProposedBlock,
   VerifyBlock,
   OnVerifyBlock,
   VoteBlock
};

enum SessionMsgType {
    NewCommittee,
    CommittedBlock,
    InvalidBlock
};

void split(const string& s, vector<string>& tokens, const string& delimiters = " ");

class SessionMsgHelper {
public:
    static string encodeNewCommittee(int epoch, RSUIdx proposer, vector<RSUIdx>& committee);
    static void decodeNewCommittee(string input, int& epoch, RSUIdx& proposer, vector<RSUIdx>& committee);
};

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
    int epoch;
    RSUIdx proposer;
    HashVal hash;
    HashVal prev;

    Block();
    void generateHash();
    void addTrustOffset(VehIdx id, int val);
    void setPrevHash(HashVal prev);
    string encode();
    void decode(string input);
};
}
