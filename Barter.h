#ifndef BARTER_H
#define BARTER_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

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

    string checkOffer(string ownerId, string bidderId, string status) {
        if (status != "OPEN") {
            return "ERROR|This item is already closed";
        }
        if (ownerId == bidderId) {
            return "ERROR|You cannot send an offer on your own item";
        }
        return "ALLOWED|Offer allowed by C++";
    }

    string acceptOffer(string ownerId, string bidderId) {
        if (ownerId == bidderId) {
            return "ERROR|Self-barter is not allowed";
        }
        srand(time(0));
        int handshakeCode = 1000 + (rand() % 9000);
        return "ACCEPTED|" + to_string(handshakeCode);
    }

    string canViewMobile(string viewerId, string ownerId, string acceptedBidderId, string status) {
        if (status == "ACCEPTED" && (viewerId == ownerId || viewerId == acceptedBidderId)) {
            return "SHOW";
        }
        return "HIDE";
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