#define CROW_MAIN
#define CROW_LOGGING_LEVEL 0
#include "crow_all.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <vector>
#include <omp.h>

using namespace std;

struct Cell;

struct Point {
	double x, y;
	Point() : x(0), y(0) {
	}
	Point(int x, int y) : x(x), y(y) {
	}
	double distanceTo(const Point &other) const {
		double dx = x - other.x;
		double dy = y - other.y;
		return std::sqrt(dx*dx + dy*dy);
	}
	void position(Point p2,Cell &c,int segmento) const;
};

struct Cell {
	enum { 
		EMPTY,
		STREET,
		PARK 
	} tipo;

	Point p;

	Cell(int x, int y);

	// Position 
	double distance,order,side;

	// Link
	Cell *fromCell, *toCell;

	int link(Cell* other) {
		// Si es de segmento distinto el link lo decide el orden.
		int link=0;

		if(this->order<other->order){
			this->toCell=other;
			other->fromCell=this;
			link++;
		}else{
			this->fromCell=other;
			other->toCell=this;
			link++;
		}
		return link;
	}
};

void Point::position(Point p2,Cell &c,int segmento) const {
	auto p1=*this;

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

	double distance=projection.distanceTo(p);

	if(distance<c.distance){
		// Lado por producto vectorial, miramos el signo de la z
		// A x B = (a2b3 - a3b2, a3b1 - a1b3, a1b2 - a2b1)
		double lado=dx*(projection.y-p.y) - dy*(projection.x-p.x);
		c.distance=distance;
		c.order=t+segmento;
		if(lado>0){
			c.side=1;
		}else{
			c.side=0;
		}
	}
}



Cell::Cell(int x, int y) {
	this->p.x = x;
	this->p.y = y;
	tipo = EMPTY;
	distance = INFINITY;
}

struct Segment{
	Point p1,p2;
	Segment(double x1,double y1,double x2,double y2){
		p1=Point(x1,y1);
		p2=Point(x2,y2);
	}

	double length() const {
		return p1.distanceTo(p2);
	}

	
};

struct Street{
	vector<Point> segments;
	Street(const std::initializer_list<Point>& list){
		// Es mejor pasarlo a puntos, el interpunto define un segmento.
		for(auto s:list){
			segments.push_back(s);
		}
	}

	Point downLeft(){
		Point p=Point(INT_MAX,INT_MAX);
		for(auto s:segments){
			p.x=min(p.x,s.x);
			p.y=min(p.y,s.y);
		}
		return p;
	}

	Point upRigh(){
		Point p=Point(-INT_MAX,-INT_MAX);
		for(auto s:segments){
			p.x=max(p.x,s.x);
			p.y=max(p.y,s.y);
		}
		return p;
	}

 	double maxSide() {
		auto dl=this->downLeft();
		auto ur=this->upRigh();
		double dx = ur.x-dl.x ;
		double dy = ur.y-dl.y ;
		return max(abs(dx),abs(dy));
	}
};

struct Grid {
	static constexpr int CELL_SIZE = 5;
	int round = 0;

	int gridWidth;
	int gridHeight;
	vector<vector<Cell*>> grid;

	Grid(int width, int height)
		: gridWidth(width), gridHeight(height), grid(width, vector<Cell*>(height)) {
		// Inicializa el grid con un patrón inicial
		// srand(time(nullptr));
		for (int x = 0; x < gridWidth; x++) {

			for (int y = 0; y < gridHeight; y++) {
				grid[x][y]=new Cell(x,y);
			}
		}
	}

	Cell* getCell(int x, int y) {
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
		// Aumentar el número de segmentos, como un array.
		int direction[]={1,-1};
		auto street=Street({Point(2,2),Point(5,5),Point(4,1)});

		vector<Cell*> cells;
		int borde=0;
		// Recorre la zona del grid que contiene el segmento con un borde de seguridad
		// Introduce todas las cell en un vector para luego ordenarlas por distance
		auto dl=street.downLeft();
		auto ur=street.upRigh();
		for (int i = dl.x-borde; i <= ur.x+borde; i++) {
			for (int j = dl.y-borde; j <= ur.y+borde; j++) {
				Cell& cell = *getCell(i, j);
				//cout<<"x:"<<cell.p.x<<" y:"<<cell.p.y<<" tipo:"<<cell.tipo<<endl;
				if(cell.p.x!=i || cell.p.y!=j){
					cout<<"Error en la cell"<<endl;
				}
				for (int ordenSegmento=0; ordenSegmento < street.segments.size()-1; ordenSegmento++) {
					auto s=street.segments[ordenSegmento];
					s.position(street.segments[ordenSegmento+1],cell,ordenSegmento);
					if(cell.tipo==Cell::EMPTY){
						cells.push_back(&cell);
						//cout<<"x:"<<cell.p.x<<" y:"<<cell.p.y<<" tipo:"<<cell.tipo<<endl;
					}
				}
			}
		}

		// Ordena las cell por distance
		std::sort(cells.begin(), cells.end(), [](Cell *a, Cell *b) {
			return a->distance < b->distance;
		});
		

		// Recorre dos veces las cell por distancia, uniendo los vecinos esgún orden y que sean del mismo lado si hay mas de un carril.
		bool salir=false;
		int links=0;
		for (int i = 0; i < cells.size(); i++) {
			Cell & cell = *cells[i];
			cout<<"x:"<<cell.p.x<<" y:"<<cell.p.y<<" distance:"<<cell.distance<<endl;
			if (cell.tipo == Cell::EMPTY) {
				// Asigna
				cell.tipo = Cell::STREET;

				int anclado=0;
				
				for (int j = 0; j < i; j++) {
					Cell * otra = cells[j];
					// ¿Son vecinas? Mide la distancia dx y dy
					auto dx = abs(cell.p.x - otra->p.x);
					auto dy = abs(cell.p.y - otra->p.y);
					if (dx <= 1 && dy <= 1) {
						// Link por orden
						links+=cell.link(otra);
					}
				}
				for (int ordenSegmento=0; ordenSegmento < street.segments.size(); ordenSegmento++) {
					auto s=street.segments[ordenSegmento];
					for (int j = 0; j <= i; j++) {
						Cell * otra = cells[j];
						if(otra->fromCell!=nullptr || otra->toCell!=nullptr){
							if(s.distanceTo(otra->p)<=sqrt(2)){
								anclado++;
								break;
							}
						}
					}
				}
					
				cout <<"links:"<<links<<"/"<< i << " anclado:" << anclado << endl;
				if(i*2-2==links && anclado>=street.segments.size()){
					salir=true;
					break;
				}
				// if(anclado>=street.maxSide()){
				// 	salir=true;
				// 	break;
				// }
				// Pause of 5 seconds
				std::this_thread::sleep_for(std::chrono::seconds(5));
			}else{
				// Print on scrren not empty
				cout << "x:" << cell.p.x << " y:" << cell.p.y << " tipo:" << cell.tipo << endl;
			}
			if(salir){
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
					Cell &cell = *grid.getCell(i, j);
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
	Grid grid(10, 10);
	Httpserver server(grid,18080);
	std::thread serverThread([&]() {
      cout << "run" << endl;
      server.run();
   });

	cout<<"Fin"<<endl;
   //grid.getCell(50, 5)->tipo = Cell::STREET;
	grid.lineTdd();		
}
