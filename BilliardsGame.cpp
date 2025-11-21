#include "BilliardsGame.h"
#include <cmath>
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Konstruktor
BilliardsGame::BilliardsGame(int windowW, int windowH, float border)
: windowWidth(windowW), windowHeight(windowH), borderThickness(border),
  window(sf::VideoMode(windowW, windowH), "Simulation Billiard Game"),
  table(windowW, windowH, borderThickness),
  cue(),
  currentGameState(BREAK),
  currentPlayer(1),
  shotMade(false),
  cueBallPocketed(false),
  isFoul(false),
  firstBallHitThisShot(false),
  ballHitRailAfterContact(false),
  firstObjectBallHit(-1),
  isDragging(false),
  currentCueAngle(0.f)
{
    // Table Bounds
    tableLeft = borderThickness;
    tableTop = borderThickness;
    tableRight = windowWidth - borderThickness;
    tableBottom = windowHeight - borderThickness;

    window.setFramerateLimit(60);

    // load font
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error loading font." << std::endl;
    }

    // create rack & holes
    balls = createRack(tableLeft, tableTop, tableRight, tableBottom);

    float holeRadius = 28.f;
    holes = {
        {tableLeft, tableTop, holeRadius}, {tableRight, tableTop, holeRadius},
        {tableLeft, tableBottom, holeRadius}, {tableRight, tableBottom, holeRadius},
        {(tableLeft + tableRight) / 2.f, tableTop, holeRadius * 0.8f},
        {(tableLeft + tableRight) / 2.f, tableBottom, holeRadius * 0.8f}
    };

    // UI text shapes
    statusText.setFont(font);
    statusText.setCharacterSize(24);
    statusText.setFillColor(sf::Color::White);
    statusText.setPosition(10.f, 10.f);

    p1Status = sf::RectangleShape(sf::Vector2f(150.f, 30.f));
    p1Status.setPosition(50.f, windowHeight - 40.f);
    p1Status.setOutlineThickness(2.f);
    p1Status.setOutlineColor(sf::Color::White);

    p2Status = sf::RectangleShape(sf::Vector2f(150.f, 30.f));
    p2Status.setPosition(windowWidth - 200.f, windowHeight - 40.f);
    p2Status.setOutlineThickness(2.f);
    p2Status.setOutlineColor(sf::Color::White);

    p1Text.setFont(font);
    p1Text.setCharacterSize(18);
    p1Text.setPosition(p1Status.getPosition().x + 5.f, p1Status.getPosition().y + 3.f);

    p2Text.setFont(font);
    p2Text.setCharacterSize(18);
    p2Text.setPosition(p2Status.getPosition().x + 5.f, p2Status.getPosition().y + 3.f);

    // init player groups
    playerGroup[1] = NONE;
    playerGroup[2] = NONE;
}

// Utility: group helpers
BallGroup BilliardsGame::getBallGroup(int ballNumber) const {
    if (ballNumber == 0) return CUE_BALL;
    if (ballNumber == 8) return EIGHT_BALL;
    if (ballNumber >= 1 && ballNumber <= 7) return SOLID;
    if (ballNumber >= 9 && ballNumber <= 15) return STRIPES;
    return NONE;
}

std::string BilliardsGame::getGroupStr(BallGroup group) const {
    if (group == SOLID) return "SOLID";
    if (group == STRIPES) return "STRIPES";
    if (group == EIGHT_BALL) return "8-BALL";
    return "NONE";
}

// Collision handler
void BilliardsGame::handleBallCollision(Ball &a, Ball &b) {
    if (a.isPocketed || b.isPocketed) return;

    sf::Vector2f diff = b.shape.getPosition() - a.shape.getPosition();
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    float radiusSum = a.shape.getRadius() + b.shape.getRadius();

    if (dist < radiusSum && dist > 0.f) {
        // firstBallHitThisShot
        if (a.ballNumber == 0 || b.ballNumber == 0) {
            firstBallHitThisShot = true;

            // firstObjectBallHit
            if (firstObjectBallHit == -1) {
                if (a.ballNumber == 0 && b.ballNumber > 0) {
                    firstObjectBallHit = b.ballNumber;
                } else if (b.ballNumber == 0 && a.ballNumber > 0) {
                    firstObjectBallHit = a.ballNumber;
                }
            }
        }

        sf::Vector2f norm = diff / dist;
        sf::Vector2f tangent(-norm.y, norm.x);

        float v1n = a.velocity.x * norm.x + a.velocity.y * norm.y;
        float v1t = a.velocity.x * tangent.x + a.velocity.y * tangent.y;
        float v2n = b.velocity.x * norm.x + b.velocity.y * norm.y;
        float v2t = b.velocity.x * tangent.x + b.velocity.y * tangent.y;

        float m1 = a.mass, m2 = b.mass;
        float v1nAfter = (v1n * (m1 - m2) + 2 * m2 * v2n) / (m1 + m2);
        float v2nAfter = (v2n * (m2 - m1) + 2 * m1 * v1n) / (m1 + m2);

        sf::Vector2f v1nVec = v1nAfter * norm;
        sf::Vector2f v1tVec = v1t * tangent;
        sf::Vector2f v2nVec = v2nAfter * norm;
        sf::Vector2f v2tVec = v2t * tangent;

        a.velocity = v1nVec + v1tVec;
        b.velocity = v2nVec + v2tVec;

        float overlap = radiusSum - dist + 0.1f;
        a.shape.move(-norm * overlap / 2.f);
        b.shape.move(norm * overlap / 2.f);
    }
}

// All balls stopped check
bool BilliardsGame::allBallsStopped(const std::vector<Ball>& ballsVec) const {
    for (const auto& b : ballsVec)
        if (!b.isPocketed && (std::abs(b.velocity.x) > 0.1f || std::abs(b.velocity.y) > 0.1f))
            return false;
    return true;
}

// createRack
std::vector<Ball> BilliardsGame::createRack(float tableLeft, float tableTop, float tableRight, float tableBottom) {
    std::vector<Ball> ballsVec;
    float radius = 12.f;

    float centerX = (tableLeft + tableRight) / 2.f;
    float centerY = (tableTop + tableBottom) / 2.f;

    // Cue Ball
    sf::Vector2f cuePos(centerX - 250.f, centerY);
    ballsVec.emplace_back(radius, cuePos, sf::Color::White, 0, false);

    // Titik tengah rak segitiga
    sf::Vector2f apex(centerX + 200.f, centerY);
    float spacing = radius * 2.f * 0.87f;

    // Nomor bola di rak
    std::vector<int> ballOrder = {
        1,
        9, 8,
        2, 10, 3,
        11, 4, 12, 5,
        13, 6, 14, 7, 15
    };

    int ballIndex = 0;
    for (int row = 0; row < 5; ++row) {
        float rowX = apex.x + row * spacing;
        float yOffset = (row * spacing) / 2.f;
        for (int col = 0; col <= row; ++col) {
            int ballNumber = ballOrder[ballIndex++];
            sf::Color ballColor;

            if (ballNumber == 0) continue;

            if (ballNumber == 8) ballColor = sf::Color::Black;
            else if (ballNumber >= 1 && ballNumber <= 7) ballColor = sf::Color(255, 165, 0);
            else if (ballNumber >= 9 && ballNumber <= 15) ballColor = sf::Color::Blue;

            float y = apex.y - yOffset + col * spacing;
            ballsVec.emplace_back(radius, sf::Vector2f(rowX, y), ballColor, ballNumber, false);
        }
    }

    return ballsVec;
}

// Event processing
void BilliardsGame::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();

        // R: Reset Game
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
            balls = createRack(tableLeft, tableTop, tableRight, tableBottom);
            currentGameState = BREAK;
            currentPlayer = 1;
            playerGroup[1] = NONE;
            playerGroup[2] = NONE;
            firstBallHitThisShot = false;
            ballHitRailAfterContact = false;
            firstObjectBallHit = -1;
        }

        // LOGIKA BALL IN HAND
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mp = (sf::Vector2f)sf::Mouse::getPosition(window);

            if (currentGameState == BALL_IN_HAND) {
                if (mp.x > tableLeft + 5.f && mp.x < tableRight - 5.f &&
                    mp.y > tableTop + 5.f && mp.y < tableBottom - 5.f) {

                    balls[0].shape.setPosition(mp);
                    balls[0].isPocketed = false;

                    currentGameState = (playerGroup[currentPlayer] == SOLID) ? SOLID_TURN : STRIPES_TURN;
                    if (playerGroup[currentPlayer] == NONE) currentGameState = OPEN_TABLE;
                }
            } else if (allBallsStopped(balls) && currentGameState != GAME_OVER) {
                sf::Vector2f cueBallPos = balls[0].shape.getPosition();
                sf::Vector2f diff = mp - cueBallPos;
                if (std::sqrt(diff.x * diff.x + diff.y * diff.y) < 30.f) {
                    isDragging = true;
                    dragStartPos = mp;
                }
            }
        }

        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            if (isDragging && allBallsStopped(balls) && currentGameState != BALL_IN_HAND && currentGameState != GAME_OVER) {
                sf::Vector2f mp = (sf::Vector2f)sf::Mouse::getPosition(window);

                sf::Vector2f shootDir = mp - balls[0].shape.getPosition();
                float len = std::sqrt(shootDir.x * shootDir.x + shootDir.y * shootDir.y);

                float maxPull = 100.f;
                float power = std::min(len, maxPull) * 20.0f;

                if (len != 0) {
                    shootDir /= len;
                    balls[0].velocity = shootDir * power;
                    shotMade = true;

                    cueBallPocketed = false;
                    pocketedObjBallsThisTurn.clear();
                    isFoul = false;

                    // RESET VARIABEL ATURAN BARU SEBELUM SHOT
                    firstBallHitThisShot = false;
                    ballHitRailAfterContact = false;
                    firstObjectBallHit = -1;
                }

                isDragging = false;
            }
        }
    }
}

// Update (physics, collision, pocket, turn resolution)
void BilliardsGame::update(float dt) {
    bool canAim = allBallsStopped(balls);

    // Cek batas meja
    for (auto &b : balls) {
        sf::Vector2f pos = b.shape.getPosition();
        float r = b.shape.getRadius();
        if (!b.isPocketed && (std::abs(b.velocity.x) > 0.1f || std::abs(b.velocity.y) > 0.1f)) {
            if (pos.x - r <= tableLeft || pos.x + r >= tableRight ||
                pos.y - r <= tableTop || pos.y + r >= tableBottom) {
                if (firstBallHitThisShot) {
                    ballHitRailAfterContact = true;
                }
            }
        }
        b.update(dt, window, tableLeft, tableTop, tableRight, tableBottom);
    }

    // collisions
    for (size_t i = 0; i < balls.size(); ++i) {
        for (size_t j = i + 1; j < balls.size(); ++j)
            handleBallCollision(balls[i], balls[j]);
    }

    // pocketing
    for (auto &h : holes) {
        for (size_t i = 0; i < balls.size(); ++i) {
            if (!balls[i].isPocketed && h.checkBallInHole(balls[i])) {
                if (balls[i].ballNumber == 0) {
                    cueBallPocketed = true;
                } else {
                    pocketedObjBallsThisTurn.push_back(balls[i].ballNumber);
                }
            }
        }
    }

    // resolve turn if shot was made and balls stopped
    if (shotMade && canAim) {
        handleTurnEnd();
        shotMade = false;
    }

    // If cue ball pocketed, reset to head string area
    if (balls[0].isPocketed && (currentGameState != GAME_OVER)) {
        float centerX = (tableLeft + tableRight) / 2.f;
        float centerY = (tableTop + tableBottom) / 2.f;
        balls[0].shape.setPosition(centerX - 100.f, centerY);
        balls[0].velocity = sf::Vector2f(0.f, 0.f);
        balls[0].isPocketed = false;
    }
}

// Turn resolution
void BilliardsGame::handleTurnEnd() {
    BallGroup currentGroup = playerGroup[currentPlayer];
    bool pocketedMyBall = false;
    bool pocketed8Ball = false;

    // 1. scratch
    if (cueBallPocketed) {
        isFoul = true;
    }

    // 2. didn't hit any ball
    if (!isFoul && !firstBallHitThisShot) {
        isFoul = true;
    }

    // 3. first hit opponent's ball (only if group set)
    if (!isFoul && currentGroup != NONE) {
        BallGroup firstHitGroup = getBallGroup(firstObjectBallHit);
        if (firstHitGroup != currentGroup && firstHitGroup != EIGHT_BALL) {
            isFoul = true;
        }
    }

    // 4. safety rule (no pocket & no rail touch after contact)
    if (!isFoul && pocketedObjBallsThisTurn.empty() && !ballHitRailAfterContact) {
        isFoul = true;
    }

    // 5. check 8-ball pocketed
    for (int number : pocketedObjBallsThisTurn) {
        if (number == 8) {
            pocketed8Ball = true;
            break;
        }
    }

    // 6. win/lose with 8-ball
    if (pocketed8Ball) {
        bool allMyBallsPocketed = (currentGroup != NONE && std::count_if(balls.begin(), balls.end(),
            [currentGroup](const Ball& b){ return (b.ballNumber>0) && ( (currentGroup==SOLID && b.ballNumber>=1 && b.ballNumber<=7) || (currentGroup==STRIPES && b.ballNumber>=9 && b.ballNumber<=15) ) && !b.isPocketed; }) == 0);

        if (allMyBallsPocketed && !isFoul) {
            currentGameState = GAME_OVER; // win
        } else {
            currentGameState = GAME_OVER;
            currentPlayer = (currentPlayer == 1) ? 2 : 1; // lose -> opponent wins
        }
    }

    // 7. determine groups if OPEN_TABLE or BREAK
    if (currentGameState == BREAK || currentGameState == OPEN_TABLE) {
        for (int number : pocketedObjBallsThisTurn) {
            BallGroup pocketedGroup = getBallGroup(number);
            if (pocketedGroup == SOLID || pocketedGroup == STRIPES) {
                playerGroup[currentPlayer] = pocketedGroup;
                playerGroup[(currentPlayer == 1) ? 2 : 1] = (pocketedGroup == SOLID) ? STRIPES : SOLID;

                currentGameState = (pocketedGroup == SOLID) ? SOLID_TURN : STRIPES_TURN;
                pocketedMyBall = true;
                break;
            }
        }
    }

    // 8. check if pocketed own ball after groups set
    if (currentGameState == SOLID_TURN || currentGameState == STRIPES_TURN) {
        for (int number : pocketedObjBallsThisTurn) {
            if (getBallGroup(number) == currentGroup) {
                pocketedMyBall = true;
                break;
            }
        }
    }

    // 9. determine next player
    if (currentGameState != GAME_OVER) {
        if (isFoul) {
            currentPlayer = (currentPlayer == 1) ? 2 : 1;
            currentGameState = BALL_IN_HAND;
        } else if (!pocketedMyBall && !pocketed8Ball) {
            currentPlayer = (currentPlayer == 1) ? 2 : 1;

            if (playerGroup[currentPlayer] == SOLID) currentGameState = SOLID_TURN;
            else if (playerGroup[currentPlayer] == STRIPES) currentGameState = STRIPES_TURN;
            else currentGameState = OPEN_TABLE;
        } else {
            // continue turn if pocketed own ball
        }
    }

    // 10. reset cue ball if scratch
    if (cueBallPocketed) {
        balls[0].velocity = sf::Vector2f(0.f, 0.f);
        balls[0].isPocketed = false;
    }

    // RESET RULE VARIABLES
    firstBallHitThisShot = false;
    ballHitRailAfterContact = false;
    firstObjectBallHit = -1;
    cueBallPocketed = false;
    pocketedObjBallsThisTurn.clear();
    isFoul = false;
}

// Render (copied & adapted)
void BilliardsGame::render() {
    window.clear(sf::Color(50, 50, 50));
    table.draw(window);
    for (auto &h : holes) h.draw(window);
    for (auto &b : balls) b.draw(window, font);

    // aiming & cue rendering
    bool canAim = allBallsStopped(balls);
    if (canAim && currentGameState != BALL_IN_HAND && currentGameState != GAME_OVER) {
        sf::Vector2f cueBallPos = balls[0].shape.getPosition();
        sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);

        sf::Vector2f targetDir = mousePos - cueBallPos;
        float len = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
        if (len != 0) targetDir /= len;
        float targetAngle = std::atan2(targetDir.y, targetDir.x) * 180.f / M_PI;

        float deltaAngle = targetAngle - currentCueAngle;
        if (deltaAngle > 180.f) deltaAngle -= 360.f;
        else if (deltaAngle < -180.f) deltaAngle += 360.f;

        const float aimSmoothingFactor = 0.15f;
        currentCueAngle += deltaAngle * aimSmoothingFactor;

        float smoothedAngleRad = currentCueAngle * M_PI / 180.f;
        sf::Vector2f dir(std::cos(smoothedAngleRad), std::sin(smoothedAngleRad));

        // Aim line
        sf::VertexArray aim(sf::LinesStrip, 30);
        for (int i = 0; i < 30; ++i) {
            sf::Vector2f p = cueBallPos + dir * (i * 25.f);
            sf::Color c(255, 255, 255, 255 - i * 8);
            aim[i].position = p;
            aim[i].color = c;
        }
        window.draw(aim);

        // Cue stick
        sf::RectangleShape stick(sf::Vector2f(160.f, 6.f));
        stick.setFillColor(sf::Color(139, 69, 19));
        stick.setOrigin(0.f, stick.getSize().y / 2.f);
        stick.setRotation(currentCueAngle + 180.f);

        if (isDragging) {
            sf::Vector2f pullVec = dragStartPos - mousePos;
            float pullLen = std::sqrt(pullVec.x * pullVec.x + pullVec.y * pullVec.y);
            float maxPull = 100.f;
            float pull = std::min(pullLen, maxPull);
            stick.setPosition(cueBallPos - dir * pull);
        } else {
            stick.setPosition(cueBallPos);
        }

        window.draw(stick);
    }

    // status string
    std::string statusStr;
    if (currentGameState == GAME_OVER) {
        statusStr = "GAME OVER! Player " + std::to_string(currentPlayer) + " MENANG!";
    } else if (currentGameState == BALL_IN_HAND) {
        statusStr = "Player " + std::to_string(currentPlayer) + ": FOUL! BALL IN HAND. Klik untuk menaruh bola.";

        BallGroup currentGroup = playerGroup[currentPlayer];

        if (cueBallPocketed) {
            statusStr += "\n(Foul: Bola Putih Masuk Lubang / Scratch)";
        } else if (!firstBallHitThisShot) {
            statusStr += "\n(Foul: Tidak Mengenai Bola Apapun)";
        } else if (currentGroup != NONE && getBallGroup(firstObjectBallHit) != currentGroup && getBallGroup(firstObjectBallHit) != EIGHT_BALL) {
            statusStr += "\n(Foul: Mengenai Bola Lawan Terlebih Dahulu)";
        } else if (pocketedObjBallsThisTurn.empty() && !ballHitRailAfterContact) {
            statusStr += "\n(Foul: Tidak Ada Bola Masuk & Tidak Ada Bola Sentuh Rail)";
        }
    } else {
        std::string stateName;
        if (currentGameState == BREAK) stateName = "BREAK";
        else if (currentGameState == OPEN_TABLE) stateName = "OPEN TABLE";
        else if (currentGameState == SOLID_TURN) stateName = "SOLID TURN";
        else if (currentGameState == STRIPES_TURN) stateName = "STRIPES TURN";

        statusStr = "PLAYER " + std::to_string(currentPlayer) + " (" + stateName + ")";
        if (playerGroup[1] != NONE) {
            statusStr += "\nP1: " + getGroupStr(playerGroup[1]) + " | P2: " + getGroupStr(playerGroup[2]);
        }
    }

    statusText.setString(statusStr);
    window.draw(statusText);

    // visual player indicators
    sf::Color p1GroupColor = sf::Color(100, 100, 100);
    if (playerGroup[1] == SOLID) p1GroupColor = sf::Color(255, 165, 0);
    else if (playerGroup[1] == STRIPES) p1GroupColor = sf::Color::Blue;

    sf::Color p2GroupColor = sf::Color(100, 100, 100);
    if (playerGroup[2] == SOLID) p2GroupColor = sf::Color(255, 165, 0);
    else if (playerGroup[2] == STRIPES) p2GroupColor = sf::Color::Blue;

    p1Status.setFillColor((currentPlayer == 1) ? p1GroupColor : sf::Color::Black);
    p2Status.setFillColor((currentPlayer == 2) ? p2GroupColor : sf::Color::Black);

    p1Text.setFillColor((currentPlayer == 1) ? sf::Color::Black : sf::Color::White);
    p2Text.setFillColor((currentPlayer == 2) ? sf::Color::Black : sf::Color::White);

    p1Text.setString("P1: " + getGroupStr(playerGroup[1]));
    p2Text.setString("P2: " + getGroupStr(playerGroup[2]));

    window.draw(p1Status);
    window.draw(p1Text);
    window.draw(p2Status);
    window.draw(p2Text);

    window.display();
}

// Run loop (replaces main logic)
void BilliardsGame::run() {
    sf::Clock clock;

    while (window.isOpen()) {
        processEvents();

        float dt = clock.restart().asSeconds();
        update(dt);

        render();
    }
}
