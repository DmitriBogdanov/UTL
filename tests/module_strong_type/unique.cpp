#include "tests/common.hpp"

#include "include/UTL/strong_type.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// Handle state tracker decoupled from the lifetime of the actual handle so we can test it
struct State {
    bool initialized = false;
    bool destroyed   = false;
};

// Handle with init & destroy functions to simulate a system API
struct AbstractHandle {
    State* state;
};

AbstractHandle create_handle(State& state) {
    AbstractHandle handle{};

    handle.state              = &state;
    handle.state->initialized = true;

    return handle;
}

void destroy_handle(AbstractHandle& handle) {
    handle.state->destroyed = true;
    handle.state            = nullptr;
}

TEST_CASE("Unique / Abstract handle") {
    using Handle = strong_type::Unique<AbstractHandle, class HandleTag, strong_type::Bind<&destroy_handle>>;

    State state;

    {
        CHECK(!state.initialized);

        Handle handle = create_handle(state);
        CHECK(state.initialized);

        CHECK(!state.destroyed);
    }
    CHECK(state.destroyed);
}