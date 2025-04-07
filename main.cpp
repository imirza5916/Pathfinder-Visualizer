#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <vector>
#include <queue>
#include <unordered_map>
#include <set>
#include <cmath>
#include <string>
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

enum CellType {
    Empty, Start, End, Wall, Visited, Path
};

struct Cell {
    sf::RectangleShape shape;
    CellType type = Empty;
    int row = 0, col = 0;
    int weight = 1; // cost to travel
};

struct Theme {
    sf::Color base = sf::Color(200, 200, 200);
    sf::Color wall = sf::Color::Black;
    sf::Color visited = sf::Color::Transparent;
    sf::Color path = sf::Color::Yellow;
    sf::Color start = sf::Color::Green;
    sf::Color end = sf::Color(255, 140, 0);
    sf::Color bg = sf::Color(245, 245, 245, 255); // light gray with full opacity
};

std::vector<Theme> themes = {
    Theme{sf::Color(200, 200, 200), sf::Color::Black, sf::Color::Transparent, sf::Color::Yellow,
          sf::Color::Green, sf::Color(255, 140, 0), sf::Color(245, 245, 245, 255)}, // Default
    Theme{sf::Color(220, 220, 220), sf::Color(80, 80, 80), sf::Color(38, 139, 210), sf::Color(181, 137, 0),
          sf::Color(133, 153, 0), sf::Color(220, 50, 47), sf::Color(250, 250, 250)},
    Theme{sf::Color(40, 40, 60), sf::Color(10, 10, 30), sf::Color(70, 130, 180), sf::Color(255, 215, 0),
          sf::Color(60, 179, 113), sf::Color(255, 69, 0), sf::Color(25, 25, 45)}
};


int currentThemeIndex = 0;
Theme theme = themes[currentThemeIndex];
const int rows = 25;
const int cols = 25;
const int gridWidth = 800;
const int windowWidth = 1000;
const int windowHeight = 600;
float cellWidth = static_cast<float>(gridWidth) / cols;
float cellHeight = static_cast<float>(windowHeight) / rows;

int animationDelay = 10;
bool useAStar = true;

int manhattan(std::pair<int, int> a, std::pair<int, int> b) {
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}
auto CellIndex = [](int r, int c) {
    return r * cols + c; };


struct RunStats {
    int visited = 0;
    int pathLength = 0;
    float durationMs = 0;
};

void drawText(sf::RenderWindow& window, const std::string& str, float x, float y, unsigned int size = 16) {
    static sf::Font font;
    static bool loaded = false;
    if (!loaded) {
        font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
        loaded = true;
    }
    sf::Text text(str, font, size);
    text.setFillColor(sf::Color::Black);
    text.setStyle(sf::Text::Bold);
    text.setPosition(x, y);
    window.draw(text);
}

RunStats astar(std::vector<std::vector<Cell>>& grid, sf::RenderWindow& window) {
    RunStats stats;
    auto startClock = sf::Clock();

    std::pair<int, int> start, end;
    bool foundStart = false;

    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            if (grid[r][c].type == Start) {
                start = {r, c};
                foundStart = true;
            }
            if (grid[r][c].type == End)
                end = {r, c};
        }

    if (!foundStart) return stats;

    std::set<std::pair<int, int>> openSet;
    std::unordered_map<int, int> gScore, fScore;
    std::unordered_map<int, std::pair<int, int>> cameFrom;
    auto index = [](int r, int c) { return r * cols + c; };

    openSet.insert(start);
    gScore[index(start.first, start.second)] = 0;
    fScore[index(start.first, start.second)] = manhattan(start, end);

    int dRow[] = {-1, 1, 0, 0};
    int dCol[] = {0, 0, -1, 1};

    while (!openSet.empty()) {
        std::pair<int, int> current;
        int minF = INT_MAX;
        for (auto pos : openSet) {
            int id = index(pos.first, pos.second);
            if (fScore.count(id) && fScore[id] < minF) {
                minF = fScore[id];
                current = pos;
            }
        }
        if (current == end) break;

        openSet.erase(current);
        auto [r, c] = current;

        for (int i = 0; i < 4; ++i) {
            int nr = r + dRow[i], nc = c + dCol[i];
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;
            Cell& neighbor = grid[nr][nc];
            if (neighbor.type == Wall) continue;

            int neighborId = index(nr, nc);
            int tentativeG = gScore[index(r, c)] + neighbor.weight;

            if (!gScore.count(neighborId) || tentativeG < gScore[neighborId]) {
                cameFrom[neighborId] = {r, c};
                gScore[neighborId] = tentativeG;
                fScore[neighborId] = tentativeG + manhattan({nr, nc}, end);
                openSet.insert({nr, nc});

                if (neighbor.type == Empty) {
                    neighbor.type = Visited;
                    neighbor.shape.setFillColor(sf::Color(theme.visited.r, theme.visited.g, theme.visited.b, 100)); // light transparent
                    stats.visited++;

                    window.clear(theme.bg);
                    for (auto& rowVec : grid)
                        for (auto& cell : rowVec)
                            window.draw(cell.shape);
                    window.display();
                    usleep(animationDelay * 1000);
                }
            }
        }
    }

    std::pair<int, int> current = end;
    while (current != start && cameFrom.count(index(current.first, current.second))) {
        auto [r, c] = current;
        if (grid[r][c].type != Start && grid[r][c].type != End) {
            grid[r][c].type = Path;
            // Final path is BLUE with transparency
            int w = grid[r][c].weight;
            if (w <= 3)
                grid[r][c].shape.setFillColor(sf::Color(0, 120, 255)); // Blue (easy)
            else if (w <= 6)
                grid[r][c].shape.setFillColor(sf::Color(255, 215, 0)); // Yellow (moderate)
            else
                grid[r][c].shape.setFillColor(sf::Color(255, 0, 0));  // Red (hard)
                stats.pathLength++;
        }
        current = cameFrom[index(r, c)];
        window.clear(theme.bg);
        for (auto& rowVec : grid)
            for (auto& cell : rowVec)
                window.draw(cell.shape);
        window.display();
        usleep(animationDelay * 1000);
    }
    // Clear all non-final visited blocks to show original terrain
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            Cell& cell = grid[r][c];
            if (cell.type == Visited) {
                cell.type = Empty;
                int shade = 255 - cell.weight * 20;
                cell.shape.setFillColor(sf::Color(shade, shade, shade));
            }
        }


    stats.durationMs = startClock.getElapsedTime().asMilliseconds();
    return stats;
}

RunStats bfs(std::vector<std::vector<Cell>>& grid, sf::RenderWindow& window) {
    RunStats stats;
    auto startClock = sf::Clock();

    std::pair<int, int> start, end;
    bool foundStart = false;

    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            if (grid[r][c].type == Start) {
                start = {r, c};
                foundStart = true;
            }
            if (grid[r][c].type == End)
                end = {r, c};
        }

    if (!foundStart) return stats;

    using Node = std::pair<int, std::pair<int, int>>; // {totalCost, {row, col}}
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    std::unordered_map<int, std::pair<int, int>> parent;
    std::unordered_map<int, int> cost;
    std::vector<std::vector<bool>> seen(rows, std::vector<bool>(cols, false));

    int startIdx = CellIndex(start.first, start.second);
    cost[startIdx] = 0;
    pq.push({0, start});

    std::queue<std::pair<int, int>> visualQueue;
    visualQueue.push(start);
    seen[start.first][start.second] = true;

    int dRow[] = {-1, 1, 0, 0};
    int dCol[] = {0, 0, -1, 1};

    // Expansion visual (classic BFS-style)
    while (!visualQueue.empty()) {
        auto [r, c] = visualQueue.front(); visualQueue.pop();

        for (int i = 0; i < 4; ++i) {
            int nr = r + dRow[i], nc = c + dCol[i];
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;
            if (seen[nr][nc] || grid[nr][nc].type == Wall) continue;

            seen[nr][nc] = true;
            visualQueue.push({nr, nc});

            if (grid[nr][nc].type == Empty) {
                grid[nr][nc].type = Visited;
                grid[nr][nc].shape.setFillColor(sf::Color(theme.visited.r, theme.visited.g, theme.visited.b, 100));
                stats.visited++;

                window.clear(theme.bg);
                for (auto& rowVec : grid)
                    for (auto& cell : rowVec)
                        window.draw(cell.shape);
                window.display();
                usleep(animationDelay * 1000);
            }
        }
    }

    // Actual shortest weighted path (Dijkstra logic)
    pq = std::priority_queue<Node, std::vector<Node>, std::greater<Node>>();
    cost.clear();
    parent.clear();

    cost[startIdx] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [currCost, pos] = pq.top(); pq.pop();
        int r = pos.first, c = pos.second;

        if (pos == end) break;

        for (int i = 0; i < 4; ++i) {
            int nr = r + dRow[i], nc = c + dCol[i];
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;
            if (grid[nr][nc].type == Wall) continue;

            int nIdx = CellIndex(nr, nc);
            int newCost = currCost + grid[nr][nc].weight;

            if (!cost.count(nIdx) || newCost < cost[nIdx]) {
                cost[nIdx] = newCost;
                parent[nIdx] = {r, c};
                pq.push({newCost, {nr, nc}});
            }
        }
    }

    // Reconstruct shortest path with weight coloring
    std::pair<int, int> current = end;
    while (current != start && parent.count(CellIndex(current.first, current.second))) {
        auto [r, c] = current;
        if (grid[r][c].type != Start && grid[r][c].type != End) {
            grid[r][c].type = Path;

            int w = grid[r][c].weight;
            if (w <= 3)
                grid[r][c].shape.setFillColor(sf::Color(0, 120, 255)); // Blue
            else if (w <= 6)
                grid[r][c].shape.setFillColor(sf::Color(255, 215, 0)); // Yellow
            else
                grid[r][c].shape.setFillColor(sf::Color(255, 0, 0));   // Red

            stats.pathLength++;
        }

        current = parent[CellIndex(r, c)];

        window.clear(theme.bg);
        for (auto& rowVec : grid)
            for (auto& cell : rowVec)
                window.draw(cell.shape);
        window.display();
        usleep(animationDelay * 1000);
    }

    // Clear visited visuals
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            if (grid[r][c].type == Visited) {
                grid[r][c].type = Empty;
                int shade = 255 - grid[r][c].weight * 20;
                grid[r][c].shape.setFillColor(sf::Color(shade, shade, shade));
            }

    stats.durationMs = startClock.getElapsedTime().asMilliseconds();
    return stats;
}


int main() {
    srand(static_cast<unsigned>(time(0)));

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(desktop, "Pathfinder Visualizer", sf::Style::Fullscreen);
    std::vector<std::vector<Cell>> grid(rows, std::vector<Cell>(cols));
    bool startPlaced = false, endPlaced = false;
    bool leftMouseDown = false, rightMouseDown = false;
    RunStats latestStats;

    float gridOffsetX = (window.getSize().x - gridWidth - 200) / 2.0f; // Horizontal center + legend space
    float gridOffsetY = (window.getSize().y - (rows * cellHeight)) / 2.0f; // Vertical center
    // Generate grid with weights once
    auto generateGrid = [&]() {
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c) {
                Cell& cell = grid[r][c];
                cell.row = r; cell.col = c;
                cell.type = Empty;
                int randVal = rand() % 3;
                if (randVal == 0) cell.weight = 1;      // Easy
                else if (randVal == 1) cell.weight = 5; // Medium
                else cell.weight = 9;                   // Hard
                cell.shape.setSize(sf::Vector2f(cellWidth - 1, cellHeight - 1));
                float gridOffsetX = (window.getSize().x - gridWidth - 200) / 2.0f;
                cell.shape.setPosition(gridOffsetX + c * cellWidth, gridOffsetY + r * cellHeight);
                sf::Color terrainColor;
                if (cell.weight == 1)
                    terrainColor = sf::Color(230, 230, 230); // light
                else if (cell.weight == 5)
                    terrainColor = sf::Color(160, 160, 160); // medium
                else
                    terrainColor = sf::Color(70, 70, 70);    // dark
                cell.shape.setFillColor(terrainColor);
            }
    };

    generateGrid();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) leftMouseDown = true;
                else if (event.mouseButton.button == sf::Mouse::Right) rightMouseDown = true;

                int col = (event.mouseButton.x - gridOffsetX) / cellWidth;
                int row = (event.mouseButton.y - gridOffsetY) / cellHeight;
                if (col < cols && row < rows) {
                    Cell& cell = grid[row][col];
                    if (cell.type == Empty && !startPlaced) {
                        cell.type = Start;
                        cell.shape.setFillColor(theme.start);
                        startPlaced = true;
                    } else if (cell.type == Empty && !endPlaced) {
                        cell.type = End;
                        cell.shape.setFillColor(theme.end);
                        endPlaced = true;
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) leftMouseDown = false;
                else if (event.mouseButton.button == sf::Mouse::Right) rightMouseDown = false;
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    latestStats = useAStar ? astar(grid, window) : bfs(grid, window);
                } else if (event.key.code == sf::Keyboard::R) {
                    generateGrid();  // correct
                    startPlaced = false;
                    endPlaced = false;
                    latestStats = {};
                } else if (event.key.code == sf::Keyboard::E) {
                    for (auto& row : grid)
                        for (auto& cell : row) {
                            if (cell.type == Visited || cell.type == Path) {
                                cell.type = Empty;
                                int shade = 255 - cell.weight * 20;
                                cell.shape.setFillColor(sf::Color(shade, shade, shade));
                            }
                        }
                    startPlaced = true; endPlaced = true;
                    latestStats = {};
                } else if (event.key.code == sf::Keyboard::A) {
                    useAStar = true;
                } else if (event.key.code == sf::Keyboard::B) {
                    useAStar = false;
                }
            }
        }

        if (leftMouseDown || rightMouseDown) {
            sf::Vector2i pos = sf::Mouse::getPosition(window);
            int col = (pos.x - gridOffsetX) / cellWidth;
            int row = (pos.y - gridOffsetY) / cellHeight;
            if (col < cols && row < rows) {
                Cell& cell = grid[row][col];
                if (leftMouseDown && cell.type == Empty) {
                    cell.type = Wall;
                    cell.shape.setFillColor(theme.wall);
                } else if (rightMouseDown && cell.type == Wall) {
                    cell.type = Empty;
                    int shade = 255 - cell.weight * 20;
                    cell.shape.setFillColor(sf::Color(shade, shade, shade));
                }
            }
        }

        window.clear(theme.bg);
        for (auto& row : grid)
            for (auto& cell : row)
                window.draw(cell.shape);

        float offsetX = gridOffsetX + gridWidth + 40;
        drawText(window, "⚡ Pathfinder Visualizer", offsetX, 10, 24);
        drawText(window, "Algorithm: " + std::string(useAStar ? "A*" : "BFS"), offsetX, 45);
        std::ostringstream timeStream;
        timeStream.precision(2);
        timeStream << std::fixed << (latestStats.durationMs / 1000.0f);
        drawText(window, "Time: " + timeStream.str() + " sec", offsetX, 70);
        drawText(window, "Possible Blocks: " + std::to_string(latestStats.visited), offsetX, 100);
        drawText(window, "Path Length: " + std::to_string(latestStats.pathLength), offsetX, 130);
        drawText(window, "", offsetX, 140);
        drawText(window, "Controls:", offsetX, 160, 20);
        drawText(window, "[A] Use A*", offsetX, 185);
        drawText(window, "[B] Use BFS", offsetX, 210);
        drawText(window, "[Space] Run", offsetX, 235);
        drawText(window, "[R] Full Reset", offsetX, 260);
        drawText(window, "[E] Soft Reset", offsetX, 285);
        drawText(window, "[Left Click] Place", offsetX, 310);
        drawText(window, "[Right Click] Erase", offsetX, 335);

        window.display();
    }
    return 0;
}

/*
g++ -std=c++17 main.cpp -o main -I/opt/homebrew/include -L/opt/homebrew/lib -lsfml-graphics -lsfml-window -lsfml-system
./main
*/