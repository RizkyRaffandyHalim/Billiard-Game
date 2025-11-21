#define _USE_MATH_DEFINES
#include <cmath>
#include "CueStick.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CueStick::CueStick(float length, float thickness, sf::Color color)
    : stickLength(length), stickThickness(thickness), isDragging(false), maxPullDistance(100.f),
      cueLine(sf::Lines, 2)
{
    // Garis pembidik
    stick.setSize(sf::Vector2f(stickLength, stickThickness));
    stick.setOrigin(stickLength / 2.f, stickThickness / 2.f);
    stick.setFillColor(sf::Color::White);

    // Tongkat pemukul
    cueLine[0].color = sf::Color(139, 69, 19);
    cueLine[1].color = sf::Color(139, 69, 19);
}

void CueStick::startDrag(sf::Vector2f mousePos) {
    isDragging = true;
    dragStartPos = mousePos;
    dragCurrentPos = mousePos;
}

void CueStick::updateDrag(sf::Vector2f mousePos) {
    if (isDragging)
        dragCurrentPos = mousePos;
}

void CueStick::endDrag() {
    isDragging = false;
}

void CueStick::reset() {
    isDragging = false;
}

void CueStick::updateAimLine(sf::Vector2f cueBallPos, sf::Vector2f mousePos) {
    // Garis pembidik putih mengikuti arah mouse
    sf::Vector2f direction = mousePos - cueBallPos;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length != 0) direction /= length;

    stick.setPosition(cueBallPos + direction * (stickLength / 2.f));
    stick.setRotation(std::atan2(direction.y, direction.x) * 180.f / static_cast<float>(M_PI));
}

void CueStick::updateStrikingStick(sf::Vector2f cueBallPos, sf::Vector2f mousePos) {
    // Tongkat coklat
    sf::Vector2f direction = mousePos - cueBallPos;
    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len != 0) direction /= len;

    float pull = 0.f;
    if (isDragging) {
        sf::Vector2f dragVec = dragCurrentPos - dragStartPos;
        pull = std::min<float>(maxPullDistance, std::sqrt(dragVec.x * dragVec.x + dragVec.y * dragVec.y));
    }

    sf::Vector2f start = cueBallPos - direction * (stickLength / 2.f + pull);
    sf::Vector2f end   = cueBallPos - direction * (stickLength / 2.f);
    cueLine[0].position = start;
    cueLine[1].position = end;
}

void CueStick::draw(sf::RenderWindow& window) {
    window.draw(stick);

    if (isDragging)
        window.draw(cueLine);
}
