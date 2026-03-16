# 🧭 Pathfinder Visualizer

An interactive **Pathfinding Visualizer** built using **C++ & SFML**, supporting **A\***, **BFS**, **Dijkstra**, and **Greedy Best-First** with animated exploration and terrain-based weights.


![Pathfinder Demo](demo.gif)

## 🚀 Features

- 🔍 **A\*** Algorithm with Manhattan Heuristic
- 🧱 **BFS** for shortest path by steps
- 📊 **Dijkstra** for guaranteed lowest cost path
- ⚡ **Greedy Best-First** for fast heuristic-based search
- 🎨 Terrain weights visually encoded:
  - Light Gray: Easy (Cost 1)
  - Medium Gray: Medium (Cost 4)
  - Dark Gray: Hard (Cost 8)
- 🎬 Smooth step-by-step animation with adjustable speed
- 🖱️ Mouse-controlled wall + start/end placement
- 📜 Run history to compare algorithm performance
- 🌈 Multiple color themes

## 🕹️ Controls

| Key | Action |
|-----|--------|
| A | Use A* Algorithm |
| B | Use BFS Algorithm |
| D | Use Dijkstra Algorithm |
| G | Use Greedy Best-First |
| Space | Run Selected Algorithm |
| R | Full Reset (New Terrain) |
| E | Soft Reset (Keep Walls) |
| H | Clear Run History |
| +/- | Adjust Animation Speed |
| T | Change Theme |
| Esc | Exit |
| Left Click | Place Start / End / Walls |
| Right Click | Remove Walls |

## 🧠 Algorithm Details

### ✅ A\*
- Uses **Manhattan Heuristic**
- Combines actual cost with estimated distance to goal
- Finds optimal path while exploring fewer nodes than Dijkstra

### ✅ BFS
- Explores layer by layer from the start
- Finds shortest path by **number of steps**
- Ignores terrain weights

### ✅ Dijkstra
- Always finds the **lowest cost** path
- Explores outward based on cumulative cost
- More thorough than A* but guarantees optimality

### ✅ Greedy Best-First
- Only uses the heuristic, ignores cost so far
- Very fast but **doesn't guarantee** optimal path
- Good for seeing how heuristics guide search

## 🎨 Path Colors

Final path is colored based on terrain difficulty:
- 🔵 Blue: Easy terrain
- 🟡 Yellow: Medium terrain
- 🔴 Red: Hard terrain

## 📜 Run History

Every algorithm run is logged with:
- Algorithm used
- Time taken
- Nodes visited
- Total path cost

Compare different algorithms on the same terrain to see how they perform!

## 🛠️ Build Instructions

Requires **SFML 3** installed on your system.

### macOS (Homebrew)
```bash
brew install sfml
g++ -std=c++17 pathfinder_visualizer.cpp -o pathfinder \
    -I/opt/homebrew/include -L/opt/homebrew/lib \
    -lsfml-graphics -lsfml-window -lsfml-system
./pathfinder
```

### Linux
```bash
sudo apt install libsfml-dev
g++ -std=c++17 pathfinder_visualizer.cpp -o pathfinder \
    -lsfml-graphics -lsfml-window -lsfml-system
./pathfinder
```

### Windows (MSYS2)
```bash
pacman -S mingw-w64-x86_64-sfml
g++ -std=c++17 pathfinder_visualizer.cpp -o pathfinder.exe \
    -lsfml-graphics -lsfml-window -lsfml-system
./pathfinder.exe
```

## 💡 What I Learned

- How different pathfinding algorithms explore and make decisions
- Why heuristics matter (A* vs Dijkstra vs Greedy)
- Real-time rendering and animation with SFML
- Structuring C++ projects with clean separation of concerns
