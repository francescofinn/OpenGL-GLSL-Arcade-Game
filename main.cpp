#include "include/glad/glad.h"
#include "GLFW/glfw3.h"

#define GLT_IMPLEMENTATION
#include "include/gltext.h"

#include <iostream>
#include <vector>
#include <cstring>
#include <tuple>

using namespace std;

const float BRICK_WIDTH = 0.15F;
const float BRICK_GAP = 0.02F;
const float BRICK_HEIGHT = 0.05f;
const float BALL_WIDTH = 0.025f;
const float BALL_HEIGHT = 0.025f;
const float PADDLE_WIDTH = 0.2f;
const float PADDLE_HEIGHT = 0.05f;

// Shader sources
const char* vertexShaderSrc = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 position;

    uniform vec2 translation;
    uniform vec2 scale;

    out vec3 fragmentColor;

    void main() {
        vec2 pos = position * scale + translation;
        gl_Position = vec4(pos, 0.0, 1.0);
    }
)glsl";

const char* fragmentShaderSrc = R"glsl(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 color;

    void main() {
        FragColor = vec4(color, 1.0);
    }
)glsl";

struct Lives {
    int lives = 3;
    GLTtext *text;
    GLTtext *gameOverText = nullptr;
    const float x = 670.0;
    const float y = 0.5;
    const float scale = 2.0;
    bool gameOverDisplayed = false;

    Lives() {
        gltInit();

        text = gltCreateText();
        gltSetText(text, "Lives: 3");

        gltBeginDraw();

        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        gltDrawText2D(text, x, y, scale);

        gltEndDraw();
    }

    ~Lives() {
        gltDeleteText(text);
        if (gameOverText) {
            gltDeleteText(gameOverText);
        }
        gltTerminate();
    }

    void decrement() {
        if (lives > 0) {
            lives--;
            updateText();
        }

        if (lives == 0 && !gameOverDisplayed) {
            displayGameOver();
        }
    }

    void updateText() {
        char livesBuffer[20];
        sprintf(livesBuffer, "Lives: %d", lives);
        gltSetText(text, livesBuffer);
    }

    void displayGameOver() {
        gameOverText = gltCreateText();
        gltSetText(gameOverText, "GAME OVER!");
        gameOverDisplayed = true;
    }

    void draw() {
        gltBeginDraw();
        gltDrawText2D(text, x, y, scale);
        if (gameOverDisplayed) gltDrawText2D(gameOverText, 250.0, 300.0, 4.0);
        gltEndDraw();
    }
};

class Score {

public: int score = 0;

    Score() {
        // Initialize glText
        gltInit();

        // Creating text
        text = gltCreateText();
        gltSetText(text, "Score: 0");

        // Begin text drawing (this for instance calls glUseProgram)
        gltBeginDraw();

        // Draw any amount of text between begin and end
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        gltDrawText2D(text, x, y, scale);

        // Finish drawing text
        gltEndDraw();
    }

    ~Score() {
        // Destroy glText
        gltDeleteText(text);
        gltTerminate();
    }

    void update(int newScore) {
        char scoreBuffer[20];
        sprintf(scoreBuffer, "Score: %d", newScore);



        gltSetText(text, scoreBuffer);
    }

    void draw() {
        gltBeginDraw();
        gltDrawText2D(text, x, y, scale);
        gltEndDraw();
    }

    GLTtext *text;
    const float x = 0.5;
    const float y = 0.5;
    const float scale = 2.0;
public: void incrementScore() {
        this->score++;
        update(score);
    }
};

struct GameObject {
    float x, y, width, height;
    float dx, dy; // Velocity components
    float r, g, b;
    bool active = true;

    unsigned int VAO; // hello
    unsigned int VBO;

    GameObject(float x, float y, float width, float height, float dx = 0, float dy = 0, int r=255, int g=0, int b=0)
            : x(x), y(y), width(width), height(height), dx(dx), dy(dy), VAO(0), VBO(0), r(r), g(g), b(b) {

        // Initialize VAO and buffer data
        glGenVertexArrays(1, &VAO);
        //unsigned int VBO;
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        float vertices[] = {
                0.0f, 0.0f,
                1.0f, 0.0f,
                1.0f, 1.0f,
                0.0f, 1.0f
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        this->r = r/255.0;
        this->g = g/255.0;
        this->b = b/255.0;
    }

    ~GameObject() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    // Delete copy constructor and copy assignment operator
    GameObject(const GameObject&) = default;
    GameObject& operator=(const GameObject&) = delete;

    // Delete move constructor and move assignment operator
    GameObject(GameObject&&) = delete;
    GameObject& operator=(GameObject&&) = delete;

    void draw(unsigned int shaderProgram) const {
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        glUniform2f(glGetUniformLocation(shaderProgram, "translation"), x, y);
        glUniform2f(glGetUniformLocation(shaderProgram, "scale"), width, height);
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), r, g, b); // Set the color

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
    }

    void move(float deltaTime) {
        x += dx * deltaTime;
        y += dy * deltaTime;
    }
};

class Paddle {
    Paddle(GameObject a, GameObject b, GameObject c) {

    }
};


// Function prototypes
void processInput(GLFWwindow *window, GameObject paddleT[]);
unsigned int compileShader(unsigned int type, const char* source);
unsigned int createShaderProgram(const char* vertexSrc, const char* fragmentSrc);
void handleCollisions(GameObject& ball, GameObject paddle[], vector<GameObject>& bricks, Score* score);
int resetAfterBallLoss(GameObject& ball, Lives* lives);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Brick Breaker - Francesco 261077490 & Kevin 261108956", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Shader setup
    unsigned int shaderProgram = createShaderProgram(vertexShaderSrc, fragmentShaderSrc);
    glUseProgram(shaderProgram);

    // left paddle colour: 233, 245, 66
    // middle paddle colour: 66, 245, 84
    // right paddle colour: 0, 149, 255

    // Game setup
    GameObject paddleT[3] = {
            {-0.3f, -0.8f, 0.2f, 0.05f, 0, 0, 233, 245, 66},
            {-0.1f, -0.8f, 0.2f, 0.05f, 0, 0, 66, 245, 84},
            {0.1f, -0.8f, 0.2f, 0.05f, 0, 0, 0, 149, 255},
    };

    GameObject ball(0.15f, -0.15f, BALL_WIDTH, BALL_HEIGHT, 0.0f, -0.3f, 255, 0, 255);
    std::vector<GameObject> bricks;
    bricks.reserve(187);

    // create da brix
    int r = 255, g = 0, b = 0;
    for (float x = -1; x <= 1; x += BRICK_WIDTH + BRICK_GAP) {
        for (float y = 1 - 2*(BRICK_HEIGHT + BRICK_GAP); y >= 0.1; y -= BRICK_HEIGHT + BRICK_GAP) {
            b += 3;
            bricks.emplace_back(x, y - BRICK_HEIGHT, BRICK_WIDTH, BRICK_HEIGHT, 0, 0, r, g, b);
            g += 6;
        }
        r -= 20;
    }

    // Escape flag
    bool noLives = false;

    // init score & lives
    auto *scoreObj = new Score();
    auto *lives = new Lives();

    double lastFrameTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double currentFrameTime = glfwGetTime();
        double deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (!noLives) {
            processInput(window, paddleT);
            handleCollisions(ball, paddleT, bricks, scoreObj);

            paddleT[0].move(deltaTime);
            paddleT[1].move(deltaTime);
            paddleT[2].move(deltaTime);
            ball.move(deltaTime);

            if (ball.y <= -1.0f) {
                resetAfterBallLoss(ball, lives);
                if (lives->gameOverDisplayed) noLives = true;
            }
        } else {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }

        paddleT[0].draw(shaderProgram);
        paddleT[1].draw(shaderProgram);
        paddleT[2].draw(shaderProgram);
        ball.draw(shaderProgram);

        size_t s = bricks.size();
        for (int i = 0; i < s; i++) {
            if (bricks.at(i).active) bricks.at(i).draw(shaderProgram);
        }

        scoreObj->draw();
        lives->draw();

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window, GameObject paddleT[]) {
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        paddleT[0].dx = -0.5f;
        paddleT[1].dx = -0.5f;
        paddleT[2].dx = -0.5f;
    }
    else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        paddleT[0].dx = 0.5f;
        paddleT[1].dx = 0.5f;
        paddleT[2].dx = 0.5f;
    }
    else {
        paddleT[0].dx = 0.0f;
        paddleT[1].dx = 0.0f;
        paddleT[2].dx = 0.0f;
    }
}

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cerr << "Failed to compile " <<
                  (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cerr << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

unsigned int createShaderProgram(const char* vertexSrc, const char* fragmentSrc) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexSrc);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int resetAfterBallLoss(GameObject& ball, Lives* lives) {
    if (ball.y <= -1.0f) {
        ball.x =0.15f;
        ball.y = -0.15f;
        ball.dx = 0.0f;
        ball.dy = -0.2f;

        if (lives->lives > 0) {
            lives->decrement();
        }
        return 1;
    }
    return 0;
}

int checkWallCollision(GameObject& ball) {
    if (((ball.x + BALL_WIDTH) >= 1.0f && (ball.y + BALL_HEIGHT) < 1.0f && ball.y > -1.0f) || (ball.x <= -1.0f && (ball.y + BALL_HEIGHT) < 1.0f && ball.y > -1.0f)) {
        return 1; // Left or right wall
    } else if ((ball.y + BALL_HEIGHT) >= 1.0f && ball.x > -1.0f && (ball.x + BALL_WIDTH) < 1.0f) {
        return 2; // Top wall
    } else if (((ball.y + BALL_HEIGHT) == 1.0f && (ball.x + BALL_WIDTH) == 1.0f) || (ball.x == -1.0f && (ball.y + BALL_HEIGHT) == 1.0f)) {
        return 3; // Corners
    }
    return 0; // No collision

}

int checkPaddleCollision(GameObject& ball, GameObject paddle[]) {
    if ((ball.x + BALL_WIDTH/2 >= paddle[0].x && ball.x + BALL_WIDTH/2 <= (paddle[0].x + PADDLE_WIDTH)) && (ball.y <= paddle[0].y + PADDLE_HEIGHT)) {
        return 1; // Left paddle
    } else if ((ball.x + BALL_WIDTH/2 >= paddle[1].x && ball.x + BALL_WIDTH/2 <= (paddle[1].x + PADDLE_WIDTH)) && (ball.y <= paddle[1].y + PADDLE_HEIGHT)) {
        return 2; // Middle paddle
    } else if ((ball.x + BALL_WIDTH/2 >= paddle[2].x && ball.x + BALL_WIDTH/2 <= (paddle[2].x + PADDLE_WIDTH)) && (ball.y <= paddle[2].y + PADDLE_HEIGHT)) {
        return 3; // Right paddle
    }
    return 0; // No collision
}

int checkBrickCollision(GameObject& brick, GameObject& ball) {
    if ((ball.x <= (brick.x + BRICK_WIDTH) && ball.x >= brick.x  && (ball.y + BALL_HEIGHT/2) >= brick.y && (ball.y + BALL_HEIGHT/2) <= (brick.y + BRICK_HEIGHT)) ||
        ((ball.x + BALL_WIDTH) >= brick.x && (ball.x + BALL_WIDTH) <= brick.x + BRICK_WIDTH && (ball.y + BALL_HEIGHT/2) >= brick.y && (ball.y + BALL_HEIGHT/2) <= (brick.y + BRICK_HEIGHT))) {
        return 1; // Left or right side
    } else if ((ball.x + BALL_WIDTH/2 >= brick.x && ball.x + BALL_WIDTH/2 <= (brick.x + BRICK_WIDTH)) && (ball.y <= brick.y + BRICK_HEIGHT) && (ball.y >= brick.y) ||
               (ball.x + BALL_WIDTH/2 >= brick.x && ball.x + BALL_WIDTH/2 <= (brick.x + BRICK_WIDTH)) && (ball.y + BRICK_HEIGHT >= brick.y) && (ball.y + BALL_HEIGHT <= brick.y + BRICK_HEIGHT)) {
        return 2; // Bottom or top
    } else if ((ball.x == brick.x + BRICK_WIDTH && ball.y == brick.y + BRICK_HEIGHT) || (ball.x == brick.x + BRICK_WIDTH && ball.y + BALL_HEIGHT == brick.y) ||
               (ball.x + BALL_WIDTH == brick.x && ball.y + BALL_HEIGHT == brick.y) || (ball.x + BALL_WIDTH == brick.x + BRICK_WIDTH && ball.y == brick.y + BRICK_HEIGHT)) {
        return 3; // Corners (TR, BR, TL, BL)
    }
    return 0;
}

void handleCollisions(GameObject& ball, GameObject paddle[], vector<GameObject>& bricks, Score* score) {
    int wallCollisionResult = checkWallCollision(ball);
    bool collidedWall = false;
    switch (wallCollisionResult) {
        case 1:
            ball.dx = -ball.dx;
            collidedWall = true;
            break;
        case 2:
            ball.dy = -ball.dy;
            collidedWall = true;
            break;
        case 3:
            ball.dx = -ball.dx;
            ball.dy = -ball.dy;
            collidedWall = true;
            break;
        default:
            break;
    }

    if (collidedWall) return;

    int paddleCollisionResult = checkPaddleCollision(ball, paddle);
    bool collidedPaddle = false;
    switch(paddleCollisionResult) {
        case 1:
            ball.dx = -0.2f;
            ball.dy = 0.2f;
            collidedPaddle = true;
            break;
        case 2:
            ball.dx = 0.0f;
            ball.dy = 0.2f;
            collidedPaddle = true;
            break;
        case 3:
            ball.dx = 0.2f;
            ball.dy = 0.2f;
            collidedPaddle = true;
            break;
        default:
            break;
    }

    if (collidedPaddle) return;

    for (int i = 0; i < bricks.size(); i++) { // Keep index based as index will be used to determine score
        if (!bricks[i].active) continue;
        bool collided = false;

        int brickCollisionResult = checkBrickCollision(bricks[i], ball);
        switch(brickCollisionResult) {
            case 1:
                ball.dx = -ball.dx;
                collided = true;
                break;
            case 2:
                ball.dy = -ball.dy;
                collided = true;
                break;
            case 3:
                ball.dx = -ball.dx;
                ball.dy = -ball.dy;
                collided = true;
                break;
            default:
                break;
        }
        if (collided) {
            bricks[i].active = false;
            score->update(++score->score);
            return;
        }
    }

}


