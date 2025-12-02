#ifndef ORCH_CORE_MAX_TASK_SLOTS
#define ORCH_CORE_MAX_TASK_SLOTS 12
#endif

// Type definition for a task function pointer used by the orchestrator.
typedef void (*TaskExecutionPtr)(void);

// Structure representing an individual task entry within the orchestrator's management.
typedef struct {
    TaskExecutionPtr func_pointer;    // Pointer to the actual function implementing the task logic.
    unsigned char is_task_enabled;    // Flag (1 for enabled, 0 for disabled) to control task execution.
    unsigned char task_priority_level; // Numerical priority (0-255), higher value typically means higher priority.
} OrchestratorTaskEntry_t;

// Main context structure for the orchestrator core, managing all tasks.
typedef struct {
    OrchestratorTaskEntry_t task_slots[ORCH_CORE_MAX_TASK_SLOTS]; // Array of predefined slots for tasks.
    unsigned char num_tasks_registered;     // Current count of tasks that have been successfully enrolled.
    unsigned char last_executed_slot_idx;   // Index of the last task slot considered for execution in a sequential strategy.
} OrchestratorCoreContext_t;

// Static global instance of the orchestrator core context, accessible throughout the module.
static OrchestratorCoreContext_t g_orchestrator_system_core;

// Initializes the orchestrator system, clearing all task slots and resetting counters.
void Orchestrator_SetupSystem(void) {
    for (unsigned char i = 0; i < ORCH_CORE_MAX_TASK_SLOTS; ++i) {
        g_orchestrator_system_core.task_slots[i].func_pointer = (TaskExecutionPtr)0; // Set function pointer to NULL.
        g_orchestrator_system_core.task_slots[i].is_task_enabled = 0;               // Disable task by default.
        g_orchestrator_system_core.task_slots[i].task_priority_level = 0;          // Set default priority.
    }
    g_orchestrator_system_core.num_tasks_registered = 0;
    g_orchestrator_system_core.last_executed_slot_idx = 0;
}

// Enrolls a new task into an available slot within the orchestrator.
// Returns 1 on successful enrollment, 0 if the function pointer is NULL or no slots are free.
unsigned char Orchestrator_EnrollTask(TaskExecutionPtr func_ptr, unsigned char priority) {
    if (func_ptr == (TaskExecutionPtr)0 || g_orchestrator_system_core.num_tasks_registered >= ORCH_CORE_MAX_TASK_SLOTS) {
        return 0; // Cannot enroll: invalid function or no more available slots.
    }

    // Search for the first empty slot (identified by a NULL function pointer).
    for (unsigned char slot_idx = 0; slot_idx < ORCH_CORE_MAX_TASK_SLOTS; ++slot_idx) {
        if (g_orchestrator_system_core.task_slots[slot_idx].func_pointer == (TaskExecutionPtr)0) {
            // Found an empty slot; populate it with the new task's details.
            g_orchestrator_system_core.task_slots[slot_idx].func_pointer = func_ptr;
            g_orchestrator_system_core.task_slots[slot_idx].is_task_enabled = 1; // Enable the task upon enrollment.
            g_orchestrator_system_core.task_slots[slot_idx].task_priority_level = priority;
            g_orchestrator_system_core.num_tasks_registered++;
            return 1; // Task successfully enrolled.
        }
    }
    return 0; // Should not be reached if num_tasks_registered check is accurate, but serves as a fallback.
}

// Executes a single cycle of the orchestrator.
// This variant iterates through all task slots and executes any task that is enrolled and enabled.
void Orchestrator_PerformCycle(void) {
    if (g_orchestrator_system_core.num_tasks_registered == 0) {
        return; // No tasks are registered, so nothing to perform.
    }

    // Iterate through all possible task slots.
    for (unsigned char current_slot_idx = 0; current_slot_idx < ORCH_CORE_MAX_TASK_SLOTS; ++current_slot_idx) {
        // Check if the current slot contains an enrolled task and if that task is enabled.
        if (g_orchestrator_system_core.task_slots[current_slot_idx].is_task_enabled &&
            g_orchestrator_system_core.task_slots[current_slot_idx].func_pointer != (TaskExecutionPtr)0) {

            // Execute the task function found in the current slot.
            g_orchestrator_system_core.task_slots[current_slot_idx].func_pointer();
        }
    }
    // The 'last_executed_slot_idx' is maintained in the context but not explicitly used in this specific 'PerformCycle'
    // implementation, as it performs a full sweep rather than a single task execution.
}

// Sets the enable/disable state for a task located at a specific slot index.
// Returns 1 on success, 0 if the index is out of bounds or the slot is empty.
unsigned char Orchestrator_SetTaskEnableState(unsigned char slot_index, unsigned char enable_state) {
    if (slot_index >= ORCH_CORE_MAX_TASK_SLOTS ||
        g_orchestrator_system_core.task_slots[slot_index].func_pointer == (TaskExecutionPtr)0) {
        return 0; // Invalid slot index or no task enrolled at this slot.
    }
    // Assign the new enable state, ensuring it is either 0 or 1.
    g_orchestrator_system_core.task_slots[slot_index].is_task_enabled = (enable_state != 0);
    return 1; // State successfully updated.
}