// City Route Finder - starter code (C++17)
// Compile: g++ -std=c++17 -O2 main.cpp -o routefinder

#include <bits/stdc++.h>
using namespace std;

enum Metric { DISTANCE = 1, TIME = 2, COST = 3 };

struct City {
    int id;
    string name;
    double x, y;  // coordinates (used later for A*)
};

struct Edge {
    int to;
    double distance, time, cost;
    double weight(Metric m) const {
        return m == DISTANCE ? distance : (m == TIME ? time : cost);
    }
};

struct Result {
    bool found = false;
    vector<int> path;
    double total = 0;
    int visited = 0;  // cities expanded (for comparing algorithms)
};

// ---------------- Graph ----------------
class Graph {
public:
    unordered_map<int, City> cities;
    unordered_map<int, vector<Edge>> adj;
    int nextId = 0;

    int addCity(const string &name, double x = 0, double y = 0) {
        if (findId(name) != -1) return -1;  // duplicate name
        int id = nextId++;
        cities[id] = {id, name, x, y};
        adj[id];  // create empty list
        return id;
    }

    static string lower(string s) {
        for (auto &ch : s) ch = tolower((unsigned char)ch);
        return s;
    }

    // Case-insensitive: "delhi", "DELHI" and "Delhi" all work
    int findId(const string &name) const {
        for (auto &[id, c] : cities)
            if (lower(c.name) == lower(name)) return id;
        return -1;
    }

    bool addRoad(int a, int b, double d, double t, double c) {
        if (!cities.count(a) || !cities.count(b) || a == b) return false;
        for (auto &e : adj[a])
            if (e.to == b) return false;  // road already exists
        adj[a].push_back({b, d, t, c});
        adj[b].push_back({a, d, t, c});  // undirected
        return true;
    }

    bool removeRoad(int a, int b) {
        if (!cities.count(a) || !cities.count(b)) return false;
        auto rm = [](vector<Edge> &v, int to) {
            auto it = remove_if(v.begin(), v.end(),
                                [&](const Edge &e) { return e.to == to; });
            bool removed = it != v.end();
            v.erase(it, v.end());
            return removed;
        };
        bool r1 = rm(adj[a], b), r2 = rm(adj[b], a);
        return r1 || r2;
    }

    bool removeCity(int id) {
        if (!cities.count(id)) return false;
        for (auto &[u, edges] : adj)
            edges.erase(remove_if(edges.begin(), edges.end(),
                                  [&](const Edge &e) { return e.to == id; }),
                        edges.end());
        adj.erase(id);
        cities.erase(id);
        return true;
    }

    void listCities() const {
        if (cities.empty()) { cout << "No cities yet.\n"; return; }
        vector<int> ids;
        for (auto &[id, c] : cities) ids.push_back(id);
        sort(ids.begin(), ids.end());
        for (int id : ids)
            cout << "  [" << id << "] " << cities.at(id).name << "\n";
    }

    void listRoads() const {
        bool any = false;
        for (auto &[u, edges] : adj)
            for (auto &e : edges)
                if (u < e.to) {
                    any = true;
                    cout << "  " << cities.at(u).name << " <-> " << cities.at(e.to).name
                         << "  (" << e.distance << " km, " << e.time << " hr, Rs "
                         << e.cost << ")\n";
                }
        if (!any) cout << "No roads yet.\n";
    }

    // File format: one city per line "C name x y", one road per line "R nameA nameB d t c"
    // City names must be single words (use underscores).
    bool save(const string &file) const {
        ofstream out(file);
        if (!out) return false;
        for (auto &[id, c] : cities) out << "C " << c.name << " " << c.x << " " << c.y << "\n";
        for (auto &[u, edges] : adj)
            for (auto &e : edges)
                if (u < e.to)
                    out << "R " << cities.at(u).name << " " << cities.at(e.to).name << " "
                        << e.distance << " " << e.time << " " << e.cost << "\n";
        return true;
    }

    bool load(const string &file) {
        ifstream in(file);
        if (!in) return false;
        cities.clear(); adj.clear(); nextId = 0;
        string type;
        while (in >> type) {
            if (type == "C") {
                string name; double x, y;
                in >> name >> x >> y;
                addCity(name, x, y);
            } else if (type == "R") {
                string a, b; double d, t, c;
                in >> a >> b >> d >> t >> c;
                addRoad(findId(a), findId(b), d, t, c);
            }
        }
        return true;
    }
};

// ---------------- PathFinder ----------------
class PathFinder {
    static vector<int> buildPath(const unordered_map<int, int> &parent, int src, int dst) {
        vector<int> path;
        for (int v = dst; v != -1; v = parent.at(v)) {
            path.push_back(v);
            if (v == src) break;
        }
        reverse(path.begin(), path.end());
        return path;
    }

public:
    // Dijkstra: shortest path by chosen metric. O((V+E) log V)
    static Result dijkstra(const Graph &g, int src, int dst, Metric m,
                           const set<int> &avoid = {}) {
        unordered_map<int, double> dist;
        unordered_map<int, int> parent;
        for (auto &[id, c] : g.cities) { dist[id] = DBL_MAX; parent[id] = -1; }

        using P = pair<double, int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        dist[src] = 0;
        pq.push({0, src});
        int visited = 0;

        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > dist[u]) continue;
            visited++;
            if (u == dst) break;
            for (auto &e : g.adj.at(u)) {
                if (avoid.count(e.to)) continue;
                double nd = d + e.weight(m);
                if (nd < dist[e.to]) {
                    dist[e.to] = nd;
                    parent[e.to] = u;
                    pq.push({nd, e.to});
                }
            }
        }

        Result r;
        r.visited = visited;
        if (dist[dst] == DBL_MAX) return r;
        r.found = true;
        r.total = dist[dst];
        r.path = buildPath(parent, src, dst);
        return r;
    }

    // BFS: fewest stops (ignores weights). O(V+E)
    static Result bfs(const Graph &g, int src, int dst, const set<int> &avoid = {}) {
        unordered_map<int, int> parent;
        for (auto &[id, c] : g.cities) parent[id] = -1;
        unordered_set<int> seen = {src};
        queue<int> q;
        q.push(src);

        while (!q.empty()) {
            int u = q.front(); q.pop();
            if (u == dst) break;
            for (auto &e : g.adj.at(u)) {
                if (avoid.count(e.to) || seen.count(e.to)) continue;
                seen.insert(e.to);
                parent[e.to] = u;
                q.push(e.to);
            }
        }

        Result r;
        if (!seen.count(dst)) return r;
        r.found = true;
        r.path = buildPath(parent, src, dst);
        r.total = r.path.size() - 1;  // number of roads taken
        return r;
    }

    // Straight-line distance in km between two cities (x = longitude, y = latitude)
    static double haversine(const City &a, const City &b) {
        const double R = 6371.0, rad = 3.14159265358979323846 / 180.0;
        double dlat = (b.y - a.y) * rad, dlon = (b.x - a.x) * rad;
        double h = sin(dlat / 2) * sin(dlat / 2) +
                   cos(a.y * rad) * cos(b.y * rad) * sin(dlon / 2) * sin(dlon / 2);
        return 2 * R * asin(sqrt(h));
    }

    // A*: Dijkstra + a guess of the remaining distance (heuristic).
    // f = g (road km so far) + h (straight-line km to destination).
    // Works for DISTANCE only, because straight-line km never exceeds road km.
    static Result aStar(const Graph &g, int src, int dst, const set<int> &avoid = {}) {
        unordered_map<int, double> gs;
        unordered_map<int, int> parent;
        for (auto &[id, c] : g.cities) { gs[id] = DBL_MAX; parent[id] = -1; }

        using P = pair<double, int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        unordered_set<int> closed;
        gs[src] = 0;
        pq.push({haversine(g.cities.at(src), g.cities.at(dst)), src});
        int visited = 0;

        while (!pq.empty()) {
            auto [f, u] = pq.top(); pq.pop();
            if (closed.count(u)) continue;
            closed.insert(u);
            visited++;
            if (u == dst) break;
            for (auto &e : g.adj.at(u)) {
                if (avoid.count(e.to)) continue;
                double ng = gs[u] + e.distance;
                if (ng < gs[e.to]) {
                    gs[e.to] = ng;
                    parent[e.to] = u;
                    pq.push({ng + haversine(g.cities.at(e.to), g.cities.at(dst)), e.to});
                }
            }
        }

        Result r;
        r.visited = visited;
        if (gs[dst] == DBL_MAX) return r;
        r.found = true;
        r.total = gs[dst];
        r.path = buildPath(parent, src, dst);
        return r;
    }
};

// ---------------- Menu helpers ----------------
void printResult(const Graph &g, const Result &r, const string &unit) {
    if (!r.found) { cout << "No route found.\n"; return; }
    cout << "Route: ";
    for (size_t i = 0; i < r.path.size(); i++) {
        cout << g.cities.at(r.path[i]).name;
        if (i + 1 < r.path.size()) cout << " -> ";
    }
    cout << "\nTotal: " << r.total << " " << unit << "\n";
}

int readCity(const Graph &g, const string &prompt) {
    string name;
    cout << prompt;
    cin >> name;
    int id = g.findId(name);
    if (id == -1) cout << "City not found.\n";
    return id;
}

void loadSampleMap(Graph &g) {
    g = Graph();
    // name, x, y (rough coordinates, just for later A*)
    vector<tuple<string, double, double>> cs = {
        {"Delhi", 77.2, 28.6}, {"Jaipur", 75.8, 26.9}, {"Agra", 78.0, 27.2},
        {"Lucknow", 80.9, 26.8}, {"Mumbai", 72.8, 19.0}, {"Pune", 73.8, 18.5},
        {"Ahmedabad", 72.6, 23.0}};
    for (auto &[n, x, y] : cs) g.addCity(n, x, y);
    auto road = [&](string a, string b, double d, double t, double c) {
        g.addRoad(g.findId(a), g.findId(b), d, t, c);
    };
    road("Delhi", "Jaipur", 280, 5, 400);
    road("Delhi", "Agra", 230, 4, 350);
    road("Agra", "Jaipur", 240, 4.5, 300);
    road("Agra", "Lucknow", 330, 5.5, 450);
    road("Delhi", "Lucknow", 550, 8, 700);
    road("Jaipur", "Ahmedabad", 670, 10, 900);
    road("Ahmedabad", "Mumbai", 530, 8, 800);
    road("Mumbai", "Pune", 150, 3, 200);
    cout << "Sample map loaded.\n";
}

void menu() {
    cout << "\n===== CITY ROUTE FINDER =====\n"
         << " 1. Add city\n"
         << " 2. Add road\n"
         << " 3. Remove city\n"
         << " 4. Remove road\n"
         << " 5. List cities\n"
         << " 6. List roads\n"
         << " 7. Shortest route (Dijkstra)\n"
         << " 8. Fewest stops (BFS)\n"
         << " 9. Save map\n"
         << "10. Load map\n"
         << "11. Load sample map\n"
         << "12. Compare Dijkstra vs A* (distance)\n"
         << " 0. Exit\n"
         << "Choice: ";
}

int main() {
    Graph g;
    int choice;
    while (true) {
        menu();
        if (!(cin >> choice)) { cin.clear(); cin.ignore(1000, '\n'); continue; }

        if (choice == 0) break;

        if (choice == 1) {
            string name; double x, y;
            cout << "Name (one word): "; cin >> name;
            cout << "x y coordinates: "; cin >> x >> y;
            cout << (g.addCity(name, x, y) == -1 ? "City already exists.\n" : "City added.\n");
        } else if (choice == 2) {
            int a = readCity(g, "From: "); if (a == -1) continue;
            int b = readCity(g, "To: ");   if (b == -1) continue;
            double d, t, c;
            cout << "Distance(km) Time(hr) Cost(Rs): "; cin >> d >> t >> c;
            cout << (g.addRoad(a, b, d, t, c) ? "Road added.\n" : "Could not add road.\n");
        } else if (choice == 3) {
            int a = readCity(g, "City to remove: "); if (a == -1) continue;
            g.removeCity(a); cout << "City removed.\n";
        } else if (choice == 4) {
            int a = readCity(g, "From: "); if (a == -1) continue;
            int b = readCity(g, "To: ");   if (b == -1) continue;
            cout << (g.removeRoad(a, b) ? "Road removed.\n" : "No such road.\n");
        } else if (choice == 5) {
            g.listCities();
        } else if (choice == 6) {
            g.listRoads();
        } else if (choice == 7) {
            int a = readCity(g, "From: "); if (a == -1) continue;
            int b = readCity(g, "To: ");   if (b == -1) continue;
            int mt;
            cout << "Optimize by 1) distance 2) time 3) cost: "; cin >> mt;
            if (mt < 1 || mt > 3) { cout << "Invalid option.\n"; continue; }
            set<int> avoid;
            char yn;
            cout << "Avoid a city? (y/n): "; cin >> yn;
            if (yn == 'y' || yn == 'Y') {
                int av = readCity(g, "City to avoid: ");
                if (av != -1 && av != a && av != b) avoid.insert(av);
            }
            string unit = mt == 1 ? "km" : (mt == 2 ? "hr" : "Rs");
            printResult(g, PathFinder::dijkstra(g, a, b, (Metric)mt, avoid), unit);
        } else if (choice == 8) {
            int a = readCity(g, "From: "); if (a == -1) continue;
            int b = readCity(g, "To: ");   if (b == -1) continue;
            printResult(g, PathFinder::bfs(g, a, b), "road(s)");
        } else if (choice == 9) {
            string f; cout << "File name: "; cin >> f;
            cout << (g.save(f) ? "Saved.\n" : "Save failed.\n");
        } else if (choice == 10) {
            string f; cout << "File name: "; cin >> f;
            cout << (g.load(f) ? "Loaded.\n" : "File not found.\n");
        } else if (choice == 11) {
            loadSampleMap(g);
        } else if (choice == 12) {
            int a = readCity(g, "From: "); if (a == -1) continue;
            int b = readCity(g, "To: ");   if (b == -1) continue;
            Result d = PathFinder::dijkstra(g, a, b, DISTANCE);
            Result s = PathFinder::aStar(g, a, b);
            cout << "\n--- Dijkstra ---\n";
            printResult(g, d, "km");
            cout << "Cities visited: " << d.visited << "\n";
            cout << "\n--- A* ---\n";
            printResult(g, s, "km");
            cout << "Cities visited: " << s.visited << "\n";
            if (d.found && d.visited > 0)
                cout << "\nA* visited " << (100 * (d.visited - s.visited) / d.visited)
                     << "% fewer cities than Dijkstra.\n";
        } else {
            cout << "Invalid choice.\n";
        }
    }
    cout << "Goodbye!\n";
    return 0;
}
