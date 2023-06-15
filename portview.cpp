#define CROW_MAIN
#define CROW_LOGGING_LEVEL 0
#include "crow_all.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <vector>

#define Distance float

using namespace std;

struct Point {
    Distance x, y;
    Point(int x, int y) : x(x), y(y) {
    }
    double distanceTo(const Point &other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::sqrt(dx*dx + dy*dy);
    }
};

struct Cell {
	Cell() {
	}

    enum { 
		EMPTY,
    	STREET,
        PARK 
	} tipo;

    Point p;

    void init(int x, int y) {
		this->x = x;
		this->y = y;
        tipo = EMPTY;
    }
};

struct Segment{
    Point p1,p2;
    Segment(Distance x1,Distance y1,Distance x2,Distance y2){
        p1=Point(x1,y1);
        p2=Point(x2,y2);
    }

    double length() const {
        return p1.distanceTo(p2);
    }

    double distanceTo(const Point &p) const {
        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        if (dx == 0 && dy == 0)
            return p1.distanceTo(p);
        double t = ((p.x - p1.x) * dx + (p.y - p1.y) * dy) / (dx * dx + dy * dy);
        if (t < 0) return p1.distanceTo(p);
        if (t > 1) return p2.distanceTo(p);
        Point projection(p1.x + t * dx, p1.y + t * dy);
        return projection.distanceTo(p);
    }
};

struct Grid {
    static constexpr int CELL_SIZE = 5;
    int round = 0;

    int gridWidth;
    int gridHeight;
    vector<vector<Cell>> grid;

    Grid(int width, int height)
        : gridWidth(width), gridHeight(height),
		  grid(width, vector<Cell>(height, Cell())) {
        // Inicializa el grid con un patrón inicial
        // srand(time(nullptr));
        for (int x = 0; x < gridWidth; x++) {
            for (int y = 0; y < gridHeight; y++) {
                grid[x][y].init(x, y);
            }
        }
    }

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

    void lineTdd(){
        auto s=Segment(50,5,50,95);
        // Measuring the distance between segment to the center of the cell
        for (int i = 0; i < gridWidth; i++) {
            for (int j = 0; j < gridHeight; j++) {
                Cell *cell = getCell(i, j);
                if (cell->tipo == Cell::EMPTY) {
                    // 


                    int d1 = (s.x1 - i) * (s.x1 - i) + (s.y1 - j) * (s.y1 - j);
                    int d2 = (s.x2 - i) * (s.x2 - i) + (s.y2 - j) * (s.y2 - j);
                    if (d1 < 10 || d2 < 10) {
                        cell->tipo = Cell::STREET;
                    }
                }
            }
        }
    }

};



// Httpserver
struct Httpserver{
	Grid& grid;
	crow::SimpleApp app;
	int port;

    static std::string to_hex_string(int value) {
        std::stringstream stream;
        stream << std::setfill ('0') << std::setw(sizeof(int)*2) 
            << std::hex << value;
        return stream.str().substr(2);
    }

    static std::string leer_html(const std::string& ruta) {
        std::ifstream archivo(ruta);
        std::stringstream buffer;
        buffer << archivo.rdbuf();
        return buffer.str();
    }

	
	Httpserver(Grid& grid,int port) : grid(grid) {
		this->port=port;

		CROW_ROUTE(app, "/")([&](){
			return leer_html("./simtravel3.html");
		});

		CROW_ROUTE(app, "/coordinates")([&grid](){
			crow::json::wvalue x;

            int vez = 0;
            for (int i = 0; i < grid.gridWidth; i++) {
                for (int j = 0; j < grid.gridHeight; j++) {
                    Cell *cell = grid.getCell(i, j);
                    if (cell->tipo == Cell::STREET) {
                        x["x"][vez] = to_hex_string(0x0000FF);
                    } else if (cell->tipo == Cell::PARK) {
                        x["x"][vez] = to_hex_string(0x00FF00);
                    } else {
                        x["x"][vez] = to_hex_string(0xFFFFFF);
                    }
                    vez++;
                }
            }

			return x;
		});

	}

	void run(){
		app.port(port).run();
	}
};

int main() {
    std::srand(std::time(0));
	Grid grid(100, 100);
    grid.lineTdd()
    //grid.getCell(50, 5)->tipo = Cell::STREET;
	Httpserver server(grid,18080);
	server.run();
}
