#pragma once
#include "ib_dtm/ib_dtm.h"
#include "common.h"
#include <iostream>
#include <fstream>

using namespace std;

namespace ib_dtm {

class IBDTMRecorder {
public:
    int idx;
    int maliciousNum;
    map<int, map<VehIdx, int>> vehTrustValues;
    map<int, map<VehIdx, bool>> markedMalicious;

    IBDTMRecorder();
    void setMaliciousVehNum(int num);
    void record(map<VehIdx, int> tvo, map<VehIdx, bool> marked);
    void dumpVehTrustValues(VehIdx id);
    void dumpMarkedMalicious();
};

}
