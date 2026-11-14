#ifndef BARTER_H
#define BARTER_H

#include <iostream>
#include <string>
#include <random>

using namespace std;

struct ItemNode {
    int itemId;
    string ownerId;
    string title;
    string status;
    ItemNode* next;

    ItemNode(int id, string oId, string t, string st) {
        itemId = id;
        ownerId = oId;
        title = t;
        status = st;
        next = NULL;
    }
};

class Barter {
private:
    ItemNode* head;

public:
    Barter() {
        head = NULL;
    }

    void addItem(int id, string ownerId, string title, string status) {
        ItemNode* newNode = new ItemNode(id, ownerId, title, status);
        if (head == NULL) {
            head = newNode;
            return;
        }
        ItemNode* temp = head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }

    // Rule 1: Validate Offer
    string checkOffer(string ownerId, string bidderId, string status) {
        if (status != "OPEN") {
            return "ERROR|This item is already closed";
        }
        if (ownerId == bidderId) {
            return "ERROR|You cannot send an offer on your own item";
        }
        return "ALLOWED|Offer allowed by C++";
    }

    // Rule 2: Accept Offer -> Generate Unique 4-Digit Handshake OTP & Unlock Numbers
    string acceptOffer(string ownerId, string bidderId) {
        if (ownerId == bidderId) {
            return "ERROR|Self-barter is not allowed";
        }
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(1000, 9999);
        int handshakeCode = dist(gen);
        return "ACCEPTED|" + to_string(handshakeCode);
    }

    // Rule 3: Privacy Guard — Reveal Mobile Numbers Only After Acceptance
    string canViewMobile(string viewerId, string ownerId, string acceptedBidderId, string status) {
        if ((status == "ACCEPTED" || status == "COMPLETED") &&
            (viewerId == ownerId || viewerId == acceptedBidderId)) {
            return "SHOW";
        }
        return "HIDE";
    }

    // Rule 4: Verify Physical Handshake OTP
    string verifyHandshake(string enteredCode, string actualCode) {
        if (actualCode == "" || enteredCode != actualCode) {
            return "ERROR|Wrong Handshake OTP";
        }
        return "COMPLETED|Barter completed via C++";
    }

    ~Barter() {
        ItemNode* curr = head;
        while (curr != NULL) {
            ItemNode* nextNode = curr->next;
            delete curr;
            curr = nextNode;
        }
    }
};

#endif