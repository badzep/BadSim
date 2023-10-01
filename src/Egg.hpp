#pragma once

#include "Body.hpp"
#include "DNA.hpp"


constexpr unsigned int HATCH_AGE = 500;


class Egg: public Body {
private:
    unsigned int age;
    float energy;
    bool hatched;
    DNA<INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT>* dna;
public:
//    Egg() {}

//    Egg(DNA<INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT> &_dna, const float _energy, const Vector2 _position) {
//        this->id = get_new_id();
//        this->radius = 1;
//        this->position.x = _position.x;
//        this->position.y = _position.y;
//        this->energy = _energy;
//        this->age = 0;
//        this->hatched = false;
//        this->dna = DNA<INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT>(_dna, false);
//        this->wrap_position();
//    }
    explicit Egg(std::istream &stream) {
        stream >> this;
    }

    Egg(DNA<INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT>* _dna, const float _energy, const Vector2 _position) {
        this->id = get_new_id();
        this->radius = 1;
        this->position.x = _position.x;
        this->position.y = _position.y;
        this->energy = _energy;
        this->age = 0;
        this->hatched = false;
        this->dna = _dna;
        this->wrap_position();
    }

    Egg(const float _energy, const Vector2 _position) {
        this->id = get_new_id();
        this->radius = 1;
        this->position.x = _position.x;
        this->position.y = _position.y;
        this->energy = _energy;
        this->age = 0;
        this->hatched = false;
        this->dna = new DNA<INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT>(false, false);
        this->wrap_position();
    }

    ~Egg() {
        // actually don't delete dna since it needs to be passed on to cell, no point in copying it
    }

    friend std::ostream &operator<<(std::ostream &stream, const Egg* egg) {
        stream << egg->id;
        stream << "\n";
        stream << egg->radius;
        stream << "\n";
        stream << egg->position.x;
        stream << "\n";
        stream << egg->position.y;
        stream << "\n";
        stream << egg->energy;
        stream << "\n";
        stream << egg->age;
        stream << "\n";
        stream << egg->hatched;
        stream << "\n";
        stream << egg->dna;
        stream << "\n";
        return stream;
    }
    friend std::istream &operator>>(std::istream &stream, Egg* egg) {
        stream >> egg->id;
        stream >> egg->radius;
        stream >> egg->position.x;
        stream >> egg->position.y;
        stream >> egg->energy;
        stream >> egg->age;
        stream >> egg->hatched;
        egg->dna = new DNA_t(stream);
        return stream;
    }

    [[nodiscard]] bool is_ready_to_hatch() const {
        return this->age >= HATCH_AGE and !this->hatched;
    }

    [[nodiscard]] bool is_hatched() const {
        return this->hatched;
    }

    void hatch() {
        this->hatched = true;
    }

    void tick() {
        this->age++;
    }

    DNA<INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT>* get_dna() {
        return this->dna;
    }

    [[nodiscard]] float get_energy() const {
        return this->energy;
    }

    [[nodiscard]] float take_energy() {
        const float _energy = this->energy;
        this->energy = 0;
        return _energy;
    }
};