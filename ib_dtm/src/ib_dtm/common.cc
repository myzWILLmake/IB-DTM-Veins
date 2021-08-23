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
    split(data[2], trustOffsetStrs, ";");
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

}
