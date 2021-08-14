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

void ApplicationLayerRSU::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        rsuID = getParentModule()->getIndex();
    }
    if (stage == 1) {
        startService(Channel::sch2, rsuID, "rsu service");
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

// void ApplicationLayerRSU::handleSelfMsg(cMessage* msg) {
//     DemoBaseApplLayer::handleSelfMsg(msg);
//     switch (msg->getKind()) {
//         case SEND_BEACON_EVT: {

//         }
//     }
// }
