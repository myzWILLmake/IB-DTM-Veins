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

int IBDTMSession::numInitStages() const {
    return std::max(cSimpleModule::numInitStages(), 2);
}

void IBDTMSession::initialize(int stage) {
    cSimpleModule::initialize(stage);
    if (stage == 0) {
    }

    if (stage == 1) {
        rsuInputBaseGateId = findGate("rsuInputs", 0);
    }
}


void IBDTMSession::handleMessage(cMessage* msg) {
    if (msg->getArrivalGate()->getBaseId() == rsuInputBaseGateId) {
        int idx = msg->getArrivalGate()->getIndex();
        handleRSUMsg(idx, msg);
    }
}

void IBDTMSession::handleRSUMsg(int idx, cMessage* msg) {

}
