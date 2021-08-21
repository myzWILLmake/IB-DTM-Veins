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
    }
    if (stage == 2) {
        rsuInputBaseGateId = findGate("rsuInputs", 0);
        sessionInputGateId = findGate("sessionInput");
//        startService(Channel::sch2, rsuID, "rsu service");
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

void ApplicationLayerRSU::handleRSUMsg(int idx, cMessage* msg) {

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
            EV << "Received block: " << block->hash << endl;
            break;
        }
    }
}

void ApplicationLayerRSU::onNewCommittee(string data) {
    RSUIdx proposer;
    vector<RSUIdx> committee;
    SessionMsgHelper::decodeNewCommittee(data, proposer, committee);
    if (rsuID == proposer) {
        // genderate new block
        generateBlock();
        return;
    } else {
        // check if in committee
        inCommittee = false;
        for (auto id : committee) {
            if (id == rsuID) {
                inCommittee = true;
                break;
            }
        }
    }

    EV << "RSU[" << rsuID << "] received NewCommittee" << endl;
    EV << "    proposer:" << proposer << " committee? " << inCommittee << endl;
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
            EV << "RSU[" << rsuID << "] received reports from VEH[" << sender << "]" << endl;
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

void ApplicationLayerRSU::generateTrustRating() {
    for (auto p : vehRecords) {
        if (p.second.size() >= 3) {
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
        }

        for (auto msg : p.second) {
            delete msg;
        }
    }

    vehRecords.clear();
}

void ApplicationLayerRSU::generateBlock() {
    EV << "generateBlock" << endl;
    generateTrustRating();
    EV << "generatedTR" << endl;
    Block* block = new Block();
    block->setPrevHash(blockchain);
    for (auto& p : vehTrustRatings) {
        block->addTrustOffset(p.first, p.second);
    }
    vehTrustRatings.clear();
    block->generateHash();
    EV << "generateHash" << endl;
    IBDTMRSUMsg* msg = new IBDTMRSUMsg();
    msg->setMsgType(RSUMsgType::ProposedBlock);
    msg->setSender(rsuID);
    string msgData = block->encode();
    msg->setData(msgData.c_str());
    send(msg, "sessionOutput");
    EV << "sessiion msg sended" << endl;
    delete block;
}

// void ApplicationLayerRSU::handleSelfMsg(cMessage* msg) {
//     DemoBaseApplLayer::handleSelfMsg(msg);
//     switch (msg->getKind()) {
//         case SEND_BEACON_EVT: {

//         }
//     }
// }
