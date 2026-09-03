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

int ini_i = -1;
int ini_j = -1;
int dest_i = -1;
int dest_j = -1;


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


struct Point
{
    int x;
    int y;
    bool state;

    PointColor color;

    // Datos necesarios para las busquedas
    bool visited = false;
    Point* prev = nullptr;

    Point(int x, int y, bool state)
        : x(x), y(y), state(state)
    {
        if (state == false)
            color = PointColor::NOT_ACTIVE;
        else
            color = PointColor::ACTIVE;
    }

    Point()
        : x(-1),
        y(-1),
        state(false),
        color(PointColor::NOT_ACTIVE)
    {
    }

    void print()
    {
        std::cout
            << "(" << x << ", " << y << ", "<< state << ")";
    }
};


class Map
{
    int size;

    // contiene todos los puntos del grafo
    // indice = y * size + x
    std::vector<Point*> points;

public:

    Map(int size, int percentage)
    {
        this->size = size;

        points.resize(size * size);

        std::bernoulli_distribution distrib(
            percentage / 100.0
        );

        for (int i = 0; i < size * size; i++)
        {
            points[i] = new Point( i % size, i / size, !distrib(gen) );
        }
    }


    ~Map()
    {
        for (Point* v : points){
            delete v;
        }
    }

    std::vector<Point*> getNeighbors(Point* current)
    {
        std::vector<Point*> neighbors;

        for (int i = 0; i < 8; i++)
        {
            int nx = current->x + dx[i];
            int ny = current->y + dy[i];

            if (nx >= 0 && nx < size && ny >= 0 &&  ny < size && points[ny * size + nx]->state) {
                neighbors.push_back(
                    points[ny * size + nx]
                );
            }
        }

        return neighbors;
    }

    Point* get_point(int i, int j)
    {
        return points[j * size + i];
    }
    
    void print()
    {
        for (int i = 0; i < size * size; i++)
        {
            points[i]->print();

            if ((points[i]->x + 1) % size == 0)
                std::cout << '\n';
        }
    }

    std::vector<Point*> blindSearch(
        Point* start,
        Point* target,
        SearchType type)
    {
        // Reiniciar busqueda previa

        for (Point* p : points)
        {
            p->visited = false;
            p->prev = nullptr;

            if (p->state)
                p->color = PointColor::ACTIVE;
        }


        std::deque<Point*> search;


        start->visited = true;

        search.push_back(start);


        bool found = false;


        while (!search.empty())
        {
            Point* current =  search.front();

            search.pop_front();


            if (current == target)
            {
                found = true;
                break;
            }


            std::vector<Point*> neighbors =
                getNeighbors(current);


            for (Point* next : neighbors)
            {
                if (!next->visited)
                {
                    next->visited = true;

                    next->color =
                        PointColor::VISITED;

                    next->prev = current;


                    if (type == SearchType::DFS) {
                        search.push_front(next);
                    }
                    else
                    {
                        search.push_back(next);
                    }
                }
            }
        }


        std::vector<Point*> path;


        if (found)
        {
            Point* p = target;


            while (p != nullptr)  {
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

    void map_search()
    {
        int i1, j1;
        int i2, j2;

        std::cout << "Inserte coordenadas: ";

        std::cin>> i1>> j1 >> i2 >> j2;


        Point* start = get_point(i1, j1);

        Point* target = get_point(i2, j2);


        if (!start->state ||!target->state)
        {
            std::cout
                << "El inicio o destino "
                << "esta desactivado.\n";

            return;
        }


        std::vector<Point*> path =  blindSearch( start, target, SearchType::DFS );


        for (Point* p : path)
        {
            p->print();
            std::cout << " ";
        }

        std::cout << '\n';
    }


    void drawMap()
    {
        float inicio = -0.85f;
        float espacio = 1.7f / (size - 1);

        glLineWidth(1.0f);

        glBegin(GL_LINES);

        for (int y = 0; y < size; y++)
        {
            for (int x = 0; x < size; x++)
            {
                Point* actual = points[y * size + x];

                if (!actual->state)
                    continue;

                float x1 = inicio + x * espacio;
                float y1 = inicio + y * espacio;

                glColor3f(1.0f, 1.0f, 1.0f);

                if (x + 1 < size)
                {
                    Point* derecha =
                        points[y * size + (x + 1)];

                    if (derecha->state)
                    {
                        float x2 =inicio + (x + 1) * espacio;

                        float y2 = y1;

                        glVertex2f(x1, y1);
                        glVertex2f(x2, y2);
                    }
                }


                if (y + 1 < size)
                {
                    Point* abajo =
                        points[(y + 1) * size + x];

                    if (abajo->state)
                    {
                        float x2 = x1;

                        float y2 =
                            inicio + (y + 1) * espacio;

                        glVertex2f(x1, y1);
                        glVertex2f(x2, y2);
                    }
                }


                if (x + 1 < size &&
                    y + 1 < size)
                {
                    Point* diagonal = points[(y + 1) * size + (x + 1)];

                    if (diagonal->state)
                    {
                        float x2 =
                            inicio + (x + 1) * espacio;

                        float y2 =
                            inicio + (y + 1) * espacio;

                        glVertex2f(x1, y1);
                        glVertex2f(x2, y2);
                    }
                }


                if (x - 1 >= 0 &&
                    y + 1 < size)
                {
                    Point* diagonal =  points[(y + 1) * size + (x - 1)];

                    if (diagonal->state) {
                        float x2 =
                            inicio + (x - 1) * espacio;

                        float y2 =
                            inicio + (y + 1) * espacio;

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

        for (int y = 0; y < size; y++)
        {
            for (int x = 0; x < size; x++)
            {
                Point* p = points[y * size + x];

                float posX = inicio + x * espacio;

                float posY = inicio + y * espacio;


                switch (p->color)
                {
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

                glVertex2f(
                    posX,
                    posY
                );
            }
        }

        glEnd();
    }
};


int main()
{
   
    Map m(20, 20);
    m.print();
    m.map_search();

    

    if (!glfwInit())
    {
        return -1;
    }


    GLFWwindow* window = glfwCreateWindow(900, 900, "Vengo a por su percetron", NULL, NULL);


    if (!window)
    {
        glfwTerminate();
        return -1;
    }


    glfwMakeContextCurrent(window);


    if (!gladLoadGL(
        glfwGetProcAddress))
    {
        glfwTerminate();
        return -1;
    }


    while (!glfwWindowShouldClose(window))
    {
        glClearColor( 0.2f,0.2f, 0.2f,1.0f);

        glClear(GL_COLOR_BUFFER_BIT);


        m.drawMap();
        

        glfwSwapBuffers(window);

        glfwPollEvents();
    }


    glfwTerminate();

    return 0;
}
