#ifndef STUDENT_H
#define STUDENT_H

#include <iostream>
#include <string>

using namespace std;

class Student {
private:
    string studentId;
    string name;
    string email;
    string mobile;

    bool checkNumber(string str, int length) {
        if ((int)str.length() != length) {
            return false;
        }
        for (int i = 0; i < length; i++) {
            if (str[i] < '0' || str[i] > '9') {
                return false;
            }
        }
        return true;
    }

public:
    Student() {
        studentId = "";
        name = "";
        email = "";
        mobile = "";
    }

    Student(string id, string n, string e, string m) {
        studentId = id;
        name = n;
        email = e;
        mobile = m;
    }

    string checkSignup(string id, string n, string e, string m, string pass, string userOtp, string realOtp) {
        if (!checkNumber(id, 11)) {
            return "ERROR|Student ID must be exactly 11 digits";
        }
        if (n.length() < 2) {
            return "ERROR|Please enter a valid name";
        }
        if (e.find('@') == string::npos || e.find('.') == string::npos) {
            return "ERROR|Invalid email format";
        }
        if (!checkNumber(m, 10)) {
            return "ERROR|Mobile number must be exactly 10 digits";
        }
        if (pass.length() < 4) {
            return "ERROR|Password must be at least 4 characters";
        }
        if (userOtp != realOtp || realOtp == "") {
            return "ERROR|Wrong OTP entered";
        }

        return "VALID|Student verified by C++";
    }

    string checkLogin(string id, string pass) {
        if (!checkNumber(id, 11)) {
            return "ERROR|Student ID must be 11 digits";
        }
        if (pass.length() < 4) {
            return "ERROR|Password must be at least 4 characters";
        }
        return "VALID|Login verified by C++";
    }
};

#endif