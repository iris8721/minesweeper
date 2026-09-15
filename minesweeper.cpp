#include "raylib.h"

#include <vector>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <algorithm>
#include <stdexcept>

enum Difficulty {
    BEGINNER,
    INTERMEDIATE,
    EXPERT
};

struct DifficultyConfig {
    int width;
    int height;
    int mines;
    const char* name;
};

const DifficultyConfig DIFFICULTIES[] = {
    {9, 9, 10, "Beginner"},
    {16, 16, 40, "Intermediate"},
    {30, 16, 99, "Expert"}
};

const int CELL_SIZE = 32;
const int TOP_BAR_HEIGHT = 80;

enum CellState {
    HIDDEN,
    REVEALED,
    FLAGGED
};

struct Cell {
    bool isMine;
    int adjacentMines;
    CellState state;

    Cell() : isMine(false), adjacentMines(0), state(HIDDEN) {}
};

struct BestTimes {
    int beginner;
    int intermediate;
    int expert;

    BestTimes() : beginner(999), intermediate(999), expert(999) {}
};

BestTimes LoadBestTimesFromFile() {
    BestTimes times;
    std::ifstream file("minesweeper_times.txt");
    if (file.is_open()) {
        file >> times.beginner >> times.intermediate >> times.expert;
    }
    return times;
}

void SaveBestTimesToFile(const BestTimes& times) {
    std::ofstream file("minesweeper_times.txt");
    if (file.is_open()) {
        file << times.beginner << " " << times.intermediate << " " << times.expert;
    }
}

class Minesweeper {
private:
    std::vector<std::vector<Cell>> grid;
    bool gameOver;
    bool gameWon;
    bool firstClick;
    int remainingMines;
    int revealedCells;
    int gridWidth;
    int gridHeight;
    int mineCount;
    Difficulty currentDifficulty;


    double startTime;
    double currentTime;
    int elapsedSeconds;
    bool timerRunning;


    BestTimes bestTimes;


    void LoadBestTimes() {
        bestTimes = LoadBestTimesFromFile();
    }


    void SaveBestTimes() {
        SaveBestTimesToFile(bestTimes);
    }


    void UpdateBestTime() {
        int* bestTime = nullptr;
        switch (currentDifficulty) {
        case BEGINNER: bestTime = &bestTimes.beginner; break;
        case INTERMEDIATE: bestTime = &bestTimes.intermediate; break;
        case EXPERT: bestTime = &bestTimes.expert; break;
        default: return;
        }

        if (elapsedSeconds < *bestTime) {
            *bestTime = elapsedSeconds;
            SaveBestTimes();
        }
    }


    void InitGrid() {
        grid.clear();
        grid.resize(gridHeight, std::vector<Cell>(gridWidth));
        gameOver = false;
        gameWon = false;
        firstClick = true;
        remainingMines = mineCount;
        revealedCells = 0;
        startTime = 0;
        currentTime = 0;
        elapsedSeconds = 0;
        timerRunning = false;
    }


    void PlaceMines(int avoidX, int avoidY) {
        int minesPlaced = 0;
        while (minesPlaced < mineCount) {
            int x = rand() % gridWidth;
            int y = rand() % gridHeight;


            if ((std::abs(x - avoidX) <= 1 && std::abs(y - avoidY) <= 1) || grid[y][x].isMine) {
                continue;
            }

            grid[y][x].isMine = true;
            minesPlaced++;
        }


        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                if (!grid[y][x].isMine) {
                    grid[y][x].adjacentMines = CountAdjacentMines(x, y);
                }
            }
        }
    }


    int CountAdjacentMines(int x, int y) {
        int count = 0;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;

                int nx = x + dx;
                int ny = y + dy;

                if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight) {
                    if (grid[ny][nx].isMine) count++;
                }
            }
        }
        return count;
    }


    void RevealCell(int x, int y) {
        if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) return;
        if (grid[y][x].state != HIDDEN) return;

        grid[y][x].state = REVEALED;
        revealedCells++;


        if (grid[y][x].isMine) {
            gameOver = true;
            timerRunning = false;
            RevealAllMines();
            return;
        }


        if (grid[y][x].adjacentMines == 0) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    RevealCell(x + dx, y + dy);
                }
            }
        }


        if (revealedCells == gridWidth * gridHeight - mineCount) {
            gameWon = true;
            gameOver = true;
            timerRunning = false;
            UpdateBestTime();
        }
    }


    void RevealAllMines() {
        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                if (grid[y][x].isMine) {
                    grid[y][x].state = REVEALED;
                }
            }
        }
    }


    Color GetNumberColor(int number) {
        switch (number) {
        case 1: return BLUE;
        case 2: return GREEN;
        case 3: return RED;
        case 4: return DARKBLUE;
        case 5: return MAROON;
        case 6: return DARKGREEN;
        case 7: return BLACK;
        case 8: return GRAY;
        default: return BLACK;
        }
    }

public:
    Minesweeper(Difficulty difficulty) {
        currentDifficulty = difficulty;
        const DifficultyConfig& config = DIFFICULTIES[difficulty];
        gridWidth = config.width;
        gridHeight = config.height;
        mineCount = config.mines;
        if (mineCount > gridWidth * gridHeight - 9) {
            throw std::runtime_error("Invalid difficulty config: mine count must leave room for a safe first click");
        }
        InitGrid();
        LoadBestTimes();
    }

    void HandleClick(int mouseX, int mouseY, bool leftClick) {
        if (gameOver) return;
        if (mouseX < 0 || mouseY < TOP_BAR_HEIGHT) return;


        int gridX = mouseX / CELL_SIZE;
        int gridY = (mouseY - TOP_BAR_HEIGHT) / CELL_SIZE;


        if (gridX < 0 || gridX >= gridWidth || gridY < 0 || gridY >= gridHeight) {
            return;
        }

        if (leftClick) {

            if (grid[gridY][gridX].state == FLAGGED) return;

            if (firstClick) {
                PlaceMines(gridX, gridY);
                firstClick = false;
                startTime = GetTime();
                timerRunning = true;
            }

            RevealCell(gridX, gridY);
        }
        else {

            if (grid[gridY][gridX].state == HIDDEN) {
                grid[gridY][gridX].state = FLAGGED;
                remainingMines = std::clamp(remainingMines - 1, 0, mineCount);
            }
            else if (grid[gridY][gridX].state == FLAGGED) {
                grid[gridY][gridX].state = HIDDEN;
                remainingMines = std::clamp(remainingMines + 1, 0, mineCount);
            }
        }
    }

    void Update() {
        if (timerRunning) {
            currentTime = GetTime();
            elapsedSeconds = (int)(currentTime - startTime);

            if (elapsedSeconds > 999) {
                elapsedSeconds = 999;
            }
        }
    }

    void Draw() {

        DrawRectangle(0, 0, gridWidth * CELL_SIZE, TOP_BAR_HEIGHT, DARKGRAY);


        if (!gameOver) {
            DrawText(TextFormat("Mines: %d", remainingMines), 10, 15, 28, WHITE);
        }


        DrawText(TextFormat("Time: %03d", elapsedSeconds), 10, 45, 28, YELLOW);


        int bestTime = 999;
        switch (currentDifficulty) {
        case BEGINNER: bestTime = bestTimes.beginner; break;
        case INTERMEDIATE: bestTime = bestTimes.intermediate; break;
        case EXPERT: bestTime = bestTimes.expert; break;
        default: break;
        }
        const char* bestTimeText;
        if (bestTime == 999) {
            bestTimeText = "Best: ---";
        }
        else {
            bestTimeText = TextFormat("Best: %03d", bestTime);
        }
        int bestTimeWidth = MeasureText(bestTimeText, 24);
        DrawText(bestTimeText, gridWidth * CELL_SIZE - bestTimeWidth - 10, 15, 24, LIGHTGRAY);


        const char* diffName = DIFFICULTIES[currentDifficulty].name;
        int diffWidth = MeasureText(diffName, 24);
        DrawText(diffName, gridWidth * CELL_SIZE - diffWidth - 10, 45, 24, LIGHTGRAY);


        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                int posX = x * CELL_SIZE;
                int posY = y * CELL_SIZE + TOP_BAR_HEIGHT;
                Cell& cell = grid[y][x];


                if (cell.state == REVEALED) {
                    if (cell.isMine) {
                        DrawRectangle(posX, posY, CELL_SIZE, CELL_SIZE, RED);
                    }
                    else {
                        DrawRectangle(posX, posY, CELL_SIZE, CELL_SIZE, LIGHTGRAY);
                    }
                }
                else {
                    DrawRectangle(posX, posY, CELL_SIZE, CELL_SIZE, GRAY);
                }


                DrawRectangleLines(posX, posY, CELL_SIZE, CELL_SIZE, DARKGRAY);


                if (cell.state == REVEALED) {
                    if (cell.isMine) {

                        DrawCircle(posX + CELL_SIZE / 2, posY + CELL_SIZE / 2, CELL_SIZE / 3, BLACK);
                        DrawCircle(posX + CELL_SIZE / 2, posY + CELL_SIZE / 2, CELL_SIZE / 6, WHITE);
                    }
                    else if (cell.adjacentMines > 0) {

                        const char* numText = TextFormat("%d", cell.adjacentMines);
                        int textWidth = MeasureText(numText, 20);
                        DrawText(numText, posX + CELL_SIZE / 2 - textWidth / 2,
                            posY + CELL_SIZE / 2 - 10, 20, GetNumberColor(cell.adjacentMines));
                    }
                }
                else if (cell.state == FLAGGED) {

                    DrawRectangle(posX + CELL_SIZE / 2 - 1, posY + 6, 2, CELL_SIZE - 12, DARKBROWN);
                    Vector2 p1 = { (float)(posX + CELL_SIZE / 2), (float)(posY + 6) };
                    Vector2 p2 = { (float)(posX + CELL_SIZE / 2), (float)(posY + 16) };
                    Vector2 p3 = { (float)(posX + CELL_SIZE - 8), (float)(posY + 11) };
                    DrawTriangle(p1, p3, p2, RED);
                }
            }
        }

        if (gameOver) {
            DrawRectangle(0, TOP_BAR_HEIGHT, gridWidth * CELL_SIZE, gridHeight * CELL_SIZE, Fade(BLACK, 0.7f));

            int popupWidth = (gridWidth * CELL_SIZE) - 40;
            if (popupWidth > 500) popupWidth = 500;
            if (popupWidth < 280) popupWidth = 280;

            int popupHeight = 200;
            int popupX = (gridWidth * CELL_SIZE - popupWidth) / 2;
            int popupY = TOP_BAR_HEIGHT + (gridHeight * CELL_SIZE - popupHeight) / 2;

            DrawRectangle(popupX, popupY, popupWidth, popupHeight, RAYWHITE);
            Rectangle popupRect = { (float)popupX, (float)popupY, (float)popupWidth, (float)popupHeight };
            DrawRectangleLinesEx(popupRect, 4, BLACK);

            const char* message;
            Color messageColor;
            int messageFontSize = 48;

            if (gameWon) {
                message = "YOU WIN!";
                messageColor = GREEN;

                int* bestTime = nullptr;
                switch (currentDifficulty) {
                case BEGINNER: bestTime = &bestTimes.beginner; break;
                case INTERMEDIATE: bestTime = &bestTimes.intermediate; break;
                case EXPERT: bestTime = &bestTimes.expert; break;
                default: break;
                }

                if (bestTime && elapsedSeconds == *bestTime) {
                    const char* newBestText = "NEW BEST TIME!";
                    int newBestWidth = MeasureText(newBestText, 24);
                    DrawText(newBestText, popupX + popupWidth / 2 - newBestWidth / 2, popupY + 80, 24, GOLD);
                }
            }
            else {
                message = "GAME OVER";
                messageColor = RED;
            }

            int messageWidth = MeasureText(message, messageFontSize);
            if (messageWidth > popupWidth - 20) {
                messageFontSize = 36;
                messageWidth = MeasureText(message, messageFontSize);
            }

            DrawText(message, popupX + popupWidth / 2 - messageWidth / 2, popupY + 30, messageFontSize, messageColor);

            const char* timeText;
            if (elapsedSeconds == 1) {
                timeText = TextFormat("Time: %d second", elapsedSeconds);
            }
            else {
                timeText = TextFormat("Time: %d seconds", elapsedSeconds);
            }

            
            int timeWidth = MeasureText(timeText, 24);
            DrawText(timeText, popupX + popupWidth / 2 - timeWidth / 2, popupY + 110, 24, DARKGRAY);

            const char* instructions = (popupWidth < 400) ? "R: Restart | ESC: Menu" : "Press R to restart or ESC for menu";
            int instructionsWidth = MeasureText(instructions, 20);
            DrawText(instructions, popupX + popupWidth / 2 - instructionsWidth / 2, popupY + 150, 20, GRAY);
        }
    }

    void Reset() {
        InitGrid();
    }

    bool IsGameOver() const { return gameOver; }

    int GetWindowWidth() const { return gridWidth * CELL_SIZE; }
    int GetWindowHeight() const { return gridHeight * CELL_SIZE + TOP_BAR_HEIGHT; }
};

struct MenuButton {
    Rectangle rect;
    const char* text;
    Difficulty difficulty;
};

void DrawMenu(const MenuButton* buttons, int buttonCount, const BestTimes& bestTimes) {
    const int WINDOW_WIDTH = 400;
    const int WINDOW_HEIGHT = 500;

    ClearBackground(DARKGRAY);


    const char* title = "MINESWEEPER";
    int titleWidth = MeasureText(title, 48);
    DrawText(title, WINDOW_WIDTH / 2 - titleWidth / 2, 40, 48, YELLOW);


    for (int i = 0; i < buttonCount; i++) {
        Color buttonColor = GRAY;
        if (CheckCollisionPointRec(GetMousePosition(), buttons[i].rect)) {
            buttonColor = LIGHTGRAY;
        }

        DrawRectangleRec(buttons[i].rect, buttonColor);
        DrawRectangleLinesEx(buttons[i].rect, 2, BLACK);

        int textWidth = MeasureText(buttons[i].text, 28);
        DrawText(buttons[i].text,
            buttons[i].rect.x + buttons[i].rect.width / 2 - textWidth / 2,
            buttons[i].rect.y + buttons[i].rect.height / 2 - 14,
            28, BLACK);
    }


    DrawText("BEST TIMES", WINDOW_WIDTH / 2 - MeasureText("BEST TIMES", 28) / 2, 360, 28, YELLOW);

    const char* beginnerTime = (bestTimes.beginner == 999) ? "Beginner: ---" : TextFormat("Beginner: %d sec", bestTimes.beginner);
    const char* intermediateTime = (bestTimes.intermediate == 999) ? "Intermediate: ---" : TextFormat("Intermediate: %d sec", bestTimes.intermediate);
    const char* expertTime = (bestTimes.expert == 999) ? "Expert: ---" : TextFormat("Expert: %d sec", bestTimes.expert);

    DrawText(beginnerTime, 50, 405, 20, WHITE);
    DrawText(intermediateTime, 50, 435, 20, WHITE);
    DrawText(expertTime, 50, 465, 20, WHITE);
}

int main() {

    srand(time(nullptr));


    const int MENU_WIDTH = 400;
    const int MENU_HEIGHT = 500;
    InitWindow(MENU_WIDTH, MENU_HEIGHT, "Minesweeper");
    SetTargetFPS(60);
    SetExitKey(0);


    bool inMenu = true;
    Difficulty currentDifficulty = BEGINNER;
    std::unique_ptr<Minesweeper> game;


    BestTimes menuBestTimes = LoadBestTimesFromFile();


    MenuButton buttons[] = {
        {{100, 120, 200, 60}, "Beginner", BEGINNER},
        {{100, 200, 200, 60}, "Intermediate", INTERMEDIATE},
        {{100, 280, 200, 60}, "Expert", EXPERT}
    };


    while (!WindowShouldClose()) {
        if (inMenu) {

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                for (int i = 0; i < 3; i++) {
                    if (CheckCollisionPointRec(mousePos, buttons[i].rect)) {
                        currentDifficulty = buttons[i].difficulty;
                        game = std::make_unique<Minesweeper>(currentDifficulty);
                        inMenu = false;


                        SetWindowSize(game->GetWindowWidth(), game->GetWindowHeight());
                        break;
                    }
                }
            }


            BeginDrawing();
            DrawMenu(buttons, 3, menuBestTimes);
            EndDrawing();
        }
        else {

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                game->HandleClick((int)mousePos.x, (int)mousePos.y, true);
            }

            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                game->HandleClick((int)mousePos.x, (int)mousePos.y, false);
            }


            if (IsKeyPressed(KEY_R)) {
                game->Reset();
            }


            if (IsKeyPressed(KEY_ESCAPE)) {
                inMenu = true;
                SetWindowSize(MENU_WIDTH, MENU_HEIGHT);


                menuBestTimes = LoadBestTimesFromFile();
            }


            game->Update();


            BeginDrawing();
            ClearBackground(RAYWHITE);
            game->Draw();
            EndDrawing();
        }
    }


    CloseWindow();

    return 0;
}
