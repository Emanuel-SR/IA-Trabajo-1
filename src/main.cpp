#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());

// array con las 8 direcciones adyacentes de un punto
int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

enum class GridColor {
    NOT_ACTIVE = 0,
    ACTIVE = 1,
    ENDPOINT = 2,
    PATH = 3,
    VISITED = 4
};

// punto: state determina si esta activo o no
struct Point {
    int x;
    int y;
    bool state; // 1: activo, 0: desactivado
    GridColor color; // 0: desactivado, 1: activo, 2: inicio / fin, 3: camino, 4: visitado pero pertenece al camino

    // Datos necesarios para las busquedas
    bool visited = false;
    Point* prev = nullptr;

    Point(int x, int y, bool state) : x(x), y(y), state(state) {
        if (state == false){
            color = GridColor::NOT_ACTIVE;
        } else { color = GridColor::ACTIVE;}
    }
    Point() : x(-1), y(-1), state(false), color(GridColor::NOT_ACTIVE) {}
    void print(){
        std::cout << "(" << this->x << ", " << this->y << ", " << this->state << ")";
    }
};

class Map {
    int size;
    // contiene todos los puntos del grafo, para acceder por indice usar indice = y * size + x
    std::vector<Point*> points;
public:
    // tamaño del mapa y porcentaje de puntos a eliminar
    Map(int size, int percentage){
        this->size = size;
        points.resize(size * size);
    
        //distribucion bernoulli para desactivar algunos puntos
        std::bernoulli_distribution distrib(percentage / 100.0);
        //Crear nodos
        for (int i = 0; i < size * size; i++){
            points[i] = new Point(i % size, i / size, !distrib(gen));
        }
    }
    ~Map(){
        for (auto v : points){
            delete v;
        }
    }

    //obtener un vector con los 8 puntos adyacentes validos(activos) de un punto cualquiera
    std::vector<Point*> getNeighbors(Point* current){
        std::vector<Point*> neighbors;

        for (int i = 0; i < 8; i++){
            int nx = current->x + dx[i];
            int ny = current->y + dy[i];

            if (nx >= 0 && nx < size && ny >= 0 && ny < size && points[ny * size + nx]->state){
                neighbors.push_back(points[ny * size + nx]);
            }
        }

        return neighbors;
    }

    //print de prueba
    void print(){
        for (int i = 0; i < size * size; i++){
            points[i]->print();
            if ((points[i]->x + 1) % size == 0) std::cout << '\n';
        }
    }

    Point* get_point(int i, int j){
        return points[j * size + i];
    }

     std::vector<Point*> depthFirstSearch(Point* start, Point* target){
        // reiniciar busqueda previa
        for (Point* p : points){
            p->visited = false;
            p->prev = nullptr;
            if (p->state){
                p->color = GridColor::ACTIVE;
            }
        }

        std::vector<Point*> search_stack;

        start->visited = true;
        search_stack.push_back(start);

        bool found = false;

        while(!search_stack.empty()){
            Point* current = search_stack.back();
            search_stack.pop_back();

            if (current == target){
                found = true;
                break;
            }

            std::vector<Point*> neighbors = getNeighbors(current);
            for (auto next : neighbors){
                if (!next->visited){
                    next->visited = true;
                    next->color = GridColor::VISITED;
                    next->prev = current;
                    search_stack.push_back(next);
                }
            }
        }

        std::vector<Point*> path;
        if (found){
            Point* p = target;
            while(p != nullptr){
                path.push_back(p);
                p->color = GridColor::PATH;
                p = p->prev;
            }
            std::reverse(path.begin(), path.end());
        }

        start->color = GridColor::ENDPOINT;
        target->color = GridColor::ENDPOINT;

        return path;
    }

    void map_search(){
        // pruebas de input
        int i1, j1, i2, j2;
        std::cout << "Inserte coordenadas: ";
        std::cin >>  i1 >> j1 >> i2 >> j2;

        std::vector<Point*> tmp = depthFirstSearch(points[j1 * size + i1], points[j2 * size + i2]);
        for (auto t : tmp){

            t->print();
        }
        std::cout << '\n';
    }
};

int main() {
    Map m(10, 20);
    m.print();
    m.map_search();

    return 0;
}