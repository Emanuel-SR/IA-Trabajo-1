#include <GL/glew.h>
#include <GLFW/glfw3.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

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
        if (state == false) {
            color = PointColor::NOT_ACTIVE;
        }
        else {
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

            if (nx >= 0 && nx < size && ny >= 0 && ny < size && getPoint(nx, ny)->state) {
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
            Point* current = search.front();
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
            std::reverse(path.begin(), path.end());
        }

        start->color =
            PointColor::ENDPOINT;
        target->color =
            PointColor::ENDPOINT;
        return path;
    }

    //Busquedas heuristicas

    void hillClimbing()
    {
        if (start_point == nullptr || target_point == nullptr) {
            std::cout << "Primero selecciona inicio y destino\n";
            return;
        }

        std::cout << "Hill Climbing seleccionado.\n";

        // Reiniciar informacion de busqueda
        for (Point* p : points) {
            p->visited = false;
            p->prev = nullptr;

            if (p->state) {
                p->color = PointColor::ACTIVE;
            }
        }

        start_point->color = PointColor::ENDPOINT;
        target_point->color = PointColor::ENDPOINT;

        //calculo de la distancia euclidiana
        auto heuristic = [this](Point* p) {
            float dx = (float)(p->x - target_point->x);
            float dy = (float)(p->y - target_point->y);

            return std::sqrt(dx * dx + dy * dy);
            };

        // lista L de nodos pendientes
        std::deque<Point*> search;

        start_point->visited = true;
        search.push_back(start_point);

        bool found = false;

        while (!search.empty()) {

            Point* current = search.front();
            search.pop_front();

            //std::cout << "Visitando: (" << current->x << ", " << current->y << ")" << "  h = " << heuristic(current) << "\n";

            if (current == target_point) {
                found = true;
                break;
            }

            std::vector<Point*> neighbors = getNeighbors(current);

            // ordenar los hijos 
            std::sort(neighbors.begin(), neighbors.end(), [heuristic](Point* a, Point* b) {
                return heuristic(a) < heuristic(b);
                }
            );

            for (int i = (int)neighbors.size() - 1; i >= 0; i--) {
                Point* next = neighbors[i];

                if (!next->visited) {
                    next->visited = true;
                    next->prev = current;
                    next->color = PointColor::VISITED;
                    search.push_front(next);
                }
            }
        }
        //reconstruir el caminito
        if (found) {
            std::vector<Point*> path;

            Point* p = target_point;

            while (p != nullptr) {
                path.push_back(p);
                p = p->prev;
            }

            std::reverse(path.begin(), path.end());

            for (Point* node : path) {
                node->color = PointColor::PATH;
            }
            start_point->color = PointColor::ENDPOINT;
            target_point->color = PointColor::ENDPOINT;

            //calcular distancia
            float distance_final = 0.0f;
            for (int i = 1; i < (int)path.size(); i++) {
                Point* previous = path[i - 1];
                Point* current = path[i];

                if (previous->x != current->x && previous->y != current->y) {
                    distance_final += std::sqrt(2.0f);
                }
                else {
                    distance_final += 1.0f;
                }
            }
            std::cout << "Distancia del camino (Hill Climbing): " << distance_final << std::endl;
        }
        else {
            std::cout << "Hill Climbing no encontro un camino.\n";
        }
    }

    void aStar() {
        if (start_point == nullptr || target_point == nullptr) {
            std::cout << "Primero selecciona inicio y destino.\n";
            return;
        }

        for (Point* p : points) {
            p->visited = false;
            p->prev = nullptr;
            if (p->state) {
                p->color = PointColor::ACTIVE;
            }
        }

        start_point->color = PointColor::ENDPOINT;
        target_point->color = PointColor::ENDPOINT;

        //definir distancia euclidiana como heuristica
        auto heuristic = [this](Point* p) {
            float dx = (float)(p->x - target_point->x);
            float dy = (float)(p->y - target_point->y);
            return std::sqrt(dx * dx + dy * dy);
            };

        //uso de vectores como estructura almacena los costos g(n) y f(n) para cada nodo
        std::vector<float> gScore(size * size, std::numeric_limits<float>::infinity());
        std::vector<float> fScore(size * size, std::numeric_limits<float>::infinity());

        //costo para el nodo inicial es 0
        int start_index = start_point->y * size + start_point->x;
        gScore[start_index] = 0.0f;
        fScore[start_index] = heuristic(start_point);

        //lista abierta, nodos a verificar
        std::vector<Point*> openSet;
        openSet.push_back(start_point);

        bool found = false;

        while (!openSet.empty()) {
            //ordenar la lista abierta para que el nodo con MENOR f(n) quede al final
            std::sort(openSet.begin(), openSet.end(), [&](Point* a, Point* b) {
                int idx_a = a->y * size + a->x;
                int idx_b = b->y * size + b->x;
                return fScore[idx_a] > fScore[idx_b]; // Orden descendente para usar pop_back
                });

            //extrae el nodo con el menor f(n)
            Point* current = openSet.back();
            openSet.pop_back();

            int current_index = current->y * size + current->x;
            current->visited = true;

            if (current != start_point && current != target_point) {
                current->color = PointColor::VISITED;
            }

            //Si trazamos un camino al fianl,terminar busqueda
            if (current == target_point) {
                found = true;
                break;
            }

            // Evaluar vecinos
            std::vector<Point*> neighbors = getNeighbors(current);
            for (Point* next : neighbors) {
                if (next->visited) continue; // Ignorar nodos ya cerrados

                //calcular el costo del movimiento hacia el vecino
                //verificamos si se trata de un nodo diagonal
                float move_cost = (current->x != next->x && current->y != next->y) ? std::sqrt(2.0f) : 1.0f;
                float tentative_gScore = gScore[current_index] + move_cost;

                int next_index = next->y * size + next->x;

                //si encontramos un camino mas corto hacia este vecino
                if (tentative_gScore < gScore[next_index]) {
                    next->prev = current;
                    gScore[next_index] = tentative_gScore;
                    fScore[next_index] = gScore[next_index] + heuristic(next);

                    //si el vecino no está en la lista abierta, lo agregamos
                    if (std::find(openSet.begin(), openSet.end(), next) == openSet.end()) {
                        openSet.push_back(next);
                    }
                }
            }
        }
        //Camino encontrado
        if (found) {
            std::vector<Point*> path;
            Point* p = target_point;

            while (p != nullptr) {
                path.push_back(p);
                p = p->prev;
            }

            std::reverse(path.begin(), path.end());

            for (Point* node : path) {
                node->color = PointColor::PATH;
            }
            start_point->color = PointColor::ENDPOINT;
            target_point->color = PointColor::ENDPOINT;

            
            float total_cost = 0.0f;
            for (size_t i = 1; i < path.size(); i++) {
                Point* previous = path[i - 1];
                Point* current = path[i];

                if (previous->x != current->x && previous->y != current->y) {
                    total_cost += std::sqrt(2.0f); // Movimiento diagonal
                }
                else {
                    total_cost += 1.0f; // Movimiento horizontal/vertical
                }
            }

            int total_nodes_expanded = 0;
            for (Point* node : points) {
                if (node->color == PointColor::VISITED || node->color == PointColor::PATH || node->color == PointColor::ENDPOINT) {
                    total_nodes_expanded++;
                }
            }

            std::cout << "Costo total del camino (Peso): " << total_cost << std::endl;
            std::cout << "Total de nodos expandidos (t_n): " << total_nodes_expanded << std::endl;
        }
        else {
            std::cout << "A* no logró encontrar un camino.\n";
        }
    }

    void mapClick(int x, int y) {
        Point* p = getPoint(x, y);

        if (!p->state) {
            return;
        }

        if (start_point == nullptr) {
            start_point = p;
            start_point->color = PointColor::ENDPOINT;

        }
        else if (target_point == nullptr && start_point != nullptr) {
            target_point = p;
            target_point->color = PointColor::ENDPOINT;

            //LLAMADA A LA BUSQUEDA
            //blindSearch(start_point, target_point, SearchType::DFS);
            std::cout << "Seleccione el algoritmo:\n";
            std::cout << std::endl;
            std::cout << "1 - DFS\n";
            std::cout << "2 - BFS\n";
            std::cout << "3 - Hill Climbing\n";
            std::cout << "4 - A*\n";
            std::cout << std::endl;

        }
        else {
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
    void executeSearch(SearchType type)
    {
        if (start_point == nullptr || target_point == nullptr)
        {
            std::cout << "Primero selecciona inicio y destino.\n";
            return;
        }

        blindSearch(start_point, target_point, type);
    }

    void drawMap() {
        float spacing_between_points = 1.7f / (size - 1);

        auto setEdgeColor = [](Point* a, Point* b) {
            bool a_is_path = (a->color == PointColor::PATH || a->color == PointColor::ENDPOINT);
            bool b_is_path = (b->color == PointColor::PATH || b->color == PointColor::ENDPOINT);

            if (a_is_path && b_is_path && (a->prev == b || b->prev == a)) {
                glColor3f(0.0f, 1.0f, 0.0f);
            }
            else {
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
                    Point* diagonal = getPoint(x - 1, y + 1);
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


void mouse_button(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        Map* m = static_cast<Map*>(glfwGetWindowUserPointer(window));
        if (!m) return;

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

bool key1Pressed = false;
bool key2Pressed = false;
bool key3Pressed = false;
bool key4Pressed = false;

void processInput(GLFWwindow* window, Map& m)
{   //esc
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    //dfs
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        if (!key1Pressed) {
            m.executeSearch(SearchType::DFS);
            std::cout << "DFS Seleccionado\n";

            key1Pressed = true;
        }
    }
    else {
        key1Pressed = false;
    }
    //bfs
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        if (!key2Pressed) {
            m.executeSearch(SearchType::BFS);
            std::cout << "BFS Seleccionado\n";

            key2Pressed = true;
        }
    }
    else {
        key2Pressed = false;
    }
    //hill climbing
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        if (!key3Pressed) {
            m.hillClimbing();
            key3Pressed = true;
        }
    }
    else {
        key3Pressed = false;
    }
    //A*
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        if (!key4Pressed) {
            std::cout << "A* Seleccionado\n";
            m.aStar();
            key4Pressed = true;
        }
    }
    else {
        key4Pressed = false;
    }
}


int main() {
    Map m(30, 20);

    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(900, 900, "Vengo por tu perceptron", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);


    glfwMakeContextCurrent(window);

    // NUEVA INICIALIZACIÓN CON GLEW (PAQUETE NUPENGL DE NUGET):
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Error al inicializar GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetWindowUserPointer(window, &m);


    glfwSetWindowUserPointer(window, &m);
    glfwSetMouseButtonCallback(window, mouse_button);



    while (!glfwWindowShouldClose(window)) {
        processInput(window, m);
        // obtener tamaño real porque hay un error en linux dx
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m.drawMap();


        glfwSwapBuffers(window);
        glfwPollEvents();
    }



    glfwTerminate();

    return 0;
}