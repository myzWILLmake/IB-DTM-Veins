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
#include "ib_dtm/ib_dtm.h"
#include "veins/veins.h"
#include "common.h"
#include "ib_dtm/IBDTMSession_m.h"

namespace ib_dtm {

class IB_DTM_API IBDTMSession : public cSimpleModule {
protected:
    int numInitStages() const override;
    void initialize(int stage) override;
    void handleMessage(cMessage* msg) override;
    
    int rsunum;
    int rsuInputBaseGateId;
    std::map<HashVal, Block*> blocks;
    Block* pendingBlock;

    void handleRSUMsg(int idx, cMessage* msg);
    void onNewCommittee();
    void onNewBlock(Block* block);
    void broadcastNewBlock(HashVal hash);
};

}

