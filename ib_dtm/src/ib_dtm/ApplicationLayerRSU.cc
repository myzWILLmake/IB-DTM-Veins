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

#include "ib_dtm/ApplicationLayerRSU.h"
#include "ib_dtm/ApplicationLayerTestMessage_m.h"

using namespace veins;
using namespace ib_dtm;
using namespace std;

Define_Module(ib_dtm::ApplicationLayerRSU);

int ApplicationLayerRSU::numInitStages() const {
    return std::max(cSimpleModule::numInitStages(), 3);
}

void ApplicationLayerRSU::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        rsuID = getParentModule()->getIndex();
        blockchain = 0;
        inCommittee = false;
        maliciousVehNum = 40;
        vehTotalNum = 200;
    }
    if (stage == 2) {
        rsuInputBaseGateId = findGate("rsuInputs", 0);
        sessionInputGateId = findGate("sessionInput");
        startService(Channel::sch2, rsuID, "rsu service");
        isMalicious = getParentModule()->par("isMalicious");
        maliciousPoss = getParentModule()->par("maliciousPoss");
        maliciousDelay = getParentModule()->par("maliciousDelay");

        if (isMalicious) {
            findHost()->getDisplayString().setTagArg("i", 1, "red");
        }
    }
}

void ApplicationLayerRSU::decodeEventData(std::string eventData, vector<BeaconMsg*>& msgs) {
    vector<string> msgstrs;
    split(eventData, msgstrs);
    for (auto& s : msgstrs) {
        BeaconMsg* msg = new BeaconMsg();
        msg->decode(s);
        msgs.push_back(msg);
    }
}

void ApplicationLayerRSU::handleMessage(cMessage* msg) {
    cGate* g = msg->getArrivalGate();
    if (g && g->getBaseId() == rsuInputBaseGateId) {
        int idx = msg->getArrivalGate()->getIndex();
        handleRSUMsg(idx, msg);
    } else if (g && g->getBaseId() == sessionInputGateId) {
        handleSessionMsg(msg);
    } else {
        DemoBaseApplLayer::handleMessage(msg);
    }
}

void ApplicationLayerRSU::handleSessionMsg(cMessage* m) {
    IBDTMSessionMsg* msg = check_and_cast<IBDTMSessionMsg*>(m);
    SessionMsgType msgType = SessionMsgType(msg->getMsgType());
    switch(msgType) {
        case NewCommittee: {
            string data = msg->getData();
            onNewCommittee(data);
            break;
        }
        case CommittedBlock: {
            string data = msg->getData();
            Block* block = new Block();
            block->decode(data);
            blocks[block->hash] = block;
            blockchain = block->hash;
            if (pendingBlocks.find(block->hash) != pendingBlocks.end()) {
                delete pendingBlocks[block->hash];
                pendingBlocks.erase(block->hash);
            }
            // EV << "Received block: " << block->hash << endl;
            break;
        }
        case InvalidBlock: {
            string data = msg->getData();
            HashVal hash = stoul(data);
            onInvalidBlock(hash);
        }
    }

    delete msg;
}

void ApplicationLayerRSU::handleRSUMsg(int idx, cMessage* m) {
    IBDTMRSUMsg* msg = check_and_cast<IBDTMRSUMsg*>(m);
    RSUMsgType msgType = RSUMsgType(msg->getMsgType());
    switch(msgType) {
        case ProposedBlock: {
            int sender = msg->getSender();
            string data = msg->getData();
            Block* block = new Block();
            block->decode(data);
            onProposedBlock(sender, block);
            break;
        }
        case VerifyBlock: {
            int sender = msg->getSender();
            string data = msg->getData();
            HashVal hash = stoul(data);
            verifyPendingBlock(sender, hash);
            break;
        }
        case OnVerifyBlock: {
            int sender = msg->getSender();
            string data = msg->getData();
            onVerifyPendingBlock(sender, data);
            break;
        }
    }

    delete msg;
}


void ApplicationLayerRSU::onProposedBlock(int proposer, Block* block) {
    if (!block) return;
    if (pendingBlocks.find(block->hash) != pendingBlocks.end()) {
        delete pendingBlocks[block->hash];
        pendingBlocks.erase(block->hash);
    }
    pendingBlocks[block->hash] = block;
    // Pseudo-verification
    IBDTMRSUMsg* msg = new IBDTMRSUMsg();
    msg->setMsgType(RSUMsgType::VerifyBlock);
    msg->setSender(rsuID);
    msg->setData(to_string(block->hash).c_str());
    send(msg, "rsuOutputs", proposer);   
}

void ApplicationLayerRSU::verifyPendingBlock(int sender, HashVal hash) {
    if (pendingBlocks.find(hash) == pendingBlocks.end()) return;
    Block* block = pendingBlocks[hash];
    if (block->proposer != rsuID) return;
    bool verifyRes = block->isForged == false;
    IBDTMRSUMsg* msg = new IBDTMRSUMsg();
    msg->setMsgType(RSUMsgType::OnVerifyBlock);
    msg->setSender(rsuID);
    string data = to_string(hash) + " ";
    if (verifyRes) {
        data += "t";
    } else {
        data += "f";
    }
    msg->setData(data.c_str());
    send(msg, "rsuOutputs", sender);
}

void ApplicationLayerRSU::onVerifyPendingBlock(int sender, string input) {
    vector<string> strs;
    split(input, strs);
    HashVal hash = stoul(strs[0]);
    bool verifyRes = strs[1] == "t";
    if (pendingBlocks.find(hash) == pendingBlocks.end()) return;
    IBDTMRSUMsg* msg = new IBDTMRSUMsg();
    msg->setMsgType(RSUMsgType::VoteBlock);
    msg->setSender(rsuID);
    string data = to_string(hash) + " ";
    bool doMalicious = false;
    if (isMalicious) {
        if (simTime() > maliciousDelay && rand()/double(RAND_MAX) < maliciousPoss) {
            doMalicious = true;
        }
    }
    if (verifyRes != doMalicious) {
        data += "t";
    } else {
        data += "f";
    }
    msg->setData(data.c_str());
    send(msg, "sessionOutput");
}

bool ApplicationLayerRSU::isInCommittee(int epoch) {
    if (epochCommittees.find(epoch) == epochCommittees.end()) return false;
    for (auto id : epochCommittees[epoch]) {
        if (id == rsuID) return true;
    }
    return false;
}

void ApplicationLayerRSU::onNewCommittee(string data) {
    RSUIdx proposer;
    vector<RSUIdx> committee;
    int epoch;
    SessionMsgHelper::decodeNewCommittee(data, epoch, proposer, committee);
    epochCommittees[epoch] = committee;
    if (rsuID == proposer) {
        // genderate new block
        generateBlock(epoch);
    }

    // EV << "RSU[" << rsuID << "] received NewCommittee" << endl;
    // EV << "    proposer:" << proposer << " committee? " << isInCommittee(epoch) << endl;
}

void ApplicationLayerRSU::onInvalidBlock(HashVal hash) {
    EV << "RSU[" << rsuID << "] onInvalidBlock " << hash << endl;
    if (pendingBlocks.find(hash) == pendingBlocks.end()) return;
    Block* block = pendingBlocks[hash];
    int epoch = block->epoch;
    epochCommittees.erase(epoch);

    delete block;
    pendingBlocks.erase(hash);
}

void ApplicationLayerRSU::onWSM(BaseFrame1609_4* frame)
{
    ApplicationLayerTestMessage* wsm = check_and_cast<ApplicationLayerTestMessage*>(frame);
    int msgType = wsm->getMsgType();
    switch(msgType) {
        case APPLICATION_MSG_TYPE_VEH: break;
        case APPLICATION_MSG_TYPE_RSU: {
            bool isAck = wsm->getIsAck();
            if (!isAck) return;
            int sender = wsm->getSender();
            std::string eventData = wsm->getEventData();
            // EV << "RSU[" << rsuID << "] received reports from VEH[" << sender << "]" << endl;
            vector<BeaconMsg*> msgs;
            decodeEventData(eventData, msgs);

            for (auto msg : msgs) {
                vehRecords[msg->sender].push_back(msg);
            }

            // EV << "    Positive VEHs:" << endl;
            // EV << "        ";
            // for (auto& m : msgs) {
            //     if (!m->isMalicious) {
            //         EV << m->sender << "@" << m->time << " ";
            //     }
            // }
            // EV << endl;

            // EV << "    Negative VEHs:" << endl;
            // EV << "        ";
            // for (auto& m : msgs) {
            //     if (m->isMalicious) {
            //         EV << m->sender << "@" << m->time << " ";
            //     }
            // }
            // EV << endl;

            // for (BeaconMsg* m : msgs) {
            //     delete m;
            // }
            break;
        }
    }
}

void ApplicationLayerRSU::generateTrustRating(bool doMalicious) {
    for (auto p : vehRecords) {
        if (p.second.size() >= 1) {
            int rating = 0;
            for (auto msg : p.second) {
                if (msg->isMalicious) {
                    rating--;
                } else {
                    rating++;
                }
            }

            if (rating > 0) {
                vehTrustRatings[p.first] = 1;
            } else if (rating < 0) {
                vehTrustRatings[p.first] = -1;
            }

            // altered
            if (doMalicious) {
                if (p.first < maliciousVehNum) vehTrustRatings[p.first] = 1;
                else vehTrustRatings[p.first] = -1;
            }
            // EV << "Trust Rating: veh[" << p.first << "] with " << vehTrustRatings[p.first] << endl;
        }

        for (auto msg : p.second) {
            delete msg;
        }
    }

    vehRecords.clear();

    // forged
    if (doMalicious) {
        for (int i=0; i<10; i++) {
            VehIdx id = std::rand() % vehTotalNum;
            if (id < maliciousVehNum) vehTrustRatings[id] = 1;
            else vehTrustRatings[id] = -1;
        }
    }

}

void ApplicationLayerRSU::generateBlock(int epoch) {
    bool doMalicious = false;
    if (isMalicious) {
        if (simTime() > maliciousDelay && rand()/double(RAND_MAX) < maliciousPoss) {
            doMalicious = true;
        }
    }
    generateTrustRating(doMalicious);
    Block* block = new Block();
    block->isForged = doMalicious;
    block->epoch = epoch;
    block->proposer = rsuID;
    block->setPrevHash(blockchain);
    for (auto& p : vehTrustRatings) {
        block->addTrustOffset(p.first, p.second);
    }
    vehTrustRatings.clear();
    block->generateHash();
    pendingBlocks[block->hash] = block;
    EV << "RSU[" << rsuID << "] generate block[" << block->hash << "]" << endl;

    string msgData = block->encode();

    IBDTMRSUMsg* msg = new IBDTMRSUMsg();
    msg->setMsgType(RSUMsgType::ProposedBlock);
    msg->setSender(rsuID);
    msg->setData(msgData.c_str());
    send(msg, "sessionOutput");

    if (epochCommittees.find(epoch) != epochCommittees.end()) {
        for (auto id : epochCommittees[epoch]) {
            if (id != rsuID) {
                IBDTMRSUMsg* msg = new IBDTMRSUMsg();
                msg->setMsgType(RSUMsgType::ProposedBlock);
                msg->setSender(rsuID);
                msg->setData(msgData.c_str());
                send(msg, "rsuOutputs", id);   
            }
        }   
    }
}

// void ApplicationLayerRSU::handleSelfMsg(cMessage* msg) {
//     DemoBaseApplLayer::handleSelfMsg(msg);
//     switch (msg->getKind()) {
//         case SEND_BEACON_EVT: {

//         }
//     }
// }
