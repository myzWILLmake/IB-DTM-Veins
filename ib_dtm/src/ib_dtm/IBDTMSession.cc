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
    }

    if (stage == 2) {
        rsuInputBaseGateId = findGate("rsuInputs", 0);
        rsunum = gateSize("rsuInputs");
        EV << "rsugatesize rsunum = " << rsunum << endl;

        // Temp: test schduled msg
        cMessage* test = new cMessage("test");
        scheduleAt(0.2, test);
    }
}


void IBDTMSession::handleMessage(cMessage* msg) {
    EV << "recevied msg at session" << endl;
    cGate* gate = msg->getArrivalGate();
    if (msg->isSelfMessage()) {
        onNewCommittee();    
    } else if (gate && gate->getBaseId() == rsuInputBaseGateId) {
        int idx = msg->getArrivalGate()->getIndex();
        handleRSUMsg(idx, msg);
    } else {
        EV << "WARNING: NO PROCESS " << msg << endl;
    }
}

void IBDTMSession::onNewCommittee() {
    IBDTMSessionMsg* msg = new IBDTMSessionMsg();
    msg->setMsgType(SessionMsgType::NewCommittee);
    msg->setData("test");
    send(msg, "rsuOutputs", 1);
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
    }
}

void IBDTMSession::onNewBlock(Block* block) {
    EV << "onNewBlock" << endl;
    //TODO: check block validation
    pendingBlock = block;
    //TODO: consensus progress
    //Temp: approve this block
    blocks[block->hash] = block;
    broadcastNewBlock(block->hash);
}

void IBDTMSession::broadcastNewBlock(HashVal hash) {
    auto block = blocks[hash];
    string msgData = block->encode();
    
    for (int i=0; i<rsunum; i++) {
        IBDTMSessionMsg* msg = new IBDTMSessionMsg();
        msg->setMsgType(SessionMsgType::CommittedBlock);
        msg->setData(msgData.c_str());
        send(msg, "rsuOutputs", i);
    }
}
