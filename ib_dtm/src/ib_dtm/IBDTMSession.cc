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

Define_Module(ib_dtm::IBDTMSession);

using namespace veins;
using namespace ib_dtm;
using namespace std;

double IBDTMStake::initEffectiveStake = 0;
double IBDTMStake::effectiveStakeUpperBound = 0;
double IBDTMStake::effectiveStakeLowerBound = 0;
int IBDTMStake::initITSstake = 0;
int IBDTMStake::numVehicles = 0;
double IBDTMStake::baseReward = 0;
double IBDTMStake::penaltyFactor = 0;
IBDTMStake::IBDTMStake() {
    itsStake = initITSstake;
    effectiveStake = initEffectiveStake;
}

void IBDTMStake::getReward() {
    double stakeFactor = double(itsStake) / numVehicles;
    double reward = baseReward * sqrt(stakeFactor);
    double before = effectiveStake;
    effectiveStake += reward;
    if (effectiveStake > effectiveStakeUpperBound) effectiveStake = effectiveStakeUpperBound;
    EV << "RSU[" << id << "] effectiveStake get reward: " << before << " -> " << effectiveStake << endl;
}

void IBDTMStake::getPunishment() {
    double stakeFactor = double(itsStake) / numVehicles;
    double p = sqrt(stakeFactor);
    if (p < 1) p = 1;
    double penalty = baseReward * p * penaltyFactor;
    double before = effectiveStake;
    effectiveStake -= penalty;
    // if (effectiveStake < effectiveStakeLowerBound) effectiveStake = effectiveStakeLowerBound;
    EV << "RSU[" << id << "] effectiveStake get punishment: " << before << " -> " << effectiveStake << endl;
}

bool IBDTMStake::isLessLowerBound() {
    return effectiveStake <= effectiveStakeLowerBound;
}

IBDTMStakeVoting::IBDTMStakeVoting() {
    effectiveStakeSum = 0;
}

bool IBDTMStakeVoting::checkNegtiveVotes() {
    if (effectiveStakeSum == 0) return false;
    double negativeVoteStakes = 0;
    for (auto& p : votes) {
        if (!p.second) {
            negativeVoteStakes += effectiveStakes[p.first];
        }
    }

    if (negativeVoteStakes / effectiveStakeSum >= 1.0/3) {
        return true;
    } else return false;
}

bool IBDTMStakeVoting::checkPositiveVotes() {
    if (effectiveStakeSum == 0) return false;
    double positiveVoteStakes = 0;
    for (auto& p : votes) {
        if (p.second) {
            positiveVoteStakes += effectiveStakes[p.first];
        }
    }

    if (positiveVoteStakes / effectiveStakeSum > 2.0/3) {
        return true;
    } else return false;
}

int IBDTMSession::numInitStages() const {
    return std::max(cSimpleModule::numInitStages(), 3);
}

void IBDTMSession::initialize(int stage) {
    cSimpleModule::initialize(stage);
    if (stage == 0) {
    }

    if (stage == 1) {
        committeeSize = par("committeeSize");
        EV << "committeeSize:" << committeeSize << endl;
        // committee = vector<RSUIdx>(committeeSize);

        IBDTMStake::initEffectiveStake = par("initEffectiveStake");
        IBDTMStake::effectiveStakeUpperBound = par("effectiveStakeUpperBound");
        IBDTMStake::effectiveStakeLowerBound = par("effectiveStakeLowerBound");
        IBDTMStake::initITSstake = par("initITSstake");
        IBDTMStake::numVehicles = par("numVehicles");
        IBDTMStake::baseReward = par("baseReward");
        IBDTMStake::penaltyFactor = par("penaltyFactor");
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

        // Temp: test schduled msg
        cMessage* test = new cMessage("test");
        scheduleAt(0.2, test);
    }
}


void IBDTMSession::handleMessage(cMessage* msg) {
    EV << "recevied msg at session" << endl;
    cGate* gate = msg->getArrivalGate();
    if (msg->isSelfMessage()) {
        newCommittee();
    } else if (gate && gate->getBaseId() == rsuInputBaseGateId) {
        int idx = msg->getArrivalGate()->getIndex();
        handleRSUMsg(idx, msg);
    } else {
        EV << "WARNING: NO PROCESS " << msg << endl;
    }
}

void IBDTMSession::newCommittee() {
    // generate new proposer and committee
    epoch++;

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
    // Temp: pick the first of the committee
    RSUIdx proposer = committee[0];
    string data = SessionMsgHelper::encodeNewCommittee(epoch, proposer, committee);
    for (auto id : committee) {
        IBDTMSessionMsg* msg = new IBDTMSessionMsg();
        msg->setMsgType(SessionMsgType::NewCommittee);
        msg->setData(data.c_str());
        send(msg, "rsuOutputs", id);
    }
}

void IBDTMSession::handleRSUMsg(int idx, cMessage* m) {
    EV << "handle RSU msg" << endl;
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
    EV << "onNewBlock" << endl;
    if (pendingBlocks.find(block->hash) != pendingBlocks.end()) {
        delete pendingBlocks[block->hash];
        pendingBlocks.erase(block->hash);
    }
    pendingBlocks[block->hash] = block;
    int epoch = block->epoch;
    if (epochCommittees.find(epoch) == epochCommittees.end()) return;
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

    EV << "IBDTMSession onVote: RSU[" << sender << "] vote [" << vote << "]" << endl;
    if (pendingBlocks.find(hash) == pendingBlocks.end()) return;
    Block* block = pendingBlocks[hash];
    int epoch = block->epoch;
    auto& votes = rsuVotes[epoch];
    votes.votes[sender] = vote;

    if (votes.checkPositiveVotes()) {
        // Block got approved
        broadcastNewBlock(block->hash);
    } else if (votes.checkNegtiveVotes()) {
        // invalid Block
        onInvalidBlock(block->hash);
    }
    // TODO: wating for timeout
}

void IBDTMSession::broadcastNewBlock(HashVal hash) {
    if (pendingBlocks.find(hash) == pendingBlocks.end()) return;
    blocks[hash] = pendingBlocks[hash];
    pendingBlocks.erase(hash);
    Block* block = blocks[hash];
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
    auto& committee = epochCommittees[epoch];

    for (auto& id : committee) {
        IBDTMSessionMsg* msg = new IBDTMSessionMsg();
        msg->setMsgType(SessionMsgType::InvalidBlock);
        msg->setData(to_string(hash).c_str());
        send(msg, "rsuOutputs", id);
    }

    processStakeAdjustment(block, false);

    rsuVotes.erase(epoch);
    delete block;
    pendingBlocks.erase(hash);
}

void IBDTMSession::kickoutRSU(RSUIdx id) {
    EV << "RSU[" << id << "] kicked out from the network." << endl;
    rsuStatus[id] = false;
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

        int recordCnt = block->getRecordCnt();
        int before = rsuStakes[block->proposer].itsStake;
        rsuStakes[block->proposer].itsStake += recordCnt;
        EV << "RSU[" << block->proposer << "] ITSstake changed: " << before << " -> " << rsuStakes[block->proposer].itsStake << endl;
    } else {
        for (auto& p : votes.votes) {
            if (p.second) rsuStakes[p.first].getPunishment();
            if (rsuStakes[p.first].isLessLowerBound()) kickoutRSU(p.first);
        }
    }
} 
