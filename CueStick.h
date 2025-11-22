#ifndef CUESTICK_H
#define CUESTICK_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

class CueStick {
public:
    CueStick(float length = 160.f, float thickness = 6.f);

    // Update logika visual stick dan aim line
    void update(sf::Vector2f cueBallPos, sf::Vector2f mousePos, bool allowAim);
    
    // Logika drag power
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
    sf::VertexArray aimLine;      

    // Logic properties
    bool isDragging;
    bool isVisible;
    float stickLength;
    float maxPullDistance;
    
    // Angle smoothing
    float currentCueAngle; 

    // Dragging state
    sf::Vector2f dragStartPos;
    sf::Vector2f dragCurrentPos;
    sf::Vector2f calculatedDir; 
};

#endif // CUESTICK_H