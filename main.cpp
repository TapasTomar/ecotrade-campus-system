#include <iostream>
#include <string>
#include "student.h"
#include "Barter.h"
#include "blooddonate.h"

using namespace std;

int main(int argc, char* argv[]) {
    // Check if command is passed
    if (argc < 2) {
        cout << "ERROR|No work given" << endl;
        return 1;
    }

    string work = argv[1];
    Student s;
    Barter b;
    BloodDonate bd;

    // 1. New Account Check
    if (work == "SIGNUP" && argc == 9) {
        string roll = argv[2];
        string name = argv[3];
        string email = argv[4];
        string phone = argv[5];
        string pass = argv[6];
        string typedOtp = argv[7];
        string savedOtp = argv[8];

        cout << s.checkSignup(roll, name, email, phone, pass, typedOtp, savedOtp) << endl;
    }
    // 2. Login Check
    else if (work == "LOGIN" && argc == 6) {
        string roll = argv[2];
        string pass = argv[3];
        string typedOtp = argv[4];
        string savedOtp = argv[5];

        cout << s.checkLogin(roll, pass, typedOtp, savedOtp) << endl;
    }
    // 3. Send Offer Check
    else if (work == "OFFER" && argc == 5) {
        string ownerRoll = argv[2];
        string buyerRoll = argv[3];
        string itemStatus = argv[4];

        cout << b.checkOffer(ownerRoll, buyerRoll, itemStatus) << endl;
    }
    // 4. Accept Offer & Make 4-Digit Code
    else if (work == "ACCEPT" && argc == 4) {
        string ownerRoll = argv[2];
        string buyerRoll = argv[3];

        cout << b.acceptOffer(ownerRoll, buyerRoll) << endl;
    }
    // 5. Hide or Show Phone Number
    else if (work == "PRIVACY" && argc == 6) {
        string userRoll = argv[2];
        string ownerRoll = argv[3];
        string buyerRoll = argv[4];
        string itemStatus = argv[5];

        cout << b.canViewMobile(userRoll, ownerRoll, buyerRoll, itemStatus) << endl;
    }
    // 6. Check 4-Digit Meetup Code
    else if (work == "HANDSHAKE" && argc == 4) {
        string typedCode = argv[2];
        string realCode = argv[3];

        cout << b.verifyHandshake(typedCode, realCode) << endl;
    }
    // 7. Blood Bank Request
    else if (work == "BLOOD" && argc == 6) {
        string bloodGroup = argv[2];
        int totalBags = stoi(argv[3]);
        int needBags = stoi(argv[4]);
        int urgentLevel = stoi(argv[5]);

        cout << bd.dispatch(bloodGroup, totalBags, needBags, urgentLevel) << endl;
    }
    // 8. Check if C++ is Working
    else if (work == "STATUS") {
        cout << "ONLINE|C++ Backend Active (student.h, Barter.h, blooddonate.h)" << endl;
    }
    else {
        cout << "ERROR|Wrong command" << endl;
        return 1;
    }

    return 0;
}