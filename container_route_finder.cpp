/* 

#include <algorithm>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <string>
#include <vector>

// 1- Binary search
// Returns the first index i with a[i] >= x (a must be sorted)
// Invariant: every index < lo has a[] < x, every index >= hi has a[] >= x
std::size_t lowerBound (const std::vector<int>& a, int x) {
    std::size_t lo = 0, hi = a.size();
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;   // not (lo+hi)/ 2: that can overflow for huge arrays
        if (a[mid] < x) lo = mid + 1;           // mid is too small: everything up to mid is excluded
        else hi = mid;                          // mid might be the answer: keep it  
    }
    return lo;                                  // lo == hi
}

// 2- Union find (Disjoint sets) with path compression and union by size
struct DSU {
    std::vector<int> parent, sz;
    explicit DSU(int n) : parent(n), sz(n, 1) {std::iota(parent.begin(), parent.end(), 0); }
    int find(int x) {return parent[x] == x ? x : parent[x] = find(parent[x]); }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (sz[a] < sz[b]) std::swap(a, b);
        parent[b] = a; sz[a] += sz[b];
        return true;
    }
};

// 3- Dijkstra's algorithm for shortest paths from a source vertex
struct Edge {int to; long long w; };
std::vector<long long> dijkstra(const std::vector<std::vector<Edge>>& g, int src, std::vector<int>&prev) {
    const long long INF = std::numeric_limits<long long>::max() / 4;
    std::vector<long long> dist(g.size(), INF);
    prev.assign(g.size(), -1);
    using Item = std::pair< long long, int>; // (distance, node(vertex))
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> pq;
    dist[src] = 0; pq.push({0, src});
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue; // stale entry: a better one has already been processed 
        for (const Edge& e : g[u])
            if (dist[u] + e.w < dist[e.to]) {
                dist[e.to] = dist[u] + e.w;
                prev[e.to] = u;
                pq.push({dist[e.to], e.to});
            }
    }
    return dist;
}
int main() {
    // Binary search tests, including the edges that usually go wrong
    std::vector<int> a = {2, 4, 4, 4, 9, 12};
    for (int x : {1, 2, 4, 5, 12, 13})
        std::cout << "lowerBound(" << x << ") = " << lowerBound(a, x) << "\n";
    // Union find: grouping lanes that share a carrier into clusters
    DSU d(6);
    d.unite(0, 1); d.unite(1, 2); d.unite(4, 5);
    std::cout << "0~2? " << (d.find(0) == d.find(2)) << " 0~4? " << (d.find(0) == d.find(4))
            << " 4~5? " << (d.find(4) == d.find(5)) << " unite(0,2) again -> " << d.unite(0, 2) << "\n";
    // Dijkstra on a tiny network: Izmir(0) -> Antwerp(1), Rotterdam(2), Hamburg(3), Abidjan(4)
    std::vector<std::string> names = {"Izmir", "Antwerp", "Rotterdam", "Hamburg", "Abidjan"};
    std::vector<std::vector<Edge>> g(5);
    auto add = [&] (int u, int v, long long w) { g[u].push_back({v, w}); g[v].push_back({u, w}); };
    add(0, 1, 9); add(0, 2, 10); add(0, 3, 12); add(1, 4, 18); add(2, 4, 16); add(1, 2, 1); add(3, 4, 30);
    std::vector<int> prev;
    auto dist = dijkstra(g, 0, prev);
    for (int v = 0; v < 5; ++v) {
        std::cout << names[v] << ": " << dist[v] << " days path: ";
        std::vector<int> path;
        for (int x = v; x != -1; x = prev[x]) path.push_back(x);
        std::reverse(path.begin(), path.end());
        for(std::size_t i =0; i <path.size(); ++i) std::cout << (i ? " -> " : "") << names[path[i]];
        std::cout << "\n";
    }
    // Stable sort: sort quotes by rate, keeping the file order of ties 
    struct Q {std::string carrier; int rate; };
    std::vector<Q> quotes = {{"A", 400}, {"B", 300}, {"C", 400}, {"D", 380}};
    std::stable_sort(quotes.begin(), quotes.end(), [] (const Q& x, const Q& y) {return x.rate < y.rate; });
    for (auto& q: quotes) std::cout << q.carrier << ":" << q.rate << " ";
    std::cout << "\n";
}


*/

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