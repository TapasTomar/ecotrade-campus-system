#ifndef BARTER_H
#define BARTER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <cstdlib>
#include <ctime>

using namespace std;

// Convert capital letters to small letters
string makeSmall(string s) {
    for (int i = 0; i < s.length(); i++) {
        if (s[i] >= 'A' && s[i] <= 'Z') {
            s[i] = s[i] + 32;
        }
    }
    return s;
}

// Base Class (Inheritance)
class Item {
public:
    int id;
    string name;
    long long ownerId;

    Item(int itemId, long long sId, string itemName) {
        id = itemId;
        ownerId = sId;
        name = itemName;
    }

    virtual void show() {
        cout << "ID: " << id << " | Item: " << name << endl;
    }
};

// Derived Class + Doubly Linked List Node
class ElectronicNode : public Item {
public:
    string condition;
    string wantItem;
    string status;          // AVAILABLE, REQUESTED, HANDSHAKE
    long long buyerId;
    string offerItem;
    int otpCode;

    ElectronicNode* next;
    ElectronicNode* prev;

    ElectronicNode(int itemId, long long sId, string itemName, string cond, string want)
        : Item(itemId, sId, itemName) {
        condition = cond;
        wantItem = want;
        status = "AVAILABLE";
        buyerId = 0;
        offerItem = "None";
        otpCode = 0;
        next = NULL;
        prev = NULL;
    }

    void show() {
        cout << "ID: " << id 
             << " | Item: " << name 
             << " | Owner: " << ownerId 
             << " | Cond: " << condition 
             << " | Wants: " << wantItem 
             << " | Status: " << status << endl;

        if (status == "REQUESTED") {
            cout << "   -> Offer from ID " << buyerId << ": " << offerItem << endl;
        }
        if (status == "HANDSHAKE") {
            cout << "   -> Offer Accepted! Waiting for 4-digit Handshake OTP." << endl;
        }
    }
};

class BarterEngine {
private:
    ElectronicNode* head;
    ElectronicNode* tail;
    int nextId;
    unordered_map<int, ElectronicNode*> itemMap;

    // Delete node from Doubly Linked List in O(1)
    void deleteNode(ElectronicNode* node) {
        if (node == NULL) return;

        if (node == head && node == tail) {
            head = NULL;
            tail = NULL;
        } 
        else if (node == head) {
            head = head->next;
            head->prev = NULL;
        } 
        else if (node == tail) {
            tail = tail->prev;
            tail->next = NULL;
        } 
        else {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }

        itemMap.erase(node->id);
        delete node;
    }

    // Save completed trade record
    void saveHistory(ElectronicNode* item) {
        ofstream file("trade_history.txt", ios::app);
        file << "Item: " << item->name 
             << " | Owner: " << item->ownerId 
             << " | Swapped With: " << item->offerItem 
             << " | Buyer: " << item->buyerId << endl;
        file.close();
    }

public:
    BarterEngine() {
        head = NULL;
        tail = NULL;
        nextId = 101;
        srand(time(0));
        loadData();
    }

    void saveData() {
        ofstream file("barter_data.txt");
        ElectronicNode* temp = head;

        while (temp != NULL) {
            file << temp->id << endl;
            file << temp->ownerId << endl;
            file << temp->name << endl;
            file << temp->condition << endl;
            file << temp->wantItem << endl;
            file << temp->status << endl;
            file << temp->buyerId << endl;
            file << temp->offerItem << endl;
            file << temp->otpCode << endl;
            temp = temp->next;
        }
        file.close();
    }

    void loadData() {
        ifstream file("barter_data.txt");
        if (!file) {
            addItem(25101281462LL, "Arduino Uno", "Working", "Breadboard", false);
            addItem(25101281063LL, "Raspberry Pi", "Good", "ESP32", false);
            addItem(25103704010LL, "Multimeter", "Used", "Soldering Iron", false);
            saveData();
            return;
        }

        int id, otp;
        long long ownerId, buyerId;
        string name, cond, want, status, offer;

        while (file >> id) {
            file >> ownerId;
            file.ignore();
            getline(file, name);
            getline(file, cond);
            getline(file, want);
            getline(file, status);
            file >> buyerId;
            file.ignore();
            getline(file, offer);
            file >> otp;

            ElectronicNode* node = new ElectronicNode(id, ownerId, name, cond, want);
            node->status = status;
            node->buyerId = buyerId;
            node->offerItem = offer;
            node->otpCode = otp;

            if (head == NULL) {
                head = tail = node;
            } else {
                tail->next = node;
                node->prev = tail;
                tail = node;
            }

            itemMap[id] = node;
            if (id >= nextId) {
                nextId = id + 1;
            }
        }
        file.close();
    }

    void addItem(long long sId, string name, string cond, string want, bool showMsg = true) {
        ElectronicNode* newNode = new ElectronicNode(nextId, sId, name, cond, want);

        if (head == NULL) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }

        itemMap[nextId] = newNode;
        if (showMsg) {
            cout << "\nItem Added! Your Item ID is: " << nextId << endl;
            saveData();
        }
        nextId++;
    }

    void showCatalogForward() {
        cout << "\n--- ALL ITEMS (OLDEST FIRST) ---\n";
        if (head == NULL) {
            cout << "No items in list.\n";
            return;
        }
        ElectronicNode* temp = head;
        while (temp != NULL) {
            temp->show();
            temp = temp->next;
        }
    }

    void showCatalogReverse() {
        cout << "\n--- ALL ITEMS (NEWEST FIRST) ---\n";
        if (tail == NULL) {
            cout << "No items in list.\n";
            return;
        }
        ElectronicNode* temp = tail;
        while (temp != NULL) {
            temp->show();
            temp = temp->prev;
        }
    }

    void showOnlyAvailable() {
        cout << "\n--- AVAILABLE ITEMS ---\n";
        ElectronicNode* temp = head;
        int found = 0;
        while (temp != NULL) {
            if (temp->status == "AVAILABLE") {
                temp->show();
                found++;
            }
            temp = temp->next;
        }
        if (found == 0) {
            cout << "No available items right now.\n";
        }
    }

    void searchItems(string keyword) {
        cout << "\n--- SEARCH RESULTS ---\n";
        string key = makeSmall(keyword);
        ElectronicNode* temp = head;
        int found = 0;

        while (temp != NULL) {
            string n1 = makeSmall(temp->name);
            string n2 = makeSmall(temp->wantItem);
            if (n1.find(key) != string::npos || n2.find(key) != string::npos) {
                temp->show();
                found++;
            }
            temp = temp->next;
        }
        if (found == 0) {
            cout << "No matching item found.\n";
        }
    }

    void requestSwap(int itemId, long long buyerId, string offer) {
        if (itemMap.find(itemId) == itemMap.end()) {
            cout << "\nItem ID not found!\n";
            return;
        }
        ElectronicNode* item = itemMap[itemId];

        if (item->ownerId == buyerId) {
            cout << "\nYou cannot buy your own item!\n";
            return;
        }
        if (item->status != "AVAILABLE") {
            cout << "\nItem already has a request!\n";
            return;
        }

        item->status = "REQUESTED";
        item->buyerId = buyerId;
        item->offerItem = offer;
        saveData();
        cout << "\nSwap Request Sent!\n";
    }

    // Buyer can cancel their pending swap offer
    void cancelSwapRequest(int itemId, long long buyerId) {
        if (itemMap.find(itemId) == itemMap.end()) {
            cout << "\nItem ID not found!\n";
            return;
        }
        ElectronicNode* item = itemMap[itemId];

        if (item->status != "REQUESTED" || item->buyerId != buyerId) {
            cout << "\nNo active offer from your ID on this item!\n";
            return;
        }

        item->status = "AVAILABLE";
        item->buyerId = 0;
        item->offerItem = "None";
        saveData();
        cout << "\nYour swap offer has been cancelled.\n";
    }

    void acceptSwap(int itemId, long long ownerId, string ownerEmail, string buyerEmail) {
        if (itemMap.find(itemId) == itemMap.end()) {
            cout << "\nItem ID not found!\n";
            return;
        }
        ElectronicNode* item = itemMap[itemId];

        if (item->ownerId != ownerId) {
            cout << "\nWrong Owner ID!\n";
            return;
        }
        if (item->status != "REQUESTED") {
            cout << "\nNo pending request for this item!\n";
            return;
        }

        int otp = 1000 + (rand() % 9000);
        item->status = "HANDSHAKE";
        item->otpCode = otp;
        saveData();

        if (ownerEmail != "") {
            string cmd1 = "python send_otp.py \"" + ownerEmail + "\" " + to_string(otp) + " \"Trade Handshake\"";
            system(cmd1.c_str());
        }
        if (buyerEmail != "") {
            string cmd2 = "python send_otp.py \"" + buyerEmail + "\" " + to_string(otp) + " \"Trade Handshake\"";
            system(cmd2.c_str());
        }

        cout << "\nOffer Accepted! 4-Digit Handshake OTP emailed to both students (Demo Code: " << otp << ")\n";
    }

    void rejectSwap(int itemId, long long ownerId) {
        if (itemMap.find(itemId) == itemMap.end()) {
            cout << "\nItem ID not found!\n";
            return;
        }
        ElectronicNode* item = itemMap[itemId];

        if (item->ownerId != ownerId) {
            cout << "\nWrong Owner ID!\n";
            return;
        }

        item->status = "AVAILABLE";
        item->buyerId = 0;
        item->offerItem = "None";
        saveData();
        cout << "\nOffer Rejected! Item is AVAILABLE again.\n";
    }

    void withdrawItem(int itemId, long long ownerId) {
        if (itemMap.find(itemId) == itemMap.end()) {
            cout << "\nItem ID not found!\n";
            return;
        }
        ElectronicNode* item = itemMap[itemId];

        if (item->ownerId != ownerId) {
            cout << "\nOnly owner can delete this item!\n";
            return;
        }
        if (item->status != "AVAILABLE") {
            cout << "\nCannot delete while swap is active!\n";
            return;
        }

        deleteNode(item);
        saveData();
        cout << "\nItem Deleted!\n";
    }

    long long getBuyerId(int itemId) {
        if (itemMap.find(itemId) != itemMap.end()) {
            return itemMap[itemId]->buyerId;
        }
        return 0;
    }

    bool verifyHandshakeAndClose(int itemId, int userOtp) {
        if (itemMap.find(itemId) == itemMap.end()) {
            cout << "\nItem ID not found!\n";
            return false;
        }
        ElectronicNode* item = itemMap[itemId];

        if (item->status != "HANDSHAKE") {
            cout << "\nItem is not ready for OTP verification!\n";
            return false;
        }
        if (item->otpCode != userOtp) {
            cout << "\nWrong OTP! Trade Failed.\n";
            return false;
        }

        cout << "\nTRADE SUCCESSFUL! " << item->name << " swapped with " << item->offerItem << endl;
        saveHistory(item);
        deleteNode(item);
        saveData();
        return true;
    }

    void showTradeHistory() {
        cout << "\n--- COMPLETED TRADES ---\n";
        ifstream file("trade_history.txt");
        if (!file) {
            cout << "No completed trades yet.\n";
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