typedef struct {
    int current_state;
    unsigned int active_tasks;
    // Potentially other shared resources or flags
} OrchestratorState;

void orchestrator_initialize(OrchestratorState* state_ptr, int initial_state_value) {
    if (state_ptr != ((void*)0)) {
        state_ptr->current_state = initial_state_value;
        state_ptr->active_tasks = 0;
    }
}

int orchestrator_dispatch_event(OrchestratorState* state_ptr, int event_id) {
    if (state_ptr == ((void*)0)) {
        return -1; // Error: invalid state pointer
    }

    // A simple state machine logic
    switch (state_ptr->current_state) {
        case 0: // Idle state
            if (event_id == 100) {
                state_ptr->current_state = 1; // Transition to working state
                state_ptr->active_tasks++;
                return 0; // Success
            }
            break;
        case 1: // Working state
            if (event_id == 200) {
                state_ptr->active_tasks++;
                return 0; // Success, another task started
            } else if (event_id == 300) {
                if (state_ptr->active_tasks > 0) {
                    state_ptr->active_tasks--;
                }
                if (state_ptr->active_tasks == 0) {
                    state_ptr->current_state = 0; // Back to idle
                }
                return 0; // Success, task completed
            }
            break;
        case 2: // Error state
            // Perhaps only a reset event can change this state
            if (event_id == 999) {
                state_ptr->current_state = 0;
                state_ptr->active_tasks = 0;
                return 0; // Success: reset
            }
            break;
    }

    return 1; // Event not handled in current state
}

int orchestrator_get_current_state(const OrchestratorState* state_ptr) {
    if (state_ptr != ((void*)0)) {
        return state_ptr->current_state;
    }
    return -1; // Error: invalid state pointer
}

unsigned int orchestrator_get_active_tasks_count(const OrchestratorState* state_ptr) {
    if (state_ptr != ((void*)0)) {
        return state_ptr->active_tasks;
    }
    return 0; // Error or no tasks
}

void orchestrator_cleanup(OrchestratorState* state_ptr) {
    if (state_ptr != ((void*)0)) {
        state_ptr->current_state = -1; // Indicate a terminated state
        state_ptr->active_tasks = 0;
    }
}