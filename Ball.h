#ifndef BALL_H
#define BALL_H
#include <SFML/Graphics.hpp>
#include <string>
#include "GameObject.h"
#include "Fisika.h"

// DEFINISI ENUM
enum BallGroup {
    NONE,
    SOLID,
    STRIPES,
    EIGHT_BALL,
    CUE_BALL
};

class Ball : public GameObject {
public:
    sf::CircleShape shape;
    Fisika physics; 

    int ballNumber; 
    bool isPocketed;
    sf::CircleShape shading;

    Ball(float radius, sf::Vector2f pos, sf::Color color, int ballNumber, bool isStripe = false); 
    
    virtual void resetPosition(sf::Vector2f pos);

    void update(float dt, float tableLeft, float tableTop, float tableRight, float tableBottom) override;
    
    virtual void draw(sf::RenderWindow& window, sf::Font& font, sf::Texture* texture = nullptr);
    
    void draw(sf::RenderWindow& window, sf::Font& font) override { draw(window, font, nullptr); }
    bool isMoving() const { return std::abs(physics.velocity.x) > 0.1f || std::abs(physics.velocity.y) > 0.1f; }

    static bool resolveCollision(Ball& a, Ball& b, bool& firstBallHitFlag, int& firstObjectHitVal);
};

#endif // BALL_H