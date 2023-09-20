#pragma once


#include <raylib.h>
#include <utility>
#include <cassert>


#include "DNA.hpp"
#include "Body.hpp"
#include "Egg.hpp"
#include "Food.hpp"
#include "Network.hpp"


typedef struct {
    bool hits;
    float hit_distance;
} RayResult;

typedef struct {
    float hit_distance;
    float hit_red;
    float hit_green;
    float hit_blue;
} Sensor;

class Stomach {
public:
    float plant_calories;
    float meat_calories;

    [[nodiscard]] float get_total_contents() const {
        return this->plant_calories + this->meat_calories;
    }
};

class Cell: public Body {
protected:
    DNA<INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT> dna;
    Network<INPUT_ACTIVATION, HIDDEN_ACTIVATION, OUTPUT_ACTIVATION, INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT> brain;
    unsigned long age;
    Vector2 velocity;
    float angle;
    float health;
    float energy;
    float base_energy;
    float waste;
    float memory1;
    float memory2;
    float memory3;
    bool want_lay_egg;
    bool want_eat;
    bool want_stab;
    Sensor sensor;
    Stomach stomach;

public:
    Cell() {

    }
    explicit Cell(Egg* egg) {
        this->id = get_new_id();
        this->dna = egg->get_dna();
        this->brain = Network<INPUT_ACTIVATION, HIDDEN_ACTIVATION, OUTPUT_ACTIVATION, INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT>(&this->dna);
        this->age = 0;
        this->waste = 0;
;       this->energy = egg->take_energy() - BASE_ENERGY;
        this->base_energy = BASE_ENERGY;
        this->health = this->dna.get_max_health();
        this->radius = this->dna.radius;
        this->position.x = egg->get_position().x;
        this->position.y = egg->get_position().y;
        this->velocity = {0.0f, 0.0f};
        this->angle = random_angle(RNG);
        this->memory1 = 0.0f;
        this->memory2 = 0.0f;
        this->memory3 = 0.0f;
        this->want_lay_egg = false;
        this->want_eat = false;
        this->want_stab = false;
        this->sensor = {};
        this->stomach = {0, 0};
        this->wrap_position();
    }

    ~Cell() = default;

    friend std::ostream &operator<<(std::ostream &stream, const Cell& cell) {
        //todo forgot brain lmao
        stream << cell.id;
        stream << "\n";
        stream << cell.radius;
        stream << "\n";
        stream << cell.position.x;
        stream << "\n";
        stream << cell.position.y;
        stream << "\n";
        stream << cell.energy;
        stream << "\n";
        stream << cell.base_energy;
        stream << "\n";
        stream << cell.age;
        stream << "\n";
        stream << cell.dna;
        stream << cell.brain;
//        stream << "\n";
        stream << cell.velocity.x;
        stream << "\n";
        stream << cell.velocity.y;
        stream << "\n";
        stream << cell.angle;
        stream << "\n";
        stream << cell.health;
        stream << "\n";
        stream << cell.energy;
        stream << "\n";
        stream << cell.waste;
        stream << "\n";
        stream << cell.memory1;
        stream << "\n";
        stream << cell.memory2;
        stream << "\n";
        stream << cell.memory3;
        stream << "\n";
        stream << cell.want_lay_egg;
        stream << "\n";
        stream << cell.want_eat;
        stream << "\n";
        stream << cell.want_stab;
        stream << "\n";
        stream << cell.sensor.hit_distance;
        stream << "\n";
        stream << cell.sensor.hit_red;
        stream << "\n";
        stream << cell.sensor.hit_green;
        stream << "\n";
        stream << cell.sensor.hit_blue;
        stream << "\n";
        stream << cell.stomach.plant_calories;
        stream << "\n";
        stream << cell.stomach.meat_calories;
        stream << "\n";
        return stream;
    }

    friend std::istream &operator>>(std::istream &stream, Cell &cell) {
        stream >> cell.id;
        stream >> cell.radius;
        stream >> cell.position.x;
        stream >> cell.position.y;
        stream >> cell.energy;
        stream >> cell.base_energy;
        stream >> cell.age;
        stream >> cell.dna;
        stream >> cell.brain;
        stream >> cell.velocity.x;
        stream >> cell.velocity.y;
        stream >> cell.angle;
        stream >> cell.health;
        stream >> cell.energy;
        stream >> cell.waste;
        stream >> cell.memory1;
        stream >> cell.memory2;
        stream >> cell.memory3;
        stream >> cell.want_lay_egg;
        stream >> cell.want_eat;
        stream >> cell.want_stab;
        stream >> cell.sensor.hit_distance;
        stream >> cell.sensor.hit_red;
        stream >> cell.sensor.hit_green;
        stream >> cell.sensor.hit_blue;
        stream >> cell.stomach.plant_calories;
        stream >> cell.stomach.meat_calories;
        return stream;
    }

    void tick() {
        this->brain.reset();

        this->brain.set_input(0, distance_to_hit_strength(this->sensor.hit_distance));
        this->brain.set_input(1, this->sensor.hit_red / 255.0f);
        this->brain.set_input(2, this->sensor.hit_green / 255.0f);
        this->brain.set_input(3, this->sensor.hit_blue / 255.0f);
        this->brain.set_input(4, this->memory1);
        this->brain.set_input(5, this->memory2);
        this->brain.set_input(6, this->memory3);

        this->brain.pass();
//        this->brain.print_output();


        const float movement = (this->brain.get_output(0) - this->brain.get_output(1) / 2.0f) * this->dna.get_speed_multiplier() * SPEED_MULTIPLIER;
        const float strafe_movement = (this->brain.get_output(2) - this->brain.get_output(3)) * this->dna.get_speed_multiplier() * SPEED_MULTIPLIER;
        const float angular_velocity = this->brain.get_output(4) * SPEED_ANGULAR_MULTIPLIER - this->brain.get_output(5) * SPEED_ANGULAR_MULTIPLIER;
        this->angle += angular_velocity * MOVEMENT_MULTIPLIER;

        this->memory1 = this->brain.get_output(6);
        this->memory2 = this->brain.get_output(7);
        this->memory3 = this->brain.get_output(8);

        this->want_eat = this->brain.get_output(9) > WANT_EAT_THRESHOLD;
        this->want_lay_egg = this->brain.get_output(10) > WANT_EGG_THRESHOLD;
        this->want_stab = this->brain.get_output(11) > WANT_STAB_THRESHOLD;

        this->angle = wrap_angle(this->angle);
        this->velocity.x = std::cos(this->angle) * movement * MOVEMENT_MULTIPLIER;
        this->velocity.y = std::sin(this->angle) * movement * MOVEMENT_MULTIPLIER;
        this->velocity.x += std::cos(this->angle + HALF_PI) * strafe_movement * MOVEMENT_MULTIPLIER;
        this->velocity.y += std::sin(this->angle + HALF_PI) * strafe_movement * MOVEMENT_MULTIPLIER;

        this->move(this->velocity);
        this->digestion();

        if (this->want_stab) {
            this->use_energy(STAB_COST_MULTIPLIER);
        }
        this->use_energy(SIZE_PASSIVE_ENERGY_COST_MULTIPLIER * this->radius * this->radius);
        this->use_energy(LINEAR_SPEED_ACTIVE_ENERGY_COST_MULTIPLIER * (std::abs(movement) + std::abs(strafe_movement)) * this->radius * this->radius);
        this->use_energy(ANGULAR_SPEED_ACTIVE_ENERGY_COST_MULTIPLIER * std::abs(angular_velocity) * this->radius * this->radius);
        if (this->energy > this->dna.get_max_energy()) {
            this->use_energy(this->energy - this->dna.get_max_energy());
        }
        this->age++;
//        printf("Health: %f, energy: %f, age: %lu\n", this->health, this->energy, this->age);
    }

    void digestion() {
        const float plant_digestion = std::min(this->stomach.plant_calories * this->dna.metabolism, this->stomach.plant_calories);
        this->stomach.plant_calories -= plant_digestion;
        const float meat_digestion = std::min(this->stomach.meat_calories * this->dna.metabolism, this->stomach.meat_calories);
        this->stomach.meat_calories -= meat_digestion;
        const float plant_energy = plant_digestion * PLANT_EFFICIENCY_COEFFICIENT * (1 - this->dna.diet);
        const float plant_waste = plant_digestion - plant_energy;
        const float meat_energy = meat_digestion * MEAT_EFFICIENCY_COEFFICIENT * this->dna.diet;
        const float meat_waste = meat_digestion - meat_energy;
        this->energy += plant_energy + meat_energy;
        this->waste += plant_waste + meat_waste;
        if (this->energy > this->dna.get_max_energy()) {
            const float energy_surplus = this->energy - this->dna.get_max_energy();
            this->use_energy(energy_surplus);
        }
    }

    [[nodiscard]] float get_eat_range() const {
        return this->radius + EAT_REACH;
    }

    [[nodiscard]] float get_stab_range() const {
        return this->radius + STAB_REACH;
    }

    [[nodiscard]] float distance_to_hit_strength(const float hit_distance) const {
        return (this->dna.vision_range - hit_distance) / this->dna.vision_range;
    }

    void stab(Cell* other_cell) {
        other_cell->health = std::max(other_cell->health - this->radius * this->radius * this->dna.diet * this->dna.diet * this->dna.diet * COMBAT_DAMAGE_MULTIPLIER, 0.0f);
    }

    [[nodiscard]] bool is_alive() const {
        return this->health > 0;
    }

    [[nodiscard]] bool is_dead() const {
        return this->health <= 0;
    }

    [[nodiscard]] Egg* lay_egg() {
        if (this->energy < LAY_EGG_COST + this->dna.egg_energy_transfer) {
            return nullptr;
        }
        this->use_energy(LAY_EGG_COST);
        return new Egg(this->dna, this->take_energy(this->dna.egg_energy_transfer), this->position);
    }

    [[nodiscard]] bool should_lay_egg() const {
        return this->want_lay_egg;
    }

    void consume(Food* food) {
        if (food->get_food_type() == PLANT) {
            this->stomach.plant_calories += food->take_calories();
        }
        else if (food->get_food_type() == MEAT) {
            this->stomach.meat_calories += food->take_calories();
        }
        food->consume();
    }

    [[nodiscard]] float get_stomach_calories() const {
        return this->stomach.get_total_contents();
    }

    [[nodiscard]] float take_stomach_calories() {
        const float _calories = this->stomach.get_total_contents();
        this->stomach.plant_calories = 0;
        this->stomach.meat_calories = 0;
        return _calories;
    }

    void use_energy(const float used_energy) {
        if (used_energy > this->energy) {
            const float energy_debt = used_energy - this->energy;
            this->health = std::max(this->health - (energy_debt * ENERGY_DEBT_DAMAGE_MULTIPLIER), 0.0f);
            this->waste += this->energy;
            this->energy = 0;
            return;
        }
        this->energy -= used_energy;
        this->waste += used_energy;
    }

    [[nodiscard]] Color get_color() override {
        return this->dna.get_color();
    }

    [[nodiscard]] float get_red() const {
        return this->dna.red;
    }
    [[nodiscard]] float get_blue() const {
        return this->dna.blue;
    }
    [[nodiscard]] float get_green() const {
        return this->dna.green;
    }

    [[nodiscard]] bool should_shit() const {
        return this->waste > SHIT_WASTE_THRESHOLD;
    }

    [[nodiscard]] float shit() {
        float _waste = this->waste;
        this->waste = 0;
        return _waste;
    }

    [[nodiscard]] Vector2 get_shit_position() const {
//        return {random_originish(RNG), random_originish(RNG)};
        return this->polar_offset(shit_offset(RNG), random_angle(RNG));
    }

    [[nodiscard]] float get_waste() const {
        return this->waste;
    }

    [[nodiscard]] float get_energy() const {
        return this->energy;
    }

    [[nodiscard]] unsigned long get_age() const {
        return this->age;
    }

    [[nodiscard]] bool does_want_eat() const {
        return this->want_eat;
    }

    [[nodiscard]] bool does_want_stab() const {
        return this->want_stab;
    }

    [[nodiscard]] float get_vision_range() const {
        return this->dna.vision_range;
    }

    void cell_vision(const Sensor _center) {
        this->sensor = _center;
    }

    [[nodiscard]] float take_energy(const float _amount) {
        this->energy -= _amount;
        return _amount;
    }
    [[nodiscard]] float take_waste(const float _amount) {
        this->waste -= _amount;
        return _amount;
    }

    [[nodiscard]] Vector2 polar_offset(const float magnitude, const float _angle) const {
        const float combined_angle = this->angle + _angle;
        return {this->position.x + std::cos(combined_angle) * magnitude, this->position.y + std::sin(combined_angle) * magnitude};
    }

    Vector2 sensor_ray(const float _angle) {
        return {this->position.x + std::cos(this->angle + _angle) * this->dna.vision_range, this->position.y + std::sin(this->angle + _angle) * this->dna.vision_range};
    }

    RayResult cast_ray(const Vector2 other_entity_position, const float other_entity_radius, const Vector2 ray_end) {
        float ray_length = std::sqrt(std::pow(ray_end.x - this->position.x, 2.0f) + std::pow(ray_end.y - this->position.y, 2.0f));
        float h = std::abs((ray_end.x - this->position.x) * (other_entity_position.y - this->position.y) - (other_entity_position.x - this->position.x) * (ray_end.y - this->position.y)) / ray_length;
        if (h >= other_entity_radius) {
            return {false, -1};
        }
        float dx = (ray_end.x - this->position.x) / ray_length;
        float dy = (ray_end.y - this->position.y) / ray_length;
        float t = dx * (other_entity_position.x - this->position.x) + dy * (other_entity_position.y - this->position.y);
        float dt = std::sqrt(other_entity_radius * other_entity_radius - h * h );

        return {true, (float) std::sqrt(std::pow(this->position.x + (t - dt) * dx - this->position.x, 2.0f) + std::pow(this->position.y + (t - dt) * dy - this->position.y, 2.0f))}; //this->get_vision_range() -
    }

    void draw() override {
        DrawLineV(this->position, this->polar_offset(this->dna.vision_range, 0), this->dna.get_color(VISION_LINE_OPACITY));
        if (this->want_stab) {
            DrawLineEx(this->position, this->polar_offset(this->radius + STAB_REACH, 0), 1.0f, RED);
        }
        DrawCircle((int) std::round(this->position.x), (int) std::round(this->position.y) , this->radius, this->dna.get_color(255));
    }

    void draw_focus_info() {
        const unsigned int FONT_SIZE = 10;
        Vector2 draw_position = {(.02f * (float) GetScreenWidth()), (.02f * (float) GetScreenHeight())};
        DrawTextEx(GetFontDefault(), TextFormat("Color: (%.0f, %.0f, %.0f)", this->get_red(), this->get_green(), this->get_blue()), draw_position,FONT_SIZE, 1, WHITE);
        draw_position.y += 15;
        DrawTextEx(GetFontDefault(), TextFormat("Health: %.2f / %.2f", this->health, this->dna.get_max_health()), draw_position,FONT_SIZE, 1, WHITE);
        draw_position.y += 15;
        DrawTextEx(GetFontDefault(), TextFormat("Energy: %.2f / %.2f", this->energy, this->dna.get_max_energy()), draw_position,FONT_SIZE, 1, WHITE);
        draw_position.y += 15;
        DrawTextEx(GetFontDefault(), TextFormat("Waste: %.2f", this->waste), draw_position,FONT_SIZE, 1, WHITE);
        draw_position.y += 15;
        DrawTextEx(GetFontDefault(), TextFormat("Stomach: %.2f", this->stomach.get_total_contents()), draw_position,FONT_SIZE, 1, WHITE);
        draw_position.y += 15;
        DrawTextEx(GetFontDefault(), TextFormat("Metabolism: %.3f", this->dna.metabolism), draw_position,FONT_SIZE, 1, WHITE);
        draw_position.y += 15;
        DrawTextEx(GetFontDefault(), TextFormat("Diet: %.3f", this->dna.diet), draw_position,FONT_SIZE, 1, WHITE);
        draw_position.y += 15;
        DrawTextEx(GetFontDefault(), TextFormat("Radius: %.2f", this->dna.radius), draw_position,FONT_SIZE, 1, WHITE);
        draw_position.y += 15;
        DrawTextEx(GetFontDefault(), TextFormat("Egg Energy Transfer: %.2f", this->dna.egg_energy_transfer), draw_position,FONT_SIZE, 1, WHITE);
        draw_position.y += 15;
        DrawTextEx(GetFontDefault(), TextFormat("Speed: %.3f", this->dna.get_speed_multiplier()), draw_position,FONT_SIZE, 1, WHITE);
        this->brain.draw();
    }

    void draw_focus_marker() {
        DrawCircleV(this->position, this->radius * 2, {245, 245, 245, 100});
    }
    [[nodiscard]] float get_base_energy() const {
        return this->base_energy;
    }

    [[nodiscard]] float take_base_energy() {
        const float _base_energy = this->base_energy;
        this->base_energy = 0;
        return _base_energy;
    }
};
