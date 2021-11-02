#include "ib_dtm/common.h"
using namespace std;

namespace ib_dtm {
void split(const string& s, vector<string>& tokens, const string& delimiters) {
    size_t lastPos = s.find_first_not_of(delimiters, 0);
    size_t pos = s.find_first_of(delimiters, lastPos);
    while (pos != string::npos || lastPos != string::npos) {
        tokens.push_back(s.substr(lastPos, pos-lastPos));
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
}

string SessionMsgHelper::encodeNewCommittee(int epoch, RSUIdx proposer, vector<RSUIdx>& committee) {
    string data = to_string(epoch) + ";" + to_string(proposer) + ";";
    for (auto idx : committee) {
        data += to_string(idx) + " ";
    }
    return data;
}

void SessionMsgHelper::decodeNewCommittee(string input, int& epoch, RSUIdx& proposer, vector<RSUIdx>& committee) {
    vector<string> data;
    split(input, data, ";");
    epoch = stoi(data[0]);
    proposer = stoi(data[1]);
    if (data.size() <= 2) return;
    vector<string> committeeStr;
    split(data[2], committeeStr);
    committee.resize(committeeStr.size());
    for (int i=0; i<committee.size(); i++) {
        committee[i] = stoi(committeeStr[i]);
    }
}

BeaconMsg::BeaconMsg(int s, simtime_t t, bool m) {
    sender = s;
    time = t;
    isMalicious = m;
}

string BeaconMsg::encode() {
    string data;
    data += to_string(sender) + "_" + time.str() + "_";
    if (isMalicious) {
        data += "t";
    } else {
        data += "f";
    }
    return data;
}

void BeaconMsg::decode(string input) {
    vector<string> data;
    split(input, data, "_");
    sender = stoi(data[0]);
    time = SimTime::parse(data[1].c_str());
    isMalicious = false;
    if (data[2] == "t") isMalicious = true;
}

Block::Block() {
    prev = 0;
    hash = 0;
    isForged = false;
}

string Block::doEncode() {
    string data = "";
    data += to_string(prev) + " ";
    for (auto p : trustOffsets) {
        data += to_string(p.first) + ":" + to_string(p.second) + ";";
    }
    return data;
}

void Block::generateHash() {
    string rawEncoded = doEncode();
    hash = std::hash<string>{}(rawEncoded);
}

void Block::setPrevHash(HashVal p) {
    prev = p;
}

void Block::addTrustOffset(VehIdx id, int val) {
    trustOffsets[id] = val;
}

string Block::encode() {
    if (hash == 0) {
        generateHash();
    }
    string rawEncoded = doEncode();
    string data = to_string(epoch) + " " + to_string(proposer) + " " + to_string(hash) + " " + rawEncoded;
    return data;
}

void Block::decode(string input) {
    vector<string> data;
    split(input, data);
    epoch = stoi(data[0]);
    proposer = stoi(data[1]);
    hash = stoul(data[2]);
    prev = stoul(data[3]);

    if (data.size() <=4) return;
    vector<string> trustOffsetStrs;
    split(data[4], trustOffsetStrs, ";");
    for (auto& str : trustOffsetStrs) {
        int pos = str.find(":");
        VehIdx id = VehIdx(stoi(str.substr(0, pos)));
        int val = stoi(str.substr(pos+1));
        trustOffsets[id] = val;
    }
}

int Block::getRecordCnt() {
    return trustOffsets.size();
}

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

double IBDTMStake::initEffectiveStake = 0;
double IBDTMStake::effectiveStakeUpperBound = 0;
double IBDTMStake::effectiveStakeLowerBound = 0;
int IBDTMStake::initITSstake = 0;
int IBDTMStake::numVehicles = 0;
double IBDTMStake::baseReward = 0;
double IBDTMStake::penaltyFactor = 0;
int IBDTMStake::traceBackEpoches = 0;
IBDTMStake::IBDTMStake() {
    itsStakeMap[0] = initITSstake;
    effectiveStake = initEffectiveStake;
}

void IBDTMStake::getReward() {
    double stakeFactor = double(getITSStake(0)) / numVehicles;
    double reward = baseReward * sqrt(stakeFactor);
    double before = effectiveStake;
    effectiveStake += reward;
    if (effectiveStake > effectiveStakeUpperBound) effectiveStake = effectiveStakeUpperBound;
    EV << "RSU[" << id << "] effectiveStake get reward: " << before << " -> " << effectiveStake << endl;
}

void IBDTMStake::getPunishment() {
    double stakeFactor = double(getITSStake(0)) / numVehicles;
    double p = sqrt(stakeFactor);
    if (p < 1) p = 1;
    double penalty = baseReward * p * penaltyFactor;
    double before = effectiveStake;
    effectiveStake -= penalty;
    if (effectiveStake < effectiveStakeLowerBound) effectiveStake = effectiveStakeLowerBound;
    EV << "RSU[" << id << "] effectiveStake get punishment: " << before << " -> " << effectiveStake << endl;
}

void IBDTMStake::checkITSStake(int epoch) {
    for (auto it = itsStakeMap.begin(); it != itsStakeMap.end(); ) {
        if (it->first <= epoch - IBDTMStake::traceBackEpoches) {
            it = itsStakeMap.erase(it);
        } else {
            it++;
        }
    }
}

void IBDTMStake::addITSStake(int epoch, int num) {
    checkITSStake(epoch);
    itsStakeMap[epoch] = num;
}

int IBDTMStake::getITSStake(int epoch) {
    if (epoch != 0) checkITSStake(epoch);
    int cnt = 0;
    for (auto &p : itsStakeMap) {
        cnt += p.second;
    }
    if (cnt < 1) cnt = 1;
    return cnt;
}

bool IBDTMStake::isLessLowerBound() {
    return effectiveStake <= effectiveStakeLowerBound;
}

IBDTMStakeVoting::IBDTMStakeVoting() {
    effectiveStakeSum = 0;
}

bool IBDTMStakeVoting::checkNegtiveVotes() {
    if (effectiveStakeSum == 0) return false;
    double negativeVoteStakes = 0;
    for (auto& p : votes) {
        if (!p.second) {
            negativeVoteStakes += effectiveStakes[p.first];
        }
    }

    if (negativeVoteStakes / effectiveStakeSum >= 1.0/3) {
        return true;
    } else return false;
}

bool IBDTMStakeVoting::checkPositiveVotes() {
    if (effectiveStakeSum == 0) return false;
    double positiveVoteStakes = 0;
    for (auto& p : votes) {
        if (p.second) {
            positiveVoteStakes += effectiveStakes[p.first];
        }
    }

    if (positiveVoteStakes / effectiveStakeSum > 2.0/3) {
        return true;
    } else return false;
}

}
