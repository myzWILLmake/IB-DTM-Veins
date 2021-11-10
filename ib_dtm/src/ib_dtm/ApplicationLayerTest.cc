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
#include <cstdlib>

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
        maliciousPoss = par("maliciousPoss");
        maliciousDelay = par("maliciousDelay");
        srand(((unsigned)time(NULL) + vehID % vehTotalNum));
        isMalicious = vehID % vehTotalNum < maliciousNum;
        if (isMalicious) {
            findHost()->getDisplayString().setTagArg("i", 1, "red");
        }

        msgSerialNo = 0;
    }

    if (stage == 1) {
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
}

void ApplicationLayerTest::recordBeaconMsg(int sender, bool isMaliciousMsg) {
    if (recordData.find(sender) == recordData.end()) {
        recordData[sender] = new BeaconMsg(sender, simTime(), isMaliciousMsg);
    } else {
        recordData[sender]->time = simTime();
        recordData[sender]->isMalicious &= isMaliciousMsg;
    }

    if (simTime() > maliciousDelay && isMalicious) {
        bool origin = recordData[sender]->isMalicious;
        recordData[sender]->isMalicious = !origin;
    }
}

string ApplicationLayerTest::encodeEventData() {
    // encode trust events to string
    string data = "";
    for (auto& p : recordData) {
        data += p.second->encode() + " ";
    }
    return data;
}

void ApplicationLayerTest::clearRecordData() {
    for (auto& p : recordData) {
        delete p.second;
    }
    recordData.clear();
}

void ApplicationLayerTest::onWSA(DemoServiceAdvertisment* wsa)
{
    int rsuID = wsa->getPsid();
    // EV << "VEH[" << vehID <<"] received RSU msg from RSU[" << rsuID << "]" << endl;
    if (!recordData.empty()) {
        lastSentRSU = simTime();
        string eventData = encodeEventData();
        clearRecordData();
        ApplicationLayerTestMessage* newwsm = new ApplicationLayerTestMessage();
        populateWSM(newwsm);
        newwsm->setMsgType(APPLICATION_MSG_TYPE_RSU);
        newwsm->setIsAck(true);
        newwsm->setSerial(this->msgSerialNo++);
        newwsm->setSender(this->vehID % vehTotalNum);
        newwsm->setEventData(eventData.c_str());
        sendDown(newwsm);
    }
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
                // if (isMaliciousMsg) {
                //     EV << "VEH["<< vehID % vehTotalNum << "] received MALICIOUS msg from VEH[" << sender << "]" << endl;
                // } else {
                //     EV << "VEH["<< vehID % vehTotalNum << "] received REAL msg from VEH[" << sender << "]" << endl;
                // }
                
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
}

void ApplicationLayerTest::handleSelfMsg(cMessage* msg)
{
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
        if (this->isMalicious) {
            if (simTime() >= maliciousDelay && rand()/double(RAND_MAX) < maliciousPoss) {
                wsm->setIsMalicious(true);
            } else {
                wsm->setIsMalicious(false);
            }
        } else {
            wsm->setIsMalicious(false);
        }
        sendDown(wsm);
        lastDroveAt = simTime();
        nextInterval = 5 + vehRng->intRand(10);
    }
}
