//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#pragma once
#include "ib_dtm/ib_dtm.h"
#include "veins/veins.h"
#include "common.h"
#include "ib_dtm/IBDTMSession_m.h"

namespace ib_dtm {

class IBDTMStake {
public:
    int itsStake;
    double effectiveStake;

    static double initEffectiveStake;
    static double effectiveStakeUpperBound;
    static double effectiveStakeLowerBound;
    static int initITSstake;
    static double baseReward;
    static double penaltyFactor;

    IBDTMStake(); 
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

class IB_DTM_API IBDTMSession : public cSimpleModule {
protected:
    int numInitStages() const override;
    void initialize(int stage) override;
    void handleMessage(cMessage* msg) override;
    
    int rsunum;
    int rsuInputBaseGateId;
    std::map<HashVal, Block*> blocks;
    std::map<HashVal, Block*> pendingBlocks;
    std::map<RSUIdx, IBDTMStake> rsuStakes;

    int epoch;
    std::map<int, vector<RSUIdx>> epochCommittees;
    std::map<int, IBDTMStakeVoting> rsuVotes;

    /* ned veriable */
    int committeeSize;

    void handleRSUMsg(int idx, cMessage* msg);
    void newCommittee();
    void onNewBlock(Block* block);
    void onVoteBlock(int sender, std::string data);
    void broadcastNewBlock(HashVal hash);
    void onInvalidBlock(HashVal hash);
};

}

