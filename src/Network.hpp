#pragma once

#include <cmath>
#include <iostream>
#include <array>


#include "DNA.hpp"
#include "Activation.hpp"


const float NEURON_SIZE = 5;
const float START_LAYER_X = 70;
const float LAYER_SPACING = 30;
const float NEURON_SPACING = 20;
const float START_NEURON_Y = 200;
const int FONT_SIZE = 10;
const Vector2 INPUT_TEXT_OFFSET = {-60, -(float) FONT_SIZE / 2};
const Vector2 OUTPUT_TEXT_OFFSET = {-10, -(float) FONT_SIZE / 2};

template<const Activation INPUT_ACTIVATION, const Activation HIDDEN_ACTIVATION, const Activation OUTPUT_ACTIVATION, const unsigned short ...LAYER_SIZES> class Network {
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

    constexpr Activation get_layer_activation(const unsigned short layer_index) {
        if (layer_index == layer_count() - 1) {
            return OUTPUT_ACTIVATION;
        }
        if (layer_index == 0) {
            return INPUT_ACTIVATION;
        }
        return HIDDEN_ACTIVATION;
    }

    constexpr static unsigned short layer_start_index(const unsigned short target_layer_index) {
        unsigned short count = 0;
        for (unsigned short layer_index = 0; layer_index < target_layer_index; layer_index++) {
            count += layer_size_at(layer_index);
        }
        return count;
    }

    float values[neuron_count()];
    float weights[weight_count()];
    float biases[neuron_count()];

    [[nodiscard]] float get_value_at(const unsigned short target_layer_index, const unsigned short neuron_index) const {
        return this->values[layer_start_index(target_layer_index) + neuron_index];
    }

    // NOTE: you can't get weights at layer 0 (because they don't exist)
    float get_weight_at(const unsigned short target_layer_index, const unsigned short neuron_index, const unsigned short input_index) {
        unsigned short count = 0;
        for (unsigned short layer_index = 1; layer_index < target_layer_index; layer_index++) {
            count += layer_size_at(layer_index) * layer_size_at(layer_index - 1);
        }
        count += neuron_index * layer_size_at(target_layer_index - 1);
        count += input_index;
        return this->weights[count];
    }

    void apply_activation(const unsigned short layer_index) {
        switch (this->get_layer_activation(layer_index)) {
            case RELU:
                for (unsigned short neuron_index = 0; neuron_index < layer_size_at(layer_index); neuron_index++) {
                    this->values[layer_start_index(layer_index) + neuron_index] = relu_activation(this->values[layer_start_index(layer_index) + neuron_index]);
                }
                return;
            case LEAKY_RELU:
                for (unsigned short neuron_index = 0; neuron_index < layer_size_at(layer_index); neuron_index++) {
                    this->values[layer_start_index(layer_index) + neuron_index] = leaky_relu_activation(this->values[layer_start_index(layer_index) + neuron_index]);
                }
                return;
            case SIGMOID:
                for (unsigned short neuron_index = 0; neuron_index < layer_size_at(layer_index); neuron_index++) {
                    this->values[layer_start_index(layer_index) + neuron_index] = sigmoid_activation(this->values[layer_start_index(layer_index) + neuron_index]);
                }
                return;
            case TANH:
                for (unsigned short neuron_index = 0; neuron_index < layer_size_at(layer_index); neuron_index++) {
                    this->values[layer_start_index(layer_index) + neuron_index] = tanh_activation(this->values[layer_start_index(layer_index) + neuron_index]);
                }
                return;
            case SQUARE_ROOT:
                for (unsigned short neuron_index = 0; neuron_index < layer_size_at(layer_index); neuron_index++) {
                    this->values[layer_start_index(layer_index) + neuron_index] = square_root_activation(this->values[layer_start_index(layer_index) + neuron_index]);
                }
                return;
            case NO_ACTIVATION:
                return;
        }
    }

    void pass_layer(const unsigned short layer_index) {
        for (unsigned short neuron_index = 0; neuron_index < layer_size_at(layer_index); neuron_index++) {
            for (unsigned short input_neuron_index = 0; input_neuron_index < layer_size_at(layer_index - 1); input_neuron_index++) {
                this->values[layer_start_index(layer_index) + neuron_index] += this->values[layer_start_index(layer_index - 1) + input_neuron_index] * this->get_weight_at(layer_index, neuron_index, input_neuron_index);
            }
        }
        this->apply_activation(layer_index);
    }

    [[nodiscard]] Vector2 get_neuron_draw_position(const unsigned short layer_index, const unsigned short neuron_index) {
        return {START_LAYER_X + LAYER_SPACING * (float) layer_index, START_NEURON_Y + NEURON_SPACING * (float) neuron_index};
    }
    [[nodiscard]] Color get_draw_color(const float value) {
        const float negativity = std::min(std::abs(std::min(value, 0.0f)), 1.0f);
        const float positivity = std::min(std::abs(std::max(value, 0.0f)), 1.0f);
        const float strength = std::min(std::abs(value), 1.0f);
        return {(unsigned char) (255.0f * negativity), (unsigned char) ((255.0f * positivity) + (255.0f * negativity / 2.0f)), (unsigned char) (255.0f * positivity), (unsigned char) (255.0f * strength)};
    }

public:
    explicit Network(std::istream &stream) {
        stream >> this;
    }

    explicit Network(DNA<LAYER_SIZES...>* dna) {
        for (unsigned short neuron_index = 0; neuron_index < neuron_count(); neuron_index++) {
            this->values[neuron_index] = 0;
        }
        for (unsigned short neuron_index = 0; neuron_index < weight_count(); neuron_index++) {
            this->weights[neuron_index] = dna->weights[neuron_index];
        }
        for (unsigned short neuron_index = 0; neuron_index < neuron_count(); neuron_index++) {
            this->biases[neuron_index] = dna->biases[neuron_index];
        }
    }

    ~Network() = default;

    void export_parameters(std::ostream &stream) const {
        for (float value: this->values) {
            stream << value;
            stream << "\n";
        }
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
        for (float &value: this->values) {
            stream >> value;
        }
        for (float &weight: this->weights) {
            stream >> weight;
        }
        for (float &bias: this->biases) {
            stream >> bias;
        }
    }

    friend std::ostream &operator<<(std::ostream &stream, const Network<INPUT_ACTIVATION, HIDDEN_ACTIVATION, OUTPUT_ACTIVATION, INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT>* network) {
        network->export_parameters(stream);
        stream << "\n";
        return stream;
    }

    friend std::istream &operator>>(std::istream &stream, Network<INPUT_ACTIVATION, HIDDEN_ACTIVATION, OUTPUT_ACTIVATION, INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT>* network) {
        network->import_parameters(stream);
        return stream;
    }

    void draw() {
        for (unsigned short neuron_index = 0; neuron_index < layer_size_at(0); neuron_index++) {
            DrawCircleV(this->get_neuron_draw_position(0, neuron_index), NEURON_SIZE, get_draw_color(this->values[layer_start_index(0) + neuron_index]));
        }
        for (unsigned short layer_index = 1; layer_index < layer_count(); layer_index++) {
            for (unsigned short neuron_index = 0; neuron_index < layer_size_at(layer_index); neuron_index++) {
                for (unsigned short input_neuron_index = 0; input_neuron_index < layer_size_at(layer_index - 1); input_neuron_index++) {
                    const float weight_value = this->values[layer_start_index(layer_index - 1) + input_neuron_index] * this->get_weight_at(layer_index, neuron_index, input_neuron_index);
                    DrawLineEx(this->get_neuron_draw_position(layer_index - 1, input_neuron_index), this->get_neuron_draw_position(layer_index, neuron_index), 2.0f, this->get_draw_color(weight_value));
                }
                DrawCircleV(this->get_neuron_draw_position(layer_index, neuron_index), NEURON_SIZE, get_draw_color(this->values[layer_start_index(layer_index) + neuron_index]));
            }
        }
        this->draw_input_text("Sensor", 0);
        this->draw_input_text("Red", 1);
        this->draw_input_text("Green", 2);
        this->draw_input_text("Blue", 3);
        this->draw_input_text("Memory 1", 4);
        this->draw_input_text("Memory 2", 5);
        this->draw_input_text("Memory 3", 6);

        this->draw_output_text("Forward", 0);
        this->draw_output_text("Backward", 1);
        this->draw_output_text("Strafe Right", 2);
        this->draw_output_text("Strafe Left", 3);
        this->draw_output_text("+Angle", 4);
        this->draw_output_text("-Angle", 5);
        this->draw_output_text("Memory 1", 6);
        this->draw_output_text("Memory 2", 7);
        this->draw_output_text("Memory 3", 8);

        this->draw_output_text("Eat", 9);
        this->draw_output_text("Lay Egg", 10);
        this->draw_output_text("Stab", 11);
    }

    void draw_input_text(const char* text, const unsigned short input_index) {
        DrawText(text, (int) (START_LAYER_X + INPUT_TEXT_OFFSET.x), (int) (START_NEURON_Y + NEURON_SPACING * (float) input_index + INPUT_TEXT_OFFSET.y), FONT_SIZE, WHITE);
    }
    void draw_output_text(const char* text, const unsigned short output_index) {
        DrawText(text, (int) (START_LAYER_X + LAYER_SPACING * (float) this->layer_count()-1) + OUTPUT_TEXT_OFFSET.x, (int) (START_NEURON_Y + NEURON_SPACING * (float) output_index + OUTPUT_TEXT_OFFSET.y), FONT_SIZE, WHITE);
    }

    void reset() {
        for (unsigned short neuron_index = 0; neuron_index < neuron_count(); neuron_index++) {
            this->values[neuron_index] = 0;
        }
    }

    void pass() {
        for (unsigned short neuron_index = 0; neuron_index < neuron_count(); neuron_index++) {
            this->values[neuron_index] += this->biases[neuron_index];
        }
        for (unsigned short layer_index = 1; layer_index < layer_count(); layer_index++) {
            this->pass_layer(layer_index);
        }
    }

    void set_input(const unsigned short input_index, const float value) {
        this->values[input_index] = value;
    }

    [[nodiscard]] float get_output(const unsigned short output_index) const {
        return get_value_at(layer_count() - 1, output_index);
    }
    void print_values() const {
        printf("Values: ");
        for (unsigned short neuron_index = 0; neuron_index < neuron_count(); neuron_index++) {
            printf("%f ", this->values[neuron_index]);
        }
        printf("\n");
    }

    void print_parameters() const {
        printf("Weights: ");
        for (unsigned short weight_index = 0; weight_index < weight_count(); weight_index++) {
            printf("%f ", this->weights[weight_index]);
        }
        printf("\n");
        printf("Biases: ");
        for (unsigned short neuron_index = 0; neuron_index < neuron_count(); neuron_index++) {
            printf("%f ", this->biases[neuron_index]);
        }
        printf("\n");
    }

    void print_output() const {
        printf("Outputs: ");
        for (unsigned short neuron_index = layer_start_index(layer_count() - 1); neuron_index < neuron_count(); neuron_index++) {
            printf("%f ", this->values[neuron_index]);
        }
        printf("\n");
    }
};

typedef Network<INPUT_ACTIVATION, HIDDEN_ACTIVATION, OUTPUT_ACTIVATION, INPUT_COUNT, LAYER_SIZE, LAYER_SIZE, OUTPUT_COUNT> Network_t;
