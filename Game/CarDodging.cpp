#include <SFML/Graphics.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <cmath>

using namespace sf;
using namespace std;

class Car {
public:
    Sprite sprite;
    float speed = 0.f;

    FloatRect bounds() const {
        return sprite.getGlobalBounds();
    }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    RenderWindow window(VideoMode(600, 800), "Car Dodging Game");
    window.setFramerateLimit(60);

    // ---------------- FONT ----------------
    Font font;
    font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf");

    Text scoreText, speedText, pauseText, gameOverText;
    scoreText.setFont(font);
    speedText.setFont(font);
    pauseText.setFont(font);
    gameOverText.setFont(font);

    scoreText.setCharacterSize(24);
    speedText.setCharacterSize(24);
    pauseText.setCharacterSize(32);
    gameOverText.setCharacterSize(30);

    scoreText.setPosition(20, 10);
    speedText.setPosition(430, 10);

    pauseText.setString("PAUSED");
    pauseText.setPosition(220, 350);

    gameOverText.setString("GAME OVER\nPress R to Restart");
    gameOverText.setPosition(135, 330);

    // ---------------- ROAD ----------------
    RectangleShape leftGrass(Vector2f(120.f, 800.f));
    RectangleShape rightGrass(Vector2f(120.f, 800.f));
    leftGrass.setFillColor(Color(34, 139, 34));
    rightGrass.setFillColor(Color(34, 139, 34));
    leftGrass.setPosition(0, 0);
    rightGrass.setPosition(480, 0);

    vector<RectangleShape> laneLines;
    for (int i = 0; i < 10; i++) {
        RectangleShape line(Vector2f(10.f, 60.f));
        line.setFillColor(Color::White);
        line.setPosition(295.f, i * 90.f);
        laneLines.push_back(line);
    }

    // ---------------- TEXTURES ----------------
    Texture playerTexture;
    playerTexture.loadFromFile("Assets_EndTermProj/Assets/WhiteCar.png");

    vector<Texture> enemyTextures(5);
    enemyTextures[0].loadFromFile("Assets_EndTermProj/Assets/RedCar1.png");
    enemyTextures[1].loadFromFile("Assets_EndTermProj/Assets/RedCar2.png");
    enemyTextures[2].loadFromFile("Assets_EndTermProj/Assets/YellowCar1.png");
    enemyTextures[3].loadFromFile("Assets_EndTermProj/Assets/YellowCar2.png");
    enemyTextures[4].loadFromFile("Assets_EndTermProj/Assets/YellowCar3.png");

    // ---------------- PLAYER CAR ----------------
    Car player;
    player.sprite.setTexture(playerTexture);
    player.sprite.setScale(0.55f, 0.55f);
    player.sprite.setPosition(270.f, 650.f);

    float playerSpeed = 360.f;

    // ---------------- ENEMIES ----------------
    vector<Car> enemies;
    vector<float> lanes = {150.f, 250.f, 350.f, 450.f};

    Clock gameClock;
    Clock spawnClock;

    int score = 0;
    float roadSpeed = 240.f;
    bool paused = false;
    bool gameOver = false;

    // ================= GAME LOOP =================
    while (window.isOpen()) {
        float dt = gameClock.restart().asSeconds();
        Event event;

        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::P && !gameOver)
                    paused = !paused;

                if (event.key.code == Keyboard::R && gameOver) {
                    enemies.clear();
                    score = 0;
                    roadSpeed = 240.f;
                    paused = false;
                    gameOver = false;
                    player.sprite.setPosition(270.f, 650.f);
                    spawnClock.restart();

                    for (int i = 0; i < laneLines.size(); i++) {
                        laneLines[i].setPosition(295.f, i * 90.f);
                    }
                }
            }
        }

        if (!paused && !gameOver) {
            // ---------------- PLAYER MOVEMENT ----------------
            if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A))
                player.sprite.move(-playerSpeed * dt, 0);

            if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D))
                player.sprite.move(playerSpeed * dt, 0);

            if (player.sprite.getPosition().x < 140)
                player.sprite.setPosition(140, 650);

            if (player.sprite.getPosition().x > 460)
                player.sprite.setPosition(460, 650);

            // ---------------- ROAD MOVEMENT ----------------
            for (auto& line : laneLines) {
                line.move(0, roadSpeed * dt);
                if (line.getPosition().y > 800)
                    line.setPosition(295.f, -60.f);
            }

            // ---------------- ENEMY SPAWN ----------------
            if (spawnClock.getElapsedTime().asSeconds() > 0.85f) {
                int laneIndex = rand() % lanes.size();
                float laneX = lanes[laneIndex];

                bool canSpawn = true;

                for (auto& e : enemies) {
                    if (abs(e.sprite.getPosition().x - laneX) < 5.f &&
                        e.sprite.getPosition().y < 180.f) {
                        canSpawn = false;
                        break;
                    }
                }

                if (canSpawn) {
                    Car enemy;
                    int texIndex = rand() % enemyTextures.size();

                    enemy.sprite.setTexture(enemyTextures[texIndex]);
                    enemy.sprite.setScale(0.55f, 0.55f);
                    enemy.sprite.setPosition(laneX, -120.f);
                    enemy.speed = roadSpeed + (rand() % 80);

                    enemies.push_back(enemy);
                    spawnClock.restart();
                }
            }

            // ---------------- ENEMY CAR MOVEMENT ----------------
            for (auto& enemy : enemies)
                enemy.sprite.move(0, enemy.speed * dt);

            // ---------------- COLLISION ----------------
            for (auto& enemy : enemies) {
                if (enemy.bounds().intersects(player.bounds())) {
                    gameOver = true;
                    break;
                }
            }

            // ---------------- REMOVE PASSED ENEMIES ----------------
            enemies.erase(
                remove_if(enemies.begin(), enemies.end(),
                    [&](Car& e) {
                        if (e.sprite.getPosition().y > 850) {
                            score++;
                            roadSpeed += 4.f;
                            return true;
                        }
                        return false;
                    }),
                enemies.end()
            );
        }

        // ---------------- HUD ----------------
        scoreText.setString("SCORE: " + to_string(score));
        speedText.setString("SPEED: " + to_string((int)roadSpeed / 50));

        // ---------------- DRAW ----------------
        window.clear(Color::Black);

        window.draw(leftGrass);
        window.draw(rightGrass);

        for (auto& line : laneLines)
            window.draw(line);

        for (auto& enemy : enemies)
            window.draw(enemy.sprite);

        window.draw(player.sprite);
        window.draw(scoreText);
        window.draw(speedText);

        if (paused)
            window.draw(pauseText);

        if (gameOver)
            window.draw(gameOverText);

        window.display();
    }
    return 0;
}
