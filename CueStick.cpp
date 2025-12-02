#define _USE_MATH_DEFINES
#include <cmath>
#include "CueStick.h"
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CueStick::CueStick(float length, float thickness)
    : stickLength(length), stickThickness(thickness), isDragging(false), isVisible(true), maxPullDistance(100.f),
      currentCueAngle(0.f), aimLine(sf::LinesStrip, 2), useTexture(false),
      targetAimLine(sf::LinesStrip, 2), showGhostBall(false)
{
    // 1. Setup Fallback Shape
    stickShape.setSize(sf::Vector2f(stickLength, thickness));
    stickShape.setFillColor(sf::Color(139, 69, 19));
    stickShape.setOrigin(0.f, thickness / 2.f);

    // 2. Setup Ghost Ball Visuals
    ghostBall.setRadius(12.f);
    ghostBall.setOrigin(12.f, 12.f);
    ghostBall.setFillColor(sf::Color::Transparent);
    ghostBall.setOutlineThickness(1.5f);
    ghostBall.setOutlineColor(sf::Color::White);

    // 3. Load Texture dari Assets
    if (stickTexture.loadFromFile("assets/Stick.png")) {
        useTexture = true;
        stickTexture.setSmooth(true);
        stickSprite.setTexture(stickTexture);

        sf::Vector2u size = stickTexture.getSize();
        float scaleX = stickLength / (float)size.x;
        float scaleY = (thickness * 9.0f) / (float)size.y;
        stickSprite.setScale(scaleX, scaleY);
        stickSprite.setOrigin(0.f, (float)size.y / 2.f);
    } else {
        std::cerr << "Warning: assets/Stick.png not found. Using default shape." << std::endl;
    }
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

// Fungsi Utama
void CueStick::update(sf::Vector2f cueBallPos, sf::Vector2f mousePos, bool allowAim, const std::vector<Ball>& balls) {
    isVisible = allowAim;
    if (!isVisible) return;

    // 1. Kalkulasi Sudut
    sf::Vector2f targetDir = mousePos - cueBallPos;
    float len = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
    if (len != 0) targetDir /= len;
    float targetAngle = std::atan2(targetDir.y, targetDir.x) * 180.f / static_cast<float>(M_PI);

    float deltaAngle = targetAngle - currentCueAngle;
    if (deltaAngle > 180.f) deltaAngle -= 360.f;
    else if (deltaAngle < -180.f) deltaAngle += 360.f;

    const float aimSmoothingFactor = 0.15f;
    currentCueAngle += deltaAngle * aimSmoothingFactor;

    float smoothedAngleRad = currentCueAngle * static_cast<float>(M_PI) / 180.f;
    calculatedDir = sf::Vector2f(std::cos(smoothedAngleRad), std::sin(smoothedAngleRad));

    // 2. Update Posisi Stick Visual
    float rotation = currentCueAngle + 180.f;
    if (useTexture) stickSprite.setRotation(rotation);
    stickShape.setRotation(rotation);

    sf::Vector2f pos;
    if (isDragging) {
        sf::Vector2f pullVec = dragStartPos - mousePos;
        float pullLen = std::sqrt(pullVec.x * pullVec.x + pullVec.y * pullVec.y);
        float pull = std::min(pullLen, maxPullDistance);
        pos = cueBallPos - calculatedDir * pull;
    } else {
        pos = cueBallPos;
    }

    if (useTexture) stickSprite.setPosition(pos);
    stickShape.setPosition(pos);

    // Logika Prediksi Tabrakan
    float closestDist = 1000.f;
    sf::Vector2f collisionPoint = cueBallPos + calculatedDir * 1000.f;
    const Ball* targetBall = nullptr;
    bool hitDetected = false;

    float ballRadius = 12.f;
    float diameter = ballRadius * 2.f;

    // Iterasi semua bola untuk mencari yang tertabrak
    for (const auto& ball : balls) {
        if (ball.isPocketed) continue;

        sf::Vector2f toBall = ball.shape.getPosition() - cueBallPos;
        float dot = toBall.x * calculatedDir.x + toBall.y * calculatedDir.y;
        if (dot <= 0) continue; 

        sf::Vector2f closestPointOnLine = cueBallPos + calculatedDir * dot;
        sf::Vector2f distVec = ball.shape.getPosition() - closestPointOnLine;
        float distToLineSq = distVec.x * distVec.x + distVec.y * distVec.y;

        if (distToLineSq < diameter * diameter) {
            float distBack = std::sqrt(diameter * diameter - distToLineSq);
            float distToCollision = dot - distBack;

            // Cari bola yang terdekat
            if (distToCollision < closestDist && distToCollision > 0) {
                closestDist = distToCollision;
                collisionPoint = cueBallPos + calculatedDir * closestDist;
                targetBall = &ball;
                hitDetected = true;
            }
        }
    }

    // Visual Garis Bidik
    aimLine[0].position = cueBallPos;
    aimLine[0].color = sf::Color(255, 255, 255, 150);
    aimLine[1].position = collisionPoint;
    aimLine[1].color = sf::Color(255, 255, 255, 50);

    // Visual Ghost Ball & Garis Target
    showGhostBall = hitDetected;
    if (hitDetected && targetBall) {
        ghostBall.setPosition(collisionPoint);
        
        sf::Vector2f impactDir = targetBall->shape.getPosition() - collisionPoint;
        float lenImp = std::sqrt(impactDir.x * impactDir.x + impactDir.y * impactDir.y);
        if (lenImp != 0) impactDir /= lenImp;

        sf::Vector2f lineEnd = targetBall->shape.getPosition() + impactDir * 40.f;

        targetAimLine[0].position = targetBall->shape.getPosition();
        targetAimLine[0].color = sf::Color::White;
        targetAimLine[1].position = lineEnd;
        targetAimLine[1].color = sf::Color::White;
    }
}

sf::Vector2f CueStick::getShootDirection() const {
    return calculatedDir;
}

float CueStick::getPower() const {
    if (!isDragging) return 0.f;
    sf::Vector2f diff = dragStartPos - dragCurrentPos;
    float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    return std::min(len, maxPullDistance) * 20.0f;
}

void CueStick::draw(sf::RenderWindow& window) {
    if (isVisible) {
        window.draw(aimLine);  
        
        if (showGhostBall) {
            window.draw(ghostBall);
            window.draw(targetAimLine);
        }

        if (useTexture) {
            window.draw(stickSprite);
        } else {
            window.draw(stickShape); 
        }
    }
}