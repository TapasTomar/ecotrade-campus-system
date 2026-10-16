#ifndef BLOOD_H
#define BLOOD_H

#include <iostream>
#include <string>
#include <queue>
#include <vector>

using namespace std;

// Stored Blood Packet in Inventory (Donor details)
struct StoredBloodPacket {
    int id;
    long long donorId;
    string bloodGroup;
    string centerName;
};

// Outbound Request (Prioritized by Urgency in Max-Heap)
struct BloodDispatchTicket {
    string hospitalName;
    string requiredGroup;
    int urgencyLevel; // 3: ICU/Critical, 2: High (OT), 1: Routine

    // Max-Heap comparator: Highest urgency on top
    bool operator<(const BloodDispatchTicket& other) const {
        return urgencyLevel < other.urgencyLevel;
    }
};

class BloodDonationEngine {
private:
    vector<StoredBloodPacket> inventory;
    priority_queue<BloodDispatchTicket> dispatchQueue;
    int nextId;

public:
    BloodDonationEngine() : nextId(501) {}

    void loadSampleData() {
        inventory.push_back({nextId++, 25101281462, "O+", "City_Health_Center"});
        inventory.push_back({nextId++, 25101281063, "B+", "Campus_Blood_Bank"});
        inventory.push_back({nextId++, 2510370401, "O+", "City_Health_Center"});
    }

    // Donor registration - Normal Inventory Storage
    void storeBlood(long long donorId, string grp, string center) {
        inventory.push_back({nextId++, donorId, grp, center});
        cout << "\nBlood packet stored successfully in inventory with Packet ID: " << nextId - 1 << "\n";
    }

    // Search & Dispatch based on Patient Request Urgency (DSA: Priority Queue)
    bool processDispatchTicket(string hosp, string grp, int urgency) {
        BloodDispatchTicket ticket = {hosp, grp, urgency};
        dispatchQueue.push(ticket);

        cout << "\n[Priority Queue] Request queued with Urgency Tier: " 
             << (urgency == 3 ? "CRITICAL (ICU)" : (urgency == 2 ? "HIGH (OT)" : "ROUTINE")) << "\n";

        // Check Inventory for matching Blood Group
        int matchIndex = -1;
        for (size_t i = 0; i < inventory.size(); i++) {
            if (inventory[i].bloodGroup == grp) {
                matchIndex = i;
                break;
            }
        }

        if (matchIndex == -1) {
            cout << "\n[Out of Stock] No available packets for Group " << grp << ".\n";
            dispatchQueue.pop();
            return false;
        }

        cout << "\nMATCH FOUND IN INVENTORY:\n";
        cout << "Packet ID:        " << inventory[matchIndex].id << "\n";
        cout << "Donor Student ID: " << inventory[matchIndex].donorId << "\n";
        cout << "Blood Group:      " << inventory[matchIndex].bloodGroup << "\n";
        cout << "Storage Center:   " << inventory[matchIndex].centerName << "\n";

        string confirm;
        cout << "\nConfirm dispatch to " << hosp << "? (yes/no): ";
        cin >> confirm;

        if (confirm == "yes") {
            inventory.erase(inventory.begin() + matchIndex);
            dispatchQueue.pop();
            cout << "\nPacket dispatched successfully to hospital.\n";
            return true;
        } else {
            dispatchQueue.pop();
            cout << "\nDispatch cancelled. Packet remains in inventory.\n";
            return false;
        }
    }
};

#endif