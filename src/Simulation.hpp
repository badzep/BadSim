#pragma once


#include <algorithm>
#include <vector>
#include <list>


#include "Food.hpp"
#include "Egg.hpp"
#include "Cell.hpp"


constexpr std::string SAVES_PATH = "saves";

class Simulation {
private:
    std::vector<Cell*> cells;
    std::list<Egg*> eggs;
    std::list<Food*> foods;

public:
    Simulation() {

    }

    Simulation(const std::string& save_path) {
        std::ifstream cell_file;
        cell_file.open(save_path + "/cells", std::ios::in);
        int i = 0;
        while (true) {
            i++;
            Cell* cell = new Cell();
            cell_file >> *cell;
            if (cell_file.eof()) {
                delete cell;
                break;
            }
            this->cells.push_back(cell);
        }
        cell_file.close();

        std::ifstream egg_file;

        egg_file.open(save_path + "/eggs", std::ios::in);
        while (true) {
            Egg* egg = new Egg();
            egg_file >> *egg;
            if (egg_file.eof()) {
                delete egg;
                break;
            }
            this->eggs.push_back(egg);
        }
        egg_file.close();

        std::ifstream plant_file;
        std::ifstream meat_file;
        plant_file.open(save_path + "/plants", std::ios::in);
        while (true) {
            Plant* food = new Plant();
            plant_file >> *food;
            if (plant_file.eof()) {
                delete food;
                break;
            }
            this->foods.push_back(food);
        }
        plant_file.close();

        meat_file.open(save_path + "/meats", std::ios::in);
        while (true) {
            Meat* food = new Meat();
            meat_file >> *food;
            if (meat_file.eof()) {
                delete food;
                break;
            }
            this->foods.push_back(food);
        }
        meat_file.close();
    }

    ~Simulation() = default;

    void save() {
        std::filesystem::create_directory(SAVES_PATH);
        const std::string save_path = std::format("{}/save_{}",SAVES_PATH, std::chrono::system_clock::now());
        std::filesystem::create_directory(save_path);
        std::ofstream cell_file;
        cell_file.open(save_path + "/cells", std::ios::out);
        for (Cell* cell: this->cells) {
            cell_file << *cell;
        }
        cell_file.close();

        std::ofstream egg_file;
        egg_file.open(save_path + "/eggs", std::ios::out);
        for (Egg* egg: this->eggs) {
            egg_file << *egg;
        }
        egg_file.close();

        std::ofstream plant_file;
        std::ofstream meat_file;
        plant_file.open(save_path + "/plants", std::ios::out);
        meat_file.open(save_path + "/meats", std::ios::out);
        for (Food* food: this->foods) {
            if (food->get_food_type() == PLANT) {
                plant_file << *food;
            } else {
                meat_file << *food;
            }
        }
        plant_file.close();
        meat_file.close();
    }

    void setup_environment() {
        const float distance = 2000.0f;
        std::normal_distribution<float> position_distribution(distance, 1000.0f);

        const unsigned short plant_count = 200;
        const unsigned short cell_count = 500;
        for (unsigned short i = 0; i < plant_count; i++) {
            this->foods.push_back(new Plant(40.0f, {position_distribution(RNG), position_distribution(RNG)}));
        }
        for (unsigned short i = 0; i < cell_count; i++) {
            this->eggs.push_back(new Egg(20.0f, {position_distribution(RNG), position_distribution(RNG)}));
        }


        for (unsigned short i = 0; i < plant_count; i++) {
            this->foods.push_back(new Plant(50.0f, {position_distribution(RNG), -position_distribution(RNG)}));
        }
        for (unsigned short i = 0; i < cell_count; i++) {
            this->eggs.push_back(new Egg(20.0f, {position_distribution(RNG), -position_distribution(RNG)}));
        }


        for (unsigned short i = 0; i < plant_count; i++) {
            this->foods.push_back(new Plant(50.0f, {-position_distribution(RNG), position_distribution(RNG)}));
        }
        for (unsigned short i = 0; i < cell_count; i++) {
            this->eggs.push_back(new Egg(20.0f, {-position_distribution(RNG), position_distribution(RNG)}));
        }


        for (unsigned short i = 0; i < plant_count; i++) {
            this->foods.push_back(new Plant(50.0f, {-position_distribution(RNG), -position_distribution(RNG)}));
        }
        for (unsigned short i = 0; i < cell_count; i++) {
            this->eggs.push_back(new Egg(20.0f, {-position_distribution(RNG), -position_distribution(RNG)}));
        }
    }

    [[nodiscard]] std::vector<Cell*>& get_cells() {
        return this->cells;
    }
    [[nodiscard]] std::list<Egg*>& get_eggs() {
        return this->eggs;
    }
    [[nodiscard]] std::list<Food*>& get_foods() {
        return this->foods;
    }

    void clear() {
        for (Cell* cell: cells) {
            if (cell->is_dead()) {

                const float calories = cell->take_waste(cell->get_waste()) + cell->take_energy(cell->get_energy()) + cell->take_stomach_calories() + cell->take_base_energy() ;
                if (calories > 0) {
                    this->foods.push_back(new Meat(calories, cell->polar_offset(cell->get_radius(), random_angle(RNG))));
                }
            }
        }

        cells.erase(std::remove_if(cells.begin(), cells.end(), [] (const Cell* cell) {
            return cell->is_dead();
        }), cells.end());

        eggs.erase(std::remove_if(eggs.begin(), eggs.end(), [] (const Egg* egg) {
            return egg->is_hatched();
        }), eggs.end());

        foods.erase(std::remove_if(foods.begin(), foods.end(), [] (const Food* food) {
            return food->is_consumed();
        }), foods.end());

    }

    void produce() {
        for (Cell* cell: cells) {
            if (cell->should_lay_egg()) {
                Egg* egg = cell->lay_egg();
                if (egg != nullptr) {
                    this->eggs.push_back(egg);
                }
            }
            if (cell->should_shit()) {
                this->foods.push_back(new Plant(cell->shit(), cell->get_shit_position()));
            }
        }

        for (Egg* egg: this->eggs) {
            egg->tick();
            if (egg->is_ready_to_hatch()) {
                egg->hatch();
                this->cells.push_back(new Cell(egg));
            }
        }

        for (Food* food: this->foods) {
            if (food->is_too_far()) {
                food->set_position({random_originish(RNG), random_originish(RNG)});
            }
        }
    }
};
