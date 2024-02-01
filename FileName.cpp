#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <windows.h>

using namespace std;

std::atomic<bool> exitflag(false);
bool found_username = false;
bool correct_password = false;
const int max_attempts = 5;
int login_attempts = 0;

string fileslocation = "C:/Users/ZAFAR COMPUTERS/Desktop/cp project/";

string username;
string password;

string messagefrom;
string filename;

std::mutex fileMutex;

std::condition_variable messageCondition;
std::queue<string> messageQueue;

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resetColor() {
    setColor(15);
}

string createFileWrite(string username) {
    string filename = username + "_message.txt";
    const string filePath = fileslocation + filename;
    ofstream file(filePath, ios::out);
    return filename;
}

void createHistory(string username, string messagefrom, string read, string write) {
    string sortedUsernames = username + messagefrom;
    sort(sortedUsernames.begin(), sortedUsernames.end());

    const string filePathH = fileslocation + sortedUsernames + ".txt";

    ofstream fileH(filePathH, ios::app);
    if (fileH.is_open()) {
        if (write == " ") {
            fileH << read << endl;
        }
        else if (read == " ") {
            fileH << username << ": " << write << endl;
        }
        fileH.close();
    }
    else {
        setColor(12);
        cout << "Error opening file." << endl;
        resetColor();
    }
}

string createHistoryFile(const string& username, const string& messagefrom) {
    string sortedUsernames = username + messagefrom;
    sort(sortedUsernames.begin(), sortedUsernames.end());

    const string filePathH = fileslocation + sortedUsernames + ".txt";
    ofstream fileHis(filePathH, ios::out);

    fileHis.close();
    return sortedUsernames;
}

void readMessage(string messagefrom) {
    string filename = messagefrom + "_message.txt";
    const string filePath = fileslocation + filename;
    string line;

    int pointer = 0;

    while (!exitflag) {
        {
            std::unique_lock<std::mutex> lock(fileMutex);

            ifstream file(filePath, ios::in);

            if (file.is_open()) {
                file.clear();
                file.seekg(0);

                for (int i = 0; i < pointer; ++i) {
                    getline(file, line);
                }

                while (getline(file, line)) {
                    size_t pos = line.find(':');
                    if (pos != string::npos) {
                        string sender = line.substr(0, pos);

                        if (sender == username) {
                            setColor(10);
                        }
                        else {
                            setColor(11);
                        }

                        cout << sender;

                        setColor(15);
                        cout << line.substr(pos) << endl;
                        resetColor();

                        pointer++;
                    }
                }

                file.close();
            }
            else {
                setColor(12);
                cout << "Error opening file." << endl;
                resetColor();
            }
        }

        std::unique_lock<std::mutex> lock(fileMutex);

        // Wait for a new message or a timeout
        messageCondition.wait_for(lock, std::chrono::milliseconds(500), [&]() { return !messageQueue.empty(); });

        // Process messages from the queue
        while (!messageQueue.empty()) {
            cout << messageQueue.front() << endl;
            messageQueue.pop();
        }
    }

    resetColor();
}

void writeMessage(string username, string filename) {
    string line;

    {
        const string filePath = fileslocation + filename;

        setColor(10);
        cout << "\n" << username << ": ";
        resetColor();

        getline(cin, line);
        setColor(5);
        cout << "message sent:";
        resetColor();
        createHistory(username, messagefrom, " ", line);

        if (line == "-1") {
            exitflag = true;
        }
        else {
            ofstream Wfile(filePath, ios::app);
            if (Wfile.is_open()) {
                Wfile << username << ": " << line << endl;

                // Add the message to the queue
                std::lock_guard<std::mutex> lock(fileMutex);
                messageQueue.push(username + ": " + line);

                // Notify the reading thread about the new message
                messageCondition.notify_one();
            }
            else {
                setColor(12);
                cout << "Error opening file";
                resetColor();
            }
        }
    }
}

bool historyFileExists(const string username, const string messagefrom) {
    string sortedUsernames = username + messagefrom;
    sort(sortedUsernames.begin(), sortedUsernames.end());

    const string filePathH = fileslocation + sortedUsernames + ".txt";
    ifstream file(filePathH);

    return file.good();
}

void lastChat(string username, string messagefrom) {
    string sortedUsernames = username + messagefrom;
    sort(sortedUsernames.begin(), sortedUsernames.end());
    string line;
    bool headerPrinted = false;

    const string filePathH = fileslocation + sortedUsernames + ".txt";

    ifstream fileHis(filePathH, ios::in);

    if (fileHis.is_open()) {
        while (getline(fileHis, line)) {
            if (!(line.empty())) {
                if (!headerPrinted) {
                    setColor(14);
                    cout << "~~~~LAST MESSAGES~~~~\n";
                    resetColor();
                    headerPrinted = true;
                }

                size_t pos = line.find(':');
                if (pos != string::npos) {
                    string sender = line.substr(0, pos);

                    if (sender == username) {
                        setColor(10);
                    }
                    else {
                        setColor(11);
                    }

                    cout << sender;

                    setColor(15);
                    cout << line.substr(pos) << endl;
                    resetColor();
                }
            }
        }
    }
}

void displayUserList() {
    ifstream userFile(fileslocation + "usernames.txt", ios::in);
    string user;
    int userNumber = 1;

    setColor(13);
    cout << "Select a user to message:\n";
    resetColor();

    while (getline(userFile, user)) {
        cout << userNumber << ". " << user << endl;
        userNumber++;
    }

    userFile.close();
}

string selectUser() {
    int selectedUserNumber;
    bool validUser = false;

    while (!validUser) {
        setColor(9);
        cout << "Enter the number of the user you want to message: ";
        resetColor();
        cin >> selectedUserNumber;

        ifstream userFile(fileslocation + "usernames.txt", ios::in);
        string user;
        int currentUserNumber = 1;

        while (getline(userFile, user)) {
            if (currentUserNumber == selectedUserNumber) {
                validUser = true;
                break;
            }
            currentUserNumber++;
        }

        userFile.close();

        if (!validUser) {
            setColor(12);
            cout << "Invalid user number. Please enter a valid user number.\n";
            resetColor();
        }
    }

    ifstream userFile(fileslocation + "usernames.txt", ios::in);
    string selectedUser;
    int currentUserNumber = 1;

    while (getline(userFile, selectedUser)) {
        if (currentUserNumber == selectedUserNumber) {
            break;
        }
        currentUserNumber++;
    }

    userFile.close();

    return selectedUser;
}

void updateMessageFile(const string& selectedUser) {
    const string filePath = fileslocation + selectedUser + "_message.txt";
    ifstream file(filePath);

    if (!file.is_open() || file.peek() == ifstream::traits_type::eof()) {
        ofstream outFile(filePath, ios::app);
        if (outFile.is_open()) {
            setColor(14);
            cout << "Welcome to the chat with " << selectedUser << "!\n";
            resetColor();
        }
        else {
            setColor(12);
            cout << "Error opening file for updating messages.\n";
            resetColor();
        }
    }
    file.close();
}

void checkUsername(const string& Eusername) {
    const string filepath = fileslocation + "usernames.txt";
    string line;

    ifstream file(filepath, ios::in);
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line == Eusername) {
                found_username = true;
                filename = createFileWrite(Eusername);
                break;
            }
        }
        file.close();
    }
}

void checkPassword(const string& Epassword) {
    const string filepath = fileslocation + "passwords.txt";
    string line;

    ifstream file(filepath, ios::in);
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line == Epassword) {
                correct_password = true;
                break;
            }
        }
        file.close();
    }
}

void registration();

int main() {
    while (true) {
        string choice;
        setColor(12);
        exitflag = false;
        cout << "<<<<<<<<<<<<<<<<<<Welcome>>>>>>>>>>>>>>>>>>" << endl;
        resetColor();
        cout << "Choose an option:\n";
        setColor(3);
        cout << "1. Register\n";
        cout << "2. Login\n";
        cout << "3. Exit\n";
        resetColor();

        cin >> choice;

        if (choice == "1") {
            registration();
        }
        else if (choice == "2") {
            string input;
            setColor(5);
            cout << "<<<<<<<<<<<<<<<<<<LOGIN>>>>>>>>>>>>>>>>>\n";
            resetColor();
            while (login_attempts < max_attempts) {
                cout << "\nUsername: ";
                cin >> username;

                cout << "Password: ";
                cin >> password;

                checkUsername(username);
                checkPassword(password);

                if (found_username && correct_password) {
                    setColor(10);
                    cout << "\nLogin successful! Welcome, " << username << "!" << endl;
                    resetColor();
                    displayUserList();
                    messagefrom = selectUser();
                    cout << "\nYou have selected to message: " << messagefrom << endl;
                    updateMessageFile(messagefrom);
                    break;
                }
                else {
                    cout << "\nLogin failed. Please try again. Attempts left: " << max_attempts - login_attempts - 1
                        << endl;
                    login_attempts++;
                }
            }

            if (!found_username || !correct_password) {
                cout << "\nMaximum login attempts reached. Access denied." << endl;
                return 0;
            }

            bool historyexist = historyFileExists(username, messagefrom);
            if (!historyexist) {
                createHistoryFile(username, messagefrom);
            }
            else {
                lastChat(username, messagefrom);
                setColor(14);
                cout << "~~~~NEWEST MESSAGES~~~~\n";
                resetColor();
                thread readThread(readMessage, messagefrom);
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));

                while (!(exitflag)) {
                    cout << "\nto type please press t: \n ";
                    cin >> input;
                    cin.ignore();
                    if (input == "t") {
                        input = "";

                        writeMessage(username, filename);
                        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                    }
                }

                readThread.join();
            }
        }
        else if (choice == "3") {
            cout << "Exiting program.\n";
            break;
        }
        else {
            cout << "Invalid choice. Please enter 1, 2, or 3.\n";
        }
    }

    return 0;
}

void registration() {
    string new_username, new_password;
    setColor(5);
    cout << "<<<<<<<<<<<<<REGISTRATION>>>>>>>>>>>>>>\n";
    resetColor();
    cout << "Enter a new username: ";
    cin >> new_username;

    const string filepath = fileslocation + "usernames.txt";
    string line;

    ifstream file(filepath, ios::in);
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line == new_username) {
                cout << "Username already exists. Please choose a different username.\n";
                file.close();
                return;
            }
        }
        file.close();
    }

    bool validPassword = false;
    do {
        cout << "Enter a new password: ";
        cin >> new_password;

        if (new_password.length() < 8) {
            setColor(12);
            cout << "Password must be at least 8 characters long.\n";
            resetColor();
        }
        else if (new_password.find_first_of("0123456789") == string::npos) {
            setColor(12);
            cout << "Password must contain at least one digit.\n";
            resetColor();
        }
        else if (new_password.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") == string::npos) {
            setColor(12);
            cout << "Password must contain at least one alphabetic character.\n";
            resetColor();
        }
        else {
            validPassword = true;
        }

    } while (!validPassword);

    ofstream userFile(filepath, ios::app);
    ofstream passFile(fileslocation + "passwords.txt", ios::app);

    if (userFile.is_open() && passFile.is_open()) {
        userFile << new_username << endl;
        passFile << new_password << endl;

        cout << "Registration successful!\n";
    }
    else {
        setColor(12);
        cout << "Error opening files for registration.\n";
        resetColor();
    }

    userFile.close();
    passFile.close();
}
