/*
    You can place walls, set start and end points, and watch algorithms like 
    A*, BFS, Dijkstra, and Greedy Best-First do their thing in real time.
    The terrain has different weights so you can see how that affects the path.
    
    Controls
    
    A, B, D, G      Pick an algorithm
    Space           Run it
    R               Reset everything
    E               Soft reset, keeps your walls
    Left Click      Place start, then end, then walls
    Right Click     Erase walls
    Plus/Minus      Speed up or slow down the animation
    T               Switch themes
    H               Clear the history
    Esc             Quit
*/

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iomanip>
#include <memory>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>


// Config

namespace Config {
    // Grid size
    constexpr int ROWS = 30;
    constexpr int COLS = 30;
    
    // Layout stuff
    constexpr int GRID_WIDTH = 720;
    constexpr int LEGEND_WIDTH = 280;
    constexpr int PADDING = 40;
    
    // Animation timing
    constexpr int DEFAULT_ANIMATION_DELAY_MS = 8;
    constexpr int MIN_ANIMATION_DELAY_MS = 1;
    constexpr int MAX_ANIMATION_DELAY_MS = 100;
    constexpr int ANIMATION_STEP = 2;
    
    // How much it costs to cross each terrain type
    constexpr int WEIGHT_EASY = 1;
    constexpr int WEIGHT_MEDIUM = 4;
    constexpr int WEIGHT_HARD = 8;
    
    // Visual tweaks
    constexpr float CELL_GAP = 1.0f;
    constexpr float CELL_OUTLINE_THICKNESS = 0.0f;
    
    // Font paths to try, in order
    const std::vector<std::string> FONT_PATHS = {
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"
    };
}


// Types

using Position = std::pair<int, int>;

struct PositionHash {
    std::size_t operator()(const Position& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 16);
    }
};


// Cell states

enum class CellType {
    Empty,
    Start,
    End,
    Wall,
    Visited,
    Frontier,
    Path
};

enum class Algorithm {
    AStar,
    BFS,
    Dijkstra,
    GreedyBestFirst
};


// Themes

struct Theme {
    std::string name;
    sf::Color background;
    sf::Color gridLine;
    sf::Color terrainEasy;
    sf::Color terrainMedium;
    sf::Color terrainHard;
    sf::Color wall;
    sf::Color start;
    sf::Color end;
    sf::Color visited;
    sf::Color frontier;
    sf::Color pathEasy;
    sf::Color pathMedium;
    sf::Color pathHard;
    sf::Color textPrimary;
    sf::Color textSecondary;
    sf::Color panelBackground;
};

class ThemeManager {
public:
    ThemeManager() {
        initializeThemes();
    }
    
    const Theme& current() const { return themes_[currentIndex_]; }
    
    void next() {
        currentIndex_ = (currentIndex_ + 1) % themes_.size();
    }
    
    void previous() {
        currentIndex_ = (currentIndex_ + themes_.size() - 1) % themes_.size();
    }
    
    size_t count() const { return themes_.size(); }
    size_t currentIndex() const { return currentIndex_; }

private:
    void initializeThemes() {
        // Light theme (default)
        themes_.push_back({
            "Light",
            sf::Color(248, 249, 250),        // background
            sf::Color(222, 226, 230),        // gridLine
            sf::Color(233, 236, 239),        // terrainEasy
            sf::Color(173, 181, 189),        // terrainMedium  
            sf::Color(108, 117, 125),        // terrainHard
            sf::Color(33, 37, 41),           // wall
            sf::Color(25, 135, 84),          // start (green)
            sf::Color(220, 53, 69),          // end (red)
            sf::Color(13, 110, 253, 80),     // visited (blue, semi-transparent)
            sf::Color(255, 193, 7, 120),     // frontier (yellow)
            sf::Color(13, 202, 240),         // pathEasy (cyan)
            sf::Color(255, 193, 7),          // pathMedium (yellow)
            sf::Color(255, 87, 51),          // pathHard (orange-red)
            sf::Color(33, 37, 41),           // textPrimary
            sf::Color(108, 117, 125),        // textSecondary
            sf::Color(255, 255, 255, 240)    // panelBackground
        });
        
        // Dark theme
        themes_.push_back({
            "Dark",
            sf::Color(25, 25, 35),           // background
            sf::Color(45, 45, 60),           // gridLine
            sf::Color(40, 42, 54),           // terrainEasy
            sf::Color(68, 71, 90),           // terrainMedium
            sf::Color(98, 102, 130),         // terrainHard
            sf::Color(10, 10, 15),           // wall
            sf::Color(80, 250, 123),         // start (green)
            sf::Color(255, 85, 85),          // end (red)
            sf::Color(139, 233, 253, 70),    // visited (cyan)
            sf::Color(241, 250, 140, 100),   // frontier (yellow)
            sf::Color(139, 233, 253),        // pathEasy
            sf::Color(241, 250, 140),        // pathMedium
            sf::Color(255, 121, 198),        // pathHard
            sf::Color(248, 248, 242),        // textPrimary
            sf::Color(189, 147, 249),        // textSecondary
            sf::Color(40, 42, 54, 245)       // panelBackground
        });
        
        // Ocean theme
        themes_.push_back({
            "Ocean",
            sf::Color(10, 25, 47),           // background
            sf::Color(23, 42, 69),           // gridLine
            sf::Color(17, 34, 64),           // terrainEasy
            sf::Color(44, 62, 93),           // terrainMedium
            sf::Color(79, 93, 117),          // terrainHard
            sf::Color(2, 12, 27),            // wall
            sf::Color(100, 255, 218),        // start (teal)
            sf::Color(255, 107, 107),        // end (coral)
            sf::Color(100, 255, 218, 50),    // visited
            sf::Color(255, 209, 102, 80),    // frontier
            sf::Color(100, 255, 218),        // pathEasy
            sf::Color(255, 209, 102),        // pathMedium
            sf::Color(255, 107, 107),        // pathHard
            sf::Color(204, 214, 246),        // textPrimary
            sf::Color(136, 146, 176),        // textSecondary
            sf::Color(23, 42, 69, 250)       // panelBackground
        });
        
        // High Contrast
        themes_.push_back({
            "High Contrast",
            sf::Color(0, 0, 0),              // background
            sf::Color(60, 60, 60),           // gridLine
            sf::Color(30, 30, 30),           // terrainEasy
            sf::Color(80, 80, 80),           // terrainMedium
            sf::Color(130, 130, 130),        // terrainHard
            sf::Color(200, 200, 200),        // wall (light for contrast)
            sf::Color(0, 255, 0),            // start
            sf::Color(255, 0, 0),            // end
            sf::Color(0, 100, 255, 100),     // visited
            sf::Color(255, 255, 0, 150),     // frontier
            sf::Color(0, 255, 255),          // pathEasy
            sf::Color(255, 255, 0),          // pathMedium
            sf::Color(255, 128, 0),          // pathHard
            sf::Color(255, 255, 255),        // textPrimary
            sf::Color(180, 180, 180),        // textSecondary
            sf::Color(20, 20, 20, 250)       // panelBackground
        });
    }
    
    std::vector<Theme> themes_;
    size_t currentIndex_ = 0;
};


// A single cell in the grid

struct Cell {
    CellType type = CellType::Empty;
    int weight = Config::WEIGHT_EASY;
    int row = 0;
    int col = 0;
    
    sf::RectangleShape shape;
    
    void setPosition(float x, float y, float width, float height) {
        shape.setSize(sf::Vector2f(width - Config::CELL_GAP, height - Config::CELL_GAP));
        shape.setPosition(sf::Vector2f(x, y));
    }
    
    void updateColor(const Theme& theme) {
        switch (type) {
            case CellType::Start:
                shape.setFillColor(theme.start);
                break;
            case CellType::End:
                shape.setFillColor(theme.end);
                break;
            case CellType::Wall:
                shape.setFillColor(theme.wall);
                break;
            case CellType::Visited:
                shape.setFillColor(theme.visited);
                break;
            case CellType::Frontier:
                shape.setFillColor(theme.frontier);
                break;
            case CellType::Path:
                // color based on how hard the terrain is
                if (weight <= Config::WEIGHT_EASY + 1) {
                    shape.setFillColor(theme.pathEasy);
                } else if (weight <= Config::WEIGHT_MEDIUM + 1) {
                    shape.setFillColor(theme.pathMedium);
                } else {
                    shape.setFillColor(theme.pathHard);
                }
                break;
            case CellType::Empty:
            default:
                // Terrain-based coloring
                if (weight <= Config::WEIGHT_EASY + 1) {
                    shape.setFillColor(theme.terrainEasy);
                } else if (weight <= Config::WEIGHT_MEDIUM + 1) {
                    shape.setFillColor(theme.terrainMedium);
                } else {
                    shape.setFillColor(theme.terrainHard);
                }
                break;
        }
    }
};


// Stats from a single algorithm run

struct RunStats {
    int nodesVisited = 0;
    int pathLength = 0;
    int pathCost = 0;
    float durationMs = 0.0f;
    bool pathFound = false;
    
    void reset() {
        nodesVisited = 0;
        pathLength = 0;
        pathCost = 0;
        durationMs = 0.0f;
        pathFound = false;
    }
};

struct HistoryEntry {
    int runNumber;
    std::string algorithmName;
    std::string algorithmShort;
    RunStats stats;
};

// Keeps track of past runs so you can compare algorithms
class RunHistory {
public:
    void addEntry(const std::string& algoName, const std::string& algoShort, const RunStats& stats) {
        HistoryEntry entry;
        entry.runNumber = static_cast<int>(entries_.size()) + 1;
        entry.algorithmName = algoName;
        entry.algorithmShort = algoShort;
        entry.stats = stats;
        entries_.push_back(entry);
        
        // only keep the last 10 runs
        if (entries_.size() > 10) {
            entries_.erase(entries_.begin());
            for (size_t i = 0; i < entries_.size(); ++i) {
                entries_[i].runNumber = static_cast<int>(i) + 1;
            }
        }
    }
    
    void clear() {
        entries_.clear();
    }
    
    const std::vector<HistoryEntry>& entries() const { return entries_; }
    
    bool empty() const { return entries_.empty(); }
    
    size_t size() const { return entries_.size(); }

private:
    std::vector<HistoryEntry> entries_;
};


// The grid that holds all the cells

class Grid {
public:
    Grid(int rows, int cols) 
        : rows_(rows), cols_(cols), cells_(rows, std::vector<Cell>(cols)) {
        for (int r = 0; r < rows_; ++r) {
            for (int c = 0; c < cols_; ++c) {
                cells_[r][c].row = r;
                cells_[r][c].col = c;
            }
        }
    }
    
    void generateTerrain() {
        static std::array<int, 3> weights = {
            Config::WEIGHT_EASY,
            Config::WEIGHT_MEDIUM,
            Config::WEIGHT_HARD
        };
        
        for (int r = 0; r < rows_; ++r) {
            for (int c = 0; c < cols_; ++c) {
                cells_[r][c].type = CellType::Empty;
                cells_[r][c].weight = weights[rand() % 3];
            }
        }
        startPos_.reset();
        endPos_.reset();
    }
    
    void updateLayout(float offsetX, float offsetY, float gridWidth, float gridHeight) {
        float cellWidth = gridWidth / cols_;
        float cellHeight = gridHeight / rows_;
        
        for (int r = 0; r < rows_; ++r) {
            for (int c = 0; c < cols_; ++c) {
                cells_[r][c].setPosition(
                    offsetX + c * cellWidth,
                    offsetY + r * cellHeight,
                    cellWidth,
                    cellHeight
                );
            }
        }
    }
    
    void updateColors(const Theme& theme) {
        for (auto& row : cells_) {
            for (auto& cell : row) {
                cell.updateColor(theme);
            }
        }
    }
    
    void draw(sf::RenderWindow& window) const {
        for (const auto& row : cells_) {
            for (const auto& cell : row) {
                window.draw(cell.shape);
            }
        }
    }
    
    Cell& at(int row, int col) { return cells_[row][col]; }
    const Cell& at(int row, int col) const { return cells_[row][col]; }
    
    Cell& at(const Position& pos) { return cells_[pos.first][pos.second]; }
    const Cell& at(const Position& pos) const { return cells_[pos.first][pos.second]; }
    
    bool isValid(int row, int col) const {
        return row >= 0 && row < rows_ && col >= 0 && col < cols_;
    }
    
    bool isValid(const Position& pos) const {
        return isValid(pos.first, pos.second);
    }
    
    bool isWalkable(int row, int col) const {
        return isValid(row, col) && cells_[row][col].type != CellType::Wall;
    }
    
    bool isWalkable(const Position& pos) const {
        return isWalkable(pos.first, pos.second);
    }
    
    int rows() const { return rows_; }
    int cols() const { return cols_; }
    
    // Start/End position management
    std::optional<Position> startPos() const { return startPos_; }
    std::optional<Position> endPos() const { return endPos_; }
    
    bool setStart(const Position& pos) {
        if (!isValid(pos) || cells_[pos.first][pos.second].type != CellType::Empty) {
            return false;
        }
        if (startPos_) {
            cells_[startPos_->first][startPos_->second].type = CellType::Empty;
        }
        startPos_ = pos;
        cells_[pos.first][pos.second].type = CellType::Start;
        return true;
    }
    
    bool setEnd(const Position& pos) {
        if (!isValid(pos) || cells_[pos.first][pos.second].type != CellType::Empty) {
            return false;
        }
        if (endPos_) {
            cells_[endPos_->first][endPos_->second].type = CellType::Empty;
        }
        endPos_ = pos;
        cells_[pos.first][pos.second].type = CellType::End;
        return true;
    }
    
    bool hasStartAndEnd() const {
        return startPos_.has_value() && endPos_.has_value();
    }
    
    void clearAlgorithmResults() {
        for (auto& row : cells_) {
            for (auto& cell : row) {
                if (cell.type == CellType::Visited || 
                    cell.type == CellType::Frontier || 
                    cell.type == CellType::Path) {
                    cell.type = CellType::Empty;
                }
            }
        }
    }
    
    void fullReset() {
        generateTerrain();
    }
    
    void softReset() {
        clearAlgorithmResults();
    }
    
    // returns the four neighboring cells you can move to
    std::vector<Position> getNeighbors(const Position& pos) const {
        static const std::array<std::pair<int, int>, 4> directions = {{
            {-1, 0}, {1, 0}, {0, -1}, {0, 1}
        }};
        
        std::vector<Position> neighbors;
        neighbors.reserve(4);
        
        for (const auto& [dr, dc] : directions) {
            int nr = pos.first + dr;
            int nc = pos.second + dc;
            if (isWalkable(nr, nc)) {
                neighbors.emplace_back(nr, nc);
            }
        }
        
        return neighbors;
    }

private:
    int rows_;
    int cols_;
    std::vector<std::vector<Cell>> cells_;
    std::optional<Position> startPos_;
    std::optional<Position> endPos_;
};


// Handles loading fonts, tries a few different paths depending on the OS

class FontManager {
public:
    static FontManager& instance() {
        static FontManager instance;
        return instance;
    }
    
    const sf::Font* getFont() const { return font_.get(); }
    bool isLoaded() const { return loaded_; }

private:
    FontManager() {
        for (const auto& path : Config::FONT_PATHS) {
            auto tempFont = std::make_unique<sf::Font>();
            if (tempFont->openFromFile(path)) {
                font_ = std::move(tempFont);
                loaded_ = true;
                break;
            }
        }
    }
    
    std::unique_ptr<sf::Font> font_;
    bool loaded_ = false;
};


// Pathfinding Algorithms
// Each algorithm extends this base class and implements run()

class PathfindingAlgorithm {
public:
    virtual ~PathfindingAlgorithm() = default;
    
    virtual std::string name() const = 0;
    virtual std::string shortName() const = 0;
    
    virtual RunStats run(
        Grid& grid,
        std::function<void()> visualize,
        std::function<void(int)> sleep
    ) = 0;

protected:
    static int manhattan(const Position& a, const Position& b) {
        return std::abs(a.first - b.first) + std::abs(a.second - b.second);
    }
    
    static int euclidean(const Position& a, const Position& b) {
        int dr = a.first - b.first;
        int dc = a.second - b.second;
        return static_cast<int>(std::sqrt(dr * dr + dc * dc));
    }
    
    // walks backwards through the parent map to build the final path
    std::vector<Position> reconstructPath(
        const std::unordered_map<Position, Position, PositionHash>& cameFrom,
        Position current,
        const Position& start
    ) {
        std::vector<Position> path;
        while (current != start) {
            path.push_back(current);
            auto it = cameFrom.find(current);
            if (it == cameFrom.end()) break;
            current = it->second;
        }
        std::reverse(path.begin(), path.end());
        return path;
    }
};


// A* uses both the actual cost so far and a heuristic to find the best path

class AStarAlgorithm : public PathfindingAlgorithm {
public:
    std::string name() const override { return "A* Search"; }
    std::string shortName() const override { return "A*"; }
    
    RunStats run(
        Grid& grid,
        std::function<void()> visualize,
        std::function<void(int)> sleep
    ) override {
        RunStats stats;
        sf::Clock clock;
        
        if (!grid.hasStartAndEnd()) return stats;
        
        Position start = *grid.startPos();
        Position end = *grid.endPos();
        
        // Priority queue: (fScore, position)
        using Node = std::pair<int, Position>;
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
        std::unordered_set<Position, PositionHash> openSetLookup;
        std::unordered_set<Position, PositionHash> closedSet;
        
        std::unordered_map<Position, int, PositionHash> gScore;
        std::unordered_map<Position, Position, PositionHash> cameFrom;
        
        gScore[start] = 0;
        int startF = manhattan(start, end);
        openSet.push({startF, start});
        openSetLookup.insert(start);
        
        while (!openSet.empty()) {
            auto [currentF, current] = openSet.top();
            openSet.pop();
            openSetLookup.erase(current);
            
            if (current == end) {
                // Reconstruct and visualize path
                auto path = reconstructPath(cameFrom, end, start);
                for (const auto& pos : path) {
                    Cell& cell = grid.at(pos);
                    if (cell.type != CellType::End) {
                        cell.type = CellType::Path;
                        stats.pathLength++;
                        stats.pathCost += cell.weight;
                        visualize();
                        sleep(1);
                    }
                }
                stats.pathFound = true;
                break;
            }
            
            closedSet.insert(current);
            
            // Visualize current node as visited
            if (current != start) {
                grid.at(current).type = CellType::Visited;
                stats.nodesVisited++;
                visualize();
                sleep(1);
            }
            
            for (const Position& neighbor : grid.getNeighbors(current)) {
                if (closedSet.count(neighbor)) continue;
                
                int tentativeG = gScore[current] + grid.at(neighbor).weight;
                
                if (!gScore.count(neighbor) || tentativeG < gScore[neighbor]) {
                    cameFrom[neighbor] = current;
                    gScore[neighbor] = tentativeG;
                    int fScore = tentativeG + manhattan(neighbor, end);
                    
                    if (!openSetLookup.count(neighbor)) {
                        openSet.push({fScore, neighbor});
                        openSetLookup.insert(neighbor);
                        
                        // Show frontier
                        if (grid.at(neighbor).type == CellType::Empty) {
                            grid.at(neighbor).type = CellType::Frontier;
                        }
                    }
                }
            }
        }
        
        // Clear frontier visualization
        for (int r = 0; r < grid.rows(); ++r) {
            for (int c = 0; c < grid.cols(); ++c) {
                if (grid.at(r, c).type == CellType::Frontier) {
                    grid.at(r, c).type = CellType::Visited;
                }
            }
        }
        
        stats.durationMs = clock.getElapsedTime().asMilliseconds();
        return stats;
    }
};


// BFS explores layer by layer, finds shortest path by number of steps

class BFSAlgorithm : public PathfindingAlgorithm {
public:
    std::string name() const override { return "Breadth-First Search"; }
    std::string shortName() const override { return "BFS"; }
    
    RunStats run(
        Grid& grid,
        std::function<void()> visualize,
        std::function<void(int)> sleep
    ) override {
        RunStats stats;
        sf::Clock clock;
        
        if (!grid.hasStartAndEnd()) return stats;
        
        Position start = *grid.startPos();
        Position end = *grid.endPos();
        
        std::queue<Position> queue;
        std::unordered_set<Position, PositionHash> visited;
        std::unordered_map<Position, Position, PositionHash> cameFrom;
        
        queue.push(start);
        visited.insert(start);
        
        while (!queue.empty()) {
            Position current = queue.front();
            queue.pop();
            
            if (current == end) {
                // Reconstruct path
                auto path = reconstructPath(cameFrom, end, start);
                for (const auto& pos : path) {
                    Cell& cell = grid.at(pos);
                    if (cell.type != CellType::End) {
                        cell.type = CellType::Path;
                        stats.pathLength++;
                        stats.pathCost += cell.weight;
                        visualize();
                        sleep(1);
                    }
                }
                stats.pathFound = true;
                break;
            }
            
            if (current != start) {
                grid.at(current).type = CellType::Visited;
                stats.nodesVisited++;
                visualize();
                sleep(1);
            }
            
            for (const Position& neighbor : grid.getNeighbors(current)) {
                if (visited.count(neighbor)) continue;
                
                visited.insert(neighbor);
                cameFrom[neighbor] = current;
                queue.push(neighbor);
                
                if (grid.at(neighbor).type == CellType::Empty) {
                    grid.at(neighbor).type = CellType::Frontier;
                }
            }
        }
        
        // Clear frontier
        for (int r = 0; r < grid.rows(); ++r) {
            for (int c = 0; c < grid.cols(); ++c) {
                if (grid.at(r, c).type == CellType::Frontier) {
                    grid.at(r, c).type = CellType::Visited;
                }
            }
        }
        
        stats.durationMs = clock.getElapsedTime().asMilliseconds();
        return stats;
    }
};


// Dijkstra always finds the lowest cost path, but explores more than A*

class DijkstraAlgorithm : public PathfindingAlgorithm {
public:
    std::string name() const override { return "Dijkstra's Algorithm"; }
    std::string shortName() const override { return "Dijkstra"; }
    
    RunStats run(
        Grid& grid,
        std::function<void()> visualize,
        std::function<void(int)> sleep
    ) override {
        RunStats stats;
        sf::Clock clock;
        
        if (!grid.hasStartAndEnd()) return stats;
        
        Position start = *grid.startPos();
        Position end = *grid.endPos();
        
        using Node = std::pair<int, Position>;
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
        std::unordered_map<Position, int, PositionHash> dist;
        std::unordered_map<Position, Position, PositionHash> cameFrom;
        std::unordered_set<Position, PositionHash> processed;
        
        dist[start] = 0;
        pq.push({0, start});
        
        while (!pq.empty()) {
            auto [d, current] = pq.top();
            pq.pop();
            
            if (processed.count(current)) continue;
            processed.insert(current);
            
            if (current == end) {
                auto path = reconstructPath(cameFrom, end, start);
                for (const auto& pos : path) {
                    Cell& cell = grid.at(pos);
                    if (cell.type != CellType::End) {
                        cell.type = CellType::Path;
                        stats.pathLength++;
                        stats.pathCost += cell.weight;
                        visualize();
                        sleep(1);
                    }
                }
                stats.pathFound = true;
                break;
            }
            
            if (current != start) {
                grid.at(current).type = CellType::Visited;
                stats.nodesVisited++;
                visualize();
                sleep(1);
            }
            
            for (const Position& neighbor : grid.getNeighbors(current)) {
                if (processed.count(neighbor)) continue;
                
                int newDist = dist[current] + grid.at(neighbor).weight;
                
                if (!dist.count(neighbor) || newDist < dist[neighbor]) {
                    dist[neighbor] = newDist;
                    cameFrom[neighbor] = current;
                    pq.push({newDist, neighbor});
                    
                    if (grid.at(neighbor).type == CellType::Empty) {
                        grid.at(neighbor).type = CellType::Frontier;
                    }
                }
            }
        }
        
        for (int r = 0; r < grid.rows(); ++r) {
            for (int c = 0; c < grid.cols(); ++c) {
                if (grid.at(r, c).type == CellType::Frontier) {
                    grid.at(r, c).type = CellType::Visited;
                }
            }
        }
        
        stats.durationMs = clock.getElapsedTime().asMilliseconds();
        return stats;
    }
};


// Greedy just chases the goal, fast but doesnt always find the best path

class GreedyBestFirstAlgorithm : public PathfindingAlgorithm {
public:
    std::string name() const override { return "Greedy Best-First"; }
    std::string shortName() const override { return "Greedy"; }
    
    RunStats run(
        Grid& grid,
        std::function<void()> visualize,
        std::function<void(int)> sleep
    ) override {
        RunStats stats;
        sf::Clock clock;
        
        if (!grid.hasStartAndEnd()) return stats;
        
        Position start = *grid.startPos();
        Position end = *grid.endPos();
        
        using Node = std::pair<int, Position>;
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
        std::unordered_set<Position, PositionHash> visited;
        std::unordered_map<Position, Position, PositionHash> cameFrom;
        
        openSet.push({manhattan(start, end), start});
        visited.insert(start);
        
        while (!openSet.empty()) {
            auto [h, current] = openSet.top();
            openSet.pop();
            
            if (current == end) {
                auto path = reconstructPath(cameFrom, end, start);
                for (const auto& pos : path) {
                    Cell& cell = grid.at(pos);
                    if (cell.type != CellType::End) {
                        cell.type = CellType::Path;
                        stats.pathLength++;
                        stats.pathCost += cell.weight;
                        visualize();
                        sleep(1);
                    }
                }
                stats.pathFound = true;
                break;
            }
            
            if (current != start) {
                grid.at(current).type = CellType::Visited;
                stats.nodesVisited++;
                visualize();
                sleep(1);
            }
            
            for (const Position& neighbor : grid.getNeighbors(current)) {
                if (visited.count(neighbor)) continue;
                
                visited.insert(neighbor);
                cameFrom[neighbor] = current;
                openSet.push({manhattan(neighbor, end), neighbor});
                
                if (grid.at(neighbor).type == CellType::Empty) {
                    grid.at(neighbor).type = CellType::Frontier;
                }
            }
        }
        
        for (int r = 0; r < grid.rows(); ++r) {
            for (int c = 0; c < grid.cols(); ++c) {
                if (grid.at(r, c).type == CellType::Frontier) {
                    grid.at(r, c).type = CellType::Visited;
                }
            }
        }
        
        stats.durationMs = clock.getElapsedTime().asMilliseconds();
        return stats;
    }
};


// Creates the right algorithm based on what the user picked

class AlgorithmFactory {
public:
    static std::unique_ptr<PathfindingAlgorithm> create(Algorithm algo) {
        switch (algo) {
            case Algorithm::AStar:
                return std::make_unique<AStarAlgorithm>();
            case Algorithm::BFS:
                return std::make_unique<BFSAlgorithm>();
            case Algorithm::Dijkstra:
                return std::make_unique<DijkstraAlgorithm>();
            case Algorithm::GreedyBestFirst:
                return std::make_unique<GreedyBestFirstAlgorithm>();
            default:
                return std::make_unique<AStarAlgorithm>();
        }
    }
};


// UI stuff

class UIPanel {
public:
    UIPanel(float x, float y, float width, float height)
        : x_(x), y_(y), width_(width), height_(height) {
        background_.setPosition(sf::Vector2f(x, y));
        background_.setSize(sf::Vector2f(width, height));
    }
    
    void draw(sf::RenderWindow& window, const Theme& theme) {
        background_.setFillColor(theme.panelBackground);
        background_.setOutlineColor(theme.gridLine);
        background_.setOutlineThickness(1.0f);
        window.draw(background_);
    }
    
    float x() const { return x_; }
    float y() const { return y_; }
    float width() const { return width_; }
    float height() const { return height_; }

private:
    float x_, y_, width_, height_;
    sf::RectangleShape background_;
};

class TextRenderer {
public:
    TextRenderer() : font_(FontManager::instance().getFont()) {}
    
    void drawText(
        sf::RenderWindow& window,
        const std::string& str,
        float x, float y,
        unsigned int size,
        const sf::Color& color,
        bool bold = false
    ) {
        if (!FontManager::instance().isLoaded() || !font_) return;
        
        sf::Text text(*font_, str, size);
        text.setFillColor(color);
        if (bold) {
            text.setStyle(sf::Text::Bold);
        }
        text.setPosition(sf::Vector2f(x, y));
        window.draw(text);
    }
    
    void drawTitle(sf::RenderWindow& window, const std::string& str, 
                   float x, float y, const sf::Color& color) {
        drawText(window, str, x, y, 22, color, true);
    }
    
    void drawHeading(sf::RenderWindow& window, const std::string& str,
                     float x, float y, const sf::Color& color) {
        drawText(window, str, x, y, 16, color, true);
    }
    
    void drawBody(sf::RenderWindow& window, const std::string& str,
                  float x, float y, const sf::Color& color) {
        drawText(window, str, x, y, 14, color, false);
    }
    
    void drawSmall(sf::RenderWindow& window, const std::string& str,
                   float x, float y, const sf::Color& color) {
        drawText(window, str, x, y, 12, color, false);
    }

private:
    const sf::Font* font_;
};


// The sidebar that shows stats, history, and controls

class Legend {
public:
    Legend(float x, float y, float width) 
        : x_(x), y_(y), width_(width), panel_(x, y - 10, width, 720) {}
    
    void draw(
        sf::RenderWindow& window,
        const Theme& theme,
        TextRenderer& text,
        Algorithm currentAlgo,
        const RunStats& stats,
        int animationDelay,
        const RunHistory& history
    ) {
        panel_.draw(window, theme);
        
        float px = x_ + 15;
        float py = y_;
        float lineHeight = 22;
        float sectionGap = 12;
        
        // Title
        text.drawTitle(window, "Pathfinder Visualizer", px, py, theme.textPrimary);
        py += lineHeight + 8;
        
        // Current Algorithm
        auto algo = AlgorithmFactory::create(currentAlgo);
        text.drawHeading(window, "Algorithm", px, py, theme.textSecondary);
        py += lineHeight - 4;
        text.drawBody(window, algo->name(), px, py, theme.textPrimary);
        py += lineHeight + sectionGap;
        
        // Statistics
        text.drawHeading(window, "Last Run", px, py, theme.textSecondary);
        py += lineHeight - 4;
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << stats.durationMs << " ms";
        text.drawSmall(window, "Time: " + oss.str(), px, py, theme.textPrimary);
        py += lineHeight - 6;
        
        text.drawSmall(window, "Visited: " + std::to_string(stats.nodesVisited), 
                      px, py, theme.textPrimary);
        py += lineHeight - 6;
        
        text.drawSmall(window, "Path: " + std::to_string(stats.pathLength) + 
                      " (cost " + std::to_string(stats.pathCost) + ")", 
                      px, py, theme.textPrimary);
        py += lineHeight - 6;
        
        text.drawSmall(window, std::string("Status: ") + 
                      (stats.pathFound ? "Found" : (stats.nodesVisited > 0 ? "No Path" : "Ready")),
                      px, py, stats.pathFound ? theme.start : theme.textSecondary);
        py += lineHeight + sectionGap;
        
        // History Section
        text.drawHeading(window, "History", px, py, theme.textSecondary);
        py += lineHeight - 2;
        
        if (history.empty()) {
            text.drawSmall(window, "No runs yet", px, py, theme.textSecondary);
            py += lineHeight - 4;
        } else {
            // Draw column headers
            text.drawSmall(window, "#", px, py, theme.textSecondary);
            text.drawSmall(window, "Algo", px + 22, py, theme.textSecondary);
            text.drawSmall(window, "Time", px + 78, py, theme.textSecondary);
            text.drawSmall(window, "Visited", px + 135, py, theme.textSecondary);
            text.drawSmall(window, "Cost", px + 195, py, theme.textSecondary);
            py += lineHeight - 6;
            
            // Draw separator line
            sf::RectangleShape separator(sf::Vector2f(width_ - 30, 1));
            separator.setPosition(sf::Vector2f(px, py));
            separator.setFillColor(theme.gridLine);
            window.draw(separator);
            py += 4;
            
            // Draw history entries, most recent first
            for (auto it = history.entries().rbegin(); it != history.entries().rend(); ++it) {
                const auto& entry = *it;
                
                sf::Color rowColor = entry.stats.pathFound ? theme.textPrimary : theme.textSecondary;
                
                text.drawSmall(window, std::to_string(entry.runNumber), px, py, rowColor);
                text.drawSmall(window, entry.algorithmShort, px + 22, py, rowColor);
                
                std::ostringstream timeStr;
                timeStr << std::fixed << std::setprecision(0) << entry.stats.durationMs;
                text.drawSmall(window, timeStr.str() + "ms", px + 78, py, rowColor);
                
                text.drawSmall(window, std::to_string(entry.stats.nodesVisited), px + 135, py, rowColor);
                text.drawSmall(window, std::to_string(entry.stats.pathCost), px + 195, py, rowColor);
                
                py += lineHeight - 6;
            }
        }
        
        py += sectionGap;
        
        // Controls - spelled out clearly
        text.drawHeading(window, "Controls", px, py, theme.textSecondary);
        py += lineHeight - 2;
        
        // Algorithms section
        text.drawSmall(window, "Algorithms:", px, py, theme.textSecondary);
        py += lineHeight - 6;
        
        std::vector<std::pair<std::string, std::string>> algoControls = {
            {"[A]", "A* Search"},
            {"[B]", "BFS (Breadth-First)"},
            {"[D]", "Dijkstra"},
            {"[G]", "Greedy Best-First"}
        };
        
        for (const auto& [key, desc] : algoControls) {
            text.drawSmall(window, key + "  " + desc, px + 10, py, theme.textPrimary);
            py += lineHeight - 6;
        }
        
        py += 4;
        
        // Actions section
        text.drawSmall(window, "Actions:", px, py, theme.textSecondary);
        py += lineHeight - 6;
        
        std::vector<std::pair<std::string, std::string>> actionControls = {
            {"[Space]", "Run Algorithm"},
            {"[R]", "Full Reset"},
            {"[E]", "Soft Reset (keep walls)"},
            {"[H]", "Clear History"}
        };
        
        for (const auto& [key, desc] : actionControls) {
            text.drawSmall(window, key + "  " + desc, px + 10, py, theme.textPrimary);
            py += lineHeight - 6;
        }
        
        py += 4;
        
        // Other controls
        text.drawSmall(window, "Other:", px, py, theme.textSecondary);
        py += lineHeight - 6;
        
        std::vector<std::pair<std::string, std::string>> otherControls = {
            {"[+/-]", "Animation Speed"},
            {"[T]", "Change Theme"},
            {"[Esc]", "Exit"}
        };
        
        for (const auto& [key, desc] : otherControls) {
            text.drawSmall(window, key + "  " + desc, px + 10, py, theme.textPrimary);
            py += lineHeight - 6;
        }
        
        py += 4;
        
        // Mouse controls
        text.drawSmall(window, "Mouse:", px, py, theme.textSecondary);
        py += lineHeight - 6;
        text.drawSmall(window, "Left Click: Place Start/End/Walls", px + 10, py, theme.textPrimary);
        py += lineHeight - 6;
        text.drawSmall(window, "Right Click: Remove Walls", px + 10, py, theme.textPrimary);
        py += lineHeight;
        
        // Terrain Legend
        text.drawHeading(window, "Terrain", px, py, theme.textSecondary);
        py += lineHeight - 2;
        
        drawColorBox(window, px, py, 14, 14, theme.terrainEasy);
        text.drawSmall(window, "Easy (cost 1)", px + 20, py, theme.textPrimary);
        py += lineHeight - 4;
        
        drawColorBox(window, px, py, 14, 14, theme.terrainMedium);
        text.drawSmall(window, "Medium (cost 4)", px + 20, py, theme.textPrimary);
        py += lineHeight - 4;
        
        drawColorBox(window, px, py, 14, 14, theme.terrainHard);
        text.drawSmall(window, "Hard (cost 8)", px + 20, py, theme.textPrimary);
        py += lineHeight - 4;
        
        drawColorBox(window, px, py, 14, 14, theme.wall);
        text.drawSmall(window, "Wall (blocked)", px + 20, py, theme.textPrimary);
    }

private:
    void drawColorBox(sf::RenderWindow& window, float x, float y, 
                      float w, float h, const sf::Color& color) {
        sf::RectangleShape box(sf::Vector2f(w, h));
        box.setPosition(sf::Vector2f(x, y));
        box.setFillColor(color);
        box.setOutlineColor(sf::Color(100, 100, 100));
        box.setOutlineThickness(1.0f);
        window.draw(box);
    }
    
    float x_, y_, width_;
    UIPanel panel_;
};


// Main application, ties everything together

class PathfinderApp {
public:
    PathfinderApp() 
        : grid_(Config::ROWS, Config::COLS)
        , currentAlgo_(Algorithm::AStar)
        , animationDelay_(Config::DEFAULT_ANIMATION_DELAY_MS)
        , isRunning_(false) {
        
        srand(static_cast<unsigned>(time(nullptr)));
        grid_.generateTerrain();
    }
    
    void run() {
        // Create fullscreen window
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        window_.create(desktop, "Pathfinder Visualizer", sf::State::Fullscreen);
        window_.setFramerateLimit(60);
        
        // Calculate layout
        calculateLayout();
        
        // Create legend
        legend_ = std::make_unique<Legend>(
            legendX_, gridOffsetY_, Config::LEGEND_WIDTH
        );
        
        // Initial setup
        grid_.updateLayout(gridOffsetX_, gridOffsetY_, gridWidth_, gridHeight_);
        grid_.updateColors(themes_.current());
        
        // Main loop
        while (window_.isOpen()) {
            handleEvents();
            handleContinuousInput();
            render();
        }
    }

private:
    void calculateLayout() {
        float windowWidth = static_cast<float>(window_.getSize().x);
        float windowHeight = static_cast<float>(window_.getSize().y);
        
        // Calculate grid dimensions (square cells)
        float maxGridWidth = windowWidth - Config::LEGEND_WIDTH - Config::PADDING * 3;
        float maxGridHeight = windowHeight - Config::PADDING * 2;
        
        float cellSize = std::min(maxGridWidth / Config::COLS, maxGridHeight / Config::ROWS);
        
        gridWidth_ = cellSize * Config::COLS;
        gridHeight_ = cellSize * Config::ROWS;
        
        // Center grid vertically, place legend to the right
        gridOffsetX_ = Config::PADDING;
        gridOffsetY_ = (windowHeight - gridHeight_) / 2.0f;
        legendX_ = gridOffsetX_ + gridWidth_ + Config::PADDING;
    }
    
    void handleEvents() {
        while (auto eventOpt = window_.pollEvent()) {
            const sf::Event& event = *eventOpt;
            
            if (event.is<sf::Event::Closed>()) {
                window_.close();
            }
            else if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
                handleKeyPress(keyPressed->code);
            }
            else if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
                handleMousePress(*mousePressed);
            }
            else if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
                handleMouseRelease(*mouseReleased);
            }
        }
    }
    
    void handleKeyPress(sf::Keyboard::Key key) {
        if (isRunning_) return;  // Ignore input during algorithm execution
        
        switch (key) {
            case sf::Keyboard::Key::Escape:
                window_.close();
                break;
                
            case sf::Keyboard::Key::A:
                currentAlgo_ = Algorithm::AStar;
                break;
                
            case sf::Keyboard::Key::B:
                currentAlgo_ = Algorithm::BFS;
                break;
                
            case sf::Keyboard::Key::D:
                currentAlgo_ = Algorithm::Dijkstra;
                break;
                
            case sf::Keyboard::Key::G:
                currentAlgo_ = Algorithm::GreedyBestFirst;
                break;
                
            case sf::Keyboard::Key::Space:
                runAlgorithm();
                break;
                
            case sf::Keyboard::Key::R:
                fullReset();
                break;
                
            case sf::Keyboard::Key::E:
                softReset();
                break;
                
            case sf::Keyboard::Key::T:
                cycleTheme();
                break;
                
            case sf::Keyboard::Key::H:
                history_.clear();
                break;
                
            case sf::Keyboard::Key::Equal:  // + key
            case sf::Keyboard::Key::Add:
                decreaseAnimationDelay();
                break;
                
            case sf::Keyboard::Key::Hyphen:  // - key
            case sf::Keyboard::Key::Subtract:
                increaseAnimationDelay();
                break;
                
            default:
                break;
        }
    }
    
    void handleMousePress(const sf::Event::MouseButtonPressed& event) {
        if (isRunning_) return;
        
        if (event.button == sf::Mouse::Button::Left) {
            leftMouseDown_ = true;
            handleLeftClick(event.position.x, event.position.y);
        } else if (event.button == sf::Mouse::Button::Right) {
            rightMouseDown_ = true;
        }
    }
    
    void handleMouseRelease(const sf::Event::MouseButtonReleased& event) {
        if (event.button == sf::Mouse::Button::Left) {
            leftMouseDown_ = false;
        } else if (event.button == sf::Mouse::Button::Right) {
            rightMouseDown_ = false;
        }
    }
    
    void handleContinuousInput() {
        if (isRunning_) return;
        
        if (!leftMouseDown_ && !rightMouseDown_) return;
        
        sf::Vector2i mousePos = sf::Mouse::getPosition(window_);
        auto gridPos = screenToGrid(mousePos.x, mousePos.y);
        
        if (!gridPos) return;
        
        Cell& cell = grid_.at(*gridPos);
        
        if (leftMouseDown_) {
            // Only place walls during drag (start/end placed on click)
            if (cell.type == CellType::Empty && 
                grid_.startPos() && grid_.endPos()) {
                cell.type = CellType::Wall;
                cell.updateColor(themes_.current());
            }
        } else if (rightMouseDown_) {
            if (cell.type == CellType::Wall) {
                cell.type = CellType::Empty;
                cell.updateColor(themes_.current());
            }
        }
    }
    
    void handleLeftClick(int screenX, int screenY) {
        auto gridPos = screenToGrid(screenX, screenY);
        if (!gridPos) return;
        
        Cell& cell = grid_.at(*gridPos);
        
        if (cell.type != CellType::Empty) return;
        
        if (!grid_.startPos()) {
            grid_.setStart(*gridPos);
            cell.updateColor(themes_.current());
        } else if (!grid_.endPos()) {
            grid_.setEnd(*gridPos);
            cell.updateColor(themes_.current());
        } else {
            cell.type = CellType::Wall;
            cell.updateColor(themes_.current());
        }
    }
    
    std::optional<Position> screenToGrid(int screenX, int screenY) {
        float relX = screenX - gridOffsetX_;
        float relY = screenY - gridOffsetY_;
        
        if (relX < 0 || relY < 0) return std::nullopt;
        
        float cellWidth = gridWidth_ / Config::COLS;
        float cellHeight = gridHeight_ / Config::ROWS;
        
        int col = static_cast<int>(relX / cellWidth);
        int row = static_cast<int>(relY / cellHeight);
        
        if (row >= 0 && row < Config::ROWS && col >= 0 && col < Config::COLS) {
            return Position{row, col};
        }
        
        return std::nullopt;
    }
    
    void runAlgorithm() {
        if (!grid_.hasStartAndEnd() || isRunning_) return;
        
        isRunning_ = true;
        grid_.clearAlgorithmResults();
        grid_.updateColors(themes_.current());
        stats_.reset();
        
        auto algo = AlgorithmFactory::create(currentAlgo_);
        
        // Visualization callback
        auto visualize = [this]() {
            grid_.updateColors(themes_.current());
            render();
        };
        
        // Sleep callback
        auto sleep = [this](int multiplier) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(animationDelay_ * multiplier)
            );
        };
        
        stats_ = algo->run(grid_, visualize, sleep);
        
        // Add to history
        history_.addEntry(algo->name(), algo->shortName(), stats_);
        
        grid_.updateColors(themes_.current());
        isRunning_ = false;
    }
    
    void fullReset() {
        grid_.fullReset();
        grid_.updateLayout(gridOffsetX_, gridOffsetY_, gridWidth_, gridHeight_);
        grid_.updateColors(themes_.current());
        stats_.reset();
    }
    
    void softReset() {
        grid_.softReset();
        grid_.updateColors(themes_.current());
        stats_.reset();
    }
    
    void cycleTheme() {
        themes_.next();
        grid_.updateColors(themes_.current());
    }
    
    void increaseAnimationDelay() {
        animationDelay_ = std::min(animationDelay_ + Config::ANIMATION_STEP, 
                                   Config::MAX_ANIMATION_DELAY_MS);
    }
    
    void decreaseAnimationDelay() {
        animationDelay_ = std::max(animationDelay_ - Config::ANIMATION_STEP,
                                   Config::MIN_ANIMATION_DELAY_MS);
    }
    
    void render() {
        window_.clear(themes_.current().background);
        
        // Draw grid
        grid_.draw(window_);
        
        // Draw legend
        legend_->draw(window_, themes_.current(), textRenderer_, 
                      currentAlgo_, stats_, animationDelay_, history_);
        
        window_.display();
    }

private:
    sf::RenderWindow window_;
    Grid grid_;
    ThemeManager themes_;
    TextRenderer textRenderer_;
    std::unique_ptr<Legend> legend_;
    
    Algorithm currentAlgo_;
    RunStats stats_;
    RunHistory history_;
    int animationDelay_;
    bool isRunning_;
    
    bool leftMouseDown_ = false;
    bool rightMouseDown_ = false;
    
    // layout measurements
    float gridOffsetX_ = 0;
    float gridOffsetY_ = 0;
    float gridWidth_ = 0;
    float gridHeight_ = 0;
    float legendX_ = 0;
};


int main() {
    PathfinderApp app;
    app.run();
    return 0;
}


/*
    Build commands

    Mac with Homebrew
    g++ -std=c++17 pathfinder_visualizer.cpp -o pathfinder \
        -I/opt/homebrew/include -L/opt/homebrew/lib \
        -lsfml-graphics -lsfml-window -lsfml-system

    Linux
    g++ -std=c++17 pathfinder_visualizer.cpp -o pathfinder \
        -lsfml-graphics -lsfml-window -lsfml-system

    Windows with MSYS2
    g++ -std=c++17 pathfinder_visualizer.cpp -o pathfinder.exe \
        -lsfml-graphics -lsfml-window -lsfml-system
*/
