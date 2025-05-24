#include <SDL.h>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
using namespace std;

/*Here we initialise constants, default values
*  like radius of dishes 
*  grid layout and offsets
*/
const SDL_Color LIVE_COLOR = { 255, 255, 255, 255 };
const SDL_Color DEAD_COLOR = { 0, 0, 0, 255 };
const SDL_Color GRID_COLOR = { 0, 0, 0, 255 };
int SCREEN_WIDTH; //width of window
int SCREEN_HEIGHT; //height of window
int PETRI_RADIUS; // radius of dish's vis
int CELL_SIZE; // grid_cell_size ( edge of a square )
int GAME_SPEED; // speed of frame update
int GRID_WIDTH; // width of grid
int GRID_HEIGHT; // height of grid
int GRID_OFFSET_X; // offset
int GRID_OFFSET_Y;
// Defaults
const int Default_SCREEN_WIDTH = 800;
const int Default_SCREEN_HEIGHT = 800;
const int Default_PETRI_RADIUS = 350;
const int Default_CELL_SIZE = 20;

vector< vector<bool> > grid;
bool isPaused = true;
bool running = true;

void drawPetriDish(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    int center_x = SCREEN_WIDTH / 2;
    int center_y = SCREEN_HEIGHT / 2;
    for (int w = 0; w < PETRI_RADIUS * 2; w++) {
        for (int h = 0; h < PETRI_RADIUS * 2; h++) {
            int dx = w - PETRI_RADIUS;
            int dy = h - PETRI_RADIUS;
            if (dx * dx + dy * dy <= PETRI_RADIUS * PETRI_RADIUS) {
                SDL_RenderDrawPoint(renderer, center_x + dx, center_y + dy);
            }
        }
    }
}

bool isInsidePetri(int x, int y) {
    int center_x = SCREEN_WIDTH / 2;
    int center_y = SCREEN_HEIGHT / 2;
    return (x - center_x) * (x - center_x) + (y - center_y) * (y - center_y) <= PETRI_RADIUS * PETRI_RADIUS;
}

/*! Function that transforms global cords to grid cords 
* uses formula: global_Cord - grid_offset / grid_cell_size
* 
*/
pair<int, int> screenToGrid(int screenX, int screenY) {
    int gridX = (screenX - GRID_OFFSET_X) / CELL_SIZE;
    int gridY = (screenY - GRID_OFFSET_Y) / CELL_SIZE;
    if ( gridX >= 0 && gridX < GRID_WIDTH 
        && gridY >= 0 && gridY < GRID_HEIGHT
        && isInsidePetri(screenX, screenY) ) 
    {
        return { gridX, gridY };
    }
    return { -1, -1 };
}

// Visualise the grid with defined grid_cell_size and color
void renderGrid(SDL_Renderer* renderer) {
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            int screenX = GRID_OFFSET_X + x * CELL_SIZE;
            int screenY = GRID_OFFSET_Y + y * CELL_SIZE;

            if (isInsidePetri(screenX + CELL_SIZE / 2, screenY + CELL_SIZE / 2)) {
                SDL_Rect cellRect = { screenX + 1, screenY + 1, CELL_SIZE - 2, CELL_SIZE - 2 };
                if (grid[y][x]) {
                    SDL_SetRenderDrawColor(renderer, LIVE_COLOR.r, LIVE_COLOR.g, LIVE_COLOR.b, LIVE_COLOR.a);
                    SDL_RenderFillRect(renderer, &cellRect);
                }
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, GRID_COLOR.r, GRID_COLOR.g, GRID_COLOR.b, GRID_COLOR.a);
    for (int x = 0; x <= GRID_WIDTH; ++x) {
        int screenX = GRID_OFFSET_X + x * CELL_SIZE;
        for (int y = 0; y <= GRID_HEIGHT; ++y) {
            int screenY = GRID_OFFSET_Y + y * CELL_SIZE;
            if (isInsidePetri(screenX, screenY) || isInsidePetri(screenX, screenY - CELL_SIZE)) {
                SDL_RenderDrawLine(renderer, screenX, screenY, screenX, screenY - CELL_SIZE);
            }
            if (isInsidePetri(screenX, screenY) || isInsidePetri(screenX - CELL_SIZE, screenY)) {
                SDL_RenderDrawLine(renderer, screenX, screenY, screenX - CELL_SIZE, screenY);
            }
        }
    }
}

// Update filed's condition with the actual one
vector<vector<bool>> updateGrid() {
    vector<vector<bool>> nextGrid = grid;
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            int liveNeighbors = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dy == 0 && dx == 0) continue;
                    int nx = (x + dx + GRID_WIDTH) % GRID_WIDTH;
                    int ny = (y + dy + GRID_HEIGHT) % GRID_HEIGHT;
                    int screenNX = GRID_OFFSET_X + nx * CELL_SIZE + CELL_SIZE / 2;
                    int screenNY = GRID_OFFSET_Y + ny * CELL_SIZE + CELL_SIZE / 2;
                    if (grid[ny][nx] && isInsidePetri(screenNX, screenNY)) {
                        liveNeighbors++;
                    }
                }
            }
            // Rules of life-cycle
            if (grid[y][x]) {
                if (liveNeighbors < 2 || liveNeighbors > 3)
                {
                    nextGrid[y][x] = false;
                }
            }
            else {
                if (liveNeighbors == 3)
                {
                    nextGrid[y][x] = true;
                }
            }
        }
    }
    return nextGrid;
}

class Core {
    /* !Class that represents the lopp
    *   of main programm part working
    * Creates window
    * Updates frame
    * Handle k/m events
    */
public:
    Core() = default;
    // life_cycle
    void main_loop() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            cerr << "SDL initialization failed: " << SDL_GetError() << endl;
            return;
        }
        SDL_Event event;

        while (running) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) // Quit
                {
                    cout << "Application closed by user";
                        running = false;
                }
                else if (event.type == SDL_KEYDOWN) // Pause
                {
                    if (event.key.keysym.sym == SDLK_SPACE) {
                        isPaused = !isPaused;
                    }
                }
                else if (event.type == SDL_MOUSEBUTTONDOWN) // Click action
                {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int mouseX, mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY);
                        pair<int, int> gridPos = screenToGrid(mouseX, mouseY);
                        if (gridPos.first != -1) {
                            grid[gridPos.second][gridPos.first] = !grid[gridPos.second][gridPos.first];
                        }
                    }
                }
            }

            // Frame update
            if (!isPaused) {
                grid = updateGrid();
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            drawPetriDish(renderer);

            renderGrid(renderer);

            SDL_RenderPresent(renderer);

            SDL_Delay(GAME_SPEED);
        }

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

private:
    SDL_Window* window = SDL_CreateWindow("Game of life", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
        SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
};

int main(int argc, char* argv[]) {
    SCREEN_WIDTH = Default_SCREEN_WIDTH;
    SCREEN_HEIGHT = Default_SCREEN_HEIGHT;
    PETRI_RADIUS = Default_PETRI_RADIUS;
    CELL_SIZE = Default_CELL_SIZE;
    GAME_SPEED = 100;

    map<string, string> argMap;
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        size_t eqPos = arg.find('=');
        if (eqPos != string::npos) {
            argMap[arg.substr(0, eqPos)] = arg.substr(eqPos + 1);
        }
    }
    // Replace default values with console args if need
    if (argMap.count("--width")) {
        try {
            SCREEN_WIDTH = stoi(argMap["--width"]);
        }
        catch (const invalid_argument& e) {
            cerr << "ERROR" << argMap["--width"] << endl;
        }
        catch (const out_of_range& e) {
            cerr << "ERROR" << argMap["--width"] << endl;
        }
    }

    if (argMap.count("--height")) {
        try {
            SCREEN_HEIGHT = stoi(argMap["--height"]);
        }
        catch (const invalid_argument& e) {
            cerr << "ERROR" << argMap["--height"] << endl;
        }
        catch (const out_of_range& e) {
            cerr << "ERROR" << argMap["--height"] << endl;
        }
    }

    if (argMap.count("--radius")) {
        try {
            PETRI_RADIUS = stoi(argMap["--radius"]);
        }
        catch (const invalid_argument& e) {
            cerr << "ERROR" << argMap["--radius"] << endl;
        }
        catch (const out_of_range& e) {
            cerr << "ERROR" << argMap["--radius"] << endl;
        }
    }

    if (argMap.count("--grid_cell_size")) {
        try {
            CELL_SIZE = stoi(argMap["--cell_size"]);
        }
        catch (const invalid_argument& e) {
            cerr << "ERROR" << argMap["--cell_size"] << endl;
        }
        catch (const out_of_range& e) {
            cerr << "ERROR" << argMap["--cell_size"] << endl;
        }
    }

    if (argMap.count("--speed")) {
        try {
            GAME_SPEED = stoi(argMap["--speed"]);
        }
        catch (const invalid_argument& e) {
            cerr << "ERROR" << argMap["--speed"] << endl;
        }
        catch (const out_of_range& e) {
            cerr << "ERROR" << argMap["--speed"] << endl;
        }
    }

    GRID_WIDTH = static_cast<int>(2.0 * PETRI_RADIUS / CELL_SIZE);
    GRID_HEIGHT = static_cast<int>(2.0 * PETRI_RADIUS / CELL_SIZE);
    GRID_OFFSET_X = SCREEN_WIDTH / 2 - (GRID_WIDTH * CELL_SIZE) / 2;
    GRID_OFFSET_Y = SCREEN_HEIGHT / 2 - (GRID_HEIGHT * CELL_SIZE) / 2;
    grid.resize(GRID_HEIGHT, vector<bool>(GRID_WIDTH, false));

    Core c;
    c.main_loop();
   
    return 0;
}
