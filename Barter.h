#ifndef BARTER_H
#define BARTER_H

#include <iostream>
#include <string>
#include <random>

using namespace std;

struct ItemNode {
    int id;
    string ownerRoll;
    string title;
    string status;
    ItemNode* next;

    ItemNode(int itemId, string owner, string itemTitle, string itemStatus) {
        id = itemId;
        ownerRoll = owner;
        title = itemTitle;
        status = itemStatus;
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

    void addItem(int id, string owner, string title, string status) {
        ItemNode* node = new ItemNode(id, owner, title, status);
        if (head == NULL) {
            head = node;
            return;
        }
        ItemNode* temp = head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = node;
    }

    string checkOffer(string ownerRoll, string buyerRoll, string status) {
        if (status != "OPEN") {
            return "ERROR|Item is already closed";
        }
        if (ownerRoll == buyerRoll) {
            return "ERROR|Cannot send offer on your own item";
        }
        return "ALLOWED|Offer allowed by C++";
    }

    string acceptOffer(string ownerRoll, string buyerRoll) {
        if (ownerRoll == buyerRoll) {
            return "ERROR|Self-barter not allowed";
        }
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(1000, 9999);
        int code = dist(gen);
        return "ACCEPTED|" + to_string(code);
    }

    string canViewMobile(string userRoll, string ownerRoll, string buyerRoll, string status) {
        if ((status == "ACCEPTED" || status == "COMPLETED") &&
            (userRoll == ownerRoll || userRoll == buyerRoll)) {
            return "SHOW";
        }
        return "HIDE";
    }

    string verifyHandshake(string typedCode, string realCode) {
        if (realCode == "" || typedCode != realCode) {
            return "ERROR|Wrong 4-digit meetup code";
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