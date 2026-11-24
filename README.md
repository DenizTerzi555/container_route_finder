- Modeled a 20-port container shipping network in C++ as a weighted graph, with cost, transit time
and weekly sailing frequency per service
- Implemented BFS for the fewest-transfer route and Dijkstra for the cheapest and fastest routes,
adding expected waiting time at transshipment ports to transit time, which was tested on Izmir to
Valencia, where the three criteria return different routes.
