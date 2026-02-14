#include "godot_cpp/core/object.hpp"
#include "player.hpp"
#include <iostream>
#include <libgodot.h>

#include <godot_cpp/classes/godot_instance.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>
#include <godot_cpp/godot.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

// godot_js.h
extern "C" char *godot_js_emscripten_get_version();
extern "C" void godot_js_os_finish_async(void (*p_callback)());

void print_web_header() {
    using namespace godot;
    // Emscripten.
    char *emscripten_version_char = godot_js_emscripten_get_version();
    godot::String emscripten_version = godot::vformat("Emscripten %s", emscripten_version_char);
    // `free()` is used here because it's not memory that was allocated by Godot.
    free(emscripten_version_char);

    // Build features.
    String thread_support =
        OS::get_singleton()->has_feature("threads") ? "multi-threaded" : "single-threaded";
    String extensions_support = OS::get_singleton()->has_feature("web_extensions")
                                    ? "GDExtension support"
                                    : "no GDExtension support";

    PackedStringArray build_configuration = {
        emscripten_version, thread_support, extensions_support
    };
    print_line(vformat("Build configuration: %s.", String(", ").join(build_configuration)));
}
#endif

extern "C" {
static void initialize_default_module(godot::ModuleInitializationLevel p_level) {
#ifdef __EMSCRIPTEN__
    if (p_level == godot::MODULE_INITIALIZATION_LEVEL_CORE) { print_web_header(); }
    if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        godot::ResourceLoader::get_singleton()->set_abort_on_missing_resources(false);
    }
#endif
    if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        GDREGISTER_CLASS(sample::PlayerNative);
    }
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

    static godot::GodotInstance *create_godot_instance(
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

    static void destroy_godot_instance(godot::GodotInstance *instance) {
        libgodot_destroy_godot_instance(instance->_owner);
    }
};

#ifndef __EMSCRIPTEN__

int main(int argc, char **argv) {
    std::cout << ">>>>>>> Custom main\n";

    godot::GodotInstance *instance = LibGodot::create_godot_instance(argc, argv);
    if (instance == nullptr) {
        fprintf(stderr, "Error creating Godot instance\n");
        return EXIT_FAILURE;
    }
    std::cout << ">>>>>>> Custom init\n";
    instance->start();

    std::cout << ">>>>>>> Custom start\n";
    while (!instance->iteration()) {}
    LibGodot::destroy_godot_instance(instance);

    return EXIT_SUCCESS;
}
#else

static godot::GodotInstance *instance = nullptr;
static bool shutdown_complete = false;

void exit_callback() {
    if (!shutdown_complete) {
        return; // Still waiting.
    }
    if (instance != nullptr) {
        std::cout << ">>>>>>> Custom destroy\n";
        LibGodot::destroy_godot_instance(instance);
        instance = nullptr;
    }

    emscripten_force_exit(EXIT_SUCCESS);
}

void cleanup_after_sync() {
    shutdown_complete = true;
}

void main_loop_callback() {
    if (instance->iteration()) {
        emscripten_cancel_main_loop();
        emscripten_set_main_loop(exit_callback, -1, false);
        godot_js_os_finish_async(cleanup_after_sync);
    }
}

int main(int argc, char **argv) {
    std::cout << ">>>>>>> Custom main\n";

    instance = LibGodot::create_godot_instance(argc, argv);
    if (instance == nullptr) {
        fprintf(stderr, "Error creating Godot instance\n");
        return EXIT_FAILURE;
    }
    std::cout << ">>>>>>> Custom init\n";

    instance->start();
    std::cout << ">>>>>>> Custom start\n";
    emscripten_set_main_loop(main_loop_callback, -1, false);
    main_loop_callback();
    return EXIT_SUCCESS;
}
#endif