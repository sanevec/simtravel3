#define CROW_MAIN
#include "crow_all.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <iomanip>


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


std::string to_hex_string(int value) {
    std::stringstream stream;
    stream << std::setfill ('0') << std::setw(sizeof(int)*2) 
           << std::hex << value;
    return stream.str().substr(2);
}

std::string leer_html(const std::string& ruta) {
    std::ifstream archivo(ruta);
    std::stringstream buffer;
    buffer << archivo.rdbuf();
    return buffer.str();
}

// Httpserver

// imprime grid
struct Httpserver{
	Grid& grid;
	crow::SimpleApp app;
	int port;
	
	Httpserver(Grid& grid,port int) : grid(grid) {
		this.port=port
		CROW_ROUTE(app, "/")([&](){
			return leer_html("./simtravel3.html");
		});

		std::srand(std::time(0));
		CROW_ROUTE(app, "/coordinates")([](){
			crow::json::wvalue x;
			

			int vez = 0;
			for (int i = 0; i < 100; i++) {
				for (int j = 0; j < 100; j++) {
					x["x"][vez++] =to_hex_string(std::rand() % 16777216);
				}
			}
			
			// Puedes seguir agregando más cuadrados de la misma manera

			return x;
		});

	}

	void run(){
		app.port(port).run();
	}
}

int main() {
	Grid grid(100, 100);
	Httpserver server(grid,18080);
	server.run();
}
