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

#include "IBDTMSession.h"

#include <random>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <iostream>
#include <fstream>

Define_Module(ib_dtm::IBDTMSession);

using namespace veins;
using namespace ib_dtm;
using namespace std;

int IBDTMSession::numInitStages() const {
    return std::max(cSimpleModule::numInitStages(), 3);
}

void IBDTMSession::initialize(int stage) {
    cSimpleModule::initialize(stage);
    if (stage == 0) {
        epoch = 0;
    }

    if (stage == 1) {
        committeeSize = par("committeeSize");
        epochTickInterval = par("epochTickInterval");
        epochSlots = par("epochSlots");
        traceBackEpoches = par("traceBackEpoches");
        enableIBDTM = par("enableIBDTM");
        numVehicles = par("numVehicles");
        EV << "committeeSize:" << committeeSize << endl;
        // committee = vector<RSUIdx>(committeeSize);

        IBDTMStake::initEffectiveStake = par("initEffectiveStake");
        IBDTMStake::effectiveStakeUpperBound = par("effectiveStakeUpperBound");
        IBDTMStake::effectiveStakeLowerBound = par("effectiveStakeLowerBound");
        IBDTMStake::initITSstake = par("initITSstake");
        IBDTMStake::numVehicles = par("numVehicles");
        IBDTMStake::baseReward = par("baseReward");
        IBDTMStake::penaltyFactor = par("penaltyFactor");
        IBDTMStake::traceBackEpoches = traceBackEpoches;
    }

    if (stage == 2) {
        rsuInputBaseGateId = findGate("rsuInputs", 0);
        rsunum = gateSize("rsuInputs");
        EV << "rsugatesize rsunum = " << rsunum << endl;

        for (int i=0; i<rsunum; i++) {
            rsuStakes[i] = IBDTMStake();
            rsuStakes[i].id = i;
            rsuStatus[i] = true;
        }

        cMessage* tick = new cMessage("epochTick");
        scheduleAt(0.5, tick);
    }
}

void IBDTMSession::epochTick() {
    // check timeout
    HashVal hash = epochBlocks[epoch];
    if (hash != INVALID_BLOCK_HASH && pendingBlocks.find(hash) != pendingBlocks.end()) {
        auto& votes = rsuVotes[epoch];
        if (!enableIBDTM || votes.checkPositiveVotes()) {
            // Block got approved
            broadcastNewBlock(hash);
        } else if (votes.checkNegtiveVotes()) {
            // invalid Block
            onInvalidBlock(hash);
        } else {
            onInvalidBlock(hash);
        }
    }

    epoch++;
    newCommittee();

    if (epoch == (2000/epochTickInterval)) {
        // dumpBlockChain();
        // recorder.dumpVehTrustValues(0);
        // recorder.dumpVehTrustValues(10);
        // recorder.dumpVehTrustValues(55);
        // recorder.dumpRSUStakes(35);
        // recorder.dumpRSUStakes(36);
        recorder.setMaliciousVehNum(40);
        recorder.dumpMarkedMalicious();
        recorder.dumpVehTrustValues();
        // recorder.dumpVehDetected();
    }

    if (epoch % epochSlots == 0) {
        processVehTrustValue();
        recorder.record(vehTrustValues, markedMalicious);
    }

    recorder.recordStakes(rsuStakes);


    cMessage* msg = new cMessage("epochTick");
    scheduleAt(simTime() + epochTickInterval, msg);
}

void IBDTMSession::handleMessage(cMessage* msg) {
    cGate* gate = msg->getArrivalGate();
    if (msg->isSelfMessage()) {
        epochTick();
    } else if (gate && gate->getBaseId() == rsuInputBaseGateId) {
        int idx = msg->getArrivalGate()->getIndex();
        handleRSUMsg(idx, msg);
    } else {
        EV << "WARNING: NO PROCESS " << msg << endl;
    }

    delete msg;
}

void IBDTMSession::newCommittee() {
    // generate new proposer and committee
    // committee
    vector<int> rsuIdxs;
    for (int i=0; i<rsunum; i++) {
        if (rsuStatus[i]) rsuIdxs.push_back(i);
    }
    std::random_device rd;
    std::mt19937 g(rd());
    shuffle(rsuIdxs.begin(), rsuIdxs.end(), g);

    int csize = rsuIdxs.size() < committeeSize ? rsuIdxs.size() : committeeSize;
    vector<RSUIdx> committee(csize);
    for (int i=0; i<csize; i++) {
        committee[i] = rsuIdxs[i];
    }

    epochCommittees[epoch] = committee;
    // proposer
    // Temp: random pick
    RSUIdx proposer;
    if (rsuIdxs.size() <= csize) {
        proposer = committee[0];
    } else {
        proposer = rsuIdxs[csize];
    }

    EV << "IBDTMSession new epoch [" << epoch << "]:" << endl;
    EV << "     proposer: " << proposer << endl;
    string committeeStr = "";
    for (auto& id : committee) {
        committeeStr += to_string(id) + " ";
    }
    EV << "     committee: " + committeeStr << endl;


    string data = SessionMsgHelper::encodeNewCommittee(epoch, proposer, committee);
    for (auto id : committee) {
        IBDTMSessionMsg* msg = new IBDTMSessionMsg();
        msg->setMsgType(SessionMsgType::NewCommittee);
        msg->setData(data.c_str());
        send(msg, "rsuOutputs", id);
    }

    IBDTMSessionMsg* msg = new IBDTMSessionMsg();
    msg->setMsgType(SessionMsgType::NewCommittee);
    msg->setMsgType(SessionMsgType::NewCommittee);
    msg->setData(data.c_str());
    send(msg, "rsuOutputs", proposer);
}

void IBDTMSession::handleRSUMsg(int idx, cMessage* m) {
    IBDTMRSUMsg* msg = check_and_cast<IBDTMRSUMsg*>(m);
    RSUMsgType msgType = RSUMsgType(msg->getMsgType());
    switch(msgType) {
        case ProposedBlock: {
            int sender = msg->getSender();
            string data = msg->getData();
            Block* block = new Block();
            block->decode(data);
            onNewBlock(block);
            break;
        }
        case VoteBlock: {
            int sender = msg->getSender();
            string data = msg->getData();
            onVoteBlock(sender, data);
            break;
        }
    }
}

void IBDTMSession::onNewBlock(Block* block) {
    if (pendingBlocks.find(block->hash) != pendingBlocks.end()) {
        delete pendingBlocks[block->hash];
        pendingBlocks.erase(block->hash);
    }
    pendingBlocks[block->hash] = block;
    int epoch = block->epoch;
    if (epochCommittees.find(epoch) == epochCommittees.end()) return;
    epochBlocks[epoch] = block->hash;
    auto& committee = epochCommittees[epoch];

    rsuVotes[block->epoch] = IBDTMStakeVoting();
    auto& votes = rsuVotes[block->epoch];
    votes.blockHash = block->hash;
    for (auto& id : committee) {
        votes.effectiveStakes[id] = rsuStakes[id].effectiveStake;
        votes.effectiveStakeSum += rsuStakes[id].effectiveStake;
    }
}

void IBDTMSession::onVoteBlock(int sender, string input) {
    vector<string> data;
    split(input, data);
    HashVal hash = stoul(data[0]);
    bool vote = data[1] == "t";

    EV << "IBDTMSession RSU[" << sender << "] vote [" << vote << "]" << endl;
    if (pendingBlocks.find(hash) == pendingBlocks.end()) return;
    Block* block = pendingBlocks[hash];
    int epoch = block->epoch;
    auto& votes = rsuVotes[epoch];
    votes.votes[sender] = vote;

}

void IBDTMSession::broadcastNewBlock(HashVal hash) {
    if (pendingBlocks.find(hash) == pendingBlocks.end()) return;
    blocks[hash] = pendingBlocks[hash];
    pendingBlocks.erase(hash);
    Block* block = blocks[hash];

    EV << "IBDTMSession block[" << block->hash << "] committed:" << endl;
    EV << "     Hash:" << block->hash << endl;
    EV << "     Prev:" << block->prev << endl;
    EV << "     TrustRatings:" << endl;
    for (auto& p : block->trustOffsets) {
        EV << "         Veh[" << p.first << "] Rating [" << p.second << "]" << endl;
    }

    string msgData = block->encode();
    int epoch = block->epoch;
    
    for (int i=0; i<rsunum; i++) {
        IBDTMSessionMsg* msg = new IBDTMSessionMsg();
        msg->setMsgType(SessionMsgType::CommittedBlock);
        msg->setData(msgData.c_str());
        send(msg, "rsuOutputs", i);
    }

    processStakeAdjustment(block, true);

    rsuVotes.erase(epoch);
}

void IBDTMSession::onInvalidBlock(HashVal hash) {
    if (pendingBlocks.find(hash) == pendingBlocks.end()) return;
    Block* block = pendingBlocks[hash];
    int epoch = block->epoch;
    epochBlocks[epoch] = INVALID_BLOCK_HASH;
    auto& committee = epochCommittees[epoch];

    for (auto& id : committee) {
        IBDTMSessionMsg* msg = new IBDTMSessionMsg();
        msg->setMsgType(SessionMsgType::InvalidBlock);
        msg->setData(to_string(hash).c_str());
        send(msg, "rsuOutputs", id);
    }

    IBDTMSessionMsg* msg = new IBDTMSessionMsg();
    msg->setMsgType(SessionMsgType::InvalidBlock);
    msg->setData(to_string(hash).c_str());
    send(msg, "rsuOutputs", block->proposer);

    processStakeAdjustment(block, false);

    rsuVotes.erase(epoch);
    delete block;
    pendingBlocks.erase(hash);
}

void IBDTMSession::kickoutRSU(RSUIdx id) {
    if (!enableIBDTM) return;
    // EV << "RSU[" << id << "] kicked out from the network." << endl;
    // rsuStatus[id] = false;
}

void IBDTMSession::processStakeAdjustment(Block* block, bool result) {
    if (!block) return;
    int epoch = block->epoch;
    if (epochCommittees.find(epoch) == epochCommittees.end()) return;
    auto& committee = epochCommittees[epoch];
    auto& votes = rsuVotes[epoch];

    if (result) {
        for (auto& p : votes.votes) {
            if (p.second) rsuStakes[p.first].getReward();
            else rsuStakes[p.first].getPunishment();
            if (rsuStakes[p.first].isLessLowerBound()) kickoutRSU(p.first);
        }

        rsuStakes[block->proposer].getReward();
        int recordCnt = block->getRecordCnt();
        int before = rsuStakes[block->proposer].getITSStake(epoch-1);
        rsuStakes[block->proposer].addITSStake(epoch, recordCnt);
        if (recordCnt > 0) {
            EV << "RSU[" << block->proposer << "] ITSstake changed: " << before << " -> " << rsuStakes[block->proposer].getITSStake(epoch) << endl;
        }
    } else {
        rsuStakes[block->proposer].getPunishment();
        for (auto& p : votes.votes) {
            if (p.second) rsuStakes[p.first].getPunishment();
            if (rsuStakes[p.first].isLessLowerBound()) kickoutRSU(p.first);
        }
    }
}

void IBDTMSession::processVehTrustValue() {
    for (int vehId = 0; vehId < numVehicles; vehId++) {
        vehTrustValues[vehId] = 0;
        markedMalicious[vehId] = false;
    }

    
    for (int ep = max(0, epoch - traceBackEpoches); ep < epoch; ep++) {
        auto block = blocks[epochBlocks[ep]];
        if (block) {
            auto tvoffsets = block->trustOffsets;
            for (auto& p : tvoffsets) {
                vehTrustValues[p.first] += p.second;
            }
        }
    }

    for (int vehId = 0; vehId < numVehicles; vehId++) {
        if (!markedMalicious[vehId] && vehTrustValues[vehId] < 0) {
            markedMalicious[vehId] = true;
        }
    }
}

void IBDTMSession::dumpBlockChain() {
    string currTime = currentDateTime();
    ofstream blockchainFile("results/blockchain_" + currTime);

    if (!blockchainFile.is_open()) {
        EV << "Cannot open file!" << endl;
        return;
    }

    for (auto& p : epochBlocks) {
        auto block = blocks[p.second];
        blockchainFile << p.first << " " << p.second << endl;
        if (block) {
            blockchainFile << "     " << block->proposer << "   " << block->prev << endl;
            for (auto& vp : block->trustOffsets) {
                blockchainFile <<  "        " << vp.first << " " << vp.second << endl;
            }
        }
    }

    blockchainFile.close();
}
