#ifndef STUDENT_H
#define STUDENT_H

#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

class Student {
public:
    long long studentId; // 11-digit support
    string name;
    string rollNo;
    string course;
    int year;

    Student() : studentId(0), name(""), rollNo(""), course(""), year(0) {}
    Student(long long id, string n, string r, string c, int y)
        : studentId(id), name(n), rollNo(r), course(c), year(y) {}

    void display() const {
        cout << "Student ID: " << studentId << " | Name: " << name 
             << " | Roll No: " << rollNo << " | Course: " << course 
             << " | Year: " << year << "\n";
    }
};

class StudentManager {
private:
    unordered_map<long long, Student> studentDB;

public:
    StudentManager() {
        loadSampleData();
    }

    void loadSampleData() {
        studentDB[25101281462] = Student(25101281462, "Tapas Tomar", "2519549", "BTech_CSE", 2);
        studentDB[25101281063] = Student(25101281063, "Muhammad Anas", "2519426", "BTech_CSE", 2);
        studentDB[2510370401]  = Student(2510370401, "Tejeswa", "2519551", "BTech_CSE", 2);
    }

    bool verifyStudent(long long id) const {
        return studentDB.find(id) != studentDB.end();
    }

    void registerStudent(long long id) {
        if (verifyStudent(id)) {
            cout << "\nStudent ID already exists.\n";
            return;
        }

        string name, roll, course;
        int year;
        cout << "Enter Full Name: "; cin >> name;
        cout << "Enter Roll Number: "; cin >> roll;
        cout << "Enter Course Name: "; cin >> course;
        cout << "Enter Academic Year: "; cin >> year;

        studentDB[id] = Student(id, name, roll, course, year);
        cout << "\nStudent registered successfully!\n";
    }

    void runDashboard() {
        int choice;
        while (true) {
            cout << "\n--- STUDENT DIRECTORY ---\n";
            cout << "1. Register New Student\n";
            cout << "2. Verify Student ID\n";
            cout << "3. View All Students\n";
            cout << "4. Back to Main Menu\n";
            cout << "Enter choice (1-4): ";
            
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(1000, '\n');
                continue;
            }

            if (choice == 1) {
                long long id;
                cout << "Enter 11-Digit Student ID: "; cin >> id;
                registerStudent(id);
            } else if (choice == 2) {
                long long id;
                cout << "Enter Student ID to Verify: "; cin >> id;
                if (verifyStudent(id)) {
                    cout << "\n[Match Found] ";
                    studentDB[id].display();
                } else {
                    cout << "\nStudent ID not registered.\n";
                }
            } else if (choice == 3) {
                cout << "\n--- REGISTERED STUDENTS ---\n";
                if (studentDB.empty()) {
                    cout << "No students found.\n";
                } else {
                    for (const auto& pair : studentDB) {
                        pair.second.display();
                    }
                }
            } else if (choice == 4) {
                break;
            }
        }
    }
};

#endif