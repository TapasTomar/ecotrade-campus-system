#ifndef BARTER_H
#define BARTER_H

#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

// Base class (OOP: Inheritance & Polymorphism)
class Item {
protected:
    int id;
    string name;
    long long ownerId;

public:
    Item(int itemId, long long sId, string itemName) 
        : id(itemId), ownerId(sId), name(itemName) {}
    
    virtual ~Item() {}

    int getId() const { return id; }
    string getName() const { return name; }
    long long getOwnerId() const { return ownerId; }

    virtual void display() const = 0;
};

// DSA Node: Doubly Linked List Node for Active Barter Catalog
class ElectronicNode : public Item {
public:
    string condition;
    string desiredItem;
    string status; // AVAILABLE, SWAP_REQUESTED, TRADED
    long long requesterId;
    string offeredItem;

    // DLL Pointers
    ElectronicNode* next;
    ElectronicNode* prev;

    ElectronicNode(int itemId, long long sId, string itemName, string cond, string want)
        : Item(itemId, sId, itemName), condition(cond), desiredItem(want), status("AVAILABLE"),
          requesterId(0), offeredItem("None"), next(nullptr), prev(nullptr) {}

    void display() const override {
        cout << "Item ID: " << id << " | Name: " << name 
             << " | Owner ID: " << ownerId 
             << " | Condition: " << condition 
             << " | Wants: " << desiredItem 
             << " | Status: " << status << "\n";
        if (status == "SWAP_REQUESTED") {
            cout << "   └── Pending Offer from Student " << requesterId 
                 << ": \"" << offeredItem << "\"\n";
        }
    }
};

// Doubly Linked List Engine with Hash Map Indexing
class BarterEngine {
private:
    ElectronicNode* head;
    ElectronicNode* tail;
    int nextId;
    unordered_map<int, ElectronicNode*> idIndex;

public:
    BarterEngine() : head(nullptr), tail(nullptr), nextId(101) {}

    ~BarterEngine() {
        ElectronicNode* curr = head;
        while (curr != nullptr) {
            ElectronicNode* temp = curr->next;
            delete curr;
            curr = temp;
        }
        head = tail = nullptr;
        idIndex.clear();
    }

    // Insert at Tail in Doubly Linked List (O(1))
    void addItem(long long sId, string name, string cond, string want) {
        ElectronicNode* newNode = new ElectronicNode(nextId, sId, name, cond, want);
        if (head == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        idIndex[nextId] = newNode;
        cout << "\nItem added successfully with ID: " << nextId << "\n";
        nextId++;
    }

    // Seed sample data for viva demo
    void loadSampleData() {
        addItem(25101281462, "Arduino Uno Rev3", "Working", "Breadboard");
        addItem(25101281063, "Raspberry Pi 3B", "Good", "ESP32 Board");
        addItem(2510370401, "Digital Multimeter", "Used", "Soldering Iron");
    }

    // Forward Traversal (Oldest First)
    void showCatalogForward() const {
        cout << "\n--- Electronic Items for Barter (Oldest First) ---\n";
        if (head == nullptr) {
            cout << "No items available right now.\n";
            return;
        }
        ElectronicNode* curr = head;
        while (curr != nullptr) {
            curr->display();
            curr = curr->next;
        }
    }

    // Backward Traversal (Newest First via prev pointer)
    void showCatalogReverse() const {
        cout << "\n--- Recent Items First ---\n";
        if (tail == nullptr) {
            cout << "No items available right now.\n";
            return;
        }
        ElectronicNode* curr = tail;
        while (curr != nullptr) {
            curr->display();
            curr = curr->prev;
        }
    }

    // Swap Request Logic via O(1) Map Lookup
    bool requestSwap(int itemId, long long reqId, string offer) {
        if (idIndex.find(itemId) == idIndex.end()) {
            cout << "\nItem ID not found.\n";
            return false;
        }

        ElectronicNode* item = idIndex[itemId];
        if (item->getOwnerId() == reqId) {
            cout << "\nYou cannot request swap on your own item.\n";
            return false;
        }
        if (item->status != "AVAILABLE") {
            cout << "\nItem is already in exchange process.\n";
            return false;
        }

        item->status = "SWAP_REQUESTED";
        item->requesterId = reqId;
        item->offeredItem = offer;
        cout << "\nSwap offer sent successfully to Owner ID " << item->getOwnerId() << "!\n";
        return true;
    }

    bool acceptSwap(int itemId, long long ownerId) {
        if (idIndex.find(itemId) == idIndex.end()) {
            cout << "\nItem ID not found.\n";
            return false;
        }

        ElectronicNode* item = idIndex[itemId];
        if (item->getOwnerId() != ownerId) {
            cout << "\nIncorrect Owner ID.\n";
            return false;
        }
        if (item->status != "SWAP_REQUESTED") {
            cout << "\nNo pending swap offer on this item.\n";
            return false;
        }

        item->status = "TRADED";
        cout << "\nTrade confirmed! Exchanged for \"" << item->offeredItem << "\".\n";
        return true;
    }
};

#endif