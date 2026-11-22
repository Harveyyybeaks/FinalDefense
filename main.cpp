#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
using namespace std;

const int INF = 1000000;

struct Location {
    int id;
    string name;
    string type;
    string area;            // part of the campus (used to group the list)
    bool extinguisher;      // fire extinguisher nearby (map legend)
    bool alarm;             // fire emergency alarm nearby (map legend)
};

struct Edge {
    int to;           // connected location
    int distance;     // in meters
    bool available;   // true = open, false = blocked
};

vector<Location> locations;      // all vertices of the graph
vector< vector<Edge> > graph;    // adjacency list: graph[i] = paths of location i

string currentArea = "";         // area that addLocation() gives to new locations
int currentLocation = -1;        // where the user is right now (asked when the program starts)

// ----------------------------------------------------------------------------
//  BUILDING THE GRAPH
// ----------------------------------------------------------------------------

int addLocation(string name, string type) {
    Location loc;
    loc.id = locations.size();
    loc.name = name;
    loc.type = type;
    loc.area = currentArea;
    loc.extinguisher = false;
    loc.alarm = false;
    locations.push_back(loc);
    graph.push_back(vector<Edge>());     // empty list of paths for the new location
    return loc.id;
}

// A walkable path works both ways, so we store it in both locations.
void addPath(int a, int b, int meters) {
    Edge e1 = {b, meters, true};
    Edge e2 = {a, meters, true};
    graph[a].push_back(e1);
    graph[b].push_back(e2);
}

void markExtinguisher(int id) { locations[id].extinguisher = true; }
void markAlarm(int id)        { locations[id].alarm = true; }

bool isDestination(int id) {
    return locations[id].type == "Exit" || locations[id].type == "Evacuation Area";
}

// Returns true if a path between a and b exists, and changes its status.
bool setPathStatus(int a, int b, bool available) {
    bool found = false;
    for (int i = 0; i < (int)graph[a].size(); i++)
        if (graph[a][i].to == b) { graph[a][i].available = available; found = true; }
    for (int i = 0; i < (int)graph[b].size(); i++)
        if (graph[b][i].to == a) graph[b][i].available = available;
    return found;
}

bool isPathAvailable(int a, int b) {
    for (int i = 0; i < (int)graph[a].size(); i++)
        if (graph[a][i].to == b) return graph[a][i].available;
    return false;
}

int getDistance(int a, int b) {
    for (int i = 0; i < (int)graph[a].size(); i++)
        if (graph[a][i].to == b) return graph[a][i].distance;
    return 0;
}

// ----------------------------------------------------------------------------
//  MAP TREE  (general tree: one parent, many children)
//  Campus -> Building -> Hallway -> Rooms / Exits
//  Every node stores the INDEXES of its children in the 'tree' vector.
// ----------------------------------------------------------------------------
struct TreeNode {
    string label;           // text for group nodes (Campus, Buildings, ...)
    int locationId;         // linked graph location (-1 = group node only)
    int parent;             // index of the parent node (-1 = root)
    vector<int> children;   // indexes of the child nodes
};

vector<TreeNode> tree;

int addTreeNode(string label, int locationId, int parent) {
    TreeNode t;
    t.label = label;
    t.locationId = locationId;
    t.parent = parent;
    tree.push_back(t);
    int index = tree.size() - 1;
    if (parent != -1)
        tree[parent].children.push_back(index);   // attach to its parent
    return index;
}

int addGroup(string label, int parent) {          // Campus, building, ...
    return addTreeNode(label, -1, parent);
}

int addPlace(int locationId, int parent) {        // a real location from the graph
    return addTreeNode(locations[locationId].name, locationId, parent);
}

void buildCampus() {

    // ========================================================================
    //  LOCATIONS
    // ========================================================================

    // ----- West Building -----
    // Two columns of rooms with a long hallway in the middle and a cross
    // hallway (where the YOU ARE HERE marker is). W1-W4 = left column, top to
    // bottom. W5-W7 = right column, top to bottom. Comlab 2 = bottom right.
    currentArea = "WEST BUILDING";
    int wN  = addLocation("West Hallway North", "Hallway");
    int wM  = addLocation("West Hallway Middle", "Hallway");
    int wS  = addLocation("West Hallway South", "Hallway");
    int w1  = addLocation("Classroom W1", "Classroom");
    int w2  = addLocation("Classroom W2", "Classroom");
    int w3  = addLocation("Classroom W3", "Classroom");
    int w4  = addLocation("Classroom W4", "Classroom");
    int w5  = addLocation("Classroom W5", "Classroom");
    int w6  = addLocation("Classroom W6", "Classroom");
    int w7  = addLocation("Classroom W7", "Classroom");
    int cl2 = addLocation("Comlab 2", "Laboratory");
    int rW  = addLocation("Restroom - West Bldg", "Restroom");
    int xWN = addLocation("Exit - West Bldg North", "Exit");
    int xWS = addLocation("Exit - West Bldg South", "Exit");

    // ----- Annex Building + West Walkway -----
    // The narrow building between the West and Center buildings (its name is
    // not written on the plan). Its rooms open to the walkway on its east side,
    // which is the dotted green evacuation route on the map.
    currentArea = "ANNEX BUILDING AND WEST WALKWAY (green dotted route)";
    int anO = addLocation("Annex Office", "Office");            // top room, no label on the plan
    int an1 = addLocation("Classroom A1", "Classroom");
    int an2 = addLocation("Classroom A2", "Classroom");
    int edt = addLocation("EdTech Lab", "Laboratory");
    int anR = addLocation("Annex Room", "Room");                // bottom room, no label on the plan
    int gN  = addLocation("West Walkway North", "Walkway");
    int gM  = addLocation("West Walkway Middle", "Walkway");
    int gS  = addLocation("West Walkway South", "Walkway");
    int gC  = addLocation("West Walkway Corner", "Walkway");    // where the green route turns east

    // ----- Center Building: upper hallway (offices and classrooms 3 and 4) -----
    currentArea = "CENTER BUILDING";
    int uW   = addLocation("Upper Hallway West", "Hallway");
    int uC   = addLocation("Upper Hallway Center", "Hallway");
    int uM   = addLocation("Upper Hallway Middle", "Hallway");
    int uE   = addLocation("Upper Hallway East", "Hallway");
    int adm  = addLocation("Admin Office", "Office");
    int off  = addLocation("Office (next to Admin)", "Office");
    int reg  = addLocation("Register Office", "Office");
    int acc  = addLocation("Accre Office", "Office");
    int conf = addLocation("Conference Room", "Conference Room");
    int sp   = addLocation("Speech Lab", "Laboratory");
    int c4   = addLocation("Classroom 4", "Classroom");
    int c3   = addLocation("Classroom 3", "Classroom");

    // ----- Center Building: center hallway (library, comlab 1, classrooms) -----
    // E1 and E2 = the two classrooms on the east side of the center hallway.
    int vN  = addLocation("Center Hallway North", "Hallway");
    int vM  = addLocation("Center Hallway Middle", "Hallway");
    int vS  = addLocation("Center Hallway South", "Hallway");
    int lib = addLocation("Library", "Library");
    int cl1 = addLocation("Comlab 1", "Laboratory");
    int e1  = addLocation("Classroom E1", "Classroom");
    int e2  = addLocation("Classroom E2", "Classroom");

    // ----- Center Building: exits -----
    int xAN = addLocation("Exit - Admin North", "Exit");
    int xUW = addLocation("Exit - Upper Hallway West", "Exit");
    int xUE = addLocation("Exit - Upper Hallway East", "Exit");
    int xLW = addLocation("Exit - Library West", "Exit");
    int xCW = addLocation("Exit - Comlab 1 West", "Exit");
    int xVS = addLocation("Exit - Center Hallway South", "Exit");
    int xE1 = addLocation("Exit - Classroom E1 East", "Exit");
    int xE2 = addLocation("Exit - Classroom E2 East", "Exit");

    // ----- Stage and Basketball Court -----
    currentArea = "STAGE AND BASKETBALL COURT";
    int stg = addLocation("Stage", "Stage");
    int crt = addLocation("Basketball Court", "Court");
    int gEn = addLocation("Gym Entrance", "Entrance");     // marked ENTRANCE on the plan (not an emergency exit)
    int xGE = addLocation("Exit - Gym East", "Exit");
    int xGS = addLocation("Exit - Gym South", "Exit");

    // ----- Other buildings -----
    currentArea = "OTHER BUILDINGS";
    int fac = addLocation("Faculty Room", "Faculty Room");
    int rN  = addLocation("Restroom - North", "Restroom");
    int cH  = addLocation("Canteen Hallway", "Hallway");
    int can = addLocation("Canteen", "Canteen");

    // ----- Designated evacuation areas -----
    currentArea = "EVACUATION AREAS";
    int eaW = addLocation("Evacuation Area - West", "Evacuation Area");
    int eaC = addLocation("Evacuation Area - Center", "Evacuation Area");
    int eaE = addLocation("Evacuation Area - East", "Evacuation Area");

    // ----- Fire extinguishers and fire emergency alarms (map legend) -----
    markExtinguisher(wN);  markAlarm(wN);
    markExtinguisher(wM);  markAlarm(wM);
    markAlarm(wS);
    markExtinguisher(gN);
    markAlarm(uC);
    markExtinguisher(uM);
    markExtinguisher(uE);
    markExtinguisher(vM);
    markExtinguisher(vS);
    markExtinguisher(gEn);
    markExtinguisher(fac);
    markExtinguisher(can);

    // ========================================================================
    //  PATHS (distance in meters)
    // ========================================================================

    // ----- West Building -----
    addPath(w1, wN, 4);
    addPath(w5, wN, 4);
    addPath(w2, wM, 6);
    addPath(w6, wM, 5);
    addPath(w3, wM, 6);
    addPath(w7, wM, 6);
    addPath(w4, wS, 5);
    addPath(cl2, wS, 3);
    addPath(rW, wS, 7);
    addPath(wN, wM, 11);
    addPath(wM, wS, 11);
    addPath(wN, xWN, 5);
    addPath(wS, xWS, 4);
    addPath(xWN, eaW, 3);
    addPath(xWS, eaW, 34);

    // ----- Annex Building and the West Walkway (green dotted route) -----
    addPath(anO, gN, 6);
    addPath(an1, gM, 5);
    addPath(an2, gM, 8);
    addPath(edt, gS, 6);
    addPath(anR, gS, 10);
    addPath(gS, gM, 12);
    addPath(gM, gN, 6);
    addPath(gN, gC, 10);
    addPath(gC, eaC, 25);      // the green route turns east, then north to the center area
    addPath(gC, eaW, 21);
    addPath(xUW, gN, 2);       // the three west exits of the Center Building
    addPath(xLW, gM, 2);       // open onto the walkway
    addPath(xCW, gS, 1);

    // ----- Center Building - upper hallway -----
    addPath(adm, uW, 4);
    addPath(sp, uW, 3);
    addPath(off, uC, 5);
    addPath(reg, uC, 6);
    addPath(acc, uM, 4);
    addPath(c4, uM, 5);
    addPath(conf, uE, 5);
    addPath(c3, uE, 5);
    addPath(uW, uC, 6);
    addPath(uC, uM, 9);
    addPath(uM, uE, 11);
    addPath(uW, xUW, 4);
    addPath(uC, xAN, 8);
    addPath(uE, xUE, 4);
    addPath(xAN, eaC, 13);
    addPath(xUE, eaE, 18);
    addPath(xUE, eaC, 27);

    // ----- Center Building - center hallway -----
    addPath(uC, vN, 8);
    addPath(vN, vM, 9);
    addPath(vM, vS, 9);
    addPath(lib, vN, 5);
    addPath(e1, vM, 6);
    addPath(cl1, vS, 6);
    addPath(e2, vS, 5);
    addPath(vS, xVS, 3);
    addPath(lib, xLW, 7);
    addPath(cl1, xCW, 7);
    addPath(e1, xE1, 6);       // classrooms E1 and E2 also have their own east exit
    addPath(e2, xE2, 7);
    addPath(xE1, eaE, 10);
    addPath(xE2, eaE, 10);
    addPath(xVS, eaE, 27);

    // ----- Stage and Basketball Court -----
    addPath(stg, crt, 21);
    addPath(stg, gEn, 14);
    addPath(stg, xGE, 13);
    addPath(crt, gEn, 18);
    addPath(crt, xGE, 19);
    addPath(crt, xGS, 19);
    addPath(gEn, eaC, 26);
    addPath(gEn, eaE, 23);
    addPath(xGS, eaE, 21);
    addPath(xGE, eaC, 64);     // long walk around the stage

    // ----- Other buildings -----
    addPath(fac, eaC, 15);
    addPath(fac, xAN, 23);
    addPath(rN, eaC, 22);
    addPath(can, cH, 5);
    addPath(cH, eaE, 9);
    addPath(cH, xVS, 26);

    // ========================================================================
    //  MAP TREE (campus layout, used to visualize the map)
    //  Every child is directly connected (by a path) to its parent place.
    // ========================================================================
    int tCampus = addGroup("NEUST Papaya Off-Campus", -1);

    int tWest = addGroup("West Building", tCampus);
    int tWN = addPlace(wN, tWest);
    addPlace(w1, tWN);
    addPlace(w5, tWN);
    addPlace(xWN, tWN);
    int tWM = addPlace(wM, tWN);
    addPlace(w2, tWM);
    addPlace(w6, tWM);
    addPlace(w3, tWM);
    addPlace(w7, tWM);
    int tWS = addPlace(wS, tWM);
    addPlace(w4, tWS);
    addPlace(cl2, tWS);
    addPlace(rW, tWS);
    addPlace(xWS, tWS);

    int tAnnex = addGroup("Annex Building and West Walkway", tCampus);
    int tGN = addPlace(gN, tAnnex);
    addPlace(anO, tGN);
    addPlace(gC, tGN);
    int tGM = addPlace(gM, tGN);
    addPlace(an1, tGM);
    addPlace(an2, tGM);
    int tGS = addPlace(gS, tGM);
    addPlace(edt, tGS);
    addPlace(anR, tGS);

    int tCenter = addGroup("Center Building", tCampus);
    int tUW = addPlace(uW, tCenter);
    addPlace(adm, tUW);
    addPlace(sp, tUW);
    addPlace(xUW, tUW);
    int tUC = addPlace(uC, tUW);
    addPlace(off, tUC);
    addPlace(reg, tUC);
    addPlace(xAN, tUC);
    int tVN = addPlace(vN, tUC);
    int tLib = addPlace(lib, tVN);
    addPlace(xLW, tLib);
    int tVM = addPlace(vM, tVN);
    int tE1 = addPlace(e1, tVM);
    addPlace(xE1, tE1);
    int tVS = addPlace(vS, tVM);
    int tCl1 = addPlace(cl1, tVS);
    addPlace(xCW, tCl1);
    int tE2 = addPlace(e2, tVS);
    addPlace(xE2, tE2);
    addPlace(xVS, tVS);
    int tUM = addPlace(uM, tUC);
    addPlace(acc, tUM);
    addPlace(c4, tUM);
    int tUE = addPlace(uE, tUM);
    addPlace(conf, tUE);
    addPlace(c3, tUE);
    addPlace(xUE, tUE);

    int tGym = addGroup("Stage and Basketball Court", tCampus);
    int tCrt = addPlace(crt, tGym);
    addPlace(stg, tCrt);
    addPlace(gEn, tCrt);
    addPlace(xGE, tCrt);
    addPlace(xGS, tCrt);

    int tOther = addGroup("Other Buildings", tCampus);
    addPlace(fac, tOther);
    addPlace(rN, tOther);
    int tCH = addPlace(cH, tOther);
    addPlace(can, tCH);

    int tEvac = addGroup("Evacuation Areas", tCampus);
    addPlace(eaW, tEvac);
    addPlace(eaC, tEvac);
    addPlace(eaE, tEvac);
}

// ----------------------------------------------------------------------------
//  DIJKSTRA'S ALGORITHM
//
//  start         : the starting location
//  dist[i]       : shortest known distance from start to location i
//  prev[i]       : the location we came from to reach i (to rebuild the route)
//  avoidBlocked  : true  -> ignore blocked paths
//                  false -> pretend every path is open
//  skipNode      : a location that may NOT be used (-1 = none)
// ----------------------------------------------------------------------------
void dijkstra(int start, vector<int>& dist, vector<int>& prev,
              bool avoidBlocked, int skipNode) {
    int n = locations.size();
    vector<bool> visited(n, false);

    dist.assign(n, INF);       // Step 2: all other distances = infinity
    prev.assign(n, -1);
    dist[start] = 0;           // Step 1: starting node = 0

    for (int count = 0; count < n; count++) {

        // Step 3: select the unvisited node with the smallest known distance
        int u = -1;
        for (int i = 0; i < n; i++)
            if (!visited[i] && dist[i] < INF && (u == -1 || dist[i] < dist[u]))
                u = i;

        // Step 8: stop when no reachable nodes remain
        if (u == -1) break;
        visited[u] = true;

        // Step 4: examine the paths connected to u
        for (int k = 0; k < (int)graph[u].size(); k++) {
            Edge e = graph[u][k];

            // Step 5: ignore blocked paths (and the skipped location)
            if (avoidBlocked && !e.available) continue;
            if (e.to == skipNode) continue;

            // Step 6: New Distance = Current Distance + Edge Weight
            int newDist = dist[u] + e.distance;
            if (newDist < dist[e.to]) {
                dist[e.to] = newDist;
                prev[e.to] = u;          // Step 7: record the predecessor
            }
        }
    }
}

// Step 9: rebuild the route from the predecessor records
vector<int> buildPath(const vector<int>& prev, int dest) {
    vector<int> path;
    for (int v = dest; v != -1; v = prev[v])
        path.push_back(v);
    reverse(path.begin(), path.end());     // we collected it backwards
    return path;
}

bool isInPath(const vector<int>& path, int id) {
    for (int i = 0; i < (int)path.size(); i++)
        if (path[i] == id) return true;
    return false;
}

// Nearest reachable exit / evacuation area (locations in 'skip' are not allowed)
int nearestDestination(const vector<int>& dist, int start, const vector<int>& skip) {
    int best = -1;
    for (int i = 0; i < (int)locations.size(); i++) {
        if (i == start || !isDestination(i)) continue;
        if (dist[i] >= INF || isInPath(skip, i)) continue;
        if (best == -1 || dist[i] < dist[best]) best = i;
    }
    return best;
}

// Fire equipment tags shown beside a location on a route
string equipmentText(int id) {
    string text = "";
    if (locations[id].extinguisher) text += "  [Fire Ext.]";
    if (locations[id].alarm)        text += "  [Fire Alarm]";
    return text;
}

// Step 10: display the route and total distance
void printRoute(const vector<int>& path, int totalDistance) {
    int dest = path[path.size() - 1];
    cout << "  Destination    : " << locations[dest].name
         << " (" << locations[dest].type << ")\n";
    cout << "  Total distance : " << totalDistance << " meters\n";
    cout << "  Route:\n";
    for (int i = 0; i < (int)path.size(); i++) {
        cout << "    " << (i + 1) << ". " << locations[path[i]].name;
        if (i > 0) cout << "  (+" << getDistance(path[i - 1], path[i]) << " m)";
        cout << equipmentText(path[i]) << "\n";
    }
}

// ----------------------------------------------------------------------------
//  EVACUATION ROUTE
//  chosen = -1 -> nearest available exit / evacuation area
//  chosen >= 0 -> route to that exit / evacuation area
// ----------------------------------------------------------------------------
void findRoute(int start, int chosen) {
    cout << "\nStarting location: " << locations[start].name << "\n";

    if (isDestination(start) && (chosen == -1 || chosen == start)) {
        cout << "You are already at a safe point.\n";
        return;
    }

    vector<int> none;                         // empty list

    // Route using only the AVAILABLE paths
    vector<int> dist, prev;
    dijkstra(start, dist, prev, true, -1);
    int best = (chosen == -1) ? nearestDestination(dist, start, none) : chosen;
    bool routeFound = (best != -1 && dist[best] < INF);

    vector<int> path;
    if (routeFound) path = buildPath(prev, best);

    // Route if NOTHING were blocked (used to tell the user that a path is blocked)
    vector<int> distAll, prevAll;
    dijkstra(start, distAll, prevAll, false, -1);
    int usual = (chosen == -1) ? nearestDestination(distAll, start, none) : chosen;

    // Alternative route: another exit / evacuation area that does not pass
    // through the main destination and is not already on the main route
    int altDest = -1;
    vector<int> distAlt, prevAlt, altPath;
    if (routeFound || chosen != -1) {
        int skipNode = routeFound ? best : chosen;
        dijkstra(start, distAlt, prevAlt, true, skipNode);
        altDest = nearestDestination(distAlt, start, path);
        if (altDest != -1) altPath = buildPath(prevAlt, altDest);
    }

    // ----- show results -----
    if (routeFound) {
        if (usual != -1 && buildPath(prevAll, usual) != path)
            cout << "\n[!] The usual shortest route (" << distAll[usual] << " m to "
                 << locations[usual].name << ") is blocked. Route recalculated.\n";
        cout << "\n=== RECOMMENDED EVACUATION ROUTE ===\n";
        printRoute(path, dist[best]);
    } else if (chosen == -1) {
        cout << "\n[!] NO ROUTE AVAILABLE to any exit or evacuation area.\n";
    } else {
        cout << "\n[!] NO ROUTE AVAILABLE to " << locations[chosen].name << ".\n";
    }

    if (altDest != -1) {
        cout << "\n=== ALTERNATIVE ROUTE ===\n";
        printRoute(altPath, distAlt[altDest]);
    } else if (routeFound) {
        cout << "\nNo alternative route available.\n";
    }

    if (routeFound || altDest != -1)
        cout << "\n[Fire Ext.] = fire extinguisher nearby   [Fire Alarm] = fire emergency alarm nearby\n";

    if (!routeFound && altDest == -1)
        cout << "\nStay in a safe place and wait for instructions from school personnel.\n";
}

// ----------------------------------------------------------------------------
//  DISPLAY AND INPUT HELPERS
// ----------------------------------------------------------------------------
void printId(int i) {
    cout << "  [";
    if (i < 10) cout << " ";
    cout << i << "] ";
}

void listLocations() {
    cout << "\nLOCATIONS\n";
    string lastArea = "";
    for (int i = 0; i < (int)locations.size(); i++) {
        if (locations[i].area != lastArea) {          // new area -> print a heading
            cout << "\n  --- " << locations[i].area << " ---\n";
            lastArea = locations[i].area;
        }
        printId(i);
        cout << locations[i].name << " (" << locations[i].type << ")";
        if (i == currentLocation) cout << "   <== YOU ARE HERE";
        cout << "\n";
    }
}

void listDestinations() {
    cout << "\nEXITS AND EVACUATION AREAS\n";
    for (int i = 0; i < (int)locations.size(); i++)
        if (isDestination(i)) {
            printId(i);
            cout << locations[i].name << "\n";
        }
}

void listPaths() {
    cout << "\nPATHS\n";
    for (int a = 0; a < (int)locations.size(); a++)
        for (int k = 0; k < (int)graph[a].size(); k++) {
            Edge e = graph[a][k];
            if (a < e.to)    // print each path only once
                cout << "  [" << a << "] " << locations[a].name << " <-> ["
                     << e.to << "] " << locations[e.to].name << " : "
                     << e.distance << " m : "
                     << (e.available ? "AVAILABLE" : "BLOCKED") << "\n";
        }
}

// Text of one tree node. For a place that is connected to its parent place,
// also show the distance of that path and whether it is blocked.
string treeText(int node) {
    TreeNode t = tree[node];
    string text = t.label;
    if (t.locationId != -1 && t.parent != -1 && tree[t.parent].locationId != -1) {
        int a = tree[t.parent].locationId;
        int b = t.locationId;
        text += " (" + to_string(getDistance(a, b)) + " m)";
        if (!isPathAvailable(a, b)) text += " [BLOCKED]";
    }
    if (t.locationId == currentLocation) text += "   <== YOU ARE HERE";
    return text;
}

// RECURSIVE: print a node, then print each of its children one level deeper
void printTree(int node, string prefix, bool isLast) {
    cout << prefix << (isLast ? "`-- " : "|-- ") << treeText(node) << "\n";
    string childPrefix = prefix + (isLast ? "    " : "|   ");
    int count = tree[node].children.size();
    for (int i = 0; i < count; i++)
        printTree(tree[node].children[i], childPrefix, i == count - 1);
}

void showMapTree() {
    cout << "\nCAMPUS MAP TREE\n";
    cout << tree[0].label << "\n";                   // root = the whole campus
    int count = tree[0].children.size();
    for (int i = 0; i < count; i++)
        printTree(tree[0].children[i], "", i == count - 1);
    cout << "\n(x m) = distance from the location above it    [BLOCKED] = path is blocked\n";
}

int readInt(string prompt, int low, int high) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= low && value <= high)
            return value;
        if (cin.eof()) exit(0);           // input closed
        cout << "Invalid input. Enter a number from " << low << " to " << high << ".\n";
        cin.clear();
        cin.ignore(1000, '\n');
    }
}

// Ask the user where they are right now. The nearest route and the route to a
// chosen exit both start from this location.
void askCurrentLocation(int last) {
    listLocations();
    currentLocation = readInt("\nWhere are you right now? Enter your location ID: ", 0, last);
    cout << "\nYour current location: " << locations[currentLocation].name
         << " (" << locations[currentLocation].area << ")\n";
}

// ----------------------------------------------------------------------------
//  MAIN PROGRAM
// ----------------------------------------------------------------------------
int main() {
    buildCampus();
    int last = locations.size() - 1;

    cout << "==============================================================\n"
         << "  NEUST PAPAYA OFF-CAMPUS\n"
         << "  Emergency Evacuation Navigation System\n"
         << "==============================================================\n";

    // The system must know where the user is BEFORE anything else.
    askCurrentLocation(last);

    int choice = -1;
    while (choice != 0) {
        cout << "\n-------------------------- MENU --------------------------\n"
             << " Current location: " << locations[currentLocation].name << "\n"
             << "----------------------------------------------------------\n"
             << " [1] Find nearest evacuation route\n"
             << " [2] Route to a chosen exit / evacuation area\n"
             << " [3] Change my current location\n"
             << " [4] Show all locations\n"
             << " [5] Show campus map tree\n"
             << " [6] Show all paths and their status\n"
             << " [7] Block / unblock a path\n"
             << " [8] Reset all paths to AVAILABLE\n"
             << " [0] Quit\n";
        choice = readInt("Select option: ", 0, 8);

        if (choice == 1) {
            findRoute(currentLocation, -1);
        }
        else if (choice == 2) {
            listDestinations();
            int dest = readInt("\nEnter destination ID: ", 0, last);
            if (isDestination(dest))
                findRoute(currentLocation, dest);
            else
                cout << "That location is not an exit or evacuation area.\n";
        }
        else if (choice == 3) {
            askCurrentLocation(last);
        }
        else if (choice == 4) {
            listLocations();
        }
        else if (choice == 5) {
            showMapTree();
        }
        else if (choice == 6) {
            listPaths();
        }
        else if (choice == 7) {
            listPaths();
            int a = readInt("\nEnter ID of the first location: ", 0, last);
            int b = readInt("Enter ID of the second location: ", 0, last);
            if (graph[a].empty() || !setPathStatus(a, b, !isPathAvailable(a, b))) {
                cout << "There is no direct path between those locations.\n";
            } else {
                cout << locations[a].name << " <-> " << locations[b].name << " is now "
                     << (isPathAvailable(a, b) ? "AVAILABLE" : "BLOCKED") << ".\n";
            }
        }
        else if (choice == 8) {
            for (int a = 0; a < (int)locations.size(); a++)
                for (int k = 0; k < (int)graph[a].size(); k++)
                    graph[a][k].available = true;
            cout << "All paths are now AVAILABLE.\n";
        }
    }

    cout << "Stay safe. Goodbye!\n";
    return 0;
}
