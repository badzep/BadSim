#pragma once


#include <list>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>


#include "Subsystem.hpp"
#include "Cell.hpp"


class PartialProcessingSubsystem: public Subsystem {
protected:
    unsigned int partial_id;
    unsigned int total;
    std::vector<Cell*> &cells;
    std::list<Egg*> &eggs;
    std::list<Food*> &foods;

    void init() override {

    }

    void interaction() {
        for (unsigned short cell1_index = this->partial_id; cell1_index < (unsigned short) cells.size(); cell1_index += this->total) {
            if (cells[cell1_index]->is_dead()) {
                continue;
            }

            Sensor center_sensor{};
            center_sensor.hit_distance = cells[cell1_index]->get_vision_range();
            const Vector2 center_ray = cells[cell1_index]->sensor_ray(0);

            float max_x;
            float min_x;
            if (cells[cell1_index]->get_x_position() >= center_ray.x) {
                max_x = cells[cell1_index]->get_x_position();
                min_x = center_ray.x;
            } else {
                max_x = center_ray.x;
                min_x = cells[cell1_index]->get_x_position();
            }
            
            float max_y;
            float min_y;
            if (cells[cell1_index]->get_y_position() >= center_ray.y) {
                max_y = cells[cell1_index]->get_y_position();
                min_y = center_ray.y;
            } else {
                max_y = center_ray.y;
                min_y = cells[cell1_index]->get_y_position();
            }

            for (unsigned short cell2_index = 0; cell2_index < (unsigned short) cells.size(); cell2_index++) {
                if (cells[cell2_index]->get_x_position() > max_x) {
                    continue;
                }
                if (cells[cell2_index]->get_y_position() > max_y) {
                    continue;
                }
                if (cells[cell2_index]->get_x_position() < min_x) {
                    continue;
                }
                if (cells[cell2_index]->get_y_position() < min_y) {
                    continue;
                }

                if (cells[cell2_index]->is_dead()) {
                    continue;
                }
                if (cell1_index == cell2_index) {
                    continue;
                }

                RayResult center_ray_result = cells[cell1_index]->cast_ray(cells[cell2_index]->get_position(), cells[cell2_index]->get_radius(), center_ray);
                if (center_ray_result.hits) {
                    if (cells[cell1_index]->does_want_stab() and center_ray_result.hit_distance <= cells[cell1_index]->get_stab_range()) {
                        cells[cell1_index]->stab(cells[cell2_index]);
                    }
                    if (center_ray_result.hit_distance < center_sensor.hit_distance) {
                        center_sensor.hit_distance = center_ray_result.hit_distance;
                        center_sensor.hit_red = cells[cell2_index]->get_red();
                        center_sensor.hit_green = cells[cell2_index]->get_green();
                        center_sensor.hit_blue = cells[cell2_index]->get_blue();
                    }
                }
            }

            for (Food* food: foods) {

                if (food->get_x_position() > max_x) {
                    continue;
                }
                if (food->get_y_position() > max_y) {
                    continue;
                }
                if (food->get_x_position() < min_x) {
                    continue;
                }
                if (food->get_y_position() < min_y) {
                    continue;
                }

                if (food->is_consumed()) {
                    continue;
                }

                RayResult center_ray_result = cells[cell1_index]->cast_ray(food->get_position(), food->get_radius(), center_ray);
                if (center_ray_result.hits) {
                    if (cells[cell1_index]->does_want_eat() and center_ray_result.hit_distance <= cells[cell1_index]->get_eat_range()) {
                        cells[cell1_index]->consume(food);
                    }
                    if (center_ray_result.hit_distance < center_sensor.hit_distance) {
                        center_sensor.hit_distance = center_ray_result.hit_distance;
                        center_sensor.hit_red = food->get_red();
                        center_sensor.hit_green = food->get_green();
                        center_sensor.hit_blue = food->get_blue();
                    }
                }
            }

            cells[cell1_index]->cell_vision(center_sensor);
        }
    }
    void tick() {
        for (unsigned short cell_index = this->partial_id; cell_index < (unsigned short) cells.size(); cell_index += this->total) {
            cells[cell_index]->tick();
        }
    }

    void update() override {
        this->do_interaction_notifier.acquire();
        this->interaction();
        this->interaction_completion_notifier.release();

        this->do_tick_notifier.acquire();
        this->tick();
        this->tick_completion_notifier.release();

    }

    void signal_shutdown() override {
        this->should_shutdown.set_data(true);

        //release all semaphores
        this->do_interaction_notifier.release();
        this->interaction_completion_notifier.release();
        this->do_tick_notifier.release();
        this->tick_completion_notifier.release();
    }

    void signal_panic() override {
        this->should_panic.set_data(true);

        //release all semaphores
        this->do_interaction_notifier.release();
        this->interaction_completion_notifier.release();
        this->do_tick_notifier.release();
        this->tick_completion_notifier.release();
    }


    void on_shutdown() override {

    }

    void on_panic() override {

    }

    [[nodiscard]] constexpr float warning_loop_second_threshold() const override {
        return -1; // Disabled
    }

    [[nodiscard]] constexpr float critical_loop_second_threshold() const override {
        return -1; // Disabled
    }


public:
    std::binary_semaphore do_interaction_notifier{0};
    std::binary_semaphore interaction_completion_notifier{0};

    std::binary_semaphore do_tick_notifier{0};
    std::binary_semaphore tick_completion_notifier{0};

    explicit PartialProcessingSubsystem(const unsigned int partial_id,
                                        const unsigned int total, std::vector<Cell*> &cells,
                                        std::list<Egg*> &eggs, std::list<Food*> &foods):
                                        partial_id(partial_id),
                                        total(total),
                                        cells(cells),
                                        eggs(eggs),
                                        foods(foods) {

    }

    ~PartialProcessingSubsystem() = default;

    [[nodiscard]] constexpr std::string id() const override {
        return "Partial Processing Subsystem";
    }

};
