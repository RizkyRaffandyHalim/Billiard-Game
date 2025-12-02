#ifndef CUESTICK_H
#define CUESTICK_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include "Ball.h"

class CueStick {
public:
    CueStick(float length = 346.f, float thickness = 6.f);

    void update(sf::Vector2f cueBallPos, sf::Vector2f mousePos, bool allowAim, const std::vector<Ball>& balls);
    
    void startDrag(sf::Vector2f mousePos);
    void updateDrag(sf::Vector2f mousePos);
    void endDrag();
    
    void draw(sf::RenderWindow& window);
    void reset();

    bool getIsDragging() const { return isDragging; }
    float getCurrentAngle() const { return currentCueAngle; }
    sf::Vector2f getShootDirection() const;
    float getPower() const;

private:
    // Visuals
    sf::RectangleShape stickShape;
    
    // ASSETS
    sf::Texture stickTexture;
    sf::Sprite stickSprite;
    bool useTexture;

    sf::VertexArray aimLine;      
    
    // Visual Prediksi
    sf::CircleShape ghostBall;
    sf::VertexArray targetAimLine;
    bool showGhostBall;

    // Logic properties
    bool isDragging;
    bool isVisible;
    float stickLength;
    float stickThickness;
    float maxPullDistance;
    
    // Angle smoothing
    float currentCueAngle; 

    // Dragging state
    sf::Vector2f dragStartPos;
    sf::Vector2f dragCurrentPos;
    sf::Vector2f calculatedDir; 
};

#endif // CUESTICK_H