typedef float REAL_TYPE;
typedef int INDEX_TYPE;

void transform_data(REAL_TYPE* output_buffer, const REAL_TYPE* input_buffer, const REAL_TYPE* kernel_weights, INDEX_TYPE input_dim, INDEX_TYPE output_dim) {
    register INDEX_TYPE k_out = 0;
    while (k_out < output_dim) {
        REAL_TYPE accumulated_val = 0.0f;
        for (register INDEX_TYPE k_in = 0; k_in < input_dim; ++k_in) {
            REAL_TYPE input_val = *(input_buffer + k_in);
            REAL_TYPE weight_val = kernel_weights[k_out * input_dim + k_in];
            accumulated_val += input_val * weight_val;
        }
        *(output_buffer + k_out) = accumulated_val;
        k_out++;
    }
}