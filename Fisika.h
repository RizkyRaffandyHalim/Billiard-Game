#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

class Fisika {
public:
    sf::Vector2f velocity;
    float mass;

    Fisika() : velocity(0.f, 0.f), mass(1.f) {}

    // Helper untuk gesekan
    void applyFriction() {
        velocity *= 0.99f;
        if (std::abs(velocity.x) < 0.05f) velocity.x = 0;
        if (std::abs(velocity.y) < 0.05f) velocity.y = 0;
    }
};