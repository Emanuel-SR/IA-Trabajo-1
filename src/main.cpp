#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());

// array con las 8 direcciones adyacentes de un punto
int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

// punto: state determina si esta activo o no
struct Point {
    int x;
    int y;
    bool state; // 1: activo, 0: desactivado
    int color;
    Point(int x, int y, bool state) : x(x), y(y), state(state) {}
    Point() : x(-1), y(-1), state(0) {}
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
            std::cout << points[i]->x << ',' << points[i]->y << ',' << points[i]->state << ' ';
            if ((points[i]->x + 1) % size == 0) std::cout << '\n';
        }
        auto v1 = getNeighbors(points[2 * size + 2]);

    }

};

int main() {
    Map m(10, 20);
    m.print();

    return 0;
}