#pragma once


#include <raylib.h>
#include <vector>
#include <list>
#include <mutex>


#include "Subsystem.hpp"
#include "Constants.hpp"
#include "Cell.hpp"
#include "ManagerSignals.hpp"


constexpr float BASE_CAMERA_MOVEMENT_SPEED = 200;
constexpr float SPRINT_CAMERA_MOVEMENT_SPEED = BASE_CAMERA_MOVEMENT_SPEED * 5;
constexpr float CAMERA_ZOOM_SPEED = 0.5f;


class RenderSubsystem: public Subsystem {
private:
    Camera2D camera;
    unsigned long focused_id;

    std::vector<Cell*> &cells;
    std::list<Egg*> &eggs;
    std::list<Food*> &foods;

    void init() override {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        SetConfigFlags(FLAG_MSAA_4X_HINT);
        InitWindow(WINDOW_SIZE, WINDOW_SIZE, "MeatColony");
        SetWindowMinSize(WINDOW_SIZE, WINDOW_SIZE);
        SetExitKey(0);

        this->paused.set_data(false);

        this->camera = {{WINDOW_SIZE / 2.0f, WINDOW_SIZE / 2.0f},
                        {WINDOW_SIZE / 2.0f, WINDOW_SIZE / 2.0f},
                        0.0f,
                        1.0f};
        this->focused_id = 0;
    }

    void update() override {
        this->input();
        this->render_notifier.acquire();
        this->render();
        this->finished_render_notifier.release();
    }

    [[nodiscard]] bool should_render(const Vector2 position) const {
        const Vector2 screen_position = GetWorldToScreen2D(position, this->camera);
        if (screen_position.x < 0) {
            return false;
        }
        if (screen_position.y < 0) {
            return false;
        }
        if (screen_position.x > (float) GetScreenWidth()) {
            return false;
        }
        if (screen_position.y > (float) GetScreenHeight()) {
            return false;
        }
        return true;
    }

    void render() {
        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode2D(this->camera);
                this->draw_focus_marker(this->focused_id);
                this->draw();
            EndMode2D();
            this->draw_focus_info(this->focused_id);
            DrawFPS((int) (.9 * GetScreenWidth()), (int) (.02 * GetScreenHeight()));
        EndDrawing();

        if(WindowShouldClose()) {
            ManagerSignals::shutdown.set_data(true);
        }
    }

    void draw() const {
        for (Food* food: this->foods) {
            if (!this->should_render(food->get_position())) {
                continue;
            }
            food->draw();
        }

        for (Egg* egg: this->eggs) {
            if (!this->should_render(egg->get_position())) {
                continue;
            }
            egg->draw();
        }

        for (Cell* cell: this->cells) {
            if (!this->should_render(cell->get_position())) {
                continue;
            }
            cell->draw();
        }
    }

    void draw_focus_info(const unsigned long _focused_id) {
        for (Cell* cell: this->cells) {
            if (cell->get_id() == _focused_id) {
                cell->draw_focus_info();
            }
        }
    }

    void draw_focus_marker(const unsigned long _focused_id) {
        for (Cell* cell: this->cells) {
            if (cell->is_dead()) {
                continue;
            }
            if (cell->get_id() == _focused_id) {
                cell->draw_focus_marker();
            }
        }
    }
    void input() {
        if (IsKeyPressed(KEY_SPACE)) {
            this->paused.manual_lock();
            this->paused.unsafe_set_data(!this->paused.unsafe_get_data());
            this->paused.manual_unlock();
        }

        if (IsKeyPressed(KEY_ONE)) {
            SetTargetFPS(10);
        }

        if (IsKeyPressed(KEY_TWO)) {
            SetTargetFPS(60);
        }

        if (IsKeyPressed(KEY_THREE)) {
            SetTargetFPS(150);
        }

        if (IsKeyPressed(KEY_FOUR)) {
            SetTargetFPS(10000);
        }

        float camera_speed = BASE_CAMERA_MOVEMENT_SPEED;
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            camera_speed =  SPRINT_CAMERA_MOVEMENT_SPEED;
        }
        if (IsKeyDown(KEY_W)) {
            this->camera.target.y -= camera_speed / (float) GetFPS();
        }

        if (IsKeyDown(KEY_A)) {
            this->camera.target.x -= camera_speed / (float) GetFPS();
        }

        if (IsKeyDown(KEY_S)) {
            this->camera.target.y += camera_speed / (float) GetFPS();
        }

        if (IsKeyDown(KEY_D)) {
            this->camera.target.x += camera_speed / (float) GetFPS();
        }

        if (IsKeyDown(KEY_E)) {
            this->camera.zoom += CAMERA_ZOOM_SPEED / (float) GetFPS();
        }

        if (IsKeyDown(KEY_Q)) {
            this->camera.zoom -= CAMERA_ZOOM_SPEED / (float) GetFPS();
            this->camera.zoom = std::max(0.01f, this->camera.zoom);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse_position = GetScreenToWorld2D(GetMousePosition(), this->camera);
            this->focused_id = this->get_cell_at(mouse_position);
        }
    }

    void on_shutdown() override {
        CloseWindow();
    }

    void on_panic() override {
        CloseWindow();
    };

    void signal_shutdown() override {
        this->should_shutdown.set_data(true);

        //release all semaphores
        this->render_notifier.release();
        this->finished_render_notifier.release();
    }

    void signal_panic() override {
        this->should_panic.set_data(true);

        //release all semaphores
        this->render_notifier.release();
        this->finished_render_notifier.release();
    }

    unsigned long get_cell_at(const Vector2 mouse_position) {
        for (Cell* cell: this->cells) {
            if (std::sqrt(std::pow(cell->get_x_position() - mouse_position.x, 2) + std::pow(cell->get_y_position()- mouse_position.y, 2)) <= cell->get_radius() * 3) {
                return cell->get_id();
            }
        }
        return 0;
    }

public:
    ThreadSafe<bool> paused;
    std::binary_semaphore render_notifier{0};
    std::binary_semaphore finished_render_notifier{0};
    RenderSubsystem(std::vector<Cell*> &cells, std::list<Egg*> &eggs, std::list<Food*> &foods): cells(cells), eggs(eggs), foods(foods)  {

    }

    ~RenderSubsystem() = default;

    [[nodiscard]] constexpr std::string id() const override {
        return "Render Subsystem";
    }

    [[nodiscard]] constexpr float warning_loop_second_threshold() const override {
        return -1; // Disabled
    }

    [[nodiscard]] constexpr float critical_loop_second_threshold() const override {
        return -1; // Disabled
    }

};
