#pragma once


#include "Body.hpp"
#include "Constants.hpp"


#include <raylib.h>


enum FoodType {
    PLANT,
    MEAT
};

class Food: public Body {
protected:
    float calories;
    bool consumed;
public:
    friend std::ostream &operator<<(std::ostream &stream, const Food* food) {
        stream << food->id;
        stream << "\n";
        stream << food->radius;
        stream << "\n";
        stream << food->position.x;
        stream << "\n";
        stream << food->position.y;
        stream << "\n";
        stream << food->calories;
        stream << "\n";
        stream << food->consumed;
        stream << "\n";
        return stream;
    }
    friend std::istream &operator>>(std::istream &stream, Food* food) {
        stream >> food->id;
        stream >> food->radius;
        stream >> food->position.x;
        stream >> food->position.y;
        stream >> food->calories;
        stream >> food->consumed;
        return stream;
    }
    [[nodiscard]] virtual FoodType get_food_type() const {
        return PLANT;
    }

    [[nodiscard]] float get_calories() const {
        return this->calories;
    }

    [[nodiscard]] float take_calories() {
        const float _calories = this->calories;
        this->calories = 0;
        return _calories;
    }

    [[nodiscard]] bool is_consumed() const {
        return this->consumed;
    }

    [[nodiscard]] bool is_too_far() const {
        const float distance = std::sqrt((this->position.x * this->position.x) + (this->position.y * this->position.y));
        return distance > POSITION_DISTANCE * 2.5f;
    }

    void add_calories(const float _calories) {
        this->calories += _calories;
    }

    void consume() {
        this->consumed = true;
    }
    void set_position(const Vector2 _position) {
        this->position.x = _position.x;
        this->position.y = _position.y;
    }

    virtual float get_red() = 0;
    virtual float get_green() = 0;
    virtual float get_blue() = 0;
};

class Meat: public Food {
public:
    explicit Meat(std::istream &stream) {
        stream >> this;
    }
    Meat(const float _calories, const Vector2 _position) {
        this->id = get_new_id();
        this->id = Body::id_count;
        this->calories = _calories;
        this->radius = std::sqrt(this->calories / 4.0f);
        this->position.x = _position.x;
        this->position.y = _position.y;
        this->consumed = false;
        this->wrap_position();
    }

    [[nodiscard]] FoodType get_food_type() const override {
        return MEAT;
    }

    Color get_color() override {
        return RED;
    }
    [[nodiscard]] float get_red() override {
        return 255.0f;
    }
    [[nodiscard]] float get_blue() override {
        return 0.0f;
    }
    [[nodiscard]] float get_green() override {
        return 0.0f;
    }
};

class Plant: public Food {
public:
//    Plant() {}
    explicit Plant(std::istream &stream) {
        stream >> this;
    }
    Plant(const float _calories, const Vector2 _position) {
        this->id = get_new_id();
        this->calories = _calories;
        this->radius = std::sqrt(this->calories / 4.0f);
        this->position.x = _position.x;
        this->position.y = _position.y;
        this->consumed = false;
        this->wrap_position();
    }

    [[nodiscard]] FoodType get_food_type() const override {
        return PLANT;
    }

    Color get_color() override {
        return GREEN;
    }

    [[nodiscard]] float get_red() override {
        return 0.0f;
    }
    [[nodiscard]] float get_blue() override {
        return 0.0f;
    }
    [[nodiscard]] float get_green() override {
        return 255.0f;
    }
};