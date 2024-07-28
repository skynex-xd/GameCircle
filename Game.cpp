#include "Engine.h"
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <sstream>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

void drawCircle(uint32_t* buffer, int cx, int cy, int radius, uint32_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                int px = cx + x;
                int py = cy + y;
                if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                    buffer[py * SCREEN_WIDTH + px] = color;
                }
            }
        }
    }
}

void drawSquare(uint32_t* buffer, int cx, int cy, int halfSize, uint32_t color) {
    for (int y = -halfSize; y <= halfSize; y++) {
        for (int x = -halfSize; x <= halfSize; x++) {
            int px = cx + x;
            int py = cy + y;
            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                buffer[py * SCREEN_WIDTH + px] = color;
            }
        }
    }
}

void drawDigit(uint32_t* buffer, int x, int y, char digit, int scale, uint32_t color) {
    const int font[10][5][3] = {
        { {1, 1, 1}, {1, 0, 1}, {1, 0, 1}, {1, 0, 1}, {1, 1, 1} },
        { {0, 1, 0}, {1, 1, 0}, {0, 1, 0}, {0, 1, 0}, {1, 1, 1} },
        { {1, 1, 1}, {0, 0, 1}, {1, 1, 1}, {1, 0, 0}, {1, 1, 1} },
        { {1, 1, 1}, {0, 0, 1}, {1, 1, 1}, {0, 0, 1}, {1, 1, 1} },
        { {1, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 0, 1}, {0, 0, 1} },
        { {1, 1, 1}, {1, 0, 0}, {1, 1, 1}, {0, 0, 1}, {1, 1, 1} },
        { {1, 1, 1}, {1, 0, 0}, {1, 1, 1}, {1, 0, 1}, {1, 1, 1} },
        { {1, 1, 1}, {0, 0, 1}, {0, 1, 0}, {1, 0, 0}, {1, 0, 0} },
        { {1, 1, 1}, {1, 0, 1}, {1, 1, 1}, {1, 0, 1}, {1, 1, 1} },
        { {1, 1, 1}, {1, 0, 1}, {1, 1, 1}, {0, 0, 1}, {1, 1, 1} }
    };

    int d = digit - '0';
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (font[d][i][j]) {
                drawSquare(buffer, x + j * scale, y + i * scale, scale / 2, color);
            }
        }
    }
}

void drawText(uint32_t* buffer, int x, int y, const string& text, int scale, uint32_t color) {
    for (char c : text) {
        if (c >= '0' && c <= '9') {
            drawDigit(buffer, x, y, c, scale, color);
        }
        x += 4 * scale;
    }
}

class GameObject {
public:
    float x, y;
    virtual void update(float dt) = 0;
    virtual void render(uint32_t* buffer) = 0;
};

class Particle : public GameObject {
public:
    float dx, dy;
    float lifetime;
    float maxLifetime;
    int initialSize;
    uint32_t color;

    Particle(float startX, float startY, float startDX, float startDY, float startLifetime, int startSize, uint32_t color) {
        x = startX;
        y = startY;
        dx = startDX;
        dy = startDY;
        lifetime = startLifetime;
        maxLifetime = startLifetime;
        initialSize = startSize;
        this->color = color;
    }

    void update(float dt) override {
        x += dx * dt;
        y += dy * dt;
        lifetime -= dt;
    }

    void render(uint32_t* buffer) override {
        if (lifetime > 0) {
            float lifeRatio = lifetime / maxLifetime;
            int currentSize = static_cast<int>(initialSize * lifeRatio);
            if (currentSize > 0) {
                drawCircle(buffer, static_cast<int>(x), static_cast<int>(y), currentSize, color);
            }
        }
    }
};

class Player : public GameObject {
public:
    float angle;
    bool rotateClockwise;
    vector<Particle> particles;

    Player(int startX, int startY) {
        x = startX;
        y = startY;
        angle = 0.0f;
        rotateClockwise = true;
    }

    void update(float dt) override {
        float rotationSpeed = 2.0f * M_PI / 2.0f;
        angle += (rotateClockwise ? rotationSpeed : -rotationSpeed) * dt;
        if (angle > 2 * M_PI) angle -= 2 * M_PI;
        if (angle < 0) angle += 2 * M_PI;

        int radius = 100;
        int centerX = SCREEN_WIDTH / 2;
        int centerY = SCREEN_HEIGHT / 2;

        int sphere1X = centerX + static_cast<int>(radius * cos(angle));
        int sphere1Y = centerY + static_cast<int>(radius * sin(angle));
        int sphere2X = centerX + static_cast<int>(radius * cos(angle + M_PI));
        int sphere2Y = centerY + static_cast<int>(radius * sin(angle + M_PI));

        particles.emplace_back(sphere1X, sphere1Y, 0, 0, 0.5f, 20, 0xFFFFFFFF);
        particles.emplace_back(sphere2X, sphere2Y, 0, 0, 0.5f, 20, 0xFFFFFFFF);

        for (auto& particle : particles) {
            particle.update(dt);
        }
        particles.erase(remove_if(particles.begin(), particles.end(), [](Particle& p) {
            return p.lifetime <= 0;
            }), particles.end());
    }

    void render(uint32_t* buffer) override {
        int radius = 100;
        int centerX = SCREEN_WIDTH / 2;
        int centerY = SCREEN_HEIGHT / 2;
        int sphereRadius = 20;

        uint32_t greenColor = 0xFF3CA741;
        drawCircle(buffer, centerX, centerY, radius, greenColor);

        int sphere1X = centerX + static_cast<int>(radius * cos(angle));
        int sphere1Y = centerY + static_cast<int>(radius * sin(angle));
        int sphere2X = centerX + static_cast<int>(radius * cos(angle + M_PI));
        int sphere2Y = centerY + static_cast<int>(radius * sin(angle + M_PI));

        drawCircle(buffer, sphere1X, sphere1Y, sphereRadius, 0xFFFFFFFF);
        drawCircle(buffer, sphere2X, sphere2Y, sphereRadius, 0xFFFFFFFF);

        for (auto& particle : particles) {
            particle.render(buffer);
        }
    }

    void toggleRotation() {
        rotateClockwise = !rotateClockwise;
    }

    bool checkCollision(int otherX, int otherY, int radius) {
        int centerX = SCREEN_WIDTH / 2;
        int centerY = SCREEN_HEIGHT / 2;
        int orbitRadius = 100;
        int sphereRadius = 20;

        int sphere1X = centerX + static_cast<int>(orbitRadius * cos(angle));
        int sphere1Y = centerY + static_cast<int>(orbitRadius * sin(angle));
        int sphere2X = centerX + static_cast<int>(orbitRadius * cos(angle + M_PI));
        int sphere2Y = centerY + static_cast<int>(orbitRadius * sin(angle + M_PI));

        return (hypot(sphere1X - otherX, sphere1Y - otherY) < sphereRadius + radius ||
            hypot(sphere2X - otherX, sphere2Y - otherY) < sphereRadius + radius);
    }

    pair<int, int> getPosition(int sphereNumber) {
        int radius = 100;
        int centerX = SCREEN_WIDTH / 2;
        int centerY = SCREEN_HEIGHT / 2;

        if (sphereNumber == 1) {
            int sphereX = centerX + static_cast<int>(radius * cos(angle));
            int sphereY = centerY + static_cast<int>(radius * sin(angle));
            return make_pair(sphereX, sphereY);
        }
        else {
            int sphereX = centerX + static_cast<int>(radius * cos(angle + M_PI));
            int sphereY = centerY + static_cast<int>(radius * sin(angle + M_PI));
            return make_pair(sphereX, sphereY);
        }
    }
};

class Enemy : public GameObject {
public:
    float dx, dy;
    float speed;
    float targetX, targetY;

    Enemy(int startX, int startY, float targetX, float targetY) {
        x = startX;
        y = startY;
        this->targetX = targetX;
        this->targetY = targetY;
        float angle = atan2(targetY - y, targetX - x);
        dx = cos(angle);
        dy = sin(angle);
        speed = 100.0f;
    }

    void update(float dt) override {
        x += dx * speed * dt;
        y += dy * speed * dt;
        if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
            x = -1;
        }
    }

    void render(uint32_t* buffer) override {
        int intX = static_cast<int>(x);
        int intY = static_cast<int>(y);
        if (intX >= 0 && intX < SCREEN_WIDTH && intY >= 0 && intY < SCREEN_HEIGHT) {
            drawSquare(buffer, intX, intY, 10, 0xFF000000);
        }
    }

    void explode(vector<Particle>& particles) {
        for (int i = 0; i < 20; i++) {
            float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * M_PI;
            float speed = 50.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 100.0f;
            particles.emplace_back(x, y, cos(angle) * speed, sin(angle) * speed, 1.0f, 10, 0xFFFF0000);
        }
    }
};

class Bonus : public GameObject {
public:
    float dx, dy;
    float speed;
    float targetX, targetY;

    Bonus(int startX, int startY, float targetX, float targetY) {
        x = startX;
        y = startY;
        this->targetX = targetX;
        this->targetY = targetY;
        float angle = atan2(targetY - y, targetX - x);
        dx = cos(angle);
        dy = sin(angle);
        speed = 100.0f;
    }

    void update(float dt) override {
        x += dx * speed * dt;
        y += dy * speed * dt;
        if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
            x = -1;
        }
    }

    void render(uint32_t* buffer) override {
        int intX = static_cast<int>(x);
        int intY = static_cast<int>(y);
        if (intX >= 0 && intX < SCREEN_WIDTH && intY >= 0 && intY < SCREEN_HEIGHT) {
            drawCircle(buffer, intX, intY, 10, 0xFFFFFF00);
        }
    }

    void explode(vector<Particle>& particles) {
        for (int i = 0; i < 20; i++) {
            float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * M_PI;
            float speed = 50.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 100.0f;
            particles.emplace_back(x, y, cos(angle) * speed, sin(angle) * speed, 1.0f, 10, 0xFFFFFF00);
        }
    }
};

class Game {
private:
    Player player;
    vector<Enemy> enemies;
    vector<Bonus> bonuses;
    vector<Particle> particles;
    int score;
    bool spacePressed;
    bool isGameOver;
    bool isExplosionAnimation;
    uint32_t backgroundColor;

public:
    Game() : player(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), score(0), spacePressed(false), isGameOver(false), isExplosionAnimation(false) {
        backgroundColor = 0xFF324961;
        addEnemy();
        addBonus();
    }

    void handleInput() {
        if (is_key_pressed(VK_SPACE)) {
            if (!spacePressed) {
                player.toggleRotation();
                spacePressed = true;
            }
        }
        else {
            spacePressed = false;
        }

        if (is_key_pressed('R') && isGameOver && !isExplosionAnimation) {
            resetGame();
        }
    }

    void update(float dt) {
        if (isGameOver && !isExplosionAnimation) return;

        if (isExplosionAnimation) {
            updateParticles(dt);
            if (particles.empty()) {
                isExplosionAnimation = false;
            }
            return;
        }

        player.update(dt);

        for (auto& enemy : enemies) {
            enemy.update(dt);
            if (player.checkCollision(static_cast<int>(enemy.x), static_cast<int>(enemy.y), 10)) {
                enemy.explode(particles);
                isGameOver = true;
                isExplosionAnimation = true;
                explodePlayer();
                clearEnemiesAndPlayer();
                return;
            }
        }
        enemies.erase(remove_if(enemies.begin(), enemies.end(), [](Enemy& e) {
            return e.x == -1;
            }), enemies.end());

        for (auto& bonus : bonuses) {
            bonus.update(dt);
            if (player.checkCollision(static_cast<int>(bonus.x), static_cast<int>(bonus.y), 10)) {
                score++;
                int bonusX = bonus.x;
                int bonusY = bonus.y;
                bonus.explode(particles);
                bonus.x = -1;
                if (score % 5 == 0) {
                    changeBackgroundColor();
                }
            }
        }
        bonuses.erase(remove_if(bonuses.begin(), bonuses.end(), [](Bonus& b) {
            return b.x == -1;
            }), bonuses.end());

        updateParticles(dt);

        drawText(reinterpret_cast<uint32_t*>(buffer), 30, 30, to_string(score), 20, 0xFFFFFFFF);

        if (bonuses.size() < 1) {
            addBonus();
        }
        if (enemies.size() < (score / 5) + 1) {
            addEnemy();
        }
    }

    void updateParticles(float dt) {
        for (auto& particle : particles) {
            particle.update(dt);
        }
        particles.erase(remove_if(particles.begin(), particles.end(), [](Particle& p) {
            return p.lifetime <= 0;
            }), particles.end());
    }

    void render(uint32_t* buffer) {
        for (int i = 0; i < SCREEN_HEIGHT; ++i) {
            for (int j = 0; j < SCREEN_WIDTH; ++j) {
                buffer[i * SCREEN_WIDTH + j] = backgroundColor;
            }
        }
        if (!isExplosionAnimation) {
            player.render(buffer);
        }
        for (auto& enemy : enemies) {
            enemy.render(buffer);
        }
        for (auto& bonus : bonuses) {
            bonus.render(buffer);
        }
        for (auto& particle : particles) {
            particle.render(buffer);
        }

        drawText(buffer, 30, 30, to_string(score), 20, 0xFFFFFFFF);
    }

    void addEnemy() {
        int side = rand() % 2;
        int startX, startY;
        int centerX = SCREEN_WIDTH / 2;
        int centerY = SCREEN_HEIGHT / 2;

        float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * M_PI;
        float radius = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 100;
        float targetX = centerX + radius * cos(angle);
        float targetY = centerY + radius * sin(angle);

        if (side == 0) {
            startX = rand() % SCREEN_WIDTH;
            startY = SCREEN_HEIGHT - 1;
        }
        else {
            startX = SCREEN_WIDTH - 1;
            startY = rand() % SCREEN_HEIGHT;
        }

        enemies.push_back(Enemy(startX, startY, targetX, targetY));
    }

    void addBonus() {
        int side = rand() % 2;
        int startX, startY;
        int centerX = SCREEN_WIDTH / 2;
        int centerY = SCREEN_HEIGHT / 2;

        float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * M_PI;
        float radius = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 100;
        float targetX = centerX + radius * cos(angle);
        float targetY = centerY + radius * sin(angle);

        if (side == 0) {
            startX = rand() % SCREEN_WIDTH;
            startY = SCREEN_HEIGHT - 1;
        }
        else {
            startX = SCREEN_WIDTH - 1;
            startY = rand() % SCREEN_HEIGHT;
        }

        bonuses.push_back(Bonus(startX, startY, targetX, targetY));
    }

    void explodePlayer() {
        pair<int, int> pos1 = player.getPosition(1);
        pair<int, int> pos2 = player.getPosition(2);

        int sphere1X = pos1.first;
        int sphere1Y = pos1.second;
        int sphere2X = pos2.first;
        int sphere2Y = pos2.second;

        for (int i = 0; i < 50; i++) {
            float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * M_PI;
            float speed = 50.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 100.0f;
            particles.emplace_back(sphere1X, sphere1Y, cos(angle) * speed, sin(angle) * speed, 1.0f, 20, 0xFFFFFFFF);
            particles.emplace_back(sphere2X, sphere2Y, cos(angle) * speed, sin(angle) * speed, 1.0f, 20, 0xFFFFFFFF);
        }
    }

    void clearEnemiesAndPlayer() {
        enemies.clear();
        player.particles.clear();
    }

    void resetGame() {
        enemies.clear();
        bonuses.clear();
        particles.clear();
        score = 0;
        isGameOver = false;
        backgroundColor = 0xFF808080;
        addEnemy();
        addBonus();
    }

    void changeBackgroundColor() {
        static vector<uint32_t> colors = {
            0xFF0C486C,
            0xFF3B8787,
            0xFF7ABD9A,
            0xFFA9DBA8,
            0xFFCFF09F
        };
        static size_t colorIndex = 0;

        backgroundColor = colors[colorIndex];
        colorIndex = (colorIndex + 1) % colors.size();
    }
};

static Game game;

void initialize() {
    srand(static_cast<unsigned>(time(nullptr)));
    game = Game();
}

void act(float dt) {
    if (is_key_pressed(VK_ESCAPE))
        schedule_quit_game();
    game.handleInput();
    game.update(dt);
}

void draw() {
    game.render(reinterpret_cast<uint32_t*>(buffer));
}

void finalize() {
}
