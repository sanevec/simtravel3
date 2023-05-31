#include <SFML/Graphics.hpp>
#include <cmath>
#include <functional>
#include <vector>

using namespace std;

struct LinealRegression {
    float sumX, sumY, sumXY, sumX2, n;
    LinealRegression() {
        sumX = sumY = sumXY = sumX2 = n = 0;
    }
    void addPoint(float x, float y) {
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
        sumXY += x * y;
        n++;
    }
    float predict(float x) {
        if (n < 2) {
            return 0;
        }
        float medx = sumX / n;
        float medy = sumY / n;
        float m = (sumXY - n * medx * medy) / (sumX2 - n * medx * medx);
        float b = medy - m * medx;
        return m * x + b;
    }

    float coeficienteDeterminacion() {
        float medx = sumX / n;
        float medy = sumY / n;
        float ssr = 0;
        float sst = 0;
        for (int i = 0; i < n; ++i) {
            float y = predict(i);
            ssr += (y - medy) * (y - medy);
            sst += (i - medx) * (i - medx);
        }
        return ssr / sst;
    }
};

struct Grid {
    static constexpr int CELL_SIZE = 5;
    int round = 0;

    Grid(int width, int height)
        : window(sf::VideoMode(width * CELL_SIZE, height * CELL_SIZE), "Game of Life"),
          gridWidth(width), gridHeight(height),
          grid(width, std::vector<bool>(height, false)),
          tempGrid(width, std::vector<bool>(height, false)) {
        // Inicializa el grid con un patrón inicial
        srand(time(nullptr));
        for (int x = 0; x < gridWidth; x++) {
            for (int y = 0; y < gridHeight; y++) {
                grid[x][y] = (rand() % 100) < 50;
            }
        }
    }

    void run() {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }

            update(*this);
            round++;
            render();
        }
    }

    sf::RenderWindow window;
    int gridWidth;
    int gridHeight;
    std::vector<std::vector<bool>> grid;
    std::vector<std::vector<bool>> tempGrid;

    bool getCell(int x, int y) {
        int newX = x % gridWidth;
        if (newX < 0) {
            newX += gridWidth;
        }
        int newY = y % gridHeight;
        if (newY < 0) {
            newY += gridHeight;
        }
        return grid[newX][newY];
    }

    int countNeighbors(int x, int y) {
        int count = 0;
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                if (i == 0 && j == 0)
                    continue;
                int newX = (x + i + gridWidth) % gridWidth;
                int newY = (y + j + gridHeight) % gridHeight;
                count += grid[newX][newY] ? 1 : 0;
            }
        }
        return count;
    }

    int countNeighborsV2(int x, int y, int radius) {
        int count = 0;
        for (int i = -radius; i <= radius; ++i) {
            for (int j = -radius; j <= radius; ++j) {
                if (i == 0 && j == 0)
                    continue;
                int newX = (x + i + gridWidth) % gridWidth;
                int newY = (y + j + gridHeight) % gridHeight;
                count += grid[newX][newY] ? 1 : 0;
            }
        }
        return count;
    }

    std::function<void(GameOfLife &)> update;

    void render() {
        window.clear();
        window.setFramerateLimit(3);

        for (int x = 0; x < gridWidth; ++x) {
            for (int y = 0; y < gridHeight; ++y) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                cell.setFillColor(grid[x][y] ? sf::Color::White : sf::Color::Black);

                window.draw(cell);
            }
        }

        window.display();
    }
};

void updateClasic(Grid &g) {
    for (int x = 0; x < g.gridWidth; ++x) {
        for (int y = 0; y < g.gridHeight; ++y) {
            int neighbors = g.countNeighbors(x, y);
            if (g.grid[x][y]) {
                g.tempGrid[x][y] = neighbors == 2 || neighbors == 3;
            } else {
                g.tempGrid[x][y] = neighbors == 3;
            }
        }
    }
    g.grid.swap(g.tempGrid);
}

void updateStreet(Grid &g) {
    for (int x = 0; x < g.gridWidth; ++x) {
        for (int y = 0; y < g.gridHeight; ++y) {
            int neighbors = g.countNeighbors(x, y);
            if (g.grid[x][y]) {
                g.tempGrid[x][y] = 1 <= neighbors && neighbors <= 4;
            } else {
                g.tempGrid[x][y] = neighbors == 3 || neighbors == 4;
            }
        }
    }
    g.grid.swap(g.tempGrid);
}

void updateStreetsV2(Grid &g) {
    int radius = 2;
    int lowerBound = (radius * 2) + 1;
    int upperBound = (radius * 2 + 1) * (radius * 2 + 1) - 1;
    int lowerThreshold = lowerBound;
    int upperThreshold = upperBound / 2;

    for (int x = 0; x < g.gridWidth; ++x) {
        for (int y = 0; y < g.gridHeight; ++y) {
            int neighbors = g.countNeighborsV2(x, y, radius);
            if (g.grid[x][y]) {
                g.tempGrid[x][y] = neighbors >= lowerThreshold && neighbors <= upperThreshold;
            } else {
                g.tempGrid[x][y] = neighbors == lowerBound || neighbors == upperBound;
            }
        }
    }
    g.grid.swap(g.tempGrid);
}

struct Street {
    float n, total;
    Street() {
        n = total = 0;
    }

    float min(float a, float b) {
        if (a < b) {
            return a;
        } else {
            return b;
        }
    }
    float max(float a, float b) {
        if (a > b) {
            return a;
        } else {
            return b;
        }
    }

    float countLine(Grid &g, int x, int y) {
        LinealRegression lhorizontal, lvertical;
        int radio = 2;
        int count = 0;
        int tam = 0;
        for (int i = -radio; i <= radio; ++i) {
            for (int j = -radio; j <= radio; ++j) {
                int newX = (x + i + g.gridWidth) % g.gridWidth;
                int newY = (y + j + g.gridHeight) % g.gridHeight;
                if (g.grid[newX][newY] == 0) {
                    lhorizontal.addPoint(i, j);
                    lvertical.addPoint(j, i);
                    count++;
                }
                tam++;
            }
        }

        if (count < tam / 3) {
            return 1;
        }
        if (tam / 3 < count) {
            return 0;
        }

        float dh = lhorizontal.coeficienteDeterminacion();
        float dv = lvertical.coeficienteDeterminacion();
        if (dh == dv) {
            return g.grid[x][y];
        }
        float v;
        if (dh > dv) {
            v = lhorizontal.predict(0);
        } else {
            v = lvertical.predict(0);
        }
        return abs(v);
    }

    std::function<void(GameOfLife &)> getFunction() {
        auto lambdaFunction = [this](Grid &g) {
            float media;
            // if(this->n==0){
            media = 0.4;
            // }else{
            // 	media=this->total/this->n;
            // }
            for (int x = 0; x < g.gridWidth; ++x) {
                for (int y = 0; y < g.gridHeight; ++y) {
                    float neighbors = countLine(g, x, y);
                    if (std::isnan(neighbors)) {
                        g.tempGrid[x][y] = g.grid[x][y];
                    } else {
                        this->n++;
                        total += neighbors;
                        g.tempGrid[x][y] = neighbors > media;
                    }
                }
            }
            g.grid.swap(g.tempGrid);
        };
        return lambdaFunction;
    }
};

void updateStreetV3(Grid &g) {
    for (int x = 0; x < g.gridWidth; ++x) {
        for (int y = 0; y < g.gridHeight; ++y) {

            int voto = 0;
            // if (g.getCell(x-1,y-1)==g.getCell(x+1,y+1)){
            // 	if (g.getCell(x-1,y-1)) voto++;
            // 	else voto--;
            // }
            for (int radio = 1; radio <= 5; radio++) {
                // if (g.getCell(x-radio,y)==g.getCell(x,y-radio)){
                // 	if (g.getCell(x-radio,y)) voto++;
                // 	else voto--;
                // }
                // if (g.getCell(x+radio,y)==g.getCell(x,y-radio)){
                // 	if (g.getCell(x+radio,y)) voto++;
                // 	else voto--;
                // }
                // if (g.getCell(x-radio,y)==g.getCell(x,y+radio)){
                // 	if (g.getCell(x-radio,y)) voto++;
                // 	else voto--;
                // }
                // if (g.getCell(x+radio,y)==g.getCell(x,y+radio)){
                // 	if (g.getCell(x+radio,y)) voto++;
                // 	else voto--;
                // }

                if (g.getCell(x - radio, y) == g.getCell(x + radio, y)) {
                    if (g.getCell(x + radio, y))
                        voto++;
                    else
                        voto--;
                }
                if (g.getCell(x, y - radio) == g.getCell(x, y + radio)) {
                    if (g.getCell(x, y + radio))
                        voto++;
                    else
                        voto--;
                }
            }
            // if (g.getCell(x-1,y+1)==g.getCell(x+1,y-1) ){
            // 	if (g.getCell(x-1,y+1)) voto++;
            // 	else voto--;
            // }
            if (voto == 0) {
                g.tempGrid[x][y] = 0; // g.getCell(x,y);
            } else {
                if (0 < voto) {
                    g.tempGrid[x][y] = 1;
                } else {
                    g.tempGrid[x][y] = 0;
                }
            }
            bool vecinasIguales = true;
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    if (g.getCell(x + i, y + j) != g.getCell(x, y)) {
                        vecinasIguales = false;
                        break;
                    }
                }
                if (!vecinasIguales) {
                    break;
                }
            }
            if (vecinasIguales) {
                g.tempGrid[x][y] = 0;
            }
        }
    }
    g.grid.swap(g.tempGrid);
};

void updateStreetV4(Grid &g) {
    int ndir = 2;
    int direcciones[][2] = {
        {0, -1},
        {-1, 0},
        {-1, -1},
        {1, -1},
    };

    for (int x = 0; x < g.gridWidth; ++x) {
        for (int y = 0; y < g.gridHeight; ++y) {
            int recuento[][2] = {
                {0, 0},
                {0, 0},
                {0, 0},
                {0, 0},
            };
            for (int i = 0; i < ndir; i++) {
                auto d = direcciones[i];
                for (int mira = 0; mira < 2; mira++) {
                    int hasta = 5;

                    // if(mira==0){ // hace las calle estrechas (negro) frente a edificios
                    // 	hasta=5;
                    // }
                    for (int j = 1; j < hasta; j++) {
                        if (g.getCell(x + d[0] * j, y + d[1] * j) == mira) {
                            recuento[i][mira]++;
                        } else {
                            break;
                        }
                    }
                    for (int j = 1; j < hasta; j++) {
                        if (g.getCell(x - d[0] * j, y - d[1] * j) == mira) {
                            recuento[i][mira]++;
                        } else {
                            break;
                        }
                    }
                }
            }
            // Quien gana?
            int ganador[] = {0, 0};
            // Recorre recuento e indentifica i,j del ganador
            // Usar el comparador < sobre >, se ven mejores las cosas ordenadas
            // Si hay varios parámetros, salvo razón histórica, pasar primero lo que ocupa mas
            // Todo struct hijo tiene puntero al padre, y se pasa al padre si se usan los dos, si la relación es 1 a 1
            for (int i = 0; i < ndir; i++) {
                for (int j = 0; j < 2; j++) {
                    if (ganador[j] < recuento[i][j]) {
                        ganador[j] = recuento[i][j];
                    }
                }
            }
            g.tempGrid[x][y] = g.getCell(x, y);
            if (ganador[0] < ganador[1]) { //&& ganador[1]/3<ganador[0]
                g.tempGrid[x][y] = 1;
            }
            if (ganador[1] < ganador[0]) { // && ganador[0]/3<ganador[1]
                g.tempGrid[x][y] = 0;
            }
        }
    }
    g.grid.swap(g.tempGrid);
};

void updateStreetV5(Grid &g) {
    int ndir = 4;
    int direcciones[][2] = {
        {0, -1},
        {-1, 0},
        {-1, -1},
        {1, -1},
    };

    for (int x = 0; x < g.gridWidth; ++x) {
        for (int y = 0; y < g.gridHeight; ++y) {
            int distinto[] = {
                0, 0, 0, 0};
            int igual[] = {
                0, 0, 0, 0};
            for (int i = 0; i < ndir; i++) {
                auto d = direcciones[i];
                int hasta = 20;
                // if(mira==0){ // hace las calle estrechas (negro) frente a edificios
                // 	hasta=5;
                // }
                for (int signo = -1; signo <= 1; signo += 2) {
                    int modo = 1;
                    for (int j = 0; j < hasta; j++) {
                        if (modo == 1) {
                            if (g.getCell(x + d[0] * j * signo, y + d[1] * j * signo) == g.getCell(x, y)) {
                                igual[i]++;
                            } else {
                                modo = 2;
                            }
                        }
                        if (modo == 2) {
                            if (g.getCell(x + d[0] * j * signo, y + d[1] * j * signo) != g.getCell(x, y)) {
                                distinto[i]++;
                            } else {
                                break;
                            }
                        }
                    }
                }
            }
            // Quien gana?
            float ganador = 0;

            for (int i = 0; i < ndir; i++) {
                float cand = distinto[i] / igual[i];
                if (ganador < cand) {
                    ganador = cand;
                }
            }
            if (2 < ganador) {
                g.tempGrid[x][y] = !g.getCell(x, y);
            } else {
                g.tempGrid[x][y] = g.getCell(x, y);
            }
        }
    }
    g.grid.swap(g.tempGrid);
};

int main() {
    Grid game(300, 300);
    // game.update = updateClasic;
    // Street street;
    // game.update = street.getFunction();
    game.update = updateStreetV5;

    game.run();

    return 0;
}

// Lo cambios se encapsulan en un puntuero
// Se activan o desactivan cambiando el puntero o haciendo que el puntero sea opcional
// Al abordar un cambio se deciden los sitios a operar, si necesita memoria y se le crea el elemento.
// Recorre recuento e indentifica i,j del ganador
// Usar el comparador < sobre >, se ven mejores las cosas ordenadas
// Si hay varios parámetros, salvo razón histórica, pasar primero lo que ocupa mas
// Todo struct hijo tiene puntero al padre, y se pasa al padre si se usan los dos, si la relación es 1 a 1