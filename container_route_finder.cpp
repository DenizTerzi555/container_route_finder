#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <limits>
#include <algorithm>

using namespace std;

struct Service {
    int to;         // index of the destination port
    double cost;    // e.g. USD per container
    double days;    // transit time in days
    int perWeek;    // weekly frequency of the service
};

struct Route {
    vector<int> ports; // indices of the ports in the route
    double totalCost = 0;
    double totalDays = 0;
    int legs = 0;
};

vector<string> portNames = {
    "Izmir", "Istanbul", "Mersin", "Piraeus", "Thessaloniki",
    "Valencia", "Barcelona", "Marseille", "Genoa", "Gioia Tauro",
    "Malta", "Port Said", "Alexandria", "Haifa", "Limassol",
    "Constanta", "Odesa", "Rotterdam", "Hamburg", "Algeciras"
};

vector<vector<Service>> graph;

void addService(int from, int to, double cost, double days, int perWeek) {
    graph[from].push_back({to, cost, days, perWeek});
    graph[to].push_back({from, cost, days, perWeek}); // assuming bidirectional service
}

double waitingDays(int perWeek) {
    if (perWeek <= 0) return numeric_limits<double>::infinity(); // no service
    return 7.0 / perWeek / 2; // average waiting time in days
}


void fillTotals(Route& r) {
    r.totalCost = 0;
    r.totalDays = 0;
    for (size_t i = 0; i + 1 < r.ports.size(); ++i) {
        int a = r.ports[i], b = r.ports[i + 1];
        for (const Service& s : graph[a]) {
            if (s.to == b) {
                r.totalCost += s.cost;
                r.totalDays += s.days;
                if (i > 0) r.totalDays += waitingDays(s.perWeek); // wait only at transhipment ports
                break;
            }
        }
    }
}

Route bfsFewestTranshipments(int src, int dst) {
    queue<int> q;
    vector<int> parent(graph.size(), -1);
    q.push(src);
    parent[src] = src;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (u == dst) break;
        for (const Service& s : graph[u]) {
            if (parent[s.to] == -1) {
                parent[s.to] = u;
                q.push(s.to);
            }
        }
    }
    Route r;
    if (parent[dst] == -1) return r;      // unreachable: empty route

    for (int v = dst; v != src; v = parent[v])
        r.ports.push_back(v);             // dst, then each parent, stopping before src
    r.ports.push_back(src);               // the loop stops before src, so add it here
    reverse(r.ports.begin(), r.ports.end());   // needs #include <algorithm>

    r.legs = r.ports.size() - 1;
    fillTotals(r);
    return r;
}

Route dijkstraCheapest(int src, int dst) {
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq;
    vector<double> dist(graph.size(), numeric_limits<double>::infinity());
    dist[src] = 0;
    pq.push({0, src});
    vector<int> parent(graph.size(), -1);
    parent[src] = src;
    while(!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (u == dst) break;
        if (d > dist[u]) continue; // stale entry
        for (const Service& s: graph[u]) {
            double newDist = dist[u] + s.cost;
            if (newDist < dist[s.to]) {
                dist[s.to] = newDist;
                parent[s.to] = u;
                pq.push({newDist, s.to});
            }
        }
    }
    Route r;
    if (parent[dst] == -1) return r;      // unreachable: empty route

    for (int v = dst; v != src; v = parent[v])
        r.ports.push_back(v);
    r.ports.push_back(src);
    reverse(r.ports.begin(), r.ports.end());
    r.legs = r.ports.size() - 1;
    fillTotals(r);
    return r;
}

Route dijkstraFastest(int src, int dst) {
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq;
    vector<double> dist(graph.size(), numeric_limits<double>::infinity());
    dist[src] = 0;
    pq.push({0, src});
    vector<int> parent(graph.size(), -1);
    parent[src] = src;
    while(!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (u == dst) break;
        if (d > dist[u]) continue; // stale entry
        for (const Service& s: graph[u]) {
            double legTime = s.days;
            if (u != src) legTime += waitingDays(s.perWeek); // add waiting time for all but the first leg
            double newDist = dist[u] + legTime;
            if (newDist < dist[s.to]) {
                dist[s.to] = newDist;
                parent[s.to] = u;
                pq.push({newDist, s.to});
            }

        } 
    }
    Route r;
    if (parent[dst] == -1) return r;      // unreachable: empty route

    for (int v = dst; v != src; v = parent[v])
        r.ports.push_back(v);
    r.ports.push_back(src);
    reverse(r.ports.begin(), r.ports.end());
    r.legs = r.ports.size() - 1;
    fillTotals(r);
    return r;
}

void printRoute(const Route& r) {
    if (r.ports.empty()) {
        cout << "No route found.\n";
        return;
    }
    cout << "Route: ";
    for (size_t i = 0; i < r.ports.size(); ++i) {
        cout << portNames[r.ports[i]];
        if (i + 1 < r.ports.size()) cout << " -> ";
    }
    cout << "\nLegs: " << r.legs
         << ", Total Cost: $" << r.totalCost
         << ", Total Days: " << r.totalDays << "\n";
}

int main() {
    graph.resize(portNames.size());

    // The three competing routes for Izmir(0) -> Valencia(5)
    addService(0, 5, 700, 9, 1);    // direct, slow
    addService(0, 3, 400, 2, 7);    // Izmir-Piraeus (fast hub route)
    addService(3, 5, 450, 4, 7);    // Piraeus-Valencia
    addService(0, 10, 150, 5, 1);   // Izmir-Malta (cheap feeder chain)
    addService(10, 6, 120, 3, 1);   // Malta-Barcelona
    addService(6, 5, 60, 1, 2);     // Barcelona-Valencia

    // Rest of the network
    addService(0, 1, 200, 1.5, 7);  // Izmir-Istanbul
    addService(1, 15, 250, 2, 3);   // Istanbul-Constanta
    addService(15, 16, 150, 1, 3);  // Constanta-Odesa
    addService(1, 4, 220, 2, 3);    // Istanbul-Thessaloniki
    addService(4, 3, 100, 1, 4);    // Thessaloniki-Piraeus
    addService(0, 2, 300, 3, 2);    // Izmir-Mersin
    addService(2, 14, 180, 1, 4);   // Mersin-Limassol
    addService(14, 13, 120, 1, 3);  // Limassol-Haifa
    addService(14, 11, 200, 2, 3);  // Limassol-Port Said
    addService(11, 12, 90, 1, 5);   // Port Said-Alexandria
    addService(12, 3, 350, 3, 2);   // Alexandria-Piraeus
    addService(3, 9, 300, 3, 3);    // Piraeus-Gioia Tauro
    addService(9, 10, 100, 1, 3);   // Gioia Tauro-Malta
    addService(9, 8, 200, 2, 4);    // Gioia Tauro-Genoa
    addService(8, 7, 120, 1, 5);    // Genoa-Marseille
    addService(7, 6, 130, 1, 4);    // Marseille-Barcelona
    addService(6, 19, 150, 2, 3);   // Barcelona-Algeciras
    addService(5, 19, 140, 2, 3);   // Valencia-Algeciras
    addService(19, 17, 400, 5, 2);  // Algeciras-Rotterdam
    addService(17, 18, 100, 1, 7);  // Rotterdam-Hamburg
    addService(8, 17, 600, 6, 2);   // Genoa-Rotterdam

    int src = 0, dst = 5;           // Izmir -> Valencia

    cout << "== Fewest transhipments ==\n";
    printRoute(bfsFewestTranshipments(src, dst));
    cout << "\n== Cheapest ==\n";
    printRoute(dijkstraCheapest(src, dst));
    cout << "\n== Fastest ==\n";
    printRoute(dijkstraFastest(src, dst));
    return 0;
}
