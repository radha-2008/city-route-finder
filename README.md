# City Route Finder (C++)

A console-based route planner for 30+ Indian cities, built from scratch in C++ using graph algorithms and the STL. It finds the best route between two cities by **distance, time or cost**, and compares **Dijkstra** against **A\*** to show how much a heuristic reduces the search.

## Features

- Add / remove cities and roads, list the whole map
- **Dijkstra**: shortest route by distance, time or cost
- **BFS**: route with the fewest stops
- **A\* (with Haversine heuristic)**: faster distance search using latitude/longitude
- **Compare mode**: runs Dijkstra and A\* side by side and reports cities visited
- "Avoid this city" option for rerouting
- Save and load maps from a text file (`india.txt` included, 31 cities, 49 roads)
- Case-insensitive city names (`delhi`, `DELHI`, `Delhi` all work)

## How to run

```
g++ -std=c++17 main.cpp -o routefinder
./routefinder          # on Windows: .\routefinder.exe
```

Then in the menu: `10` -> `india.txt` to load the map, and `12` to compare Dijkstra vs A\*.

## Sample output

```
From: delhi
To: chennai

--- Dijkstra ---
Route: Delhi -> Agra -> Bhopal -> Nagpur -> Hyderabad -> Chennai
Total: 2185 km
Cities visited: 27

--- A* ---
Route: Delhi -> Agra -> Bhopal -> Nagpur -> Hyderabad -> Chennai
Total: 2185 km
Cities visited: 11

A* visited 59% fewer cities than Dijkstra.
```

## Dijkstra vs A\* (measured on the included India map)

| Route | Dijkstra (cities visited) | A\* (cities visited) | Same route found |
|---|---|---|---|
| Delhi to Chennai | 27 | 11 | Yes |
| Amritsar to Kochi | 30 | 20 | Yes |
| Mumbai to Kolkata | 30 | 12 | Yes |

## Data structures and complexity

| Component | Structure | Why |
|---|---|---|
| Map | Adjacency list (`unordered_map<int, vector<Edge>>`) | Sparse graph, O(V + E) space |
| Dijkstra | Min-heap (`priority_queue`) | O((V + E) log V) |
| BFS | Queue | O(V + E) |
| A\* | Min-heap + Haversine heuristic | Same worst case as Dijkstra, fewer nodes in practice |

## Design notes

- **Why A\* only for distance:** the Haversine (straight-line) distance never exceeds the road distance, so it is an *admissible* heuristic and A\* stays optimal. For time or cost there is no such guarantee, so those use Dijkstra.
- **Why Dijkstra and not BFS for weights:** BFS minimizes the number of roads, not total distance.
- **Negative edges:** Dijkstra assumes non-negative weights, which holds for km, hours and rupees.

## Project structure

```
main.cpp     Graph, PathFinder (Dijkstra, BFS, A*), menu
india.txt    Sample map: 31 cities, 49 roads
```

Map file format: `C Name longitude latitude` for a city, `R CityA CityB km hours cost` for a road. City names are single words.

## Notes

Road distances, times and costs in `india.txt` are approximate and for demonstration only.

## Possible improvements

- Split into header files (`Graph.h`, `PathFinder.h`)
- Alternate routes (k-shortest paths)
- Visualization with SFML
- Real road data from OpenStreetMap
