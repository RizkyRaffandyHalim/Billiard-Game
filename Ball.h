#ifndef BALL_H
#define BALL_H

#include <SFML/Graphics.hpp>
#include <string>

class Ball {
public:
    sf::CircleShape shape;
    sf::Vector2f velocity;

    int ballNumber; 
    bool isPocketed;
    float mass;

    sf::CircleShape shading;

    // Parameter isStripe dipertahankan, tetapi ditandai agar tidak memicu warning unused
    Ball(float radius, sf::Vector2f pos, sf::Color color, int ballNumber, bool isStripe = false); 
    
    void resetPosition(sf::Vector2f pos);
    void update(float dt, const sf::RenderWindow& window,
                float tableLeft, float tableTop, float tableRight, float tableBottom);
    void draw(sf::RenderWindow& window, sf::Font& font);
};

#endif // BALL_H
