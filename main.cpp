#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <fstream>
#include <limits>
#include <algorithm>
#include <iomanip>

using namespace std;

const int LOCATIONS = 16;
const int CENTER_COUNT = 2;
const int INF = numeric_limits<int>::max();

const string DATA_FILE = "evacuees.txt";

struct Edge {
    int destination;
    int distance;
    bool blocked;
};

struct Evacuee {
    string id;
    string name;
    int location;
    bool evacuated;
    int center;
};

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
             << e.center << "\n";
    }

    file.close();
}

void loadEvacuees() {

    ifstream file(DATA_FILE);

    if (!file) {
        return;
    }

    Evacuee e;

    while (file >> quoted(e.id)
                >> quoted(e.name)
                >> e.location
                >> e.evacuated
                >> e.center) {

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
    cin >> ws;
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

    showLocations();

    cout << "\nEnter Current Location (1-16): ";

    if (!(cin >> e.location)) {

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Invalid input!\n";
        return;
    }

    if (e.location < 1 || e.location > LOCATIONS) {
        cout << "Invalid location!\n";
        return;
    }

    e.location--;
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

    cout << "\nEnter first location number: ";

    if (!(cin >> a)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input!\n";
        return;
    }

    cout << "Enter second location number: ";

    if (!(cin >> b)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input!\n";
        return;
    }

    if (a < 1 || a > LOCATIONS ||
        b < 1 || b > LOCATIONS || a == b) {

        cout << "Invalid locations!\n";
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
    cout << "Enter choice: ";

    if (!(cin >> status)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input!\n";
        return;
    }

    if (status != 1 && status != 2) {
        cout << "Invalid choice!\n";
        return;
    }

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
    cin >> ws;
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

        cout << "Enter center number: ";

        if (!(cin >> centerChoice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input!\n";
            return;
        }

        if (centerChoice < 1 ||
            centerChoice > CENTER_COUNT) {

            cout << "Invalid center!\n";
            return;
        }

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
        cout << "10. Exit\n";

        cout << "============================================\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice)) {

            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Please enter a valid number!\n";
            continue;
        }

        switch (choice) {

            case 1:
                showMap();
                break;

            case 2: {

                int start;

                showLocations();

                cout << "\nEnter starting location (1-16): ";

                if (!(cin >> start)) {

                    cin.clear();
                    cin.ignore(
                        numeric_limits<streamsize>::max(), '\n'
                    );

                    cout << "Invalid input!\n";
                    break;
                }

                if (start < 1 || start > LOCATIONS) {

                    cout << "Invalid location!\n";

                } else {

                    findEvacuationRoute(start - 1);
                }

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
                saveEvacuees();

                cout << "\nRecords saved.\n";
                cout << "Exiting NEUST Evacuation System.\n";
                break;

            default:
                cout << "Invalid choice! Please try again.\n";
        }

    } while (choice != 10);

    return 0;
}