#include "IBDTMRecorder.h"
using namespace std;

namespace ib_dtm {
    
IBDTMRecorder::IBDTMRecorder() {
    idx = 0;
}

void IBDTMRecorder::setMaliciousVehNum(int num) {
    maliciousNum = num;
}

void IBDTMRecorder::record(map<VehIdx, int> tvo, map<VehIdx, bool> marked, map<RSUIdx, IBDTMStake>& rsuStakes) {
    idx++;
    vehTrustValues[idx] = tvo;
    markedMalicious[idx] = marked;

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

    for (auto &p : vehTrustValues) {
        tvoFile << p.first << "," << p.second[id] << endl;
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

    for (auto &p : rsuEffeStakes) {
        rsuFile << p.first << "," << p.second[id] << "," << rsuITSStakes[p.first][id] << endl;
    }

    rsuFile.close();
}

void IBDTMRecorder::dumpMarkedMalicious() {
    string currTime = currentDateTime();
    ofstream maliciousFile("results/malicious_" + currTime);

    if (!maliciousFile.is_open()) {
        EV << "Cannot open malicious file!" << endl;
        return;
    }

    for (auto &p : markedMalicious) {
        int TP = 0;
        int TN = 0;
        int FP = 0;
        int FN = 0;
        for (auto &vp : p.second) {
            if (vp.first < maliciousNum && vp.second) TP++;
            else if (vp.first < maliciousNum && !vp.second) FN++;
            else if (vp.first >= maliciousNum && vp.second) FP++;
            else if (vp.first >= maliciousNum && !vp.second) TN++;
        }
        maliciousFile << p.first << "," << TP << "," << TN << "," << FP << "," << FN << endl;
    }


    maliciousFile.close();
}

}
