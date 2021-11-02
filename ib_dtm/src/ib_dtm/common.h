#pragma once
#include <iostream>
#include <string>
#include <stdio.h>
#include <time.h>

#include "ib_dtm/ib_dtm.h"


using namespace std;

namespace ib_dtm {
typedef size_t HashVal;
typedef int RSUIdx;
typedef int VehIdx;

const static HashVal INVALID_BLOCK_HASH = 0;

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
const std::string currentDateTime();

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
    string doEncode();
public:
    int epoch;
    RSUIdx proposer;
    HashVal hash;
    HashVal prev;
    map<VehIdx, int> trustOffsets;
    bool isForged;

    Block();
    void generateHash();
    void addTrustOffset(VehIdx id, int val);
    void setPrevHash(HashVal prev);
    string encode();
    void decode(string input);
    int getRecordCnt();
};

class IBDTMStake {
public:
    RSUIdx id;
    // int itsStake;
    double effectiveStake;
    std::map<int, int> itsStakeMap;

    static double initEffectiveStake;
    static double effectiveStakeUpperBound;
    static double effectiveStakeLowerBound;
    static int initITSstake;
    static int numVehicles;
    static double baseReward;
    static double penaltyFactor;
    static int traceBackEpoches;

    IBDTMStake();
    void checkITSStake(int epoch);
    void addITSStake(int epoch, int num);
    int getITSStake(int epoch);
    void getReward();
    void getPunishment();
    bool isLessLowerBound();
};

class IBDTMStakeVoting {
public:
    HashVal blockHash;
    double effectiveStakeSum;
    std::map<RSUIdx, bool> votes;
    std::map<RSUIdx, int> effectiveStakes;

    IBDTMStakeVoting();
    bool checkNegtiveVotes();
    bool checkPositiveVotes();
};

}
