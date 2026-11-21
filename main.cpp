// ============================================================================
//  NEUST Papaya Off-Campus Emergency Evacuation Navigation System
//  Title Defense Project - BSIT-2A
//
//  Data structure : Weighted graph G = (V, E) stored as an adjacency list
//  Algorithm      : Dijkstra's shortest-path algorithm
//
//  Compile : g++ -std=c++17 -o evacuation evacuation_navigation.cpp
//  Run     : ./evacuation
//
//  CAMPUS DATA
//  The locations, exits, evacuation areas at path connection sa 
//  buildCampus() ang nakapaskil na "Emergency Evacuation Plan" ng campus.
//  Walang scale yung plano, so every distance is an ESTIMATE measured on the
//  plan photo (about 20 pixels = 1 meter, using the basketball court as the
//  reference).
// ============================================================================

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

static const double INF = numeric_limits<double>::infinity();

// ---------------------------------------------------------------------------
//  Location / Node Record:  Location ID, Location name, Location type
// ---------------------------------------------------------------------------
enum class LocationType {
    Classroom, Office, Laboratory, Library, FacultyRoom, ConferenceRoom,
    Canteen, Hallway, Walkway, Facility, Exit, EvacuationArea
};

string typeName(LocationType t) {
    switch (t) {
        case LocationType::Classroom:      return "Classroom";
        case LocationType::Office:         return "Office";
        case LocationType::Laboratory:     return "Laboratory";
        case LocationType::Library:        return "Library";
        case LocationType::FacultyRoom:    return "Faculty Room";
        case LocationType::ConferenceRoom: return "Conference Room";
        case LocationType::Canteen:        return "Canteen";
        case LocationType::Hallway:        return "Hallway";
        case LocationType::Walkway:        return "Walkway";
        case LocationType::Facility:       return "Facility";
        case LocationType::Exit:           return "Emergency Exit";
        case LocationType::EvacuationArea: return "Evacuation Area";
    }
    return "Unknown";
}

struct Location {
    int id;
    string name;
    LocationType type;
};

// ---------------------------------------------------------------------------
//  Path / Edge Record:  Connected location, Distance, Availability status
// ---------------------------------------------------------------------------
struct Edge {
    int to;
    double distance;   // meters
    bool available;    // false = blocked
};

struct SearchResult {          // output of Dijkstra
    vector<double> dist;
    vector<int> prev;          // predecessor of each node
};

struct Route {
    bool found = false;
    int destination = -1;
    double distance = INF;
    vector<int> path;
};

// -----------------------------------
//  Campus graph (adjacency list)
// -----------------------------------
class CampusGraph {
public:
    int addLocation(const string& name, LocationType type) {
        int id = static_cast<int>(nodes_.size());
        nodes_.push_back({id, name, type});
        adj_.emplace_back();
        return id;
    }

    // Walkable paths go both ways, kaya tig-isang edge record ang sine-save kada direksyon.
    void addPath(int a, int b, double meters) {
        adj_[a].push_back({b, meters, true});
        adj_[b].push_back({a, meters, true});
    }

    int size() const { return static_cast<int>(nodes_.size()); }
    const Location& location(int id) const { return nodes_[id]; }
    const vector<Edge>& neighbors(int id) const { return adj_[id]; }

    bool isDestination(int id) const {
        return nodes_[id].type == LocationType::Exit ||
               nodes_[id].type == LocationType::EvacuationArea;
    }

    const Edge* findEdge(int a, int b) const {
        for (const Edge& e : adj_[a])
            if (e.to == b) return &e;
        return nullptr;
    }

    // Every walkable path listed once (a < b).
    vector<pair<int, int>> allPaths() const {
        vector<pair<int, int>> list;
        for (int a = 0; a < size(); ++a)
            for (const Edge& e : adj_[a])
                if (a < e.to) list.push_back({a, e.to});
        return list;
    }

    void setPathStatus(int a, int b, bool available) {
        for (Edge& e : adj_[a]) if (e.to == b) e.available = available;
        for (Edge& e : adj_[b]) if (e.to == a) e.available = available;
    }

    // Blocks/unblocks every path that touches one location (e.g. a blocked exit).
    void setLocationStatus(int id, bool available) {
        for (const Edge& e : adj_[id]) setPathStatus(id, e.to, available);
    }

    void resetAllPaths() {
        for (auto& list : adj_)
            for (Edge& e : list) e.available = true;
    }

    // ----------------------- Dijkstra's algorithm --------------------------
    //  respectBlocked : ignore blocked/unavailable connections
    //  target         : stop once this node is settled (-1 = compute all)
    //  excluded       : node that may not be used (-1 = none)
    SearchResult dijkstra(int start, bool respectBlocked,
                          int target = -1, int excluded = -1) const {
        int n = size();
        SearchResult r{vector<double>(n, INF), vector<int>(n, -1)};  // steps 1-2
        vector<bool> visited(n, false);
        using Item = pair<double, int>;
        priority_queue<Item, vector<Item>, greater<Item>> pq;

        r.dist[start] = 0;
        pq.push({0, start});

        while (!pq.empty()) {
            auto [d, u] = pq.top();                    // step 3: smallest distance
            pq.pop();
            if (visited[u]) continue;
            visited[u] = true;
            if (u == target) break;                    // step 8: destination reached

            for (const Edge& e : adj_[u]) {            // step 4: examine paths
                if (respectBlocked && !e.available) continue;   // step 5
                if (e.to == excluded) continue;
                double newDist = d + e.distance;       // New = Current + Weight
                if (newDist < r.dist[e.to]) {          // step 6
                    r.dist[e.to] = newDist;
                    r.prev[e.to] = u;                  // step 7
                    pq.push({newDist, e.to});
                }
            }
        }
        return r;
    }

private:
    vector<Location> nodes_;
    vector<vector<Edge>> adj_;
};

// --------------------------------
//  Route helpers
// --------------------------------
Route makeRoute(const SearchResult& r, int dest) {         // step 9: reconstruct
    Route route;
    if (dest < 0 || r.dist[dest] == INF) return route;
    route.found = true;
    route.destination = dest;
    route.distance = r.dist[dest];
    for (int v = dest; v != -1; v = r.prev[v]) route.path.push_back(v);
    reverse(route.path.begin(), route.path.end());
    return route;
}

// Pinakamalapit na madadaanang exit o evacuation area. Nodes in `skip` are not candidates.
Route nearestDestination(const CampusGraph& g, const SearchResult& r, int start,
                         const vector<int>& skip = {}) {
    int best = -1;
    for (int i = 0; i < g.size(); ++i) {
        if (i == start || !g.isDestination(i) || r.dist[i] == INF) continue;
        if (find(skip.begin(), skip.end(), i) != skip.end()) continue;
        if (best == -1 || r.dist[i] < r.dist[best]) best = i;
    }
    return makeRoute(r, best);
}

void printRoute(const CampusGraph& g, const Route& route) {   // step 10: display
    const Location& dest = g.location(route.destination);
    cout << "  Selected destination : " << dest.name
         << " (" << typeName(dest.type) << ")\n";
    cout << fixed << setprecision(1);
    cout << "  Total distance       : " << route.distance << " meters\n";
    cout << "  Route (in order):\n";
    for (size_t i = 0; i < route.path.size(); ++i) {
        const Location& loc = g.location(route.path[i]);
        cout << "    " << (i + 1) << ". " << loc.name << " [" << typeName(loc.type) << "]";
        if (i > 0) {
            const Edge* e = g.findEdge(route.path[i - 1], route.path[i]);
            if (e) cout << "  (+" << e->distance << " m)";
        }
        cout << "\n";
    }
}

// ---------------------------------------------------------------------------
//  Main evacuation feature
//  chosen = -1  -> automatically pick the nearest available exit / evac area
//  chosen >= 0  -> route to that specific destination
// ---------------------------------------------------------------------------
void showEvacuationRoute(const CampusGraph& g, int start, int chosen) {
    const Location& s = g.location(start);
    cout << "\nStarting location: " << s.name << " (" << typeName(s.type) << ")\n";

    if (g.isDestination(start) && (chosen == -1 || chosen == start)) {
        cout << "You are already at a safe point. Proceed to the nearest evacuation area.\n";
        return;
    }

    SearchResult usable = g.dijkstra(start, true);    // blocked paths excluded
    SearchResult ideal  = g.dijkstra(start, false);   // pretend all paths are open

    Route normal  = (chosen == -1) ? nearestDestination(g, ideal, start)
                                   : makeRoute(ideal, chosen);
    Route primary = (chosen == -1) ? nearestDestination(g, usable, start)
                                   : makeRoute(usable, chosen);

    // Alternative route: Papunta sa ibang exit o lugar. Posibleng hindi nito dadaanin ang main destination, 
    // at hindi na isasama ang mga exit na nasa main route na.
    // (that would only be a shorter piece of the same route).
    Route alt;
    int excluded = primary.found ? primary.destination : chosen;
    if (primary.found || chosen != -1) {
        SearchResult altRes = g.dijkstra(start, true, -1, excluded);
        alt = nearestDestination(g, altRes, start,
                                 primary.found ? primary.path : vector<int>{});
    }

    if (primary.found) {
        if (normal.found && normal.path != primary.path) {
            cout << "\n[!] The usual shortest route ("
                 << fixed << setprecision(1) << normal.distance << " m to "
                 << g.location(normal.destination).name
                 << ") is unavailable due to blocked path(s). Route recalculated.\n";
        }
        cout << "\n=== RECOMMENDED EVACUATION ROUTE ===\n";
        printRoute(g, primary);
    } else {
        cout << "\n[!] NO ROUTE AVAILABLE to "
             << (chosen == -1 ? string("any exit or evacuation area")
                              : g.location(chosen).name)
             << ". All connecting paths are blocked.\n";
    }

    if (alt.found) {
        cout << "\n=== ALTERNATIVE ROUTE ===\n";
        printRoute(g, alt);
    } else if (primary.found) {
        cout << "\nNo alternative route available (other exits are unreachable).\n";
    }

    if (!primary.found && !alt.found) {
        cout << "\nNo safe route found. Stay calm, remain in a safe area, and wait for\n"
                "instructions from authorized school personnel.\n";
    }
}

// ------------------------------------
//  Console input / display helpers
// ------------------------------------
string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        cout << "\nInput closed. Exiting.\n";
        exit(0);
    }
    size_t b = line.find_first_not_of(" \t\r\n");
    size_t e = line.find_last_not_of(" \t\r\n");
    return (b == string::npos) ? "" : line.substr(b, e - b + 1);
}

int readInt(const string& prompt, int lo, int hi) {
    while (true) {
        istringstream iss(readLine(prompt));
        int v;
        char extra;
        if (iss >> v && !(iss >> extra) && v >= lo && v <= hi) return v;
        cout << "Invalid input. Enter a number from " << lo << " to " << hi << ".\n";
    }
}

string lowerCase(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return s;
}

void listLocations(const CampusGraph& g) {
    cout << "\n ID  Location                           Type\n"
            "---- ---------------------------------- ----------------------\n";
    for (int i = 0; i < g.size(); ++i) {
        const Location& l = g.location(i);
        cout << setw(3) << i << "  " << left << setw(34) << l.name
             << " " << typeName(l.type) << right << "\n";
    }
}

void listPathsOf(const CampusGraph& g, int id) {
    cout << "\nPaths connected to " << g.location(id).name << ":\n" << fixed << setprecision(1);
    const vector<Edge>& list = g.neighbors(id);
    for (size_t i = 0; i < list.size(); ++i)
        cout << "  [" << (i + 1) << "] " << left << setw(34) << g.location(list[i].to).name
             << right << setw(6) << list[i].distance << " m  "
             << (list[i].available ? "AVAILABLE" : "BLOCKED") << "\n";
}

void listAllPaths(const CampusGraph& g) {
    cout << "\nAll walkable paths:\n" << fixed << setprecision(1);
    for (const auto& p : g.allPaths()) {
        const Edge* e = g.findEdge(p.first, p.second);
        string label = g.location(p.first).name + " <-> " + g.location(p.second).name;
        cout << "  " << left << setw(64) << label << right << setw(6) << e->distance
             << " m  " << (e->available ? "AVAILABLE" : "BLOCKED") << "\n";
    }
}

// Pick a location by ID or by (part of) its name.  "?" lists every location.
int readLocation(const CampusGraph& g, const string& prompt) {
    while (true) {
        string line = readLine(prompt + " (ID or name, ? = list all): ");
        if (line.empty()) continue;
        if (line == "?") { listLocations(g); continue; }

        if (all_of(line.begin(), line.end(), [](unsigned char c) { return isdigit(c); })
            && line.size() <= 6) {
            int id = stoi(line);
            if (id >= 0 && id < g.size()) return id;
            cout << "No location with that ID.\n";
            continue;
        }

        string key = lowerCase(line);
        vector<int> matches;
        for (int i = 0; i < g.size(); ++i) {
            string nm = lowerCase(g.location(i).name);
            if (nm == key) return i;                       // exact name wins
            if (nm.find(key) != string::npos) matches.push_back(i);
        }
        if (matches.size() == 1) return matches[0];
        if (matches.empty()) {
            cout << "No location matches \"" << line << "\".\n";
        } else {
            cout << "Several locations match. Type the ID or a longer name:\n";
            for (int m : matches)
                cout << "  " << setw(3) << m << "  " << g.location(m).name << "\n";
        }
    }
}

// ------------------------
//  CAMPUS DATA 
// ------------------------
CampusGraph buildCampus() {
    CampusGraph g;
    using T = LocationType;

    // ---- West Building (Building A) ----
    int a1 = g.addLocation("Classroom A1", T::Classroom);
    int a2 = g.addLocation("Classroom A2", T::Classroom);
    int a3 = g.addLocation("Classroom A3", T::Classroom);
    int a4 = g.addLocation("Classroom A4", T::Classroom);
    int a5 = g.addLocation("Classroom A5", T::Classroom);
    int a6 = g.addLocation("Classroom A6", T::Classroom);
    int a7 = g.addLocation("Classroom A7", T::Classroom);
    int cl2 = g.addLocation("Comlab 2", T::Laboratory);
    int rrW = g.addLocation("Restroom (West)", T::Facility);
    int hwA = g.addLocation("West Hallway A", T::Hallway);
    int hwB = g.addLocation("West Hallway B", T::Hallway);
    int hwC = g.addLocation("West Hallway C (Cross)", T::Hallway);
    int hwD = g.addLocation("West Hallway D", T::Hallway);
    int hwE = g.addLocation("West Hallway E", T::Hallway);
    int exWN = g.addLocation("Exit - West Bldg North", T::Exit);
    int exWS = g.addLocation("Exit - West Bldg South", T::Exit);

    // ---- Building B (beside the evacuation walkway) ----
    int b1 = g.addLocation("Room B1 (unlabeled)", T::Facility);
    int b2 = g.addLocation("Classroom B2", T::Classroom);
    int b3 = g.addLocation("Classroom B3", T::Classroom);
    int edt = g.addLocation("EdTech Lab", T::Laboratory);
    int b4 = g.addLocation("Room B4 (unlabeled)", T::Facility);

    // ---- Evacuation walkway (green dotted path on the plan) ----
    int wk0 = g.addLocation("Evac Walkway Junction", T::Walkway);
    int wk1 = g.addLocation("Evac Walkway 1", T::Walkway);
    int wk2 = g.addLocation("Evac Walkway 2", T::Walkway);
    int wk3 = g.addLocation("Evac Walkway 3", T::Walkway);
    int wk4 = g.addLocation("Evac Walkway 4", T::Walkway);
    int wk5 = g.addLocation("Evac Walkway 5", T::Walkway);
    int wkN = g.addLocation("Evac Walkway North Bend", T::Walkway);

    // ---- Center Building (Building C) ----
    int adm = g.addLocation("Admin Office", T::Office);
    int off = g.addLocation("Office", T::Office);
    int reg = g.addLocation("Register Office", T::Office);
    int acc = g.addLocation("Accre Office", T::Office);
    int conf = g.addLocation("Conference Room", T::ConferenceRoom);
    int spl = g.addLocation("Speech Lab", T::Laboratory);
    int c4 = g.addLocation("Classroom 4", T::Classroom);
    int c3 = g.addLocation("Classroom 3", T::Classroom);
    int lib = g.addLocation("Library", T::Library);
    int cl1 = g.addLocation("Comlab 1", T::Laboratory);
    int e1 = g.addLocation("Classroom E1", T::Classroom);
    int e2 = g.addLocation("Classroom E2", T::Classroom);
    int uA = g.addLocation("Upper Hallway A", T::Hallway);
    int uB = g.addLocation("Upper Hallway B", T::Hallway);
    int uC = g.addLocation("Upper Hallway C", T::Hallway);
    int uD = g.addLocation("Upper Hallway D", T::Hallway);
    int uE = g.addLocation("Upper Hallway E", T::Hallway);
    int uF = g.addLocation("Upper Hallway F", T::Hallway);
    int uG = g.addLocation("Upper Hallway G", T::Hallway);
    int vA = g.addLocation("Center Hallway A", T::Hallway);
    int vB = g.addLocation("Center Hallway B", T::Hallway);
    int vC = g.addLocation("Center Hallway C", T::Hallway);
    int xCN = g.addLocation("Exit - Admin North", T::Exit);
    int xCW1 = g.addLocation("Exit - Upper Hallway West", T::Exit);
    int xCW2 = g.addLocation("Exit - Library West", T::Exit);
    int xCW3 = g.addLocation("Exit - Comlab 1 West", T::Exit);
    int xCE1 = g.addLocation("Exit - Upper Hallway East", T::Exit);
    int xCE2 = g.addLocation("Exit - Classroom E1 East", T::Exit);
    int xCE3 = g.addLocation("Exit - Classroom E2 East", T::Exit);
    int xCS = g.addLocation("Exit - Center Hallway South", T::Exit);

    // ---- Canteen, Faculty Room, Restroom (North) ----
    int can = g.addLocation("Canteen", T::Canteen);
    int cnh = g.addLocation("Canteen Hallway", T::Hallway);
    int fac = g.addLocation("Faculty Room", T::FacultyRoom);
    int rrN = g.addLocation("Restroom (North)", T::Facility);

    // ---- Stage and Basketball Court ----
    int stg = g.addLocation("Stage", T::Facility);
    int crt = g.addLocation("Basketball Court", T::Facility);
    int enc = g.addLocation("Court Entrance (Exit)", T::Exit);
    int xK1 = g.addLocation("Exit - Court Northeast", T::Exit);
    int xK2 = g.addLocation("Exit - Court South", T::Exit);

    // ---- Designated evacuation areas ----
    int eaW = g.addLocation("Evacuation Area - West", T::EvacuationArea);
    int eaC = g.addLocation("Evacuation Area - Center", T::EvacuationArea);
    int eaE = g.addLocation("Evacuation Area - East", T::EvacuationArea);

    // ---- West Building: rooms <-> hallway ----
    g.addPath(a1, hwA, 4);
    g.addPath(a2, hwA, 4);
    g.addPath(a3, hwB, 4);
    g.addPath(a4, hwB, 4);
    g.addPath(a5, hwD, 5);
    g.addPath(a6, hwD, 4);
    g.addPath(a7, hwE, 5);
    g.addPath(cl2, hwE, 4);
    g.addPath(rrW, hwE, 9);
    g.addPath(rrW, exWS, 5);

    // ---- West Building: hallway chain and exits ----
    g.addPath(hwA, hwB, 6);
    g.addPath(hwB, hwC, 4);
    g.addPath(hwC, hwD, 4);
    g.addPath(hwD, hwE, 7);
    g.addPath(hwA, exWN, 5);
    g.addPath(hwE, exWS, 4);
    g.addPath(exWN, eaW, 3);
    g.addPath(exWS, eaW, 34);

    // ---- Evacuation walkway and Building B ----
    g.addPath(wk0, wk1, 5);
    g.addPath(wk1, wk2, 5);
    g.addPath(wk2, wk3, 6);
    g.addPath(wk3, wk4, 6);
    g.addPath(wk4, wk5, 5);
    g.addPath(wk0, wkN, 8);
    g.addPath(wk0, eaW, 22);
    g.addPath(wkN, eaC, 19);
    g.addPath(b1, wk1, 5);   
    g.addPath(b2, wk3, 6);
    g.addPath(b3, wk4, 6);
    g.addPath(edt, wk5, 7);
    g.addPath(b4, wk5, 14);

    // ---- Center Building: upper hallway ----
    g.addPath(uA, uB, 4);
    g.addPath(uB, uC, 3);
    g.addPath(uC, uD, 4);
    g.addPath(uD, uE, 5);
    g.addPath(uE, uF, 7);
    g.addPath(uF, uG, 6);
    g.addPath(adm, uA, 5);
    g.addPath(off, uB, 4);
    g.addPath(spl, uB, 5);
    g.addPath(reg, uD, 4);
    g.addPath(c4, uD, 6);
    g.addPath(acc, uE, 4);
    g.addPath(conf, uF, 4);
    g.addPath(c3, uF, 4);

    // ---- Center Building: center hallway ----
    g.addPath(uC, vA, 12);
    g.addPath(vA, vB, 9);
    g.addPath(vB, vC, 7);
    g.addPath(lib, vA, 5);
    g.addPath(e1, vA, 5);
    g.addPath(cl1, vB, 6);
    g.addPath(e2, vB, 6);

    // ---- Center Building: exits ----
    g.addPath(xCN, uC, 8);
    g.addPath(xCW1, uA, 3);
    g.addPath(xCW2, lib, 11);
    g.addPath(xCW3, cl1, 10);
    g.addPath(xCE1, uG, 2);
    g.addPath(xCE2, e1, 9);
    g.addPath(xCE3, e2, 10);
    g.addPath(xCS, vC, 2);
    g.addPath(xCW1, wk2, 2);
    g.addPath(xCW2, wk3, 2);
    g.addPath(xCW3, wk5, 2);
    g.addPath(xCN, eaC, 18);
    g.addPath(xCE1, eaE, 18);
    g.addPath(xCE2, eaE, 13);
    g.addPath(xCE3, eaE, 14);
    g.addPath(xCS, eaE, 35);

    // ---- Canteen, Faculty Room, Restroom (North) ----
    g.addPath(can, cnh, 8);
    g.addPath(cnh, eaE, 9);
    g.addPath(cnh, xCE3, 13);
    g.addPath(fac, eaC, 19);
    g.addPath(fac, xCN, 26);
    g.addPath(rrN, eaC, 27);
    g.addPath(rrN, enc, 26);

    // ---- Stage and Basketball Court ----
    g.addPath(stg, crt, 19);
    g.addPath(stg, xK1, 19);
    g.addPath(crt, xK1, 23);
    g.addPath(crt, xK2, 29);
    g.addPath(crt, enc, 21);
    g.addPath(stg, enc, 22);
    g.addPath(enc, eaC, 36);
    g.addPath(enc, eaE, 33);
    g.addPath(xK1, eaE, 58);
    g.addPath(xK2, eaE, 29);

    return g;
}

// ---------------------------------------------------------------------------
//  Program entry
// ---------------------------------------------------------------------------
int main() {
    CampusGraph campus = buildCampus();

    cout << "==============================================================\n"
            "  NUEVA ECIJA UNIVERSITY OF SCIENCE AND TECHNOLOGY\n"
            "  PAPAYA OFF-CAMPUS\n"
            "  Emergency Evacuation Navigation System\n"
            "==============================================================\n";

    while (true) {
        cout << "\n------------------------- MAIN MENU -------------------------\n"
                " [1] Find nearest evacuation route\n"
                " [2] Route to a chosen exit / evacuation area\n"
                " [3] Show all locations\n"
                " [4] Show all paths and their status\n"
                " [5] Block / unblock a path\n"
                " [6] Block / unblock all paths of a location (e.g. an exit)\n"
                " [7] Reset all paths to AVAILABLE\n"
                " [0] Quit\n";
        int choice = readInt("Select option: ", 0, 7);

        if (choice == 0) {
            cout << "Stay safe. Goodbye!\n";
            break;
        }

        if (choice == 1) {
            int start = readLocation(campus, "\nYour current location");
            showEvacuationRoute(campus, start, -1);

        } else if (choice == 2) {
            int start = readLocation(campus, "\nYour current location");
            int dest = readLocation(campus, "\nDestination exit / evacuation area");
            if (!campus.isDestination(dest)) {
                cout << "That location is not an exit or evacuation area.\n";
            } else {
                showEvacuationRoute(campus, start, dest);
            }

        } else if (choice == 3) {
            listLocations(campus);

        } else if (choice == 4) {
            listAllPaths(campus);

        } else if (choice == 5) {
            int id = readLocation(campus, "\nFirst location of the path");
            if (campus.neighbors(id).empty()) {
                cout << "That location has no connected paths.\n";
            } else {
                listPathsOf(campus, id);
                int n = readInt("Path number to block/unblock (0 = cancel): ", 0,
                                static_cast<int>(campus.neighbors(id).size()));
                if (n != 0) {
                    const Edge e = campus.neighbors(id)[n - 1];
                    campus.setPathStatus(id, e.to, !e.available);
                    cout << campus.location(id).name << " <-> " << campus.location(e.to).name
                         << " is now " << (e.available ? "BLOCKED" : "AVAILABLE") << ".\n";
                }
            }

        } else if (choice == 6) {
            int id = readLocation(campus, "\nLocation");
            int action = readInt("[1] Block all its paths  [2] Unblock all its paths: ", 1, 2);
            campus.setLocationStatus(id, action == 2);
            cout << "All paths of " << campus.location(id).name << " are now "
                 << (action == 2 ? "AVAILABLE" : "BLOCKED") << ".\n";

        } else if (choice == 7) {
            campus.resetAllPaths();
            cout << "All paths are now AVAILABLE.\n";
        }
    }
    return 0;
}
    