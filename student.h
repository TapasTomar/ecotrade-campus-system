#ifndef STUDENT_H
#define STUDENT_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <cstdlib>
#include <ctime>

using namespace std;

int readSafeInt() {
    int x;
    while (!(cin >> x)) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input! Enter numbers only: ";
    }
    return x;
}

long long readSafeLong() {
    long long x;
    while (!(cin >> x)) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid ID! Enter digits only: ";
    }
    return x;
}

class Student {
public:
    long long studentId;
    string name;
    string rollNo;
    string course;
    int year;
    string email;
    string phone;
    string bloodGroup;
    int ecoPoints;

    Student() {
        studentId = 0;
        name = "";
        rollNo = "";
        course = "";
        year = 0;
        email = "";
        phone = "";
        bloodGroup = "";
        ecoPoints = 0;
    }

    Student(long long id, string n, string r, string c, int y, string mail, string ph, string bg, int pts = 0) {
        studentId = id;
        name = n;
        rollNo = r;
        course = c;
        year = y;
        email = mail;
        phone = ph;
        bloodGroup = bg;
        ecoPoints = pts;
    }

    void show() {
        cout << "ID: " << studentId 
             << " | Name: " << name 
             << " | Roll: " << rollNo 
             << " | Course: " << course 
             << " | Year: " << year 
             << " | Blood: " << bloodGroup 
             << " | Eco-Points: " << ecoPoints 
             << " | Phone: " << phone
             << " | Email: " << email << endl;
    }
};

class StudentManager {
private:
    unordered_map<long long, Student> studentMap;
    long long currentUserId;

    bool isValidId(long long id) {
        return (id >= 10000000000LL && id <= 99999999999LL);
    }

    bool isValidEmail(string mail) {
        int atPos = mail.find('@');
        int dotPos = mail.find('.', atPos);
        return (atPos != string::npos && dotPos != string::npos && atPos > 0);
    }

    bool verifyOTPWithAttempts(string targetEmail, string purpose) {
        int otp = 100000 + (rand() % 900000);
        string cmd = "python send_otp.py \"" + targetEmail + "\" " + to_string(otp) + " \"" + purpose + "\"";
        system(cmd.c_str());

        cout << "\n[OTP SENT] 6-Digit OTP sent to " << targetEmail << " (Demo Code: " << otp << ")\n";

        int attempts = 3;
        while (attempts > 0) {
            cout << "Enter 6-Digit OTP (" << attempts << " attempts left): ";
            int userOtp = readSafeInt();

            if (userOtp == otp) return true;
            attempts--;
            if (attempts > 0) cout << "Wrong OTP! Try again.\n";
        }
        cout << "\n[SECURITY LOCK] 3 wrong attempts! Session blocked.\n";
        return false;
    }

public:
    StudentManager() {
        currentUserId = 0;
        srand(time(0));
        loadData();
    }

    void saveData() {
        ofstream file("students_data.txt");
        for (auto pair : studentMap) {
            Student s = pair.second;
            file << s.studentId << endl;
            file << s.name << endl;
            file << s.rollNo << endl;
            file << s.course << endl;
            file << s.year << endl;
            file << s.email << endl;
            file << s.phone << endl;
            file << s.bloodGroup << endl;
            file << s.ecoPoints << endl;
        }
        file.close();
    }

    void loadData() {
        ifstream file("students_data.txt");
        if (!file) {
            saveData();
            return;
        }

        long long id;
        int yr, pts;
        string n, r, c, mail, ph, bg;

        while (file >> id) {
            file.ignore();
            getline(file, n);
            getline(file, r);
            getline(file, c);
            file >> yr;
            file.ignore();
            getline(file, mail);
            getline(file, ph);
            getline(file, bg);
            file >> pts;

            studentMap[id] = Student(id, n, r, c, yr, mail, ph, bg, pts);
        }
        file.close();
    }

    bool isVerified(long long id) {
        return studentMap.find(id) != studentMap.end();
    }

    string getEmail(long long id) {
        if (isVerified(id)) return studentMap[id].email;
        return "";
    }

    string getPhone(long long id) {
        if (isVerified(id)) return studentMap[id].phone;
        return "Not Available";
    }

    string getName(long long id) {
        if (isVerified(id)) return studentMap[id].name;
        return "Unknown";
    }

    long long getCurrentUser() { return currentUserId; }
    bool isLoggedIn() { return currentUserId != 0; }

    void logout() {
        if (currentUserId != 0) {
            cout << "\nLogged out from " << studentMap[currentUserId].name << ".\n";
            currentUserId = 0;
        }
    }

    void addEcoPoints(long long id, int points) {
        if (isVerified(id)) {
            studentMap[id].ecoPoints += points;
            saveData();
        }
    }

    bool loginWithOTP(long long id) {
        if (!isValidId(id)) {
            cout << "\nInvalid ID! Student ID must be 11 digits.\n";
            return false;
        }
        if (!isVerified(id)) {
            cout << "\nStudent ID not registered! Please register first.\n";
            return false;
        }

        Student s = studentMap[id];
        if (verifyOTPWithAttempts(s.email, "Login")) {
            currentUserId = id;
            cout << "\n[LOGIN SUCCESS] Welcome, " << s.name << " (Eco-Points: " << s.ecoPoints << ")!\n";
            return true;
        }
        return false;
    }

    bool registerStudent(long long id) {
        if (!isValidId(id)) {
            cout << "\nInvalid ID! Student ID must be 11 digits.\n";
            return false;
        }
        if (isVerified(id)) {
            cout << "\nStudent ID already registered!\n";
            return false;
        }

        string n, r, c, mail, ph, bg;
        int yr;

        cin.ignore();
        cout << "Enter Full Name: ";
        getline(cin, n);
        cout << "Enter Roll Number: ";
        getline(cin, r);
        cout << "Enter Course (e.g., BTech_CSE): ";
        getline(cin, c);
        cout << "Enter Year (1-4): ";
        yr = readSafeInt();
        if (yr < 1 || yr > 4) {
            cout << "\nInvalid Year! Must be between 1 and 4.\n";
            return false;
        }

        cin.ignore();
        cout << "Enter 10-Digit Mobile Number: ";
        getline(cin, ph);
        if (ph.length() != 10) {
            cout << "\nInvalid Mobile Number! Must be 10 digits.\n";
            return false;
        }

        cout << "Enter Blood Group (e.g., O+, B+, O-): ";
        getline(cin, bg);
        cout << "Enter Personal Gmail (e.g., name@gmail.com): ";
        getline(cin, mail);

        if (!isValidEmail(mail)) {
            cout << "\nInvalid Email format!\n";
            return false;
        }

        if (!verifyOTPWithAttempts(mail, "Registration")) {
            cout << "\nRegistration Cancelled.\n";
            return false;
        }

        studentMap[id] = Student(id, n, r, c, yr, mail, ph, bg, 10);
        currentUserId = id;
        saveData();
        cout << "\n[REGISTERED & LOGGED IN] Welcome, " << n << "! (+10 Welcome Eco-Points)\n";
        return true;
    }

    void updateEmailWithOTP() {
        if (!isLoggedIn()) return;
        string newMail;
        cin.ignore();
        cout << "Enter New Email: ";
        getline(cin, newMail);

        if (!isValidEmail(newMail)) {
            cout << "\nInvalid Email format!\n";
            return;
        }

        if (verifyOTPWithAttempts(newMail, "Email Update")) {
            studentMap[currentUserId].email = newMail;
            saveData();
            cout << "\nEmail updated successfully!\n";
        }
    }

    void showMyProfile() {
        if (!isLoggedIn()) return;
        cout << "\n--- MY PROFILE ---\n";
        studentMap[currentUserId].show();
    }

    void showAllStudents() {
        cout << "\n--- REGISTERED CAMPUS STUDENTS ---\n";
        for (auto pair : studentMap) {
            pair.second.show();
        }
    }
};

#endif