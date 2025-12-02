typedef enum {
    ORCH_TASK_IDLE,
    ORCH_TASK_RUNNING,
    ORCH_TASK_PAUSED,
    ORCH_TASK_DONE
} OrchestratorTaskStatus;

typedef struct {
    unsigned int task_id;
    OrchestratorTaskStatus status;
    void* task_data;
    unsigned long last_execution_time;
} OrchestratorTask;

int orchestrator_schedule_task(OrchestratorTask* task_list, unsigned int max_tasks, OrchestratorTask* new_task) {
    if (task_list == NULL || new_task == NULL) {
        return -1; // Invalid input
    }

    for (unsigned int i = 0; i < max_tasks; ++i) {
        if (task_list[i].status == ORCH_TASK_IDLE) { // Find an empty slot
            task_list[i].task_id = new_task->task_id;
            task_list[i].status = new_task->status;
            task_list[i].task_data = new_task->task_data;
            task_list[i].last_execution_time = 0; // Initialize execution time
            return (int)i; // Return the index of the scheduled task
        }
    }
    return -2; // No available slots
}