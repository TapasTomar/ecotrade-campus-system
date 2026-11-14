#ifndef BLOODDONATE_H
#define BLOODDONATE_H

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <ctime>

using namespace std;

class BloodPacket {
public:
    int packetId;
    long long donorId;
    string bloodGroup;
    string centerName;

    BloodPacket(int pId, long long dId, string bGroup, string cName) {
        packetId = pId;
        donorId = dId;
        bloodGroup = bGroup;
        centerName = cName;
    }

    void show() {
        cout << "Packet ID: " << packetId 
             << " | Group: " << bloodGroup 
             << " | Donor ID: " << donorId 
             << " | Center: " << centerName << endl;
    }
};

class EmergencyRequest {
public:
    string hospitalName;
    string requiredGroup;
    int urgency;        // 3 = Critical (ICU), 2 = High (OT), 1 = Routine
    int arrivalOrder;

    EmergencyRequest(string hName, string bGroup, int urg, int order) {
        hospitalName = hName;
        requiredGroup = bGroup;
        urgency = urg;
        arrivalOrder = order;
    }

    bool operator<(const EmergencyRequest& other) const {
        if (urgency != other.urgency) {
            return urgency < other.urgency;
        }
        return arrivalOrder > other.arrivalOrder;
    }
};

class BloodBankEngine {
private:
    vector<BloodPacket> inventory;
    priority_queue<EmergencyRequest> emergencyHeap;
    unordered_map<long long, long long> lastDonationMap;
    int nextPacketId;
    int requestCounter;

    void saveDispatchLog(string hospital, string group, int packetId, string tier) {
        ofstream file("blood_dispatch_history.txt", ios::app);
        file << "Hospital: " << hospital 
             << " | Group: " << group 
             << " | Packet ID: " << packetId 
             << " | Priority: " << tier << endl;
        file.close();
    }

    // Persistent 90-day cooldown tracker (survives packet dispatch)
    void saveCooldowns() {
        ofstream file("donor_cooldowns.txt");
        for (auto pair : lastDonationMap) {
            file << pair.first << " " << pair.second << endl;
        }
        file.close();
    }

    void loadCooldowns() {
        ifstream file("donor_cooldowns.txt");
        if (!file) return;
        long long dId, ts;
        while (file >> dId >> ts) {
            lastDonationMap[dId] = ts;
        }
        file.close();
    }

public:
    BloodBankEngine() {
        nextPacketId = 501;
        requestCounter = 1;
        loadCooldowns();
        loadData();
    }

    void saveData() {
        ofstream file("blood_data.txt");
        for (int i = 0; i < inventory.size(); i++) {
            file << inventory[i].packetId << endl;
            file << inventory[i].donorId << endl;
            file << inventory[i].bloodGroup << endl;
            file << inventory[i].centerName << endl;
        }
        file.close();
        saveCooldowns();
    }

    void loadData() {
        ifstream file("blood_data.txt");
        if (!file) {
            donateBlood(25101281462LL, "O+", "Campus_Health_Center", false);
            donateBlood(25101281063LL, "B+", "City_Blood_Bank", false);
            donateBlood(25103704010LL, "O-", "Campus_Health_Center", false);
            saveData();
            return;
        }

        int pId;
        long long dId;
        string grp, center;

        while (file >> pId) {
            file >> dId;
            file.ignore();
            getline(file, grp);
            getline(file, center);

            inventory.push_back(BloodPacket(pId, dId, grp, center));
            if (pId >= nextPacketId) {
                nextPacketId = pId + 1;
            }
        }
        file.close();
    }

    bool donateBlood(long long donorId, string bGroup, string center, bool showMsg = true) {
        long long now = time(0);
        long long ninetyDays = 90LL * 24 * 60 * 60;

        if (showMsg && lastDonationMap.find(donorId) != lastDonationMap.end()) {
            long long timePassed = now - lastDonationMap[donorId];
            if (timePassed < ninetyDays) {
                long long daysLeft = (ninetyDays - timePassed) / (24 * 60 * 60);
                cout << "\n[MEDICAL BLOCK] Donor ID " << donorId 
                     << " is under mandatory 90-day cooldown (" << daysLeft << " days left).\n";
                return false;
            }
        }

        inventory.push_back(BloodPacket(nextPacketId, donorId, bGroup, center));
        lastDonationMap[donorId] = now;

        if (showMsg) {
            cout << "\n[SUCCESS] Blood Packet Stored! Packet ID: " << nextPacketId << endl;
            saveData();
        }
        nextPacketId++;
        return true;
    }

    void showInventory() {
        cout << "\n--- AVAILABLE BLOOD PACKETS (" << inventory.size() << ") ---\n";
        if (inventory.empty()) {
            cout << "No blood packets available.\n";
            return;
        }
        for (int i = 0; i < inventory.size(); i++) {
            inventory[i].show();
        }
    }

    void showStockSummary() {
        string groups[8] = {"A+", "A-", "B+", "B-", "O+", "O-", "AB+", "AB-"};
        cout << "\n--- BLOOD BANK STOCK SUMMARY ---\n";
        for (int g = 0; g < 8; g++) {
            int count = 0;
            for (int i = 0; i < inventory.size(); i++) {
                if (inventory[i].bloodGroup == groups[g]) {
                    count++;
                }
            }
            cout << groups[g] << " : " << count << " units";
            if (count == 0) {
                cout << "  [LOW STOCK ALERT!]";
            }
            cout << endl;
        }
    }

    void addEmergencyRequest(string hospital, string bGroup, int urgency) {
        if (urgency < 1 || urgency > 3) {
            cout << "\nInvalid Urgency! Enter 1, 2, or 3.\n";
            return;
        }
        emergencyHeap.push(EmergencyRequest(hospital, bGroup, urgency, requestCounter));
        requestCounter++;
        cout << "\n[QUEUED] Request added to Priority Queue! (Pending: " << emergencyHeap.size() << ")\n";
    }

    void processNextDispatch() {
        if (emergencyHeap.empty()) {
            cout << "\nNo pending emergency requests!\n";
            return;
        }

        EmergencyRequest topReq = emergencyHeap.top();
        emergencyHeap.pop();

        string tier = "ROUTINE";
        if (topReq.urgency == 3) tier = "CRITICAL (ICU)";
        else if (topReq.urgency == 2) tier = "HIGH (OT)";

        cout << "\n--- PROCESSING TOP PRIORITY REQUEST ---\n";
        cout << "Hospital: " << topReq.hospitalName 
             << " | Needed: " << topReq.requiredGroup 
             << " | Priority: " << tier << endl;

        // 1. Exact blood group match
        for (int i = 0; i < inventory.size(); i++) {
            if (inventory[i].bloodGroup == topReq.requiredGroup) {
                cout << "[DISPATCHED] Exact Match! Packet ID " << inventory[i].packetId 
                     << " (" << inventory[i].bloodGroup << ") sent!\n";
                saveDispatchLog(topReq.hospitalName, inventory[i].bloodGroup, inventory[i].packetId, tier);
                inventory.erase(inventory.begin() + i);
                saveData();
                return;
            }
        }

        // 2. Universal Donor O- Fallback
        if (topReq.requiredGroup != "O-") {
            for (int i = 0; i < inventory.size(); i++) {
                if (inventory[i].bloodGroup == "O-") {
                    cout << "[UNIVERSAL FALLBACK] Allocated O- Packet ID " << inventory[i].packetId << "!\n";
                    saveDispatchLog(topReq.hospitalName, "O- (Universal)", inventory[i].packetId, tier);
                    inventory.erase(inventory.begin() + i);
                    saveData();
                    return;
                }
            }
        }

        cout << "[SHORTAGE ALERT] Neither " << topReq.requiredGroup << " nor O- available!\n";
    }

    void showDispatchHistory() {
        cout << "\n--- BLOOD DISPATCH HISTORY ---\n";
        ifstream file("blood_dispatch_history.txt");
        if (!file) {
            cout << "No dispatches recorded yet.\n";
            return;
        }
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
    }
};

#endif