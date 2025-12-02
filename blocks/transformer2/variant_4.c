typedef struct {
    const float* param_matrix;
    const float* offset_vector;
    int input_count;
    int output_count;
} xfmr2_params_t;

int xfmr2_setup_module(xfmr2_params_t* config, const float* matrix_ref, const float* bias_ref, int in_c, int out_c) {
    if (!config || !matrix_ref || !bias_ref || in_c <= 0 || out_c <= 0) {
        return -1;
    }
    config->param_matrix = matrix_ref;
    config->offset_vector = bias_ref;
    config->input_count = in_c;
    config->output_count = out_c;
    return 0;
}

int xfmr2_run_forward(const xfmr2_params_t* config, const float* input_array, float* output_array) {
    if (!config || !input_array || !output_array || config->input_count <= 0 || config->output_count <= 0) {
        return -1;
    }

    const int INPUT_DIM = config->input_count;
    const int OUTPUT_DIM = config->output_count;

    for (int idx_out = 0; idx_out < OUTPUT_DIM; ++idx_out) {
        output_array[idx_out] = config->offset_vector[idx_out];
    }

    for (int idx_in = 0; idx_in < INPUT_DIM; ++idx_in) {
        const float current_input_value = input_array[idx_in];
        for (int idx_out = 0; idx_out < OUTPUT_DIM; ++idx_out) {
            output_array[idx_out] += current_input_value * config->param_matrix[idx_out * INPUT_DIM + idx_in];
        }
    }

    return 0;
}