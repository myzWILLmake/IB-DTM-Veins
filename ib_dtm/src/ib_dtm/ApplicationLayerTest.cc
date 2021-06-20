//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "ib_dtm/ApplicationLayerTest.h"

#include "ib_dtm/ApplicationLayerTestMessage_m.h"

using namespace veins;
using namespace ib_dtm;
using namespace std;

Define_Module(ib_dtm::ApplicationLayerTest);

void ApplicationLayerTest::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        lastSentRSU = simTime();
        currentSubscribedServiceId = -1;

        vehID = getParentModule()->getIndex();
        vehRng = getRNG(0);
        nextInterval = 5 + vehRng->intRand(10);

        vehTotalNum = par("vehTotalNum");
        maliciousNum = par("maliciousNum");
        isMalicious = vehID % vehTotalNum < maliciousNum;
        if (isMalicious) {
            findHost()->getDisplayString().setTagArg("i", 1, "red");
        }

        msgSerialNo = 0;
    }

    if (traci) {
        auto roadlist = traci->getRoadIds();
        std::vector<std::string> roadvec;
        for (auto& road : roadlist) {
            if (road.find(":") == std::string::npos) {
                roadvec.push_back(road);
            }
        }
        auto target = roadvec[vehRng->intRand(roadvec.size())];
        if (traciVehicle) {
            traciVehicle->changeTarget(target);
            EV_DEBUG << "Vehicle " << vehID << " change target to " << target << endl;
        }
    }
}

void ApplicationLayerTest::recordBeaconMsg(int sender, bool isMaliciousMsg) {
    if (recordData.find(sender) == recordData.end()) {
        recordData[sender] = isMaliciousMsg;
    } else {
        recordData[sender] = recordData[sender] && isMaliciousMsg;
    }
}

string ApplicationLayerTest::encodeEventData() {
    // encode trust events to string
    string data = "";
    vector<int> positiveIDs;
    vector<int> negativeIDs;
    for (auto& p : recordData) {
        if (!p.second) {
            positiveIDs.push_back(p.first);
        } else {
            negativeIDs.push_back(p.first);
        }
    }
    for (int i=0; i<positiveIDs.size(); i++) {
        data += to_string(positiveIDs[i]) + " ";
    }
    data += ";";
    for (int i=0; i<negativeIDs.size(); i++) {
        data += to_string(negativeIDs[i]) + " ";
    }
    return data;
}

void ApplicationLayerTest::onWSA(DemoServiceAdvertisment* wsa)
{
    int rsuID = wsa->getPsid();
    EV << "VEH[" << vehID <<"] received RSU msg from RSU[" << rsuID << "]" << endl;
    if (!recordData.empty()) {
        lastSentRSU = simTime();
        string eventData = encodeEventData();
        recordData.clear();
        ApplicationLayerTestMessage* newwsm = new ApplicationLayerTestMessage();
        populateWSM(newwsm);
        newwsm->setMsgType(APPLICATION_MSG_TYPE_RSU);
        newwsm->setIsAck(true);
        newwsm->setSerial(this->msgSerialNo++);
        newwsm->setSender(this->vehID % vehTotalNum);
        newwsm->setEventData(eventData.c_str());
        sendDown(newwsm);
    }

//    if (currentSubscribedServiceId == -1) {
//        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
//        currentSubscribedServiceId = wsa->getPsid();
//        if (currentOfferedServiceId != wsa->getPsid()) {
//            stopService();
//            startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
//        }
//    }
}

void ApplicationLayerTest::onWSM(BaseFrame1609_4* frame)
{
    ApplicationLayerTestMessage* wsm = check_and_cast<ApplicationLayerTestMessage*>(frame);

    // findHost()->getDisplayString().setTagArg("i", 1, "green");
    int msgType = wsm->getMsgType();
    switch (msgType) {
        case APPLICATION_MSG_TYPE_VEH: {
            int serial = wsm->getSerial();
            int sender = wsm->getSender();
            if (sender != this->vehID % vehTotalNum) {
                bool isMaliciousMsg = wsm->getIsMalicious();
                if (isMaliciousMsg) {
                    EV << "VEH["<< vehID % vehTotalNum << "] received MALICIOUS msg from VEH[" << sender << "]" << endl;
                } else {
                    EV << "VEH["<< vehID % vehTotalNum << "] received REAL msg from VEH[" << sender << "]" << endl;
                }
                
                recordBeaconMsg(sender, isMaliciousMsg);
            }
            break;
        }
        case APPLICATION_MSG_TYPE_RSU: {
            bool isAck = wsm->getIsAck();
            if (isAck) return;
            break;
        }
    }


//
//    if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getDemoData(), 9999);
//    if (!sentMessage) {
//        sentMessage = true;
//        // repeat the received traffic update once in 2 seconds plus some random delay
//        wsm->setSenderAddress(myId);
//        wsm->setSerial(3);
//        scheduleAt(simTime() + 2 + uniform(0.01, 0.2), wsm->dup());
//    }
}

void ApplicationLayerTest::handleSelfMsg(cMessage* msg)
{
    // if (ApplicationLayerTestMessage* wsm = dynamic_cast<ApplicationLayerTestMessage*>(msg)) {
    //     // send this message on the service channel until the counter is 3 or higher.
    //     // this code only runs when channel switching is enabled
    //     sendDown(wsm->dup());
    //     wsm->setSerial(wsm->getSerial() + 1);
    //     if (wsm->getSerial() >= 3) {
    //         // stop service advertisements
    //         stopService();
    //         delete (wsm);
    //     }
    //     else {
    //         scheduleAt(simTime() + 1, wsm);
    //     }
    // }
    // else {
    //     DemoBaseApplLayer::handleSelfMsg(msg);
    // }
}

void ApplicationLayerTest::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    if (simTime() - lastDroveAt >= nextInterval) {
        // findHost()->getDisplayString().setTagArg("i", 1, "red");
        
        this->msgSerialNo++;
        ApplicationLayerTestMessage* wsm = new ApplicationLayerTestMessage();
        populateWSM(wsm);
        wsm->setMsgType(APPLICATION_MSG_TYPE_VEH);
        wsm->setSerial(this->msgSerialNo);
        wsm->setSender(this->vehID % vehTotalNum);
        wsm->setIsMalicious(this->isMalicious);
        sendDown(wsm);
        lastDroveAt = simTime();
        nextInterval = 5 + vehRng->intRand(10);
    }
    // stopped for for at least 10s?
//    if (mobility->getSpeed() < 1) {
//        if (simTime() - lastDroveAt >= 10 && sentMessage == false) {
//            findHost()->getDisplayString().setTagArg("i", 1, "red");
//            sentMessage = true;
//
//            ApplicationLayerTestMessage* wsm = new ApplicationLayerTestMessage();
//            populateWSM(wsm);
//            wsm->setDemoData(mobility->getRoadId().c_str());
//
//            // host is standing still due to crash
//            if (dataOnSch) {
//                startService(Channel::sch2, 42, "Traffic Information Service");
//                // started service and server advertising, schedule message to self to send later
//                scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
//            }
//            else {
//                // send right away on CCH, because channel switching is disabled
//                sendDown(wsm);
//            }
//        }
//    }
//    else {
//        lastDroveAt = simTime();
//    }
}
