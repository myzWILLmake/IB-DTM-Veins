#include "IBDTMRecorder.h"
using namespace std;

namespace ib_dtm {
    
IBDTMRecorder::IBDTMRecorder() {
}

void IBDTMRecorder::setMaliciousVehNum(int num) {
    maliciousNum = num;
}

void IBDTMRecorder::record(map<VehIdx, int> tvo, map<VehIdx, bool> marked) {
    vehTrustValues.push_back(tvo);
    markedMalicious.push_back(marked);
}

void IBDTMRecorder::recordStakes(map<RSUIdx, IBDTMStake>& rsuStakes) {
    rsuEffeStakes.push_back(map<RSUIdx, double>{});
    rsuITSStakes.push_back(map<RSUIdx, int>{});
    int idx = rsuEffeStakes.size() - 1;
    for (auto &p : rsuStakes) {
        rsuEffeStakes[idx][p.first] = p.second.effectiveStake;
        rsuITSStakes[idx][p.first] = p.second.getITSStake(0);
    }
}

void IBDTMRecorder::dumpVehTrustValues(VehIdx id) {
    string currTime = currentDateTime();
    ofstream tvoFile("results/tvo_" + to_string(id) + "_" + currTime);

    if (!tvoFile.is_open()) {
        EV << "Cannot open tvo file!" << endl;
        return;
    }

    for (int i=0; i<vehTrustValues.size(); i++) {
        tvoFile << i << "," << vehTrustValues[i][id] << endl;
    }

    tvoFile.close();
}

void IBDTMRecorder::dumpRSUStakes(RSUIdx id) {
    string currTime = currentDateTime();
    ofstream rsuFile("results/rsu_" + to_string(id) + "_" + currTime);

    if (!rsuFile.is_open()) {
        EV << "Cannot open rsu file!" << endl;
        return;
    }  

    for (int i=0; i<rsuEffeStakes.size(); i++) {
        rsuFile << i << "," << rsuEffeStakes[i][id] << "," << rsuITSStakes[i][id] << endl;
    }

    rsuFile.close();
}

void IBDTMRecorder::dumpVehDetected() {
    string currTime = currentDateTime();
    ofstream vehDetectedFile("results/veh_" + currTime);

    if (!vehDetectedFile.is_open()) {
        EV << "Cannot open malicious file!" << endl;
        return;
    }


    map<VehIdx, int> vehDetectedMap;
    for (int i=0; i<markedMalicious.size(); i++) {
        auto& p = markedMalicious[i];
        for (auto &vp : p) {
            if (vp.second && vehDetectedMap.find(vp.first) == vehDetectedMap.end()) {
                vehDetectedMap[vp.first] = i;
            }
        }
    }

    for (auto &p : vehDetectedMap) {
        vehDetectedFile << p.first << "," << p.second << endl;
    }

    vehDetectedFile.close();
}

void IBDTMRecorder::dumpMarkedMalicious() {
    string currTime = currentDateTime();
    ofstream maliciousFile("results/malicious_" + currTime);

    if (!maliciousFile.is_open()) {
        EV << "Cannot open malicious file!" << endl;
        return;
    }

    for (int i=0; i<markedMalicious.size(); i++) {
        auto& p = markedMalicious[i];
        int TP = 0;
        int TN = 0;
        int FP = 0;
        int FN = 0;
        for (auto &vp : p) {
            if (vp.first < maliciousNum && vp.second) TP++;
            else if (vp.first < maliciousNum && !vp.second) FN++;
            else if (vp.first >= maliciousNum && vp.second) FP++;
            else if (vp.first >= maliciousNum && !vp.second) TN++;
        }
        maliciousFile << i << "," << TP << "," << TN << "," << FP << "," << FN << endl;
    }

    maliciousFile.close();
}

}
