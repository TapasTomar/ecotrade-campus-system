#include <iostream>
#include <string>
#include "student.h"
#include "Barter.h"
#include "blooddonate.h"

using namespace std;

int main() {
    StudentManager studentMgr;
    BarterEngine barterMgr;
    BloodBankEngine bloodMgr;

    while (true) {
        if (!studentMgr.isLoggedIn()) {
            cout << "\n==================================================\n";
            cout << "   ECOTRADE: CAMPUS BARTER & BLOOD BANK PORTAL    \n";
            cout << "==================================================\n";
            cout << "1. Student Login (with Real Gmail OTP)\n";
            cout << "2. New Student Registration (with Real Gmail OTP)\n";
            cout << "3. View Registered Campus Students\n";
            cout << "4. Exit\n";
            cout << "Enter Choice: ";
            int choice = readSafeInt();

            if (choice == 1) {
                cout << "Enter 11-Digit Student ID: ";
                long long id = readSafeLong();
                studentMgr.loginWithOTP(id);
            } 
            else if (choice == 2) {
                cout << "Enter New 11-Digit Student ID: ";
                long long id = readSafeLong();
                studentMgr.registerStudent(id);
            } 
            else if (choice == 3) {
                studentMgr.showAllStudents();
            } 
            else if (choice == 4) {
                cout << "\nGoodbye!\n";
                break;
            } 
            else {
                cout << "Invalid choice!\n";
            }
            continue;
        }

        long long activeId = studentMgr.getCurrentUser();

        cout << "\n==================================================\n";
        cout << "   ECOTRADE DASHBOARD  |  Active ID: " << activeId << endl;
        cout << "==================================================\n";
        cout << "1. Electronic E-Waste Barter Marketplace\n";
        cout << "2. Campus Emergency Blood Bank\n";
        cout << "3. My Profile & Eco-Points\n";
        cout << "4. Update Email (OTP Verified)\n";
        cout << "5. Logout\n";
        cout << "6. Save & Exit\n";
        cout << "Enter Choice: ";
        int choice = readSafeInt();

        if (choice == 1) {
            while (true) {
                cout << "\n--- E-WASTE BARTER MARKETPLACE ---\n";
                cout << "1. View All Items (Oldest First)\n";
                cout << "2. View All Items (Newest First)\n";
                cout << "3. View Only Available Items\n";
                cout << "4. Search Item by Name\n";
                cout << "5. List My Electronic Item\n";
                cout << "6. Send Swap Offer\n";
                cout << "7. Cancel My Pending Offer\n";
                cout << "8. Accept Offer (Email 4-Digit Handshake OTP)\n";
                cout << "9. Reject Offer\n";
                cout << "10. Verify 4-Digit Handshake OTP & Complete Trade\n";
                cout << "11. Delete My Own Listing\n";
                cout << "12. View Completed Trade History\n";
                cout << "13. Back to Dashboard\n";
                cout << "Choice: ";
                int bChoice = readSafeInt();

                if (bChoice == 1) {
                    barterMgr.showCatalogForward();
                } 
                else if (bChoice == 2) {
                    barterMgr.showCatalogReverse();
                } 
                else if (bChoice == 3) {
                    barterMgr.showOnlyAvailable();
                } 
                else if (bChoice == 4) {
                    string kw;
                    cin.ignore();
                    cout << "Enter keyword to search: ";
                    getline(cin, kw);
                    barterMgr.searchItems(kw);
                } 
                else if (bChoice == 5) {
                    string name, cond, want;
                    cin.ignore();
                    cout << "Enter Item Name: ";
                    getline(cin, name);
                    cout << "Enter Condition (Working/Good/Used): ";
                    getline(cin, cond);
                    cout << "Enter Item You Want in Exchange: ";
                    getline(cin, want);
                    barterMgr.addItem(activeId, name, cond, want);
                } 
                else if (bChoice == 6) {
                    cout << "Enter Target Item ID: ";
                    int itemId = readSafeInt();
                    string offer;
                    cin.ignore();
                    cout << "Enter Item You Are Offering: ";
                    getline(cin, offer);
                    barterMgr.requestSwap(itemId, activeId, offer);
                } 
                else if (bChoice == 7) {
                    cout << "Enter Item ID to Cancel Offer: ";
                    int itemId = readSafeInt();
                    barterMgr.cancelSwapRequest(itemId, activeId);
                }
                else if (bChoice == 8) {
                    cout << "Enter Item ID to Accept Offer: ";
                    int itemId = readSafeInt();
                    long long buyerId = barterMgr.getBuyerId(itemId);
                    barterMgr.acceptSwap(itemId, activeId, studentMgr.getEmail(activeId), studentMgr.getEmail(buyerId));
                } 
                else if (bChoice == 9) {
                    cout << "Enter Item ID to Reject Offer: ";
                    int itemId = readSafeInt();
                    barterMgr.rejectSwap(itemId, activeId);
                } 
                else if (bChoice == 10) {
                    cout << "Enter Item ID: ";
                    int itemId = readSafeInt();
                    cout << "Enter 4-Digit Handshake OTP: ";
                    int otp = readSafeInt();
                    long long buyerId = barterMgr.getBuyerId(itemId);
                    if (barterMgr.verifyHandshakeAndClose(itemId, otp)) {
                        studentMgr.addEcoPoints(activeId, 10);
                        studentMgr.addEcoPoints(buyerId, 10);
                        cout << "[REWARD] +10 Eco-Points credited to both traders!\n";
                    }
                } 
                else if (bChoice == 11) {
                    cout << "Enter Your Item ID to Delete: ";
                    int itemId = readSafeInt();
                    barterMgr.withdrawItem(itemId, activeId);
                } 
                else if (bChoice == 12) {
                    barterMgr.showTradeHistory();
                } 
                else if (bChoice == 13) {
                    break;
                } 
                else {
                    cout << "Invalid choice!\n";
                }
            }
        }
        else if (choice == 2) {
            while (true) {
                cout << "\n--- CAMPUS EMERGENCY BLOOD BANK ---\n";
                cout << "1. View Available Blood Packets\n";
                cout << "2. View Stock Summary & Low Stock Alerts\n";
                cout << "3. Donate Blood Packet (90-Day Cooldown Check)\n";
                cout << "4. Submit Hospital Emergency Request (Max-Heap)\n";
                cout << "5. Dispatch Highest Priority Blood Request\n";
                cout << "6. View Dispatch Audit History\n";
                cout << "7. Back to Dashboard\n";
                cout << "Choice: ";
                int blChoice = readSafeInt();

                if (blChoice == 1) {
                    bloodMgr.showInventory();
                } 
                else if (blChoice == 2) {
                    bloodMgr.showStockSummary();
                } 
                else if (blChoice == 3) {
                    string grp, center;
                    cout << "Enter Your Blood Group (e.g., O+, B+, O-): ";
                    cin >> grp;
                    cin.ignore();
                    cout << "Enter Storage Center Name: ";
                    getline(cin, center);
                    if (bloodMgr.donateBlood(activeId, grp, center)) {
                        studentMgr.addEcoPoints(activeId, 25);
                        cout << "[REWARD] +25 Eco-Points added to your profile!\n";
                    }
                } 
                else if (blChoice == 4) {
                    string hosp, grp;
                    cin.ignore();
                    cout << "Enter Hospital Name: ";
                    getline(cin, hosp);
                    cout << "Enter Required Blood Group: ";
                    cin >> grp;
                    cout << "Enter Urgency (1=Routine, 2=High/OT, 3=Critical/ICU): ";
                    int urg = readSafeInt();
                    bloodMgr.addEmergencyRequest(hosp, grp, urg);
                } 
                else if (blChoice == 5) {
                    bloodMgr.processNextDispatch();
                } 
                else if (blChoice == 6) {
                    bloodMgr.showDispatchHistory();
                } 
                else if (blChoice == 7) {
                    break;
                } 
                else {
                    cout << "Invalid choice!\n";
                }
            }
        }
        else if (choice == 3) {
            studentMgr.showMyProfile();
        }
        else if (choice == 4) {
            studentMgr.updateEmailWithOTP();
        }
        else if (choice == 5) {
            studentMgr.logout();
        }
        else if (choice == 6) {
            studentMgr.saveData();
            barterMgr.saveData();
            bloodMgr.saveData();
            cout << "\nAll data saved! Goodbye.\n";
            break;
        }
        else {
            cout << "Invalid choice!\n";
        }
    }

    return 0;
}