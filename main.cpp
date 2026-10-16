#include <iostream>
#include <string>
#include "student.h"
#include "blooddonate.h"
#include "Barter.h"

using namespace std;

int main() {
    StudentManager studentMgr;
    BarterEngine barterMgr;
    BloodDonationEngine bloodMgr;

    // Load initial sample data
    barterMgr.loadSampleData();
    bloodMgr.loadSampleData();

    int choice = 0;

    while (choice != 4) {
        cout << "\n============================================\n";
        cout << "    CAMPUS ECO-TRADE & DONATION SYSTEM      \n";
        cout << "============================================\n";
        cout << "1. Student Portal\n";
        cout << "2. Electronic Barter Marketplace\n";
        cout << "3. Campus Blood Bank\n";
        cout << "4. Exit\n";
        cout << "Enter choice (1-4): ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        if (choice == 1) {
            studentMgr.runDashboard();
        } 
        else if (choice == 2) {
            int bChoice = 0;
            while (bChoice != 6) {
                cout << "\n--- ELECTRONIC BARTER MENU ---\n";
                cout << "1. View All Listings (Oldest First)\n";
                cout << "2. View Recent Listings (Newest First)\n";
                cout << "3. Post Electronic Item\n";
                cout << "4. Request Item Swap\n";
                cout << "5. Accept Swap Offer\n";
                cout << "6. Back to Main Menu\n";
                cout << "Enter option (1-6): ";

                if (!(cin >> bChoice)) {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    continue;
                }

                if (bChoice == 1) {
                    barterMgr.showCatalogForward();
                } 
                else if (bChoice == 2) {
                    barterMgr.showCatalogReverse();
                } 
                else if (bChoice == 3) {
                    long long sId;
                    string name, cond, want;
                    cout << "Enter Student ID: "; cin >> sId;
                    cin.ignore(1000, '\n');
                    cout << "Enter Item Name: "; getline(cin, name);
                    cout << "Enter Condition (Working/Faulty): "; getline(cin, cond);
                    cout << "Item Wanted in Exchange: "; getline(cin, want);

                    barterMgr.addItem(sId, name, cond, want);
                } 
                else if (bChoice == 4) {
                    int itemId;
                    long long reqId;
                    string offer;
                    cout << "Enter Item ID to swap: "; cin >> itemId;
                    cout << "Enter Your Student ID: "; cin >> reqId;
                    cin.ignore(1000, '\n');
                    cout << "What item are you offering in return?: "; getline(cin, offer);
                    barterMgr.requestSwap(itemId, reqId, offer);
                } 
                else if (bChoice == 5) {
                    int itemId;
                    long long ownerId;
                    cout << "Enter Item ID: "; cin >> itemId;
                    cout << "Enter Your Student ID: "; cin >> ownerId;
                    barterMgr.acceptSwap(itemId, ownerId);
                }
            }
        } 
        else if (choice == 3) {
            int bloodChoice = 0;
            while (bloodChoice != 3) {
                cout << "\n--- BLOOD BANK & DONATION MENU ---\n";
                cout << "1. Search and Dispatch Blood (Request by Urgency)\n";
                cout << "2. Register Donated Blood Unit\n";
                cout << "3. Back to Main Menu\n";
                cout << "Enter option (1-3): ";

                if (!(cin >> bloodChoice)) {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    continue;
                }

                if (bloodChoice == 1) {
                    cin.ignore(1000, '\n');
                    string grp, hosp;
                    int urgency = 1;

                    cout << "Enter Hospital/Center Requesting Blood: "; 
                    getline(cin, hosp);

                    cout << "Enter Required Blood Group (e.g. O+, B+): "; 
                    getline(cin, grp);

                    cout << "Select Patient Urgency Level [1: Routine, 2: High/OT, 3: ICU/Emergency]: "; 
                    if (!(cin >> urgency)) {
                        cin.clear();
                        cin.ignore(1000, '\n');
                        urgency = 1;
                    }

                    bloodMgr.processDispatchTicket(hosp, grp, urgency);
                } 
                else if (bloodChoice == 2) {
                    long long donorId = 0;
                    string grp, center;

                    cout << "Enter Donor Student ID (11 Digits): "; 
                    if (!(cin >> donorId)) {
                        cin.clear();
                        cin.ignore(1000, '\n');
                        continue;
                    }

                    cout << "Enter Blood Group: "; cin >> grp;
                    cout << "Enter Donation Center / Camp Name: "; cin >> center;

                    cin.ignore(1000, '\n');
                    bloodMgr.storeBlood(donorId, grp, center);
                }
            }
        } 
        else if (choice == 4) {
            cout << "\nClosing application. Goodbye.\n";
        }
    }

    return 0;
}