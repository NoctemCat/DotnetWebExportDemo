#include "player.hpp"
#include <godot_cpp/classes/global_constants.hpp>

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace sample {

void Player::_bind_methods() {
    using namespace godot;
    ClassDB::bind_method(D_METHOD("get_pivot"), &Player::get_pivot);
    ClassDB::bind_method(D_METHOD("set_pivot", "pivode"), &Player::set_pivot);
    ADD_PROPERTY(
        PropertyInfo(Variant::Type::OBJECT, "pivot", PROPERTY_HINT_NODE_TYPE, "Node3D"),
        "set_pivot", "get_pivot"
    );

    ClassDB::bind_method(D_METHOD("get_sensitivity"), &Player::get_sensitivity);
    ClassDB::bind_method(D_METHOD("set_sensitivity", "sensitivity"), &Player::set_sensitivity);
    ADD_PROPERTY(
        PropertyInfo(Variant::Type::FLOAT, "sensitivity"), "set_sensitivity", "get_sensitivity"
    );

    ClassDB::bind_method(D_METHOD("get_speed"), &Player::get_speed);
    ClassDB::bind_method(D_METHOD("set_speed", "speed"), &Player::set_speed);
    ADD_PROPERTY(PropertyInfo(Variant::Type::FLOAT, "speed"), "set_speed", "get_speed");
}

void Player::_ready() {
    if (godot::Engine::get_singleton()->is_editor_hint()) { return; }

    godot::Input::get_singleton()->set_mouse_mode(godot::Input::MOUSE_MODE_CAPTURED);
}

void Player::_physics_process(double delta) {
    if (godot::Engine::get_singleton()->is_editor_hint()) { return; }

    using namespace godot;
    Input *input = Input::get_singleton();

    if (input->is_action_just_pressed("pause")) {
        if (input->get_mouse_mode() == Input::MOUSE_MODE_CAPTURED) {
            Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_VISIBLE);
            get_tree()->set_pause(true);
        } else {
            Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
            get_tree()->set_pause(false);
        }
    }

    if (get_tree()->is_paused()) { return; }

    process_mouse();

    if (!is_on_floor()) { set_velocity(get_velocity() + get_gravity() * delta); }

    if (input->is_action_just_pressed("jump") && is_on_floor()) {
        Vector3 vel = get_velocity();
        set_velocity(Vector3(vel.x, jump_velocity, vel.z));
    }

    Vector2 input_dir = input->get_vector("left", "right", "forward", "backward");
    Vector3 direction =
        (get_transform().basis.xform(Vector3(input_dir.x, 0.0, input_dir.y))).normalized();

    float player_speed = speed;
    if (input->is_action_pressed("run")) { player_speed *= 2; }

    Vector3 vel = get_velocity();
    if (!direction.is_zero_approx()) {
        vel.x = direction.x * player_speed;
        vel.z = direction.z * player_speed;
    } else {
        vel.x = Math::move_toward(vel.x, 0.0f, speed);
        vel.z = Math::move_toward(vel.z, 0.0f, speed);
    }
    set_velocity(vel);
    move_and_slide();
}

void Player::process_mouse() {
    godot::Vector2 mouse_motion{};
    for (const auto &motion : input_buffer) {
        mouse_motion += motion;
    }
    input_buffer.clear();

    rotate_y(godot::UtilityFunctions::deg_to_rad(-mouse_motion.x * sensitivity));
    pivot->rotate_x(godot::UtilityFunctions::deg_to_rad(-mouse_motion.y * sensitivity));
    godot::Vector3 pivot_rot = pivot->get_rotation();
    pivot->set_rotation(
        godot::Vector3(
            godot::Math::clamp((double)pivot_rot.x, -Math_PI / 2.0, Math_PI / 2.0), pivot_rot.y,
            pivot_rot.z
        )
    );
}

void Player::_unhandled_input(const godot::Ref<godot::InputEvent> &event) {
    if (godot::Engine::get_singleton()->is_editor_hint()) { return; }
    if (get_tree()->is_paused()) { return; }

    auto ptr = godot::Object::cast_to<godot::InputEventMouseMotion>(event.ptr());
    if (ptr && !ptr->get_screen_relative().is_zero_approx()) {
        input_buffer.emplace_back(ptr->get_screen_relative());
    }
}

godot::Node3D *Player::get_pivot() {
    return pivot;
}
void Player::set_pivot(godot::Node3D *in_pivot) {
    pivot = in_pivot;
}
float Player::get_sensitivity() {
    return sensitivity;
}
void Player::set_sensitivity(float in_sensitivity) {
    sensitivity = in_sensitivity;
}
float Player::get_speed() {
    return speed;
}
void Player::set_speed(float in_speed) {
    speed = in_speed;
}

} // namespace sample
