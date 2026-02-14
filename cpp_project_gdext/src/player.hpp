#pragma once

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <vector>

namespace sample {

class Player : public godot::CharacterBody3D {
    GDCLASS(Player, godot::CharacterBody3D)

public:
    void _ready() override;
    void _physics_process(double delta) override;
    void process_mouse();
    void _unhandled_input(const godot::Ref<godot::InputEvent> &event) override;
    static void _bind_methods();

    godot::Node3D *pivot{nullptr};
    float sensitivity{0.5f};
    float speed{5.0f};
    float jump_velocity{4.5f};
    std::vector<godot::Vector2> input_buffer{};

    godot::Node3D *get_pivot();
    void set_pivot(godot::Node3D *pivot);
    float get_sensitivity();
    void set_sensitivity(float sensitivity);
    float get_speed();
    void set_speed(float speed);
};

} // namespace sample
