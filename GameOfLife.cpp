#include <iostream>
#include <vector>
#include <pthread.h>
#include <chrono>
#include <thread>
#include <string>
#include <armadillo>
using namespace std;

class Life;

struct miniLife {
    Life *life;
    int startRow;
    int endRow;
};

void *updateMiniLife(void *arg);

class Cell{
public:
    Cell(int currentState, int nextGenState) : currentState(currentState), nextGenState(nextGenState){}
    int currentState;
    int nextGenState;
};

class Life {
public:
    Life(int width, int height);
    double getCellState(int x, int y) const;
    int countALiveNeighbors(int x, int y) const;
    int countALiveNeighborsConvolution(int x, int y) const;
    void updateNextGenCellState(int x, int y, int aliveNeighbors);
    void updateNextGen();
    void threadedUpdateNextGen(int numThreads);
    void displayGrid() const;

    int getHeigth() const { return height; }
    int getWidth() const { return width; }
    void initializePatterns(string pattern);
private:
    int width;
    int height;
    vector<vector<Cell> > grid;

};

Life::Life(int width, int height) : width(width), height(height)
{
    grid = vector<vector<Cell> >(width, vector<Cell>(height, Cell(0,0)));
}

double Life::getCellState(int x, int y) const {
    if (x < 0 || x >= this->width || y < 0 || y >= this->height)
        return 0;
    return static_cast<double>(grid[x][y].currentState);
}

int Life::countALiveNeighbors(int x, int y) const {
    int aliveNeighbors = 0;
    for (int i = x - 1; i <= x + 1; i++) {
        for (int j = y - 1; j <= y + 1; j++) {
            if (i == x && j == y) continue;
            aliveNeighbors += getCellState(i, j);
        }
    }
    return aliveNeighbors;
}

int Life::countALiveNeighborsConvolution(int x, int y) const {
    arma::mat kernel = {{1, 1, 1},
                        {1, 0, 1},
                        {1, 1, 1}};

    arma::mat cellState = {{getCellState(x - 1, y - 1), getCellState(x, y - 1), getCellState(x + 1, y - 1)},
                           {getCellState(x - 1, y), getCellState(x, y), getCellState(x + 1, y)},
                           {getCellState(x - 1, y + 1), getCellState(x, y + 1), getCellState(x + 1, y + 1)}};

    arma::mat result = arma::conv2(cellState, kernel, "same");

    return result(1, 1);
}

void Life::updateNextGenCellState(int x, int y, int aliveNeighbors) {
    grid[x][y].nextGenState = grid[x][y].currentState;

    // live cell dies of underpopulation (less than 2 neighbors) or overpopulation (more than 3 neighbors)
    if (grid[x][y].currentState == 1) {
        if (aliveNeighbors < 2 || aliveNeighbors > 3) {
            grid[x][y].nextGenState = 0;
        }
    }
    // dead cell becomes alive when it has 3 neighbors
    else{
        if (aliveNeighbors == 3) {
            grid[x][y].nextGenState = 1;
        }
    }
}

void Life::updateNextGen() {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            //int aliveNeighbors = countALiveNeighbors(x, y);
            int aliveNeighbors = countALiveNeighborsConvolution(x, y);
            updateNextGenCellState(x, y, aliveNeighbors);
        }
    }

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            grid[x][y].currentState = grid[x][y].nextGenState;
        }
    }
}

void *updateMiniLife(void *arg) {
    miniLife *data = (miniLife *)arg;
    Life *life = data->life;

    for (int row = data->startRow; row < data->endRow; row++) {
        for (int pixel = 0; pixel < life->getWidth(); pixel++) {
            //int aliveNeighbors = life->countALiveNeighbors(x, y);
            int aliveNeighbors = life->countALiveNeighborsConvolution(row, pixel);
            life->updateNextGenCellState(row, pixel, aliveNeighbors);
        }
    }

    pthread_exit(0);
}

void Life::threadedUpdateNextGen(int numThreads) {
    pthread_t threads[numThreads];
    miniLife miniLives[numThreads];

    int rowsPerThread = height / numThreads;

    for (int i = 0; i < numThreads; i++) {
        miniLives[i].life = this;
        miniLives[i].startRow = i * rowsPerThread;
        miniLives[i].endRow = (i + 1) * rowsPerThread;
        if (i == numThreads - 1)
            miniLives[i].endRow = height;
        pthread_create(&threads[i], NULL, updateMiniLife, (void *)&miniLives[i]);
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            grid[x][y].currentState = grid[x][y].nextGenState;
        }
    }
}

void Life::displayGrid() const {
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            cout << (grid[x][y].currentState ? "█" : " ");
        }
        cout << endl;
    }
    cout << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
}

void Life::initializePatterns(string pattern) {
    if(pattern == "Square") {
        int A[20][20] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0},
                         {0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0},
                         {0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0}};

        for(int i = 0; i < 20; i++)
            for(int j = 0; j < 20; j++)
                grid[i][j].currentState = A[i][j];

    }

    if(pattern == "Blinker") {
        int A[20][20] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
                         {0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
                         {0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0}};

        for(int i = 0; i < 20; i++)
            for(int j = 0; j < 20; j++)
                grid[i][j].currentState = A[i][j];
    }

   if(pattern == "Glider") {
        int A[20][20] = {{0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

        for(int i = 0; i < 20; i++)
            for(int j = 0; j < 20; j++)
                grid[i][j].currentState = A[i][j];
   }

}

int main(int argc, char* argv[]) {
    string pattern;

    // Check if a valid pattern is passed
    if (argc == 1) {
        std::cout << "Please enter a pattern (Square, Glider, or Blinker): ";
        std::cin >> pattern;
    } else if (argc == 2) {
        pattern = argv[1];
    } else {
        std::cerr << "Usage: ./gol <pattern_name>\n";
        return 1;
    }

    int width = 20, height = 20, numThreads = 4, generations = 32;
    Life game(width, height);

    game.initializePatterns(pattern);

    while (generations--) {
        system("clear");
        game.displayGrid();
        game.threadedUpdateNextGen(numThreads);
    }

    return 0;
}

//g++ -pthread gof.cpp -o gol -I/opt/homebrew/Cellar/armadillo/14.2.2/include -L/opt/homebrew/Cellar/armadillo/14.2.2/lib -larmadillo -std=c++17