#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include "Ball.h"
#include "Table.h"
#include "Hole.h"
#include "CueStick.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ===============================================
// 🔹 ENUM ATURAN BILLIARD
// ===============================================

enum GameState {
    BREAK,          // Pukulan pembuka
    OPEN_TABLE,     // Meja terbuka, kelompok belum ditentukan
    SOLID_TURN,     // Giliran pemain Solid
    STRIPES_TURN,   // Giliran pemain Stripes
    BALL_IN_HAND,   // Giliran pemain berikutnya setelah Foul, menempatkan bola putih
    GAME_OVER
};

enum BallGroup {
    NONE,
    SOLID,      // 1-7
    STRIPES,    // 9-15
    EIGHT_BALL, // 8
    CUE_BALL    // 0
};

BallGroup getBallGroup(int ballNumber) {
    if (ballNumber == 0) return CUE_BALL;
    if (ballNumber == 8) return EIGHT_BALL;
    if (ballNumber >= 1 && ballNumber <= 7) return SOLID;
    if (ballNumber >= 9 && ballNumber <= 15) return STRIPES;
    return NONE;
}

std::string getGroupStr(BallGroup group) {
    if (group == SOLID) return "SOLID";
    if (group == STRIPES) return "STRIPES";
    if (group == EIGHT_BALL) return "8-BALL";
    return "NONE";
}

// 🔹 VARIABEL GLOBAL UNTUK MELACAK ATURAN BARU (Dari permintaan sebelumnya)
bool firstBallHitThisShot = false;
bool ballHitRailAfterContact = false;

// 🔹 VARIABEL GLOBAL BARU UNTUK MELACAK BOLA PERTAMA YANG DITABRAK
int firstObjectBallHit = -1; // -1: Belum ada yang ditabrak, 0: Cue Ball, 1-15: Bola Objek

// ===============================================
//  Fungsi: Tumbukan Antar Bola
// ===============================================
void handleBallCollision(Ball &a, Ball &b) {
    if (a.isPocketed || b.isPocketed) return;

    sf::Vector2f diff = b.shape.getPosition() - a.shape.getPosition();
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    float radiusSum = a.shape.getRadius() + b.shape.getRadius();

    if (dist < radiusSum && dist > 0.f) {
        // 🔹 LOGIKA TAMBAHAN: firstBallHitThisShot
        if (a.ballNumber == 0 || b.ballNumber == 0) { // Cek jika salah satu adalah bola putih
            firstBallHitThisShot = true;
            
            // 🔹 LOGIKA TAMBAHAN: firstObjectBallHit
            // Cek apakah bola putih (a=0 atau b=0) menabrak bola bernomor (>0)
            if (firstObjectBallHit == -1) { // Hanya catat bola pertama
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

// ===============================================
//  Cek apakah semua bola sudah berhenti
// ===============================================
bool allBallsStopped(const std::vector<Ball>& balls) {
    for (const auto& b : balls)
        if (!b.isPocketed && (std::abs(b.velocity.x) > 0.1f || std::abs(b.velocity.y) > 0.1f))
            return false;
    return true;
}

// ===============================================
//  Susunan Bola Awal (Rapi dan Simetris Tengah Kanan)
// ===============================================
std::vector<Ball> createRack(float tableLeft, float tableTop, float tableRight, float tableBottom) {
    std::vector<Ball> balls;
    float radius = 12.f;

    float centerX = (tableLeft + tableRight) / 2.f;
    float centerY = (tableTop + tableBottom) / 2.f;

    // Cue Ball (putih) - index 0
    sf::Vector2f cuePos(centerX - 250.f, centerY);
    balls.emplace_back(radius, cuePos, sf::Color::White, 0, false);

    // Titik tengah rak segitiga
    sf::Vector2f apex(centerX + 200.f, centerY);
    float spacing = radius * 2.f * 0.87f;

    // Nomor bola yang akan di-rack (sesuai aturan 8-Ball)
    std::vector<int> ballOrder = {
        1,       
        9, 8,    // Bola 8 di tengah
        2, 10, 3,  
        11, 4, 12, 5, 
        13, 6, 14, 7, 15 
    }; 
    
    int ballIndex = 0;

    // Bentuk segitiga rapi & simetris
    for (int row = 0; row < 5; ++row) {
        float rowX = apex.x + row * spacing;
        float yOffset = (row * spacing) / 2.f;
        for (int col = 0; col <= row; ++col) {
            int ballNumber = ballOrder[ballIndex++];
            sf::Color ballColor;

            if (ballNumber == 0) continue; 

            // Penentuan Warna Sesuai Nomor Bola 8
            if (ballNumber == 8) ballColor = sf::Color::Black;
            else if (ballNumber >= 1 && ballNumber <= 7) ballColor = sf::Color(255, 165, 0); // Solid Orange/Yellow
            else if (ballNumber >= 9 && ballNumber <= 15) ballColor = sf::Color::Blue; // Stripes Blue/Red

            float y = apex.y - yOffset + col * spacing;
            balls.emplace_back(radius, sf::Vector2f(rowX, y), ballColor, ballNumber, false);
        }
    }

    return balls;
}


// ===============================================
//  MAIN PROGRAM
// ===============================================
int main() {
    int windowWidth = 1080;
    int windowHeight = 600;
    float borderThickness = 50.f;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Simulation Billiard Game");
    window.setFramerateLimit(60);

    Table table(windowWidth, windowHeight, borderThickness);
    float tableLeft = borderThickness, tableTop = borderThickness;
    float tableRight = windowWidth - borderThickness, tableBottom = windowHeight - borderThickness;

    std::vector<Ball> balls = createRack(tableLeft, tableTop, tableRight, tableBottom);

    float holeRadius = 28.f;
    std::vector<Hole> holes = {
        {tableLeft, tableTop, holeRadius}, {tableRight, tableTop, holeRadius},
        {tableLeft, tableBottom, holeRadius}, {tableRight, tableBottom, holeRadius},
        {(tableLeft + tableRight) / 2.f, tableTop, holeRadius * 0.8f},
        {(tableLeft + tableRight) / 2.f, tableBottom, holeRadius * 0.8f}
    };

    CueStick cue; // Tidak digunakan secara penuh, tapi variabelnya tetap dipertahankan
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error loading font." << std::endl;
        return -1;
    }

    sf::Clock clock;
    bool isDragging = false;
    sf::Vector2f dragStartPos; 
    float currentCueAngle = 0.f; 

    // 🔹 Variabel State Permainan
    GameState currentGameState = BREAK;
    int currentPlayer = 1; 
    BallGroup playerGroup[3] = {NONE, NONE, NONE}; 
    bool shotMade = false; 
    
    // Variabel yang diperiksa setelah bola berhenti
    bool cueBallPocketed = false;
    std::vector<int> pocketedObjBallsThisTurn; 
    bool isFoul = false;
    
    // 🔹 INISIALISASI VARIABEL ATURAN BARU
    firstBallHitThisShot = false;
    ballHitRailAfterContact = false;
    firstObjectBallHit = -1; 

    // Teks Status Utama
    sf::Text statusText("", font, 24);
    statusText.setFillColor(sf::Color::White);
    statusText.setPosition(10.f, 10.f);

    // 🔹 VISUAL STATUS PLAYER (P1) - Ukuran Disesuaikan Lebih Pas
    sf::RectangleShape p1Status(sf::Vector2f(150.f, 30.f)); // Lebar diperbesar lagi menjadi 150f
    p1Status.setPosition(50.f, windowHeight - 40.f); 
    p1Status.setOutlineThickness(2.f);
    p1Status.setOutlineColor(sf::Color::White);
    sf::Text p1Text("P1", font, 18);
    // Posisi teks diatur dengan padding
    p1Text.setPosition(p1Status.getPosition().x + 5.f, p1Status.getPosition().y + 3.f); 

    // 🔹 VISUAL STATUS PLAYER (P2) - Ukuran Disesuaikan Lebih Pas
    sf::RectangleShape p2Status(sf::Vector2f(150.f, 30.f)); // Lebar diperbesar lagi menjadi 150f
    // Posisi X disesuaikan agar tetap di kanan (windowWidth - lebar - padding)
    p2Status.setPosition(windowWidth - 200.f, windowHeight - 40.f); 
    p2Status.setOutlineThickness(2.f);
    p2Status.setOutlineColor(sf::Color::White);
    sf::Text p2Text("P2", font, 18);
    // Posisi teks diatur dengan padding
    p2Text.setPosition(p2Status.getPosition().x + 5.f, p2Status.getPosition().y + 3.f);

    while (window.isOpen()) {
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
                // Reset variabel aturan baru
                firstBallHitThisShot = false;
                ballHitRailAfterContact = false;
                firstObjectBallHit = -1;
            }

            // ==========================================================
            // LOGIKA BALL IN HAND (Penempatan Bola Putih)
            // ==========================================================
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
                    // Logika Drag Start
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
                    
                    // Arah tembakan adalah BERLAWANAN dengan arah drag (CueBallPos - DragEndPos)
                    sf::Vector2f shootDir = mp - balls[0].shape.getPosition();
                    float len = std::sqrt(shootDir.x * shootDir.x + shootDir.y * shootDir.y);
                    
                    float maxPull = 100.f;
                    // Power disesuaikan agar sesuai dengan visual drag (pull 100 -> power 2000)
                    float power = std::min(len, maxPull) * 20.0f; 

                    if (len != 0) {
                        shootDir /= len; 
                        balls[0].velocity = shootDir * power; 
                        shotMade = true;
                        
                        cueBallPocketed = false;
                        pocketedObjBallsThisTurn.clear();
                        isFoul = false;
                        
                        // 🔹 RESET VARIABEL ATURAN BARU SEBELUM SHOT
                        firstBallHitThisShot = false;
                        ballHitRailAfterContact = false;
                        firstObjectBallHit = -1; // RESET ATURAN BARU
                    }
                    
                    isDragging = false;
                }
            }
        }

        float dt = clock.restart().asSeconds();
        bool canAim = allBallsStopped(balls);
        
        // 🔹 LOGIKA TAMBAHAN: Cek batas meja (Rail Hit) untuk setiap bola yang bergerak
        for (auto &b : balls) {
            sf::Vector2f pos = b.shape.getPosition();
            float r = b.shape.getRadius();
            if (!b.isPocketed && (std::abs(b.velocity.x) > 0.1f || std::abs(b.velocity.y) > 0.1f)) {
                 if (pos.x - r <= tableLeft || pos.x + r >= tableRight || 
                    pos.y - r <= tableTop || pos.y + r >= tableBottom) {
                    // Ini adalah hit bantalan (rail hit)
                    if (firstBallHitThisShot) { // Hanya cek rail hit setelah kontak pertama
                        ballHitRailAfterContact = true;
                    }
                 }
            }
            // Panggil update seperti biasa:
            b.update(dt, window, tableLeft, tableTop, tableRight, tableBottom);
        }

        // ==========================================================
        // UPDATE DAN DETEKSI POCKETING
        // ==========================================================
        // Cek benturan antar bola. firstBallHitThisShot dan firstObjectBallHit di-set di handleBallCollision.
        for (size_t i = 0; i < balls.size(); ++i) {
            for (size_t j = i + 1; j < balls.size(); ++j)
                handleBallCollision(balls[i], balls[j]);
        }

        // Cek pocketing dan kumpulkan bola yang masuk
        for (auto &h : holes) {
            for (size_t i = 0; i < balls.size(); ++i) {
                if (!balls[i].isPocketed && h.checkBallInHole(balls[i])) {
                    if (balls[i].ballNumber == 0) { // Cue Ball (Scratch)
                        cueBallPocketed = true;
                    } else { // Bola Objek (termasuk 8)
                        pocketedObjBallsThisTurn.push_back(balls[i].ballNumber);
                    }
                }
            }
        }
        
        // ==========================================================
        // LOGIKA PENYELESAIAN GILIRAN (Setelah Bola Berhenti)
        // ==========================================================
        if (shotMade && canAim) {
            shotMade = false;
            
            BallGroup currentGroup = playerGroup[currentPlayer];
            bool pocketedMyBall = false;
            bool pocketed8Ball = false;
            
            // 1. Cek Foul (Scratch)
            if (cueBallPocketed) {
                isFoul = true;
            }
            
            // 2. Cek Foul Aturan: Tidak Mengenai Bola Apapun
            if (!isFoul && !firstBallHitThisShot) {
                isFoul = true;
            }
            
            // 🔹 3. Cek Foul Aturan: Mengenai Bola Lawan Terlebih Dahulu
            // Hanya berlaku jika kelompok sudah ditentukan (bukan BREAK atau OPEN_TABLE)
            if (!isFoul && currentGroup != NONE) {
                BallGroup firstHitGroup = getBallGroup(firstObjectBallHit);
                // Cek apakah bola yang ditabrak pertama BUKAN kelompok sendiri DAN BUKAN bola 8
                if (firstHitGroup != currentGroup && firstHitGroup != EIGHT_BALL) {
                    isFoul = true;
                }
            }
            
            // 4. Cek Foul Aturan: Tidak Sah Setelah Kontak Awal
            // (Tidak ada bola masuk ATAU tidak ada bola sentuh rail setelah kontak awal)
            if (!isFoul && pocketedObjBallsThisTurn.empty() && !ballHitRailAfterContact) {
                 isFoul = true;
            }
            
            // 5. Cek Bola 8 Masuk
            for (int number : pocketedObjBallsThisTurn) {
                if (number == 8) {
                    pocketed8Ball = true;
                    break;
                }
            }
            
            // 6. Cek Menang/Kalah (Bola 8)
            if (pocketed8Ball) {
                bool allMyBallsPocketed = (currentGroup != NONE && std::count_if(balls.begin(), balls.end(), 
                    [currentGroup](const Ball& b){ return getBallGroup(b.ballNumber) == currentGroup && !b.isPocketed; }) == 0);

                if (allMyBallsPocketed && !isFoul) { 
                    currentGameState = GAME_OVER; // MENANG SAH
                } else {
                    currentGameState = GAME_OVER;
                    currentPlayer = (currentPlayer == 1) ? 2 : 1; // KALAH, lawan menang
                }
            } 
            
            // 7. Tentukan Kelompok (Open Table)
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
            
            // 8. Cek Bola Sendiri Masuk (Setelah Kelompok Ditentukan)
            if (currentGameState == SOLID_TURN || currentGameState == STRIPES_TURN) {
                for (int number : pocketedObjBallsThisTurn) {
                    if (getBallGroup(number) == currentGroup) {
                        pocketedMyBall = true;
                        break;
                    }
                }
            }


            // 9. Penentuan Giliran Berikutnya
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
                    // Melanjutkan giliran jika bola sendiri masuk secara sah
                }
            }


            // 10. Reset Posisi Bola Putih Jika Scratch
            if (cueBallPocketed) {
                balls[0].velocity = sf::Vector2f(0.f, 0.f);
                balls[0].isPocketed = false; 
            }
            
            // 🔹 RESET SEMUA VARIABEL ATURAN BARU setelah giliran berakhir
            firstBallHitThisShot = false;
            ballHitRailAfterContact = false;
            firstObjectBallHit = -1;
        }
        
        // Aturan: Jika bola putih masuk lubang pada giliran normal, bola putih diletakkan di area head string (tengah kiri)
        if (balls[0].isPocketed && (currentGameState != GAME_OVER)) {
            float centerX = (tableLeft + tableRight) / 2.f;
            float centerY = (tableTop + tableBottom) / 2.f;
            balls[0].shape.setPosition(centerX - 100.f, centerY); 
            balls[0].velocity = sf::Vector2f(0.f, 0.f);
            balls[0].isPocketed = false; 
        }


        // ==============================
        //  RENDER
        // ==============================
        window.clear(sf::Color(50, 50, 50));
        table.draw(window);
        for (auto &h : holes) h.draw(window);
        for (auto &b : balls) b.draw(window, font);

        // ==========================================================
        // LOGIKA BIDIK (Aiming)
        // ==========================================================
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

            // Garis bidik
            sf::VertexArray aim(sf::LinesStrip, 30);
            for (int i = 0; i < 30; ++i) {
                sf::Vector2f p = cueBallPos + dir * (i * 25.f); 
                sf::Color c(255, 255, 255, 255 - i * 8);
                aim[i].position = p;
                aim[i].color = c;
            }
            window.draw(aim);

            // Tongkat coklat
            sf::RectangleShape stick(sf::Vector2f(160.f, 6.f));
            stick.setFillColor(sf::Color(139, 69, 19));
            
            // Set Origin di UJUNG TONGKAT (titik kontak x=0)
            stick.setOrigin(0.f, stick.getSize().y / 2.f); 
            
            // Rotasi dibalik 180 derajat (menunjuk ke belakang bola)
            stick.setRotation(currentCueAngle + 180.f); 

            if (isDragging) {
                // Hitung jarak tarikan mouse
                sf::Vector2f pullVec = dragStartPos - mousePos;
                float pullLen = std::sqrt(pullVec.x * pullVec.x + pullVec.y * pullVec.y);
                float maxPull = 100.f; 
                float pull = std::min(pullLen, maxPull);

                // Posisi tongkat digeser MUNDUR
                stick.setPosition(cueBallPos - dir * pull);
                
            } else {
                // Posisi default: ujung tongkat (origin 0.f) berada tepat di cueBallPos
                stick.setPosition(cueBallPos);
            }

            window.draw(stick);
        }

        // ==========================================================
        // TAMPILAN STATUS GAME & VISUAL GILIRAN
        // ==========================================================
        std::string statusStr;
        if (currentGameState == GAME_OVER) {
            statusStr = "GAME OVER! Player " + std::to_string(currentPlayer) + " MENANG!";
        } else if (currentGameState == BALL_IN_HAND) {
            statusStr = "Player " + std::to_string(currentPlayer) + ": FOUL! BALL IN HAND. Klik untuk menaruh bola.";
            
            // START MODIFIKASI: Keterangan Foul yang lebih akurat
            BallGroup currentGroup = playerGroup[currentPlayer]; 
            
            // Urutan prioritas untuk menampilkan penyebab FOUL:
            
            // 1. Scratch (Foul Paling Fatal)
            if (cueBallPocketed) {
                statusStr += "\n(Foul: Bola Putih Masuk Lubang / Scratch)";
            } 
            // 2. Tidak Mengenai Bola Apapun
            else if (!firstBallHitThisShot) {
                statusStr += "\n(Foul: Tidak Mengenai Bola Apapun)";
            } 
            // 3. Mengenai Bola Lawan Terlebih Dahulu (Hanya dicek jika kontak terjadi & kelompok sudah ditentukan)
            else if (currentGroup != NONE && getBallGroup(firstObjectBallHit) != currentGroup && getBallGroup(firstObjectBallHit) != EIGHT_BALL) {
                 statusStr += "\n(Foul: Mengenai Bola Lawan Terlebih Dahulu)";
            } 
            // 4. Safety Rule Violation (Tidak Ada Bola Masuk & Tidak Ada Sentuhan Rail)
            // KOREKSI LOGIKA PENTING: Kondisi ini sekarang benar-benar menguji kegagalan aturan safety.
            else if (pocketedObjBallsThisTurn.empty() && !ballHitRailAfterContact) {
                 statusStr += "\n(Foul: Tidak Ada Bola Masuk & Tidak Ada Bola Sentuh Rail)";
            }
            // END MODIFIKASI: Keterangan Foul yang lebih akurat

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


        // Render Visual Giliran
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

    return 0;
}