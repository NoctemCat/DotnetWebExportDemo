#include "godot_cpp/core/object.hpp"
#include "player.hpp"
#include <iostream>
#include <libgodot.h>

#include <godot_cpp/classes/godot_instance.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>
#include <godot_cpp/godot.hpp>

#include <string>
#include <vector>

extern "C" {

static void initialize_default_module(godot::ModuleInitializationLevel p_level) {
    if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) { return; }

    GDREGISTER_CLASS(sample::Player);
}

static void uninitialize_default_module(godot::ModuleInitializationLevel p_level) {
    if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) { return; }
}

GDExtensionBool GDE_EXPORT gdextension_default_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization *r_initialization
) {
    godot::GDExtensionBinding::InitObject init_object(
        p_get_proc_address, p_library, r_initialization
    );

    init_object.register_initializer(initialize_default_module);
    init_object.register_terminator(uninitialize_default_module);
    init_object.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_object.init();
}
}

class LibGodot {
public:
    LibGodot() {}

    godot::GodotInstance *create_godot_instance(
        int p_argc, char *p_argv[],
        GDExtensionInitializationFunction p_init_func = gdextension_default_init
    ) {

        GDExtensionObjectPtr instance = libgodot_create_godot_instance(p_argc, p_argv, p_init_func);
        if (instance == nullptr) { return nullptr; }
        // ::godot::internal::_call_native_mb_ret();
        return reinterpret_cast<godot::GodotInstance *>(
            godot::internal::get_object_instance_binding(instance)
        );
    }

    void destroy_godot_instance(godot::GodotInstance *instance) {
        libgodot_destroy_godot_instance(instance->_owner);
    }

private:
    void *handle = nullptr;
    GDExtensionObjectPtr (*func_libgodot_create_godot_instance)(
        int, char *[], GDExtensionInitializationFunction
    ) = nullptr;
    void (*func_libgodot_destroy_godot_instance)(GDExtensionObjectPtr) = nullptr;
};

extern "C" bool godotsharp_game_main_init(void *, void *, const void **, int32_t) {
    return false;
}

int main(int argc, char **argv) {
    std::cout << ">>>>>>> Custom main\n";
    LibGodot libgodot;

    godot::GodotInstance *instance = libgodot.create_godot_instance(argc, argv);
    if (instance == nullptr) {
        fprintf(stderr, "Error creating Godot instance\n");
        return EXIT_FAILURE;
    }
    std::cout << ">>>>>>> Custom init\n";
    instance->start();

    std::cout << ">>>>>>> Custom start\n";
    while (!instance->iteration()) {}
    libgodot.destroy_godot_instance(instance);

    return EXIT_SUCCESS;
}