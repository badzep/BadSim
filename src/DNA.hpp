#pragma once


#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <array>


#include "Constants.hpp"


class Range {
public:
    float min;
    float max;
    Range(const float _min, const float _max): min(_min), max(_max) {    }
    [[nodiscard]] float get_average() const {
        return (this->min + this->max) / 2;
    }
    [[nodiscard]] float validate(const float value) const {
        return std::max(this->min, std::min(this->max, value));
    }
};

std::normal_distribution<float> random_weight(0.0f, 0.5f);
std::normal_distribution<float> weight_mutation(0.0f, 0.004f * MUTATION_MULTIPLIER);

std::normal_distribution<float> random_bias(0.0f, 0.1f);
std::normal_distribution<float> bias_mutation(0.0f, 0.002f * MUTATION_MULTIPLIER);

const Range RADIUS_RANGE = Range(1.0f, 7.0f);
std::normal_distribution<float> random_radius(2.0f, 0.5f);
std::normal_distribution<float> radius_mutation(0.0f, 0.005f * MUTATION_MULTIPLIER);

const Range DIET_RANGE = Range(0.2f, 0.8f); // 0 = only plant, 1 = only_meat
std::normal_distribution<float> random_diet(0.5f, 0.15f); // 0 = only plant, 1 = only_meat
std::normal_distribution<float> diet_mutation(0.0f, 0.0025f * MUTATION_MULTIPLIER);

const Range SPEED_RANGE = Range(0.1f, 1.0f);
std::normal_distribution<float> random_speed(0.5f, 0.25f);
std::normal_distribution<float> speed_mutation(0.0f, 0.001f * MUTATION_MULTIPLIER);

const Range VISION_RANGE = Range(0.1f, 500.0f);
std::normal_distribution<float> random_vision_range(100.0f, 30.0f);
std::normal_distribution<float> vision_range_mutation(0.0f, 0.0025f * MUTATION_MULTIPLIER);

const Range EGG_ENERGY_TRANSFER_RANGE = Range(BASE_ENERGY, 100.0f);
std::normal_distribution<float> random_egg_energy_transfer(17.0f, 1.0f);
//std::normal_distribution<float> egg_energy_transfer_mutation(0.0f, 0.005f * MUTATION_MULTIPLIER);
std::normal_distribution<float> egg_energy_transfer_mutation(0.0f, 0.0075f * MUTATION_MULTIPLIER);

const Range METABOLISM_RANGE = Range(0.0f, 1.0f);
std::normal_distribution<float> random_metabolism(5e-3, 1e-3);
std::normal_distribution<float> metabolism_mutation(0.0f, 1e-6 * MUTATION_MULTIPLIER);

const Range COLOR_RANGE = Range(15, 235);
std::normal_distribution<float> random_color(COLOR_RANGE.get_average(), 20.0f);
std::normal_distribution<float> color_mutation(0.0f, 0.08 * MUTATION_MULTIPLIER);

template <const unsigned short ...LAYER_SIZES> class DNA {
private:
    constexpr static unsigned short layer_count() {
        unsigned short count = 0;
        for (unsigned short v: {LAYER_SIZES...}) {
            count++;
        }
        return count;
    }

    constexpr static std::array<unsigned short, layer_count()> layer_sizes() {
        std::array<unsigned short, layer_count()> x = {LAYER_SIZES...};
        return x;
    }

    constexpr static unsigned short layer_size_at(const unsigned short index) {
        return layer_sizes()[index];
    }

    constexpr static unsigned short neuron_count() {
        unsigned short count = 0;
        for (unsigned short layer_index = 0; layer_index < layer_count(); layer_index++) {
            count += layer_size_at(layer_index);
        }
        return count;
    }

    constexpr static unsigned short weight_count() {
        unsigned short count = 0;
        for (unsigned short layer_index = 1; layer_index < layer_count(); layer_index++) {
            count += layer_size_at(layer_index) * layer_size_at(layer_index - 1);
        }
        return count;
    }
public:
    float radius;
    float diet; // 0 = only plant, 1 = only_meat
    float speed;
    float vision_range;
    float egg_energy_transfer;
    float metabolism;

    float red;
    float green;
    float blue;

    float weights[weight_count()];
    float biases[neuron_count()];

    DNA(bool _1, bool _2) {
        this->radius = RADIUS_RANGE.validate(random_radius(RNG));
        this->diet = DIET_RANGE.validate(random_diet(RNG));
        this->speed = SPEED_RANGE.validate(random_speed(RNG));
        this->vision_range = VISION_RANGE.validate(random_vision_range(RNG));
        this->egg_energy_transfer = EGG_ENERGY_TRANSFER_RANGE.validate(random_egg_energy_transfer(RNG));
        this->metabolism = METABOLISM_RANGE.validate(random_metabolism(RNG));

        this->red = COLOR_RANGE.validate(random_color(RNG));
        this->green = COLOR_RANGE.validate(random_color(RNG));
        this->blue = COLOR_RANGE.validate(random_color(RNG));

        for (unsigned short weight_index = 0; weight_index < weight_count(); weight_index++) {
            this->weights[weight_index] = random_weight(RNG);
        }
        for (unsigned short bias_index = 0; bias_index < neuron_count(); bias_index++) {
            this->biases[bias_index] = random_bias(RNG);
        }
    }

    explicit DNA(std::istream &stream) {
        stream >> this;
    }
    explicit DNA(DNA<INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT>* parent) {
        this->radius = RADIUS_RANGE.validate(parent->radius + radius_mutation(RNG));
        this->diet = DIET_RANGE.validate(parent->diet + diet_mutation(RNG));
        this->speed = SPEED_RANGE.validate(parent->speed + speed_mutation(RNG));
        this->vision_range = VISION_RANGE.validate(parent->vision_range + vision_range_mutation(RNG));
        this->egg_energy_transfer = EGG_ENERGY_TRANSFER_RANGE.validate(parent->egg_energy_transfer + egg_energy_transfer_mutation(RNG));
        this->metabolism = METABOLISM_RANGE.validate(parent->metabolism + metabolism_mutation(RNG));

        this->red = COLOR_RANGE.validate(parent->red + color_mutation(RNG));
        this->green = COLOR_RANGE.validate(parent->green + color_mutation(RNG));
        this->blue = COLOR_RANGE.validate(parent->blue + color_mutation(RNG));

        for (unsigned short weight_index = 0; weight_index < weight_count(); weight_index++) {
            this->weights[weight_index] = parent->weights[weight_index] + weight_mutation(RNG);
        }
        for (unsigned short bias_index = 0; bias_index < neuron_count(); bias_index++) {
            this->biases[bias_index] = parent->biases[bias_index] + bias_mutation(RNG);
        }
    }

    ~DNA() {}

    void export_parameters(std::ostream &stream) const {
        for (float weight: this->weights) {
            stream << weight;
            stream << "\n";
        }
        for (float bias: this->biases) {
            stream << bias;
            stream << "\n";
        }
    }
    void import_parameters(std::istream &stream) {
        for (float &weight: this->weights) {
            stream >> weight;
        }
        for (float &bias: this->biases) {
            stream >> bias;
        }
    }

    friend std::ostream &operator<<(std::ostream &stream, const DNA<INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT>* dna) {
        stream << dna->radius;
        stream << "\n";
        stream << dna->diet;
        stream << "\n";
        stream << dna->speed;
        stream << "\n";
        stream << dna->vision_range;
        stream << "\n";
        stream << dna->egg_energy_transfer;
        stream << "\n";
        stream << dna->metabolism;
        stream << "\n";
        stream << dna->red;
        stream << "\n";
        stream << dna->blue;
        stream << "\n";
        stream << dna->green;
        stream << "\n";
        dna->export_parameters(stream);
        return stream;
    }

    friend std::istream &operator>>(std::istream &stream, DNA<INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT>* dna) {
        stream >> dna->radius;
        stream >> dna->diet;
        stream >> dna->speed;
        stream >> dna->vision_range;
        stream >> dna->egg_energy_transfer;
        stream >> dna->metabolism;
        stream >> dna->red;
        stream >> dna->blue;
        stream >> dna->green;
        dna->import_parameters(stream);
        return stream;
    }

    [[nodiscard]] float get_max_health() const {
        return this->radius * this->radius * RADIUS_MAX_HEALTH_CONVERSION_MULTIPLIER;
    }

    [[nodiscard]] float get_max_energy() const {
        return this->radius * this->radius * RADIUS_MAX_ENERGY_CONVERSION_MULTIPLIER;
    }

    [[nodiscard]] Color get_color() const {
        return {(unsigned char) std::round(this->red), (unsigned char) std::round(this->green), (unsigned char) std::round(this->blue), 255};
    }

    [[nodiscard]] Color get_color(const unsigned char opacity) const {
        return {(unsigned char) std::round(this->red), (unsigned char) std::round(this->green), (unsigned char) std::round(this->blue), opacity};
    }

    [[nodiscard]] float get_speed_multiplier() const {
        return this->speed;
    }

    [[nodiscard]] const float* get_weights() const {
        return this->weights;
    }

    [[nodiscard]] const float* get_biases() const {
        return this->biases;
    }

    void set_weight(const unsigned short index, const float value) {
        this->weights[index] = value;
    }

    void set_bias(const unsigned short index, const float value) {
        this->biases[index] = value;
    }
};

typedef DNA<INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT> DNA_t;
