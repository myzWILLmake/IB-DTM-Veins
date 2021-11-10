#pragma once
#include "ib_dtm/ib_dtm.h"
#include "common.h"
#include <iostream>
#include <fstream>

using namespace std;

namespace ib_dtm {

class IBDTMRecorder {
public:
    int maliciousNum;
    vector<map<VehIdx, int>> vehTrustValues;
    vector<map<VehIdx, bool>> markedMalicious;
    vector<map<RSUIdx, double>> rsuEffeStakes;
    vector<map<RSUIdx, int>> rsuITSStakes;

    IBDTMRecorder();
    void setMaliciousVehNum(int num);
    void record(map<VehIdx, int> tvo, map<VehIdx, bool> marked);
    void recordStakes(map<RSUIdx, IBDTMStake>& rsuStakes);
    void dumpRSUStakes(RSUIdx id);
    void dumpVehTrustValues(VehIdx id);
    void dumpVehDetected();
    void dumpMarkedMalicious();
};

}
