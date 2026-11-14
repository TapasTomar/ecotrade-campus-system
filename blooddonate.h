#ifndef BLOODDONATE_H
#define BLOODDONATE_H

#include <iostream>
#include <string>
#include <queue>
#include <vector>

using namespace std;

struct BloodRequest {
    string group;
    int urgency; // 1 = Critical, 2 = High, 3 = Normal
    int units;

    bool operator>(const BloodRequest& other) const {
        return urgency > other.urgency;
    }
};

class BloodDonate {
private:
    priority_queue<BloodRequest, vector<BloodRequest>, greater<BloodRequest>> pq;

    bool isValidGroup(string bg) {
        string groups[8] = {"A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"};
        for (int i = 0; i < 8; i++) {
            if (bg == groups[i]) {
                return true;
            }
        }
        return false;
    }

public:
    BloodDonate() {}

    string dispatch(string group, int available, int requested, int urgency) {
        if (!isValidGroup(group)) {
            return "ERROR|Invalid blood group";
        }
        if (requested <= 0) {
            return "ERROR|Requested units must be at least 1";
        }

        pq.push({group, urgency, requested});
        BloodRequest topReq = pq.top();
        pq.pop();

        if (available < topReq.units) {
            return "ERROR|Insufficient blood units in stock";
        }

        int remaining = available - topReq.units;
        string level = "NORMAL";
        if (topReq.urgency == 1) {
            level = "CRITICAL";
        } else if (topReq.urgency == 2) {
            level = "HIGH";
        }

        return "DISPATCHED|" + to_string(remaining) + "|" + level;
    }
};

#endif