#include "tests/common.hpp"

#include "include/UTL/strong_type.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// Handle state tracker decoupled from the lifetime of the actual handle so we can test it
struct handle_state {
    bool initialized = false;
    bool destroyed   = false;
};

// Handle with init & destroy functions to simulate a system API
struct abstract_handle {
    handle_state* state;
};

abstract_handle create_handle(handle_state& state) {
    abstract_handle handle{};

    handle.state              = &state;
    handle.state->initialized = true;

    return handle;
}

void destroy_handle(abstract_handle& handle) {
    handle.state->destroyed = true;
    handle.state            = nullptr;
}

TEST_CASE("Unique / Abstract handle") {
    using handle = strong_type::unique<abstract_handle, class HandleTag, strong_type::bind<&destroy_handle>>;

    handle_state state;

    {
        CHECK(!state.initialized);

        handle handle = create_handle(state);
        CHECK(state.initialized);

        CHECK(!state.destroyed);
    }
    
    CHECK(state.destroyed);
}