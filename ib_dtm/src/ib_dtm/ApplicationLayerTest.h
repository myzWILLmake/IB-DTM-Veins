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

#pragma once

#include "ib_dtm/common.h"
#include "ib_dtm/ib_dtm.h"
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"

namespace ib_dtm {

class IB_DTM_API ApplicationLayerTest : public veins::DemoBaseApplLayer {
public:
    void initialize(int stage) override;

protected:
    simtime_t lastDroveAt;
    simtime_t lastSentRSU;
    int nextInterval;
    bool sentMessage;
    int currentSubscribedServiceId;
    cRNG* vehRng;
    int vehID;

    // malicious
    int vehTotalNum;
    int maliciousNum;
    bool isMalicious;

    // message
    int msgSerialNo;

    // record
//    std::map<int, bool> recordData;
    std::map<int, BeaconMsg*> recordData;
protected:
    void recordBeaconMsg(int sender, bool isMaliciousMsg);
    std::string encodeEventData();
    void clearRecordData();

    void onWSM(veins::BaseFrame1609_4* wsm) override;
    void onWSA(veins::DemoServiceAdvertisment* wsa) override;

    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
};

} // namespace ib_dtm
