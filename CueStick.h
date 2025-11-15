#ifndef CUESTICK_H
#define CUESTICK_H

#include <SFML/Graphics.hpp>
#include <cmath>

class CueStick {
public:
    CueStick(float length = 180.f, float thickness = 6.f, sf::Color color = sf::Color::White);

    void startDrag(sf::Vector2f mousePos);
    void updateDrag(sf::Vector2f mousePos);
    void endDrag();

    void updateAimLine(sf::Vector2f cueBallPos, sf::Vector2f mousePos);
    void updateStrikingStick(sf::Vector2f cueBallPos, sf::Vector2f mousePos);
    void draw(sf::RenderWindow& window);
    void reset();

    // 🔹 Buka akses supaya bisa dibaca dari main.cpp
    sf::Vector2f dragStartPos;
    sf::Vector2f dragCurrentPos;

private:
    sf::RectangleShape stick;     // 🔹 Sekarang ini garis putih pembidik (selalu muncul)
    sf::VertexArray cueLine;      // 🔹 Ini tongkat coklat, hanya muncul saat didrag
    bool isDragging;
    float stickLength;
    float stickThickness;
    float maxPullDistance;
};

#endif // CUESTICK_H
