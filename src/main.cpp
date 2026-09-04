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

float V_START = 0.85f;
float H_START = -0.85f;

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
    std::vector<Point*> points;
    Point* start_point = nullptr;
    Point* target_point = nullptr;
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

    Point* getPoint(int i, int j) {
        return points[j * size + i];
    }

    int getSize() {
        return this->size;
    }

    std::vector<Point*> getNeighbors(Point* current) {
        std::vector<Point*> neighbors;

        for (int i = 0; i < 8; i++) {
            int nx = current->x + dx[i];
            int ny = current->y + dy[i];

            if (nx >= 0 && nx < size && ny >= 0 &&  ny < size && getPoint(nx, ny)->state) {
                neighbors.push_back(getPoint(nx, ny));
            }
        }
        return neighbors;
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

    void mapClick(int x, int y){
        Point* p = getPoint(x, y);

        if(!p->state){
            return;
        }

        if(start_point == nullptr){
            start_point = p;
            start_point->color = PointColor::ENDPOINT;

        } else if (target_point == nullptr && start_point != nullptr){
            target_point = p;
            target_point->color = PointColor::ENDPOINT;

            //LLAMADA A LA BUSQUEDA
            blindSearch(start_point, target_point, SearchType::DFS);
        } else {
            for (auto pt : points) {
                pt->visited = false;
                pt->prev = nullptr;
                if (pt->state) pt->color = PointColor::ACTIVE;

            }
            start_point = p;
            target_point = nullptr;
            start_point->color = PointColor::ENDPOINT;
        }

    }

    void drawMap() {
        float spacing_between_points = 1.7f / (size - 1);

        auto setEdgeColor = [](Point* a, Point* b){
            bool a_is_path = (a->color == PointColor::PATH || a->color == PointColor::ENDPOINT);
            bool b_is_path = (b->color == PointColor::PATH || b->color == PointColor::ENDPOINT);
        
            if (a_is_path && b_is_path && (a->prev == b || b->prev == a)){
                glColor3f(0.0f, 1.0f, 0.0f);
            } else {
                glColor3f(1.0f, 1.0f, 1.0f);
            }

        };

        glLineWidth(1.0f);
        glBegin(GL_LINES);

        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                Point* current = getPoint(x, y);

                if (!current->state)
                    continue;

                float x1 = H_START + x * spacing_between_points;
                float y1 = V_START - y * spacing_between_points;

                // derecha
                if (x + 1 < size) {
                    Point* right = getPoint(x + 1, y);
                    if (right->state) {
                        float x2 = H_START + (x + 1) * spacing_between_points;
                        float y2 = y1;

                        setEdgeColor(current, right);
                        glVertex2f(x1, y1);
                        glVertex2f(x2, y2);
                    }
                }

                // abajo
                if (y + 1 < size) {
                    Point* down = getPoint(x, y + 1);
                    if (down->state) {
                        float x2 = x1;
                        float y2 = V_START - (y + 1) * spacing_between_points;

                        setEdgeColor(current, down);
                        glVertex2f(x1, y1);
                        glVertex2f(x2, y2);
                    }
                }

                // right diagonal
                if (x + 1 < size && y + 1 < size) {
                    Point* diagonal = getPoint(x + 1, y + 1);
                    if (diagonal->state) {
                        float x2 = H_START + (x + 1) * spacing_between_points;
                        float y2 = V_START - (y + 1) * spacing_between_points;

                        setEdgeColor(current, diagonal);
                        glVertex2f(x1, y1);
                        glVertex2f(x2, y2);
                    }
                }


                if (x - 1 >= 0 && y + 1 < size) {
                    Point* diagonal =  getPoint(x - 1, y + 1);
                    if (diagonal->state) {
                        float x2 = H_START + (x - 1) * spacing_between_points;
                        float y2 = V_START - (y + 1) * spacing_between_points;

                        setEdgeColor(current, diagonal);
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
                Point* p = getPoint(x, y);

                float posX = H_START + x * spacing_between_points;
                float posY = V_START - y * spacing_between_points;

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


void mouse_button (GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        Map* m = static_cast<Map*>(glfwGetWindowUserPointer(window));
        if(!m) return;

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        float ndcX = (2.0f * mouseX / width) - 1.0f;
        float ndcY = 1.0f - (2.0f * mouseY / height);

        float spacing_between_points = 1.7f / (m->getSize() - 1);


        int mapX = std::round((ndcX - H_START) / spacing_between_points);
        int mapY = std::round((V_START - ndcY) / spacing_between_points);

        if (mapX >= 0 && mapX < m->getSize() && mapY >= 0 && mapY < m->getSize()) {
            m->mapClick(mapX, mapY);
        }
    }
}

int main() {
    Map m(20, 20);

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

    glfwSetWindowUserPointer(window, &m);
    glfwSetMouseButtonCallback(window, mouse_button);

    while (!glfwWindowShouldClose(window)) {
        // obtener tamaño real porque hay un error en linux dx
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor( 0.2f,0.2f, 0.2f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m.drawMap();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
