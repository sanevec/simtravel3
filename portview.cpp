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
    Point() : x(0), y(0) {
    }
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
		this->p.x = x;
		this->p.y = y;
		tipo = EMPTY;
		distance = INFINITY;
	}

	// Position 
	Distance distance,order,side;

	// Link
	Cell *fromCell, *toCell;

	void link(Cell &other) {
		// Si es de segmento distinto el link lo decide el orden.

		if(this->order<other.order){
			this->toCell=&other;
			other.fromCell=this;
		}else{
			this->fromCell=&other;
			other.toCell=this;
		}
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

	void position(Cell &c,int segmento) const {

		if(c.tipo!=Cell::EMPTY) return;

		auto p=c.p;
		double dx = p2.x - p1.x;
		double dy = p2.y - p1.y;
		if (dx == 0 && dy == 0)
			return;
		double t = ((p.x - p1.x) * dx + (p.y - p1.y) * dy) / (dx * dx + dy * dy);
		if (t < 0) return;
		if (t > 1) return;

		Point projection(p1.x + t * dx, p1.y + t * dy);
		/*
		se puede determinar en qué lado de una línea se encuentra un punto utilizando la ecuación 
		de la línea y comparándola con las coordenadas del punto. Si consideramos la ecuación de una 
		línea en el plano en su forma general Ax + By + C = 0, un punto (x1, y1) está en:
		Un lado de la línea si Ax1 + By1 + C > 0
		El otro lado si Ax1 + By1 + C < 0
		Exactamente en la línea si Ax1 + By1 + C = 0

		Si tienes dos puntos, digamos p1(x1, y1) y p2(x2, y2), puedes encontrar la ecuación de la 
		línea en formato Ax + By + C = 0 siguiendo estos pasos:
		Calcula la pendiente (m) de la línea como: m = (y2 - y1) / (x2 - x1).
		Luego, los coeficientes A, B y C se pueden obtener de la siguiente manera:
		A es igual a -m.
		B es igual a 1.
		C es igual a m*x1 - y1.
		*/

		Distance distance=projection.distanceTo(p);

		if(distance<c.distance){
			// Lado por producto vectorial, miramos el signo de la z
			// A x B = (a2b3 - a3b2, a3b1 - a1b3, a1b2 - a2b1)
			Distance lado=dx*(projection.y-p.y) - dy*(projection.x-p.x);
			c.distance=distance;
			c.order=t+segmento;
			if(lado>0){
				c.side=1;
			}else{
				c.side=0;
			}
		}
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

	Cell &getCell(int x, int y) {
		// Lattice 
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

	std::function<void(Grid &)> update;

	void lineTdd(){
		auto s=Segment(50,5,50,95);
		int ordenSegmento=0;

		int borde=5;
		// Recorre la zona del grid que contiene el segmento con un borde de seguridad
		// Introduce todas las cell en un vector para luego ordenarlas por distance
		vector<Cell*> cells;

		for (int i = s.p1.x-borde; i <= s.p2.x+borde; i++) {
			for (int j = s.p1.y-borde; j <= s.p2.y+borde; j++) {
				Cell cell = getCell(i, j);
				cout<<"x:"<<cell.p.x<<" y:"<<cell.p.y<<" tipo:"<<cell.tipo<<endl;
				s.position(cell,ordenSegmento);
				if(cell.tipo==Cell::EMPTY){
					cells.push_back(&cell);
				}
			}
		}

		// Ordena las cell por distance
		std::sort(cells.begin(), cells.end(), [](Cell *a, Cell *b) {
			return a->distance < b->distance;
		});

		// Recorre dos veces las cell por distancia, uniendo los vecinos esgún orden y que sean del mismo lado si hay mas de un carril.
		bool pendiente;
		for (int i = 0; i < cells.size(); i++) {
			Cell *cell = cells[i];
			cout<<"x:"<<cell->p.x<<" y:"<<cell->p.y<<" tipo:"<<cell->tipo<<endl;
			if (cell->tipo == Cell::EMPTY) {
				// Asigna
				cell->tipo = Cell::STREET;

				pendiente=i<1;
				
				for (int j = 0; j < i; j++) {
					Cell *otra = cells[j];
					// ¿Son vecinas? Mide la distancia dx y dy
					auto dx = abs(cell->p.x - otra->p.x);
					auto dy = abs(cell->p.y - otra->p.y);
					if (dx <= 1 && dy <= 1) {
						// Link por orden
						cell->link(*otra);
					}
					// Identifica si hay otra no linkada 
					if(otra->fromCell!=nullptr && otra->toCell==nullptr){
						pendiente=true;
					}
				}
				if(!pendiente){
					break;
				}
			}else{
				// Print on scrren not empty
				cout << "x:" << cell->p.x << " y:" << cell->p.y << " tipo:" << cell->tipo << endl;

			}
			if(!pendiente){
				break;
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
                    Cell cell = grid.getCell(i, j);
                    if (cell.tipo == Cell::STREET) {
                        x["x"][vez] = to_hex_string(0x0000FF);
                    } else if (cell.tipo == Cell::PARK) {
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
   grid.lineTdd();
    //grid.getCell(50, 5)->tipo = Cell::STREET;
	Httpserver server(grid,18080);
	server.run();
}
