#include <SFML/Graphics.hpp>
#include <cmath>
#include <functional>
#include <vector>

using namespace std;

struct Cell {
	Cell() {
	}

    enum { 
		EMPTY,
    	STREET,
        PARK 
	} tipo;

    int x, y;

    void init(int x, int y) {
		this->x = x;
		this->y = y;
        tipo = EMPTY;
    }

	sf::Color getColor() {
		switch (tipo) {
		case EMPTY:
			return sf::Color::White;
		case STREET:
			return sf::Color::Black;
		case PARK:
			return sf::Color::Blue;
		}
	}
};

struct Grid {
    static constexpr int CELL_SIZE = 5;
    int round = 0;

    Grid(int width, int height)
        : window(sf::VideoMode(width * CELL_SIZE, height * CELL_SIZE), "Sevilla City"),
          gridWidth(width), gridHeight(height),
		  grid(width, vector<Cell>(height, Cell())) {
        // Inicializa el grid con un patrón inicial
        // srand(time(nullptr));
        for (int x = 0; x < gridWidth; x++) {
            for (int y = 0; y < gridHeight; y++) {
                grid[x][y].init(x, y);
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
    vector<vector<Cell>> grid;

    Cell *getCell(int x, int y) {
        int newX = x % gridWidth;
        if (newX < 0) {
            newX += gridWidth;
        }
        int newY = y % gridHeight;
        if (newY < 0) {
            newY += gridHeight;
        }
        return &grid[newX][newY];
    }

    std::function<void(Grid &)> update;

    void render() {
        window.clear();
        window.setFramerateLimit(3);

        for (int x = 0; x < gridWidth; ++x) {
            for (int y = 0; y < gridHeight; ++y) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                cell.setFillColor(grid[x][y].getColor());

                window.draw(cell);
            }
        }

        window.display();
    }
};

void updateStreetV5(Grid &g) {
	int n = (rand() % 3+1)*2;
	// Get two points of the grid
	int xo1 = rand() % g.gridWidth;
	int yo1 = rand() % g.gridHeight;
	int xo2 = rand() % g.gridWidth;
	int yo2 = rand() % g.gridHeight;

	int in=0;
	for(;;){

		int num=0;
		for(int direction=0;direction<2;direction++){
			if(direction==1){
				int dx = xo2 - xo1;
				int dy = yo2 - yo1;

				xo2=xo1-dx;
				yo2=yo1-dy;
			}
			int dx = abs(xo2 - xo1);
			int dy = abs(yo2 - yo1);
			int x0 =xo1,y0=yo1;
			int x2 =xo2,y2=yo2;		
			if(dx<dy){
				x0+=in;
				x2+=in;
			}else{
				y0+=in;
				y2+=in;
			}
			bool firstSteep=true;
			// Use Bresenham's line algorithm to get all the points between the two points
			int sx = (x0 < x2) ? 1 : -1;
			int sy = (y0 < y2) ? 1 : -1;
			int err = dx - dy;

			while (true) {
				// Mark the current cell as a street
				if(direction==0 || !firstSteep){
					auto cell = g.getCell(x0, y0);
					if (cell->tipo == Cell::STREET) break;
					cell->tipo = Cell::STREET;
					num++;
				}
				firstSteep=false;

				// Break the loop if we've reached the second point
				//if (x0 == x2 && y0 == y2) break;
				// if x0,y0 exit the grid break
				if (x0 < 0 || x0 >= g.gridWidth || y0 < 0 || y0 >= g.gridHeight) break;
				// Calculate the error for the next iteration
				int e2 = 2 * err;
				if (e2 > -dy) {
					err -= dy;
					x0 += sx;
				}
				if (e2 < dx) {
					err += dx;
					y0 += sy;
				}
			}


		}
		in++;
		if (num<in*40) break;
	}
};

int main() {
    Grid game(300, 300);
    game.update = updateStreetV5;

    game.run();

    return 0;
}