#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <fstream>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>
#include <cctype>
#include <cstdlib>

using namespace std;

const int LOCATIONS = 16;
const int CENTER_COUNT = 2;
const int INF = numeric_limits<int>::max();

const string DATA_FILE = "evacuees.txt";

const int COURSE_COUNT = 4;
const string COURSES[COURSE_COUNT] = {"BSIT", "BEED", "BSED", "BSBA"};

struct Edge {
    int destination;
    int distance;
    bool blocked;
};

struct Evacuee {
    string id;
    string name;
    string course;
    string section;
    int location;
    bool evacuated;
    int center;
};

string toUpper(string s) {
    for (char& c : s) {
        c = toupper((unsigned char)c);
    }
    return s;
}

// Reads a whole line and only accepts a number from minValue to maxValue.
// Letters, symbols, blanks, or out-of-range numbers show an error and ask again.
int readInt(const string& prompt, int minValue, int maxValue) {

    string line;

    while (true) {

        cout << prompt;

        if (!getline(cin, line)) {
            cout << "\nInput closed. Exiting.\n";
            exit(0);
        }

        size_t first = line.find_first_not_of(" \t\r");
        size_t last = line.find_last_not_of(" \t\r");

        if (first == string::npos) {
            cout << "Error: Input cannot be empty. Please enter a number from "
                 << minValue << " to " << maxValue << ".\n";
            continue;
        }

        line = line.substr(first, last - first + 1);

        bool digitsOnly = line.size() <= 9;

        for (char c : line) {
            if (!isdigit((unsigned char)c)) {
                digitsOnly = false;
            }
        }

        if (!digitsOnly) {
            cout << "Error: Invalid input \"" << line
                 << "\". Please enter a number from "
                 << minValue << " to " << maxValue << ".\n";
            continue;
        }

        int value = stoi(line);

        if (value < minValue || value > maxValue) {
            cout << "Error: " << value << " is not a choice. Please enter a number from "
                 << minValue << " to " << maxValue << ".\n";
            continue;
        }

        return value;
    }
}

// Only accepts y or n.
bool askYesNo(const string& prompt) {

    string line;

    while (true) {

        cout << prompt;

        if (!getline(cin, line)) {
            cout << "\nInput closed. Exiting.\n";
            exit(0);
        }

        if (line == "y" || line == "Y") {
            return true;
        }

        if (line == "n" || line == "N") {
            return false;
        }

        cout << "Error: Please enter only y or n.\n";
    }
}

struct Center {
    int location;
    int capacity;
    int occupants;
};

string places[LOCATIONS] = {
    "Faculty Room",
    "Classroom Wing",
    "Administration Office",
    "Register Office",
    "Accreditation Office",
    "Conference Room",
    "Speech Laboratory",
    "Library",
    "Computer Laboratory 1",
    "Computer Laboratory 2",
    "EdTech Laboratory",
    "Canteen",
    "Basketball Court",
    "Stage",
    "Main Hall Evacuation Area",
    "Basketball Court Evacuation Area"
};

vector<Edge> graph[LOCATIONS];
vector<Evacuee> evacuees;

Center centers[CENTER_COUNT] = {
    {14, 200, 0},
    {15, 150, 0}
};

int findEdge(int from, int to) {
    for (int i = 0; i < graph[from].size(); i++) {
        if (graph[from][i].destination == to) {
            return i;
        }
    }

    return -1;
}

void addPath(int a, int b, int distance) {
    graph[a].push_back({b, distance, false});
    graph[b].push_back({a, distance, false});
}

void createGraph() {

    addPath(0, 14, 35);
    addPath(1, 14, 45);
    addPath(2, 3, 15);
    addPath(3, 4, 12);
    addPath(4, 5, 15);
    addPath(3, 14, 20);
    addPath(6, 7, 12);
    addPath(6, 14, 18);
    addPath(7, 8, 15);
    addPath(8, 14, 30);
    addPath(9, 10, 20);
    addPath(10, 1, 40);
    addPath(10, 14, 35);
    addPath(11, 12, 20);
    addPath(12, 13, 10);
    addPath(12, 15, 15);
    addPath(14, 15, 60);
}

void showMap() {

    cout << "\n";
    cout << "========================================================\n";
    cout << "             NEUST EMERGENCY EVACUATION MAP\n";
    cout << "========================================================\n\n";

    cout << "                    [FACULTY ROOM]\n";
    cout << "                          |\n";
    cout << "              [MAIN HALL EVACUATION AREA]\n";
    cout << "                          |\n\n";

    cout << " CLASSROOM WING       MAIN HALLWAY         STAGE\n";
    cout << " +-------------+     +--------------+    +----------+\n";
    cout << " | CLASSROOMS  |     | ADMIN OFFICE |    |          |\n";
    cout << " +-------------+     +--------------+    |  STAGE   |\n";
    cout << " | CLASSROOMS  |     | REGISTER     |    |          |\n";
    cout << " +-------------+     +--------------+    +----------+\n";
    cout << " | COMLAB 2    |     | ACCREDITATION|\n";
    cout << " +-------------+     +--------------+\n";
    cout << "                     | CONFERENCE   |\n";
    cout << "                     +--------------+\n\n";

    cout << " +-------------+     +--------------+    +----------------+\n";
    cout << " | SPEECH LAB  |-----|   HALLWAY    |    |                |\n";
    cout << " +-------------+     +--------------+    |  BASKETBALL    |\n";
    cout << " |             |     |              |    |     COURT      |\n";
    cout << " |   LIBRARY   |     | CLASSROOMS   |    |                |\n";
    cout << " |             |     |              |    |                |\n";
    cout << " +-------------+     +--------------+    +----------------+\n";
    cout << "       |                     |                    |\n";
    cout << " +-------------+     +--------------+        [EXIT]\n";
    cout << " | COMLAB 1    |     | EDTECH LAB   |\n";
    cout << " +-------------+     +--------------+\n";
    cout << "        |                    |\n";
    cout << "        +---------+----------+\n";
    cout << "                  |\n";
    cout << "               [CANTEEN]\n";

    cout << "\n========================================================\n";
    cout << " LEGEND\n";
    cout << " [LOCATION] = Room or area\n";
    cout << " -----      = Sample connection\n";
    cout << " |          = Sample connection\n";
    cout << "========================================================\n";

    cout << "\nNOTE: Simplified schematic, NOT an exact floor plan.\n";
    cout << "Follow official campus evacuation instructions.\n";
}

void showLocations() {

    cout << "\n========== CAMPUS LOCATIONS ==========\n";

    for (int i = 0; i < LOCATIONS; i++) {

        cout << i + 1 << ". " << places[i];

        if (i == 14 || i == 15) {
            cout << " [SAMPLE EVACUATION CENTER]";
        }

        cout << "\n";
    }
}

void saveEvacuees() {

    ofstream file(DATA_FILE);

    if (!file) {
        cout << "Error saving records!\n";
        return;
    }

    for (const Evacuee& e : evacuees) {

        file << quoted(e.id) << " "
             << quoted(e.name) << " "
             << e.location << " "
             << e.evacuated << " "
             << e.center << " "
             << quoted(e.course) << " "
             << quoted(e.section) << "\n";
    }

    file.close();
}

void loadEvacuees() {

    ifstream file(DATA_FILE);

    if (!file) {
        return;
    }

    string line;

    while (getline(file, line)) {

        if (line.empty()) {
            continue;
        }

        istringstream in(line);
        Evacuee e;

        if (!(in >> quoted(e.id)
                 >> quoted(e.name)
                 >> e.location
                 >> e.evacuated
                 >> e.center)) {
            continue;
        }

        // Older records have no course/section fields
        if (!(in >> quoted(e.course) >> quoted(e.section))) {
            e.course = "N/A";
            e.section = "N/A";
        }

        if (e.location < 0 || e.location >= LOCATIONS) {
            continue;
        }

        if (e.center < -1 || e.center >= CENTER_COUNT) {
            continue;
        }

        bool duplicate = false;

        for (const Evacuee& existing : evacuees) {
            if (existing.id == e.id) {
                duplicate = true;
                break;
            }
        }

        if (duplicate) {
            continue;
        }

        evacuees.push_back(e);

        if (e.evacuated && e.center >= 0) {
            centers[e.center].occupants++;
        }
    }

    file.close();
}

vector<int> findShortestPath(
    int start,
    int destination,
    int& totalDistance
) {

    vector<int> distance(LOCATIONS, INF);
    vector<int> parent(LOCATIONS, -1);

    priority_queue<
        pair<int, int>,
        vector<pair<int, int>>,
        greater<pair<int, int>>
    > pq;

    distance[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {

        int currentDistance = pq.top().first;
        int current = pq.top().second;

        pq.pop();

        if (currentDistance != distance[current]) {
            continue;
        }

        for (const Edge& edge : graph[current]) {

            if (edge.blocked) {
                continue;
            }

            int next = edge.destination;

            int newDistance =
                currentDistance + edge.distance;

            if (newDistance < distance[next]) {

                distance[next] = newDistance;
                parent[next] = current;

                pq.push({newDistance, next});
            }
        }
    }

    totalDistance = distance[destination];
    vector<int> path;

    if (totalDistance == INF) {
        return path;
    }

    int current = destination;

    while (current != -1) {
        path.push_back(current);
        current = parent[current];
    }

    reverse(path.begin(), path.end());

    return path;
}

void findEvacuationRoute(int start) {

    int bestCenter = -1;
    int shortestDistance = INF;

    vector<int> bestPath;

    for (int i = 0; i < CENTER_COUNT; i++) {

        if (centers[i].occupants >= centers[i].capacity) {
            continue;
        }

        int distance;

        vector<int> path = findShortestPath(
            start,
            centers[i].location,
            distance
        );

        if (!path.empty() && distance < shortestDistance) {

            shortestDistance = distance;
            bestCenter = i;
            bestPath = path;
        }
    }

    if (bestCenter == -1) {

        cout << "\nNo available reachable evacuation center.\n";
        cout << "Contact campus safety personnel.\n";

        return;
    }

    cout << "\n========== SHORTEST EVACUATION ROUTE ==========\n\n";
    for (int i = 0; i < bestPath.size(); i++) {

        cout << places[bestPath[i]];

        if (i < bestPath.size() - 1) {

            int from = bestPath[i];
            int to = bestPath[i + 1];

            int edgeIndex = findEdge(from, to);

            if (edgeIndex != -1) {

                cout << "\n       |\n";
                cout << "       | "
                     << graph[from][edgeIndex].distance
                     << " meters\n";
                cout << "       v\n";
            }
        }
    }

    cout << "\nTotal Distance: "
         << shortestDistance << " meters\n";

    cout << "Destination: "
         << places[centers[bestCenter].location] << "\n";

    int minutes = (shortestDistance + 59) / 60;

    cout << "Estimated Walking Time: "
         << minutes << " minute(s)\n";

    cout << "Available Spaces: "
         << centers[bestCenter].capacity
                - centers[bestCenter].occupants
         << "\n";

    cout << "\nReminder: Verify that the route is safe and open.\n";
}

void registerEvacuee() {

    Evacuee e;

    cout << "\n========== REGISTER EVACUEE ==========\n";

    cout << "Enter ID Number: ";
    getline(cin, e.id);

    if (e.id.empty()) {
        cout << "ID cannot be empty.\n";
        return;
    }

    for (const Evacuee& existing : evacuees) {

        if (existing.id == e.id) {
            cout << "ID already registered!\n";
            return;
        }
    }

    cout << "Enter Full Name: ";
    getline(cin, e.name);

    if (e.name.empty()) {
        cout << "Name cannot be empty.\n";
        return;
    }

    cout << "\nSelect Course:\n";

    for (int i = 0; i < COURSE_COUNT; i++) {
        cout << i + 1 << ". " << COURSES[i] << "\n";
    }

    int courseChoice = readInt("Enter course number: ", 1, COURSE_COUNT);

    e.course = COURSES[courseChoice - 1];

    cout << "Enter Section (e.g., 3A): ";
    getline(cin, e.section);
    e.section = toUpper(e.section);

    if (e.section.empty()) {
        cout << "Section cannot be empty.\n";
        return;
    }

    showLocations();

    e.location = readInt("\nEnter Current Location (1-16): ", 1, LOCATIONS) - 1;
    e.evacuated = false;
    e.center = -1;

    evacuees.push_back(e);

    saveEvacuees();

    cout << "\nEvacuee registered successfully!\n";
}

void displayEvacuees() {

    cout << "\n========== EVACUEE RECORDS ==========\n";

    if (evacuees.empty()) {
        cout << "No registered evacuees.\n";
        return;
    }

    for (int i = 0; i < evacuees.size(); i++) {

        cout << "\nRecord #" << i + 1 << "\n";

        cout << "ID: "
             << evacuees[i].id << "\n";

        cout << "Name: "
             << evacuees[i].name << "\n";

        cout << "Course: "
             << evacuees[i].course << "\n";

        cout << "Section: "
             << evacuees[i].section << "\n";

        cout << "Location: "
             << places[evacuees[i].location] << "\n";

        cout << "Status: "
             << (evacuees[i].evacuated
                 ? "Evacuated"
                 : "Not Yet Confirmed Evacuated")
             << "\n";

        if (evacuees[i].center >= 0) {

            cout << "Center: "
                 << places[
                     centers[evacuees[i].center].location
                 ] << "\n";
        }
    }
}

void updateRouteStatus() {

    int a, b, status;

    showLocations();

    a = readInt("\nEnter first location number (1-16): ", 1, LOCATIONS);
    b = readInt("Enter second location number (1-16): ", 1, LOCATIONS);

    if (a == b) {
        cout << "Error: The two locations must be different.\n";
        return;
    }

    a--;
    b--;

    int edgeIndex = findEdge(a, b);

    if (edgeIndex == -1) {
        cout << "No direct connection between these locations.\n";
        return;
    }

    cout << "\n1. Block Route\n";
    cout << "2. Reopen Route\n";

    status = readInt("Enter choice (1-2): ", 1, 2);

    int reverseIndex = findEdge(b, a);

    graph[a][edgeIndex].blocked = (status == 1);
    graph[b][reverseIndex].blocked = (status == 1);

    cout << "\nRoute "
         << (status == 1 ? "blocked" : "reopened")
         << " successfully!\n";
}

void markEvacuated() {

    if (evacuees.empty()) {
        cout << "\nNo registered evacuees.\n";
        return;
    }

    string id;

    cout << "\nEnter Evacuee ID: ";
    getline(cin, id);

    for (Evacuee& e : evacuees) {

        if (e.id != id) {
            continue;
        }

        if (e.evacuated) {
            cout << "Already marked as evacuated.\n";
            return;
        }

        int centerChoice;

        cout << "\nSelect the center where the person arrived:\n";

        for (int i = 0; i < CENTER_COUNT; i++) {

            cout << i + 1 << ". "
                 << places[centers[i].location]
                 << " (Available: "
                 << centers[i].capacity
                        - centers[i].occupants
                 << ")\n";
        }

        centerChoice = readInt("Enter center number: ", 1, CENTER_COUNT);

        int selected = centerChoice - 1;

        if (centers[selected].occupants >=
            centers[selected].capacity) {

            cout << "Center is already full!\n";
            return;
        }

        e.evacuated = true;
        e.center = selected;

        centers[selected].occupants++;

        saveEvacuees();

        cout << "\nEvacuation status updated successfully!\n";
        cout << "Confirm the person's physical arrival first.\n";

        return;
    }

    cout << "Evacuee ID not found!\n";
}

void showCenterCapacity() {

    cout << "\n========== EVACUATION CENTER CAPACITY ==========\n";

    for (int i = 0; i < CENTER_COUNT; i++) {

        cout << "\nCenter: "
             << places[centers[i].location] << "\n";

        cout << "Capacity: "
             << centers[i].capacity << "\n";

        cout << "Recorded Occupants: "
             << centers[i].occupants << "\n";

        cout << "Available Spaces: "
             << centers[i].capacity
                    - centers[i].occupants << "\n";
    }
}

void showDepartmentSummary() {

    cout << "\n========== EVACUEES PER COURSE / SECTION ==========\n";

    // course -> section -> indexes of evacuees
    map<string, map<string, vector<int>>> groups;

    for (const string& c : COURSES) {
        groups[c];   // show every course, even with no evacuees
    }

    for (int i = 0; i < evacuees.size(); i++) {
        groups[evacuees[i].course][evacuees[i].section].push_back(i);
    }

    for (const auto& courseEntry : groups) {

        int courseTotal = 0;
        int courseEvacuated = 0;

        for (const auto& sectionEntry : courseEntry.second) {
            for (int idx : sectionEntry.second) {
                courseTotal++;
                if (evacuees[idx].evacuated) {
                    courseEvacuated++;
                }
            }
        }

        cout << "\n" << courseEntry.first
             << "  (Total: " << courseTotal
             << " | Evacuated: " << courseEvacuated
             << " | Not Yet Confirmed: "
             << courseTotal - courseEvacuated << ")\n";

        for (const auto& sectionEntry : courseEntry.second) {

            int total = sectionEntry.second.size();
            int done = 0;

            for (int idx : sectionEntry.second) {
                if (evacuees[idx].evacuated) {
                    done++;
                }
            }

            cout << "  Section " << sectionEntry.first
                 << ": " << done << "/" << total
                 << " evacuated\n";

            for (int idx : sectionEntry.second) {

                if (evacuees[idx].evacuated) {
                    continue;
                }

                cout << "     - NOT CONFIRMED: "
                     << evacuees[idx].name
                     << " (" << evacuees[idx].id << ")"
                     << " last at "
                     << places[evacuees[idx].location] << "\n";
            }
        }
    }
}

void deleteEvacuee() {

    if (evacuees.empty()) {
        cout << "\nNo registered evacuees.\n";
        return;
    }

    string id;

    cout << "\nEnter Evacuee ID to delete: ";
    getline(cin, id);

    for (int i = 0; i < (int)evacuees.size(); i++) {

        if (evacuees[i].id != id) {
            continue;
        }

        if (!askYesNo("Delete " + evacuees[i].name
                      + " (" + evacuees[i].course + " "
                      + evacuees[i].section + ")? (y/n): ")) {
            cout << "Deletion cancelled.\n";
            return;
        }

        // Free the space in the evacuation center
        if (evacuees[i].evacuated && evacuees[i].center >= 0) {
            centers[evacuees[i].center].occupants--;
        }

        evacuees.erase(evacuees.begin() + i);

        saveEvacuees();

        cout << "Record deleted successfully!\n";
        return;
    }

    cout << "Evacuee ID not found!\n";
}

void clearAllData() {

    if (evacuees.empty()) {
        cout << "\nThere is no saved data to delete.\n";
        return;
    }

    cout << "\nThis will permanently delete ALL "
         << evacuees.size() << " evacuee record(s).\n";
    cout << "Type YES to confirm: ";

    string confirm;
    getline(cin, confirm);

    if (confirm != "YES") {
        cout << "Cancelled. No data was deleted.\n";
        return;
    }

    evacuees.clear();

    for (int i = 0; i < CENTER_COUNT; i++) {
        centers[i].occupants = 0;
    }

    saveEvacuees();

    cout << "All saved data deleted.\n";
}

void showGraph() {

    cout << "\n========== WEIGHTED GRAPH ==========\n";
    cout << "Sample distances are in meters.\n";

    for (int i = 0; i < LOCATIONS; i++) {

        cout << "\n" << places[i] << ":\n";

        for (const Edge& edge : graph[i]) {

            cout << "  -> "
                 << places[edge.destination]
                 << " | Distance: "
                 << edge.distance << " m"
                 << " | Status: "
                 << (edge.blocked ? "BLOCKED" : "OPEN")
                 << "\n";
        }
    }
}

int main() {

    int choice;

    createGraph();
    loadEvacuees();

    do {

        cout << "\n";
        cout << "============================================\n";
        cout << "         NEUST EVACUATION SYSTEM\n";
        cout << "============================================\n";

        cout << "1. Show Visible Campus Map\n";
        cout << "2. Find Shortest Evacuation Route\n";
        cout << "3. Register Evacuee\n";
        cout << "4. Display Evacuees\n";
        cout << "5. Block or Reopen Route\n";
        cout << "6. Mark Evacuee as Evacuated\n";
        cout << "7. Show Evacuation Center Capacity\n";
        cout << "8. Show Campus Locations\n";
        cout << "9. Show Weighted Graph\n";
        cout << "10. Show Evacuees per Course/Section\n";
        cout << "11. Delete Evacuee Record\n";
        cout << "12. Delete All Saved Data\n";
        cout << "13. Exit\n";

        cout << "============================================\n";
        choice = readInt("Enter your choice: ", 1, 13);

        switch (choice) {

            case 1:
                showMap();
                break;

            case 2: {

                int start;

                showLocations();

                start = readInt(
                    "\nEnter starting location (1-16): ", 1, LOCATIONS
                );

                findEvacuationRoute(start - 1);

                break;
            }

            case 3:
                registerEvacuee();
                break;

            case 4:
                displayEvacuees();
                break;

            case 5:
                updateRouteStatus();
                break;

            case 6:
                markEvacuated();
                break;

            case 7:
                showCenterCapacity();
                break;

            case 8:
                showLocations();
                break;

            case 9:
                showGraph();
                break;

            case 10:
                showDepartmentSummary();
                break;

            case 11:
                deleteEvacuee();
                break;

            case 12:
                clearAllData();
                break;

            case 13:
                saveEvacuees();

                cout << "\nRecords saved.\n";
                cout << "Exiting NEUST Evacuation System.\n";
                break;

            default:
                cout << "Invalid choice! Please try again.\n";
        }

    } while (choice != 13);

    return 0;
}