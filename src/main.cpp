#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <vector>
#include <deque>
#include <algorithm>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());

// array con las 8 direcciones adyacentes de un punto
int dx[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
int dy[] = { -1, -1, -1, 0, 0, 1, 1, 1 };

enum class PointColor {
    NOT_ACTIVE,
    ACTIVE,
    ENDPOINT,
    PATH,
    VISITED
};

enum class SearchType {
    DFS,
    BFS
};


struct Point {
    int x;
    int y;
    bool state;

    PointColor color;

    // datos necesarios para las busquedas
    bool visited = false;
    Point* prev = nullptr;

    Point(int x, int y, bool state) : x(x), y(y), state(state) {
        if (state == false){
            color = PointColor::NOT_ACTIVE;
        } else {
            color = PointColor::ACTIVE;
        }
    }

    Point() : x(-1), y(-1), state(false), color(PointColor::NOT_ACTIVE) {}
};


class Map {
    int size;
    // contiene todos los puntos del grafo
    // indice = y * size + x
    std::vector<Point*> points;

public:

    Map(int size, int percentage) {
        this->size = size;
        points.resize(size * size);

        std::bernoulli_distribution distrib(percentage / 100.0);

        for (int i = 0; i < size * size; i++) {
            points[i] = new Point(i % size, i / size, !distrib(gen));
        }
    }


    ~Map() {
        for (Point* v : points) {
            delete v;
        }
    }

    std::vector<Point*> getNeighbors(Point* current) {
        std::vector<Point*> neighbors;

        for (int i = 0; i < 8; i++) {
            int nx = current->x + dx[i];
            int ny = current->y + dy[i];

            if (nx >= 0 && nx < size && ny >= 0 &&  ny < size && points[ny * size + nx]->state) {
                neighbors.push_back(points[ny * size + nx]);
            }
        }
        return neighbors;
    }

    Point* get_point(int i, int j) {
        return points[j * size + i];
    }

    std::vector<Point*> blindSearch(Point* start, Point* target, SearchType type) {
        // Reiniciar busqueda previa
        for (Point* p : points) {
            p->visited = false;
            p->prev = nullptr;

            if (p->state) {
                p->color = PointColor::ACTIVE;
            }
        }

        std::deque<Point*> search;
        start->visited = true;
        search.push_back(start);
        bool found = false;

        while (!search.empty()) {
            Point* current =  search.front();
            search.pop_front();

            if (current == target) {
                found = true;
                break;
            }


            std::vector<Point*> neighbors = getNeighbors(current);
            for (Point* next : neighbors) {
                if (!next->visited) {
                    next->visited = true;
                    next->color = PointColor::VISITED;
                    next->prev = current;

                    if (type == SearchType::DFS) {
                        search.push_front(next);
                    }
                    else {
                        search.push_back(next);
                    }
                }
            }
        }

        std::vector<Point*> path;
        if (found) {
            Point* p = target;
            while (p != nullptr) {
                path.push_back(p);
                p->color =
                    PointColor::PATH;
                p = p->prev;
            }
            std::reverse(path.begin(), path.end() );
        }

        start->color =
            PointColor::ENDPOINT;
        target->color =
            PointColor::ENDPOINT;
        return path;
    }

    void map_search() {
        //pendiente
    }


    void drawMap() {
        float v_start = 0.85f;
        float h_start = -0.85f;
        float spacing_between_points = 1.7f / (size - 1);

        glLineWidth(1.0f);
        glBegin(GL_LINES);

        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                Point* actual = points[y * size + x];

                if (!actual->state)
                    continue;

                float x1 = h_start + x * spacing_between_points;
                float y1 = v_start - y * spacing_between_points;

                glColor3f(1.0f, 1.0f, 1.0f);

                // derecha
                if (x + 1 < size) {
                    Point* right = points[y * size + (x + 1)];
                    if (right->state) {
                        float x2 = h_start + (x + 1) * spacing_between_points;
                        float y2 = y1;

                        glVertex2f(x1, y1);
                        glVertex2f(x2, y2);
                    }
                }

                // abajo
                if (y + 1 < size) {
                    Point* abajo = points[(y + 1) * size + x];
                    if (abajo->state) {
                        float x2 = x1;
                        float y2 = v_start - (y + 1) * spacing_between_points;

                        glVertex2f(x1, y1);
                        glVertex2f(x2, y2);
                    }
                }

                // right diagonal
                if (x + 1 < size && y + 1 < size) {
                    Point* diagonal = points[(y + 1) * size + (x + 1)];
                    if (diagonal->state) {
                        float x2 = h_start + (x + 1) * spacing_between_points;
                        float y2 = v_start - (y + 1) * spacing_between_points;

                        glVertex2f(x1, y1);
                        glVertex2f(x2, y2);
                    }
                }


                if (x - 1 >= 0 && y + 1 < size) {
                    Point* diagonal =  points[(y + 1) * size + (x - 1)];
                    if (diagonal->state) {
                        float x2 = h_start + (x - 1) * spacing_between_points;

                        float y2 = v_start - (y + 1) * spacing_between_points;

                        glVertex2f(x1, y1);
                        glVertex2f(x2, y2);
                    }
                }
            }
        }

        glEnd();

        //puntos
        glPointSize(8.0f);
        glBegin(GL_POINTS);

        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                Point* p = points[y * size + x];

                float posX = h_start + x * spacing_between_points;
                float posY = v_start - y * spacing_between_points;

                switch (p->color) {
                case PointColor::NOT_ACTIVE:
                    // Rojo oscuro
                    glColor3f(0.2f, 0.2f, 0.2f);  
                    break;

                case PointColor::ACTIVE:
                    // Blanco
                    glColor3f(1.0f, 1.0f, 1.0f);    
                    break;

                case PointColor::ENDPOINT:
                    // Azul
                    glColor3f(0.0f, 0.5f, 1.0f);
                    break;

                case PointColor::PATH:
                    // Verde
                    glColor3f(0.0f, 1.0f, 0.0f);
                    break;


                case PointColor::VISITED:
                    // Amarillo
                    glColor3f(1.0f, 1.0f, 0.0f);

                    break;
                }

                glVertex2f(posX, posY);
            }
        }
        glEnd();
    }
};


int main() {
    Map m(20, 20);
    m.map_search();

    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(900, 900, "Vengo por tu perceptron", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);


    if (!gladLoadGL(glfwGetProcAddress)) {
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        glClearColor( 0.2f,0.2f, 0.2f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m.drawMap();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
