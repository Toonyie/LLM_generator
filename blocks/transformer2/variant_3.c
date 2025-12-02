#include <stddef.h> 

void apply_transformer2_variant3(const float* restrict input_data_ptr,
                                 float* restrict output_data_ptr,
                                 size_t data_length,
                                 float smoothing_factor) {
    if (data_length == 0 || smoothing_factor < 0.0f || smoothing_factor > 1.0f) {
        return;
    }

    output_data_ptr[0] = input_data_ptr[0];

    size_t k = 1;
    if (data_length > 1) {
        do {
            float current_input_val = input_data_ptr[k];
            float previous_output_val = output_data_ptr[k - 1];
            
            output_data_ptr[k] = (current_input_val * smoothing_factor) + (previous_output_val * (1.0f - smoothing_factor));
            k++;
        } while (k < data_length);
    }
}