#pragma once


#include <raylib.h>
#include <random>
#include <chrono>
#include "Activation.hpp"

constexpr float MAP_SIZE = 10000;
constexpr float HALF_MAP_SIZE = MAP_SIZE / 2.0f;

constexpr float TAU = PI * 2.0f;
constexpr float HALF_PI = PI / 2.0f;
const unsigned int WINDOW_SIZE = 1000;


constexpr float EAT_REACH = 5.0f;
constexpr float STAB_REACH = 5.0f;


[[nodiscard]] float wrap_angle(float angle) {
    while (angle > TAU) {
        angle -= TAU;
    }
    while (angle < -TAU) {
        angle += TAU;
    }
    return angle;
}
constexpr unsigned short INPUT_COUNT = 7;
constexpr unsigned short LAYER_SIZE = 15;
constexpr unsigned short OUTPUT_COUNT = 12;

constexpr float MUTATION_MULTIPLIER = 5.0f;

const unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine RNG (seed);

std::uniform_real_distribution<float> uniform_percent(0, 100);

std::uniform_real_distribution<float> random_angle(0, PI * 2);


constexpr float RADIUS_MAX_HEALTH_CONVERSION_MULTIPLIER = 1;
constexpr float RADIUS_MAX_ENERGY_CONVERSION_MULTIPLIER = 10;

constexpr float SPEED_MULTIPLIER = 1.0f;
constexpr float SPEED_ANGULAR_MULTIPLIER = .05;
constexpr float MOVEMENT_MULTIPLIER = 2.0f;

constexpr unsigned char VISION_LINE_OPACITY = 70;

constexpr float ALMOST_UNIVERSAL_ENERGY_COST_MULTIPLIER = 2.5f;
constexpr float SIZE_PASSIVE_ENERGY_COST_MULTIPLIER = 2e-4f * ALMOST_UNIVERSAL_ENERGY_COST_MULTIPLIER;
constexpr float LINEAR_SPEED_ACTIVE_ENERGY_COST_MULTIPLIER = 5.0e-4f * ALMOST_UNIVERSAL_ENERGY_COST_MULTIPLIER * 2.0f;
constexpr float ANGULAR_SPEED_ACTIVE_ENERGY_COST_MULTIPLIER = 2.5e-4f * ALMOST_UNIVERSAL_ENERGY_COST_MULTIPLIER * 0.0f;
constexpr float STAB_COST_MULTIPLIER = 5e-6f * ALMOST_UNIVERSAL_ENERGY_COST_MULTIPLIER;

constexpr float LAY_EGG_COST = 1.0f;

constexpr float PLANT_EFFICIENCY_COEFFICIENT = 0.75f;
constexpr float MEAT_EFFICIENCY_COEFFICIENT = 0.99f;

constexpr float WANT_STAB_THRESHOLD = 0.51f;
constexpr float WANT_EAT_THRESHOLD = 0.40f;
constexpr float WANT_EGG_THRESHOLD = 0.40f;

constexpr float SHIT_WASTE_THRESHOLD = 20.0f;
constexpr float ENERGY_DEBT_DAMAGE_MULTIPLIER = 100.0f;
constexpr float COMBAT_DAMAGE_MULTIPLIER = 0.1f;

constexpr float BASE_ENERGY = 15.0f;

std::normal_distribution<float> shit_offset(50.0f, 7.5f);
constexpr float POSITION_DISTANCE = 2100.0f;
std::normal_distribution<float> random_originish(0.0f, POSITION_DISTANCE);
constexpr bool WRAP_POSITION = false;

constexpr Activation INPUT_ACTIVATION = NO_ACTIVATION;
constexpr Activation HIDDEN_ACTIVATION = LEAKY_RELU;
constexpr Activation OUTPUT_ACTIVATION = SIGMOID;
