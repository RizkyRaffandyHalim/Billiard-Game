#define _USE_MATH_DEFINES
#include <cmath>
#include "CueStick.h"
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CueStick::CueStick(float length, float thickness)
    : stickLength(length), isDragging(false), isVisible(true), maxPullDistance(100.f),
      currentCueAngle(0.f), aimLine(sf::LinesStrip, 30)
{
    // Setup Stick
    stickShape.setSize(sf::Vector2f(stickLength, thickness));
    stickShape.setFillColor(sf::Color(139, 69, 19));
    stickShape.setOrigin(0.f, thickness / 2.f);
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
    isVisible = true;
}

void CueStick::update(sf::Vector2f cueBallPos, sf::Vector2f mousePos, bool allowAim) {
    isVisible = allowAim;
    if (!isVisible) return;

    // 1. Target Angle berdasarkan posisi mouse
    sf::Vector2f targetDir = mousePos - cueBallPos;
    float len = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
    if (len != 0) targetDir /= len;
    float targetAngle = std::atan2(targetDir.y, targetDir.x) * 180.f / static_cast<float>(M_PI);

    // 2. Smoothing Angle
    float deltaAngle = targetAngle - currentCueAngle;
    if (deltaAngle > 180.f) deltaAngle -= 360.f;
    else if (deltaAngle < -180.f) deltaAngle += 360.f;

    const float aimSmoothingFactor = 0.15f;
    currentCueAngle += deltaAngle * aimSmoothingFactor;

    float smoothedAngleRad = currentCueAngle * static_cast<float>(M_PI) / 180.f;
    calculatedDir = sf::Vector2f(std::cos(smoothedAngleRad), std::sin(smoothedAngleRad));

    // 3. Update Posisi & Rotasi Stick
    stickShape.setRotation(currentCueAngle + 180.f); 

    if (isDragging) {
        sf::Vector2f pullVec = dragStartPos - mousePos;
        float pullLen = std::sqrt(pullVec.x * pullVec.x + pullVec.y * pullVec.y);
        float pull = std::min(pullLen, maxPullDistance);
        stickShape.setPosition(cueBallPos - calculatedDir * pull);
    } else {
        stickShape.setPosition(cueBallPos);
    }

    // 4. Update Garis Bidik
    for (int i = 0; i < 30; ++i) {
        sf::Vector2f p = cueBallPos + calculatedDir * (i * 25.f);
        sf::Color c(255, 255, 255, 255 - i * 8); // Efek fading
        aimLine[i].position = p;
        aimLine[i].color = c;
    }
}

sf::Vector2f CueStick::getShootDirection() const {
    return calculatedDir;
}

float CueStick::getPower() const {
    if (!isDragging) return 0.f;
    sf::Vector2f pullVec = dragStartPos - dragCurrentPos; 
    sf::Vector2f diff = dragStartPos - dragCurrentPos;
    float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    return std::min(len, maxPullDistance) * 20.0f;
}

void CueStick::draw(sf::RenderWindow& window) {
    if (isVisible) {
        window.draw(aimLine);  
        window.draw(stickShape); 
    }
}