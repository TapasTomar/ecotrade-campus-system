#include <iostream>
#include <string>
#include "student.h"
#include "Barter.h"
#include "blooddonate.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "ERROR|No work given" << endl;
        return 1;
    }

    string work = argv[1];
    Student s;
    Barter b;
    BloodDonate bd;

    if (work == "SIGNUP" && argc == 9) {
        cout << s.checkSignup(argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8]) << endl;
    }
    else if (work == "LOGIN" && argc == 6) {
        cout << s.checkLogin(argv[2], argv[3], argv[4], argv[5]) << endl;
    }
    else if (work == "OFFER" && argc == 5) {
        cout << b.checkOffer(argv[2], argv[3], argv[4]) << endl;
    }
    else if (work == "ACCEPT" && argc == 4) {
        cout << b.acceptOffer(argv[2], argv[3]) << endl;
    }
    else if (work == "PRIVACY" && argc == 6) {
        cout << b.canViewMobile(argv[2], argv[3], argv[4], argv[5]) << endl;
    }
    else if (work == "HANDSHAKE" && argc == 4) {
        cout << b.verifyHandshake(argv[2], argv[3]) << endl;
    }
    else if (work == "BLOOD" && argc == 6) {
        cout << bd.dispatch(argv[2], stoi(argv[3]), stoi(argv[4]), stoi(argv[5])) << endl;
    }
    else if (work == "STATUS") {
        cout << "ONLINE|C++ Backend Active (student.h, Barter.h, blooddonate.h)" << endl;
    }
    else {
        cout << "ERROR|Wrong command" << endl;
        return 1;
    }

    return 0;
}