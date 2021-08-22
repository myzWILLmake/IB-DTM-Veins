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
#pragma once
#include "ib_dtm/common.h"
#include "ib_dtm/ib_dtm.h"
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "ib_dtm/IBDTMSession_m.h"

namespace ib_dtm {

class IB_DTM_API ApplicationLayerRSU : public veins::DemoBaseApplLayer {
public:
    void initialize(int stage) override;
protected:
    int rsuID;
    bool inCommittee;

    int rsuInputBaseGateId;
    int sessionInputGateId;
    std::map<VehIdx, std::vector<BeaconMsg*>> vehRecords;
    std::map<VehIdx, int> vehTrustRatings;
    std::map<HashVal, Block*> blocks;
    std::map<int, std::vector<RSUIdx>> epochCommittees;
    HashVal blockchain;
    int numInitStages() const override;
    void onWSM(veins::BaseFrame1609_4* wsm) override;
    void handleMessage(cMessage* msg) override;
    void handleRSUMsg(int idx, cMessage* msg);
    void handleSessionMsg(cMessage* msg);
    void decodeEventData(std::string eventData, vector<BeaconMsg*>& msgs);
    // void handleSelfMsg(cMessage* msg) override;

    void onNewCommittee(std::string data);
    bool isInCommittee(int epoch);

    void generateTrustRating();
    void generateBlock(int epoch);
};

}

