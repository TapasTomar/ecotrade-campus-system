#include <iostream>
#include <string>
#include "student.h"
#include "Barter.h"
#include "blooddonate.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "ERROR|No command given" << endl;
        return 1;
    }

    string command = argv[1];
    Student st;
    Barter bt;
    BloodDonate bd;

    if (command == "SIGNUP" && argc == 9) {
        cout << st.checkSignup(argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8]) << endl;
    }
    else if (command == "LOGIN" && argc == 4) {
        cout << st.checkLogin(argv[2], argv[3]) << endl;
    }
    else if (command == "OFFER" && argc == 5) {
        cout << bt.checkOffer(argv[2], argv[3], argv[4]) << endl;
    }
    else if (command == "ACCEPT" && argc == 4) {
        cout << bt.acceptOffer(argv[2], argv[3]) << endl;
    }
    else if (command == "PRIVACY" && argc == 6) {
        cout << bt.canViewMobile(argv[2], argv[3], argv[4], argv[5]) << endl;
    }
    else if (command == "BLOOD" && argc == 6) {
        cout << bd.dispatch(argv[2], stoi(argv[3]), stoi(argv[4]), stoi(argv[5])) << endl;
    }
    else if (command == "STATUS") {
        cout << "ONLINE|C++ Backend Active (student.h, Barter.h, blooddonate.h)" << endl;
    }
    else {
        cout << "ERROR|Invalid C++ command" << endl;
        return 1;
    }

    return 0;
}