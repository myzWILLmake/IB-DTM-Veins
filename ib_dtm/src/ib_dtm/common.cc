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

}
