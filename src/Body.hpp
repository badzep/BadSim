#pragma once

#include <raylib.h>
#include <cmath>
#include <random>
#include "Constants.hpp"


class Body {
protected:
    unsigned long id;
    Vector2 position;
    float radius;

public:
    static unsigned long id_count;
    void move(const Vector2 movement) {
        this->position.x += movement.x;
        this->position.y += movement.y;
        this->wrap_position();
    }
    void wrap_position() {
        if (WRAP_POSITION) {
            if (this->position.x > HALF_MAP_SIZE) {
                this->position.x -= MAP_SIZE;
            }
            else if (this->position.x < -HALF_MAP_SIZE) {
                this->position.x += MAP_SIZE;
            }
            if (this->position.y > HALF_MAP_SIZE) {
                this->position.y -= MAP_SIZE;
            }
            else if (this->position.y < -HALF_MAP_SIZE) {
                this->position.y += MAP_SIZE;
            }
        }
    }

    [[nodiscard]] Vector2 get_position() const {
        return this->position;
    }

    [[nodiscard]] float get_x_position() const {
        return this->position.x;
    }

    [[nodiscard]] float get_y_position() const {
        return this->position.y;
    }

    [[nodiscard]] unsigned long get_id() const {
        return this->id;
    }
    [[nodiscard]] float get_radius() const {
        return this->radius;
    }
    [[nodiscard]] virtual Color get_color() {
        return WHITE;
    }
    virtual void draw() {
        DrawCircle((int) std::round(this->position.x), (int) std::round(this->position.y) , this->radius, this->get_color());
    }
};

static unsigned long get_new_id() {
    Body::id_count++;
    return Body::id_count;
}
unsigned long Body::id_count;