#ifndef STUDENT_H
#define STUDENT_H

#include <iostream>
#include <string>

using namespace std;

class Student {
private:
    bool isDigits(string text, int exactLen) {
        if ((int)text.length() != exactLen) {
            return false;
        }
        for (int i = 0; i < exactLen; i++) {
            if (text[i] < '0' || text[i] > '9') {
                return false;
            }
        }
        return true;
    }

public:
    Student() {}

    string checkSignup(string roll, string name, string email, string phone, string pass, string typedOtp, string savedOtp) {
        if (!isDigits(roll, 11)) {
            return "ERROR|Student ID must be 11 digits";
        }
        if (name.length() < 2) {
            return "ERROR|Enter a valid name";
        }
        if (email.find('@') == string::npos || email.find('.') == string::npos) {
            return "ERROR|Invalid email address";
        }
        if (!isDigits(phone, 10)) {
            return "ERROR|Mobile number must be 10 digits";
        }
        if (pass.length() < 4) {
            return "ERROR|Password must be at least 4 characters";
        }
        if (savedOtp == "" || typedOtp != savedOtp) {
            return "ERROR|Wrong or expired OTP";
        }
        return "VALID|Signup verified by C++";
    }

    string checkLogin(string roll, string pass, string typedOtp, string savedOtp) {
        if (!isDigits(roll, 11)) {
            return "ERROR|Student ID must be 11 digits";
        }
        if (pass.length() < 4) {
            return "ERROR|Password must be at least 4 characters";
        }
        if (typedOtp != "NONE" && (savedOtp == "" || typedOtp != savedOtp)) {
            return "ERROR|Wrong or expired Login OTP";
        }
        return "VALID|Login verified by C++";
    }
};

#endif