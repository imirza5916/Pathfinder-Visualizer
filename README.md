# 🧭 Pathfinder Visualizer

An interactive **Pathfinding Visualizer** built using **C++ & SFML**, supporting **A\*** and **BFS** with animated exploration and terrain-based weights.

## 🎥 Demo

https://user-images.githubusercontent.com/your-demo-link.mp4 *(optional)*

## 🚀 Features

- 🔍 **A\*** Algorithm (Manhattan Distance)
- 🧱 **BFS** with Weighted Terrain Support
- 🎨 Terrain weights visually encoded:
  - Light Gray: Easy (Low Cost)
  - Dark Gray: Hard (High Cost)
- 🎬 Smooth step-by-step animation
- 🖱️ Mouse-controlled wall + start/end placement
- 🌈 Themed UI (with future theme toggle support)

## 🕹️ Controls

| Key | Action                      |
|-----|-----------------------------|
| A   | Use A* Algorithm            |
| B   | Use BFS Algorithm           |
| Space | Run Selected Algorithm   |
| R   | Full Reset (New Terrain)    |
| E   | Soft Reset (Keep Terrain)   |
| Left Click  | Place Wall / Start / End |
| Right Click | Remove Wall         |

## 🧠 Algorithm Details

### ✅ A\*
- Uses **Manhattan Heuristic**
- Expands in a circular/diamond shape
- Final path reflects terrain cost (colors vary)

### ✅ BFS (Weighted)
- Classic **BFS with weights** added
- Final path is computed with terrain in mind
- Expansion visual mimics BFS, but path respects weights
- Final path color:
  - 🔵 Easy (Blue)
  - 🟡 Medium (Yellow)
  - 🔴 Hard (Red)

## 🛠️ Build Instructions

Ensure SFML is installed on your system.
