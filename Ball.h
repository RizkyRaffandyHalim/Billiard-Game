#ifndef BALL_H
#define BALL_H

#include <SFML/Graphics.hpp>
#include <string>
#include "GameObject.h"
#include "Fisika.h"

class Ball : public GameObject {
public:
    sf::CircleShape shape;
    Fisika physics; // Komponen Fisika (Velocity & Mass)

    int ballNumber; 
    bool isPocketed;
    sf::CircleShape shading;

    Ball(float radius, sf::Vector2f pos, sf::Color color, int ballNumber, bool isStripe = false); 
    
    virtual void resetPosition(sf::Vector2f pos);

    // Override dari GameObject
    void update(float dt, float tableLeft, float tableTop, float tableRight, float tableBottom) override;
    void draw(sf::RenderWindow& window, sf::Font& font) override;

    // Helper untuk mengecek gerakan dari komponen fisika
    bool isMoving() const { return std::abs(physics.velocity.x) > 0.1f || std::abs(physics.velocity.y) > 0.1f; }
};

#endif // BALL_H