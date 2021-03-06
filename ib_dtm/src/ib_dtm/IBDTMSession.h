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
#include "IBDTMRecorder.h"
#include "ib_dtm/IBDTMSession_m.h"

namespace ib_dtm {

class IB_DTM_API IBDTMSession : public cSimpleModule {
protected:
    int numInitStages() const override;
    void initialize(int stage) override;
    void handleMessage(cMessage* msg) override;
    
    int rsunum;
    int rsuInputBaseGateId;
    std::map<int, HashVal> epochBlocks;
    std::map<HashVal, Block*> blocks;
    std::map<HashVal, Block*> pendingBlocks;

    int epoch;
    std::map<int, vector<RSUIdx>> epochCommittees;
    std::map<int, IBDTMStakeVoting> rsuVotes;
    std::map<RSUIdx, bool> rsuStatus;
    std::map<RSUIdx, IBDTMStake> rsuStakes;
    std::map<VehIdx, int> vehTrustValues;
    std::map<VehIdx, bool> markedMalicious;

    int numVehicles;
    IBDTMRecorder recorder;

    /* ned veriable */
    int committeeSize;
    double epochTickInterval;
    int traceBackEpoches;
    bool enableIBDTM;
    int epochSlots;
    void epochTick();
    void handleRSUMsg(int idx, cMessage* msg);
    void newCommittee();
    void onNewBlock(Block* block);
    void onVoteBlock(int sender, std::string data);
    void broadcastNewBlock(HashVal hash);
    void onInvalidBlock(HashVal hash);
    void processStakeAdjustment(Block* block, bool result);
    void kickoutRSU(RSUIdx id);

    void processVehTrustValue();
    void dumpBlockChain();
};

}

