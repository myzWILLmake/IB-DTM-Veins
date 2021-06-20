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

void split(const string& s, vector<string>& tokens, const string& delimiters = " ") {
    size_t lastPos = s.find_first_not_of(delimiters, 0);
    size_t pos = s.find_first_of(delimiters, lastPos);
    while (pos != string::npos || lastPos != string::npos) {
        tokens.push_back(s.substr(lastPos, pos-lastPos));
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
}

void decodeEventData(const string& data, vector<int>& positiveIDs, vector<int>& negativeIDs) {
    int deliIdx = data.find(';');
    string positiveStr = data.substr(0, deliIdx);
    vector<string> posStrIds;
    split(positiveStr, posStrIds);
    for (auto& s : posStrIds) {
        positiveIDs.push_back(stoi(s));
    }

    string negativeStr = data.substr(deliIdx+1);
    vector<string> negStrIds;
    split(negativeStr, negStrIds);
    for (auto& s : negStrIds) {
        negativeIDs.push_back(stoi(s));
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
            vector<int> positiveIDs;
            vector<int> negativeIDs;
            decodeEventData(eventData, positiveIDs, negativeIDs);
            EV << "    Positive VEHs:" << endl;
            EV << "        ";
            for (auto& id : positiveIDs) {
                EV << id << " ";
            }
            EV << endl;

            EV << "    Negative VEHs:" << endl;
            EV << "        ";
            for (auto& id : negativeIDs) {
                EV << id << " ";
            }
            EV << endl;
            break;
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
