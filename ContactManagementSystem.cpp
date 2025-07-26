#include <iostream>
#include <string>
#include <stack>
#include <queue>
#include <fstream>
using namespace std;

class Contact {
public:
    string contact_name;
    string phoneNumber;
    string email;
    string address;
    Contact() = default;
    void setName(const string& name) { contact_name = name; }
    void setPhoneNumber(const string& phoneNumber) { this->phoneNumber = phoneNumber; }
    void setEmail(const string& email) { this->email = email; }
    void setAddress(const string& address) { this->address = address; }
    string getName() const { return contact_name; }
    string getPhoneNumber() const { return phoneNumber; }
    string getEmail() const { return email; }
    string getAddress() const { return address; }
    void display() const {
        cout << "Name: " << contact_name << endl;
        cout << "Phone Number: " << phoneNumber << endl;
        cout << "Email: " << email << endl;
        cout << "Address: " << address << endl;
    }
    void enterdetails(string name, string phoneNumber, string email, string address) {
        setName(name);
        setPhoneNumber(phoneNumber);
        setEmail(email);
        setAddress(address);
    }
};

class Node {
public:
    Contact data;
    Node* next;
    Node* prev;
    Node() : next(nullptr), prev(nullptr) {}
};

struct OperationType {
    static const int Add = 0;
    static const int Delete = 1;
    static const int Update = 2;
};

struct Operation {
    int type;
    Contact before;
    Contact after;
    string description;
};
string toLower(const string& str) {
    string result = str;
    for (char& c : result) {
        c = tolower(c);
    }
    return result;
}

class contactList {
    Node* head;
    Node* tail;
    stack<Operation> undoStack;
    stack<Operation> redoStack;
    queue<string> opHistory;
    friend void updateFile(const contactList cl);

    Node* findContactByPhone(const string& number) {
        Node* current = head;
        while (current) {
            if (current->data.getPhoneNumber() == number) return current;
            current = current->next;
        }
        return nullptr;
    }
   

public:
    contactList() : head(nullptr), tail(nullptr) {}
    ~contactList() {
        Node* current = head;
        while (current) {
            Node* toDelete = current;
            current = current->next;
            delete toDelete;
        }
        head = tail = nullptr;
    }
	bool isEmpty() const {
		return head == nullptr;
	}

    void addContact(const string& name, const string& phoneNumber, const string& email, const string& address) {
        Node* itr = new Node;
        itr->data.enterdetails(name, phoneNumber, email, address);
        itr->next = nullptr;
        itr->prev = tail;
        if (tail) tail->next = itr;
        else head = itr;
        tail = itr;
        undoStack.push({ OperationType::Add, {}, itr->data, "Added: " + name });
        opHistory.push("Added: " + name);
        while (!redoStack.empty()) redoStack.pop();
    }

    bool deleteContact(const string& number) {
        Node* current = findContactByPhone(number);
        if (!current) return false;
        Contact deleted = current->data;
        string name = deleted.getName();
        if (current->prev) current->prev->next = current->next;
        else head = current->next;
        if (current->next) current->next->prev = current->prev;
        else tail = current->prev;
        delete current;
        undoStack.push({ OperationType::Delete, deleted, {}, "Deleted: " + name });
        opHistory.push("Deleted: " + name);
        while (!redoStack.empty()) redoStack.pop();
        return true;
    }


    bool deleteContactByName(const string& name) {
        Node* current = head;
        while (current) {
            if (toLower(current->data.getName()) == toLower(name)) {
                Contact deleted = current->data;
                if (current->prev) current->prev->next = current->next;
                else head = current->next;
                if (current->next) current->next->prev = current->prev;
                else tail = current->prev;
                delete current;

                undoStack.push({ OperationType::Delete, deleted, {}, "Deleted by name: " + deleted.getName() });
                opHistory.push("Deleted by name: " + deleted.getName());
                while (!redoStack.empty()) redoStack.pop();
                return true;
            }
            current = current->next;
        }
        return false;
    }







    bool updateContact(const string& number, const string& new_name, const string& newPhoneNumber, const string& newEmail, const string& newAddress) {
        Node* current = findContactByPhone(number);
        if (!current) return false;
        Contact before = current->data;
        current->data.setName(new_name);
        current->data.setPhoneNumber(newPhoneNumber);
        current->data.setEmail(newEmail);
        current->data.setAddress(newAddress);
        undoStack.push({ OperationType::Update, before, current->data, "Updated: " + before.getName() + " to " + new_name });
        opHistory.push("Updated: " + before.getName() + " to " + new_name);
        while (!redoStack.empty()) redoStack.pop();
        return true;
    }

    void displayContacts() const {
        Node* current = head;
        if (!current) {
            cout << "No contacts available..." << endl;
            return;
        }
        while (current) {
            current->data.display();
            current = current->next;
            cout << "------------------" << endl;
        }
    }
    void searchByName(const string& name) const {
        Node* current = head;
        bool anyMatch = false;
        while (current) {
            if (toLower(current->data.getName()) == toLower(name)) {
                if (!anyMatch) {
                    cout << "Contact(s) found with name: " << name << endl;
                }
                current->data.display();
                anyMatch = true;
            }
            current = current->next;
        }
        if (!anyMatch) {
            cout << "No contact found with name: " << name << endl;
        }
    }

    
    void searchByPhone(const string& number) const {
        Node* current = head;
        bool anyMatch = false;
        while (current) {
            if (current->data.getPhoneNumber() == number) {
                if (!anyMatch) {
                    cout << "Contact(s) found with number: " << number << endl;
                }
                current->data.display();
                anyMatch = true;
                break;
            }
            current = current->next;
        }
        if (!anyMatch) {
            cout << "No contact found with number: " << number << endl;
        }
    }





    

    bool undoLastOperation() {
        if (undoStack.empty()) return false;
        Operation op = undoStack.top(); undoStack.pop();
        switch (op.type) {
        case OperationType::Add:
            deleteContact(op.after.getPhoneNumber());
            break;
        case OperationType::Delete:
            addContact(op.before.getName(), op.before.getPhoneNumber(), op.before.getEmail(), op.before.getAddress());
            break;
        case OperationType::Update:
            updateContact(op.after.getPhoneNumber(), op.before.getName(), op.before.getPhoneNumber(), op.before.getEmail(), op.before.getAddress());
            break;
        }
        redoStack.push(op);
        return true;
    }

    bool redoLastOperation() {
        if (redoStack.empty()) return false;
        Operation op = redoStack.top(); redoStack.pop();
        switch (op.type) {
        case OperationType::Add:
            addContact(op.after.getName(), op.after.getPhoneNumber(), op.after.getEmail(), op.after.getAddress());
            break;
        case OperationType::Delete:
            deleteContact(op.before.getPhoneNumber());
            break;
        case OperationType::Update:
            updateContact(op.before.getPhoneNumber(), op.after.getName(), op.after.getPhoneNumber(), op.after.getEmail(), op.after.getAddress());
            break;
        }
        undoStack.push(op);
        return true;
    }

    void displayOperationHistory() const {
        if (opHistory.empty()) {
            cout << "No operations to display." << endl;
            return;
        }
        queue<string> temp = opHistory;
        while (!temp.empty()) {
            cout << temp.front() << endl;
            temp.pop();
        }
    }
};

void readContactFromFile(contactList& cl)
{
    ifstream fin("Contacts.txt");
    if (!fin) {
        cout << "Error opening file." << endl;
        return;
    }
    string fname, lname, number, email, address;
    while (fin >> fname >> lname >> number >> email) {
        getline(fin, address); 
        if (!address.empty() && address[0] == ' ') address = address.substr(1);
        string fullName = fname + " " + lname;
        cl.addContact(fullName, number, email, address);
    }
    fin.close();
}

void updateFile(const contactList cl) {
    ofstream fout("Contacts.txt");
    if (!fout) {
        cout << "Error opening file." << endl;
        return;
    }
    Node* current = cl.head;
    while (current) {
        fout << current->data.getName() << " "
            << current->data.getPhoneNumber() << " "
            << current->data.getEmail() << " "
            << current->data.getAddress() << endl;
        current = current->next;
    }
    fout.close();
}

bool isValidPhoneNumber(const string& number) {
    if (number.length() < 12) return false;
    for (int i = 0; i < number.length(); ++i) {
        if (!isdigit(number[i])) return false;
    }
    return true;
}

int main() {
    contactList cl;
    readContactFromFile(cl);

    if (cl.isEmpty()) {
        cout << "No contacts in your Contact List yet!" << endl;
    }
    else {
        cout << "Your Contact List is as follows:\n";
        cl.displayContacts();
    }

    string input, fname, lname, phone, email, address;
    string newFname, newLname, newPhone, newEmail, newAddress;

    while (true) {
        cout << "\n===== Contact Manager Menu =====" << endl;
        cout << "1. Add Contact\n";
        cout << "2. Delete Contact by Phone Number\n";
        cout << "3. Update Contact\n";
        cout << "4. Search by Name\n";
        cout << "5. Search by Phone Number\n";
        cout << "6. Display All Contacts\n";
        cout << "7. Display Operation History\n";
        cout << "8. Delete Contact by Name\n";
        cout << "9. Exit\n";
        cout << "Enter your choice: ";
        getline(cin, input);

        if (input == "1") {
            cout << "Enter first name: ";
            getline(cin, fname);
            cout << "Enter last name: ";
            getline(cin, lname);
            cout << "Enter 12-digit phone number (e.g., 92xxxxxxxxxx): ";
            while (true) {
                getline(cin, phone);
                if (isValidPhoneNumber(phone)) break;
                cout << "Invalid phone number. Try again: ";
            }
            cout << "Enter email: ";
            getline(cin, email);
            cout << "Enter address: ";
            getline(cin, address);

            cl.addContact(fname + " " + lname, phone, email, address);
            cout << "Contact added successfully.\n";

        }
        else if (input == "2") {
            cout << "Enter phone number of contact to delete: ";
            while (true) {
                getline(cin, phone);
                if (isValidPhoneNumber(phone)) break;
                cout << "Invalid phone number. Try again: ";
            }

            if (cl.deleteContact(phone)) {
                cout << "Contact deleted successfully.\n";
            }
            else {
                cout << "Contact not found.\n";
                continue;
            }

        }
        else if (input == "3") {
            cout << "Enter current phone number of contact to update: ";
            while (true) {
                getline(cin, phone);
                if (isValidPhoneNumber(phone)) break;
                cout << "Invalid phone number. Try again: ";
            }

            cout << "Enter new first name: ";
            getline(cin, newFname);
            cout << "Enter new last name: ";
            getline(cin, newLname);
            cout << "Enter new phone number: ";
            while (true) {
                getline(cin, newPhone);
                if (isValidPhoneNumber(newPhone)) break;
                cout << "Invalid phone number. Try again: ";
            }
            cout << "Enter new email: ";
            getline(cin, newEmail);
            cout << "Enter new address: ";
            getline(cin, newAddress);

            if (cl.updateContact(phone, newFname + " " + newLname, newPhone, newEmail, newAddress)) {
                cout << "Contact updated successfully.\n";
            }
            else {
                cout << "Contact not found.\n";
                continue;
            }

        }
        else if (input == "4") {
            cout << "Enter name to search: ";
            string searchName;
            getline(cin, searchName);
            cl.searchByName(searchName);

        }
        else if (input == "5") {
            cout << "Enter phone number to search: ";
            string searchPhone;
            getline(cin, searchPhone);
            cl.searchByPhone(searchPhone);

        }
        else if (input == "6") {
            cl.displayContacts();

        }
        else if (input == "7") {
            cl.displayOperationHistory();

        }
        else if (input == "8") {
            cout << "Enter full name of contact to delete: ";
            string delName;
            getline(cin, delName);
            if (cl.deleteContactByName(delName)) {
                cout << "Contact deleted successfully.\n";
            }
            else {
                cout << "Contact not found.\n";
                continue;
            }

        }
        else if (input == "9") {
            updateFile(cl);
            cout << "Contacts saved. Goodbye!\n";
            break;

        }
        else {
            cout << "Invalid input. Try again.\n";
            continue;
        }

        // Undo prompt for modifying actions
        if (input == "1" || input == "2" || input == "3" || input == "8") {
            string choice;
            cout << "\nDo you want to undo this action? (yes/no): ";
            getline(cin, choice);

            if (choice == "yes" || choice == "Yes" || choice == "YES" || choice == "y" || choice == "Y") {
                if (cl.undoLastOperation())
                    cout << "Undo successful.\n";
                else
                    cout << "Nothing to undo.\n";

                cout << "Do you want to redo it? (yes/no): ";
                getline(cin, choice);
                if (choice == "yes" || choice == "Yes" || choice == "YES" || choice == "y" || choice == "Y") {
                    if (cl.redoLastOperation())
                        cout << "Redo successful.\n";
                    else
                        cout << "Nothing to redo.\n";
                }
            }
        }

        string cont;
        cout << "\nDo you wish to continue? (yes/no): ";
        getline(cin, cont);

        if (cont == "no" || cont == "No" || cont == "NO" || cont == "n" || cont == "N") {
            updateFile(cl);
            cout << "Contacts saved. Goodbye!\n";
            break;
        }
        else if (cont == "yes" || cont == "Yes" || cont == "YES" || cont == "y" || cont == "Y") {
            system("cls");  
        }
        else {
            cout << "Invalid input. Exiting for safety.\n";
            updateFile(cl);
            break;
        }
    }

    return 0;
}
