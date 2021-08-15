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

#include "RSUInserter.h"
Define_Module(ib_dtm::RSUInserter);

using namespace veins;
using namespace ib_dtm;


RSUInserter::RSUInserter() {
    // TODO Auto-generated constructor stub

}

RSUInserter::~RSUInserter() {
    // TODO Auto-generated destructor stub
}

int RSUInserter::numInitStages() const
{
    return std::max(cSimpleModule::numInitStages(), 2);
}

void RSUInserter::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage != 1) {
        return;
    }

    isGrid = par("isGrid");
    xGridSize = par("xGridSize");
    yGridSize = par("yGridSize");
    roadLength = par("roadLength");

    insertRSU();
}

void RSUInserter::insertRSU() {
    cModule* parentmod =  getParentModule();
    if (!parentmod) error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("org.car2x.veins.nodes.RSU");
    if (!nodeType) error("Module RSU not found");

    int rsunum = (xGridSize+1) * (yGridSize + 1);
    int idx = 0;
    int xOffset = 25;
    int yOffset = 25;
    std::vector<cModule*> rsus;
    for (int x = 0; x<=xGridSize; x++) {
        for (int y = 0; y<=yGridSize; y++) {
            cModule* mod = nodeType->create("rsu", parentmod, rsunum, idx++);
            mod->finalizeParameters();
            mod->setGateSize("rsuInputs", rsunum);
            mod->setGateSize("rsuOutputs", rsunum);
            mod->buildInside();
            auto mob = mod->getSubmodule("mobility");
            mob->par("x") = x*roadLength + xOffset;
            mob->par("y") = y*roadLength + yOffset;
            mod->scheduleStart(0);
            mod->callInitialize();
            rsus.push_back(mod);
        }
    }

    cModule* session = parentmod->getSubmodule("ibdtmSession");
    session->setGateSize("rsuInputs", rsunum);
    session->setGateSize("rsuOutputs", rsunum);
    for (int i=0; i<rsunum; i++) {
        auto x = rsus[i];
        cGate* xSessionInGate = x->gate("sessionInput");
        cGate* xSessionOutGate = x->gate("sessionOutput");
        cGate* sessionInGate = session->gate("rsuInputs", i);
        cGate* sessionOutGate = session->gate("rsuOutputs", i);
        sessionOutGate->connectTo(xSessionInGate);
        xSessionOutGate->connectTo(sessionInGate);

        for (int j=i+1; j<rsunum; j++) {
            auto y = rsus[j];
            cGate* xInGate = x->gate("rsuInputs", j);
            cGate* yInGate = y->gate("rsuInputs", i);
            cGate* xOutGate = x->gate("rsuOutputs", j);
            cGate* yOutGate = y->gate("rsuOutputs", i);
            xOutGate->connectTo(yInGate);
            yOutGate->connectTo(xInGate);
        }
    }


}
