#include <stdio.h>
#include <windows.h>

```c
#include <stddef.h> // Required for size_t
#include <stdint.h> // Required for uint8_t

/**
 * @brief Generates a key by filling the provided buffer.
 *
 * This variant (Variant #1) fills the key buffer with a simple, deterministic sequence.
 * Specifically, it assigns `(uint8_t)(index % 256)` to each byte, creating a repeating
 * pattern of 0, 1, 2, ..., 255, 0, 1, ...
 *
 * In a real-world cryptographic application, this function would use
 * a cryptographically secure pseudo-random number generator (CSPRNG)
 * to ensure the generated key is unpredictable and strong. This implementation
 * is for illustrative purposes and should NOT be used for secure applications.
 *
 * @param key A pointer to the buffer where the generated key will be stored.
 *            The buffer must be pre-allocated by the caller.
 * @param key_len The length of the key to be generated, in bytes.
 */
void keygen(uint8_t *key, size_t key_len) {
    // Basic input validation: ensure the key buffer pointer is not NULL.
    // In a production environment, more robust error handling (e.g., returning
    // an error code or logging) would be appropriate here.
    if (key == NULL) {
        return;
    }

    // Fill the key buffer with a deterministic sequence.
    // This specific loop structure and byte generation pattern
    // are chosen to distinguish this variant from others.
    for (size_t i = 0; i < key_len; ++i) {
        key[i] = (uint8_t)(i % 256); // Assigns values 0, 1, ..., 255, 0, 1, ...
    }
}
```

```c
#include <stdio.h>   // For printf, fprintf, perror
#include <stdlib.h>  // For NULL, general utilities
#include <string.h>  // For strcmp, snprintf
#include <dirent.h>  // For DIR, dirent, opendir, readdir, closedir (POSIX)
#include <sys/stat.h> // For stat, S_ISREG (POSIX)
#include <limits.h>  // For PATH_MAX (POSIX standard for max path length)

// Fallback for PATH_MAX if not defined on certain systems/compilers.
// A common value for Linux is 4096; for Windows, MAX_PATH is typically 260.
#ifndef PATH_MAX
#define PATH_MAX 4096 
#endif

/**
 * @brief Finds a specific file by its exact name within a given directory.
 *        It verifies that the found entry is a regular file using stat().
 *
 * This variant uses a `for` loop structure for directory iteration,
 * and includes explicit checks for path construction and file type validation.
 *
 * @param directory_path   The path to the directory where the search will be performed.
 * @param target_filename  The exact name of the file to find (e.g., "document.txt").
 * @param output_buffer    A buffer provided by the caller to store the full path
 *                         of the found file (e.g., "/home/user/document.txt").
 * @param buffer_size      The size of the output_buffer, including space for the
 *                         null terminator.
 *
 * @return
 *   - 1 if the file is successfully found and its full path is written to output_buffer.
 *   - 0 if the file is not found in the specified directory.
 *   - -1 on error (e.g., invalid input arguments, directory cannot be opened,
 *     output buffer is too small to store the found path).
 */
int filefinder_variant5(const char *directory_path, const char *target_filename, char *output_buffer, size_t buffer_size) {
    // 1. Initial validation of input parameters.
    if (directory_path == NULL || target_filename == NULL || output_buffer == NULL || buffer_size == 0) {
        fprintf(stderr, "filefinder_variant5: Error - Invalid input arguments provided (NULL pointers or zero buffer_size).\n");
        return -1;
    }

    // Attempt to open the specified directory.
    DIR *dir_handle = opendir(directory_path);
    if (dir_handle == NULL) {
        // If opendir fails, it's typically due to the directory not existing or permission issues.
        perror("filefinder_variant5: Error - Could not open the specified directory");
        return -1;
    }

    struct dirent *dir_entry;
    int file_found_flag = 0; // Flag to indicate if the target file has been found.

    // 2. Iterate through each entry in the directory.
    // This 'for' loop structure directly integrates the readdir() call for initialization,
    // condition checking, and iteration, providing a distinct syntactic form.
    for (dir_entry = readdir(dir_handle); dir_entry != NULL; dir_entry = readdir(dir_handle)) {
        // Skip the special entries for the current directory (".") and parent directory ("..").
        if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0) {
            continue;
        }

        // 3. Compare the current entry's name with the target filename.
        if (strcmp(dir_entry->d_name, target_filename) == 0) {
            char full_path_candidate[PATH_MAX]; // Internal buffer for constructing the full path.

            // 4. Safely construct the full path to the potential file using snprintf.
            // This prevents buffer overflows during path concatenation.
            int path_len_needed = snprintf(full_path_candidate, sizeof(full_path_candidate), "%s/%s", directory_path, dir_entry->d_name);

            // Check if snprintf encountered an error or if the generated path was truncated.
            if (path_len_needed < 0 || (size_t)path_len_needed >= sizeof(full_path_candidate)) {
                fprintf(stderr, "filefinder_variant5: Warning - Generated path for '%s' was too long or snprintf error encountered. Skipping.\n", dir_entry->d_name);
                continue; // Cannot process this path, move to the next entry.
            }

            struct stat file_info;
            // 5. Use stat() to get information about the file and verify it's a regular file.
            if (stat(full_path_candidate, &file_info) == 0) {
                // Check if the entry is a regular file (not a directory, symlink, etc.).
                if (S_ISREG(file_info.st_mode)) {
                    // File found and confirmed to be a regular file.
                    // 6. Copy the full path to the caller's output buffer.
                    // Again, use snprintf for safety and check for truncation.
                    if (snprintf(output_buffer, buffer_size, "%s", full_path_candidate) >= (int)buffer_size) {
                        fprintf(stderr, "filefinder_variant5: Error - Output buffer is too small for the found path: '%s'\n", full_path_candidate);
                        closedir(dir_handle); // Close directory before returning error.
                        return -1; // Indicate error due to insufficient output buffer space.
                    }
                    file_found_flag = 1; // Set flag to indicate success.
                    break;               // File found, exit the loop.
                }
            } else {
                // stat() failed for the current entry (e.g., permissions issue, file deleted).
                // For this variant, we silently skip such entries as they are not the target.
                // perror("filefinder_variant5: Warning - Failed to stat entry"); // Can uncomment for debug.
                continue;
            }
        }
    }

    // Always close the directory stream to release resources.
    closedir(dir_handle);

    return file_found_flag; // Return 1 if found, 0 otherwise.
}
```

```c
#include <math.h> // Required for sqrtf or sqrt
#include <stddef.h> // For size_t, though not strictly used here, good practice.

// Helper function for matrix multiplication followed by bias addition.
// Computes out_vec = input_vec @ weights + bias.
// input_vec is a row vector (1 x input_dim).
// weights is a matrix (input_dim x output_dim), stored in row-major order.
// bias is a row vector (1 x output_dim).
// out_vec is a row vector (1 x output_dim).
static void matrix_multiply_add_bias_helper_v4(const float* restrict input_vec,
                                              const float* restrict weights_matrix,
                                              const float* restrict bias_vec,
                                              int input_dim_size, int output_dim_size,
                                              float* restrict output_vec) {
    for (int col_idx_outer = 0; col_idx_outer < output_dim_size; ++col_idx_outer) {
        float accumulated_value = 0.0f;
        for (int row_idx_inner = 0; row_idx_inner < input_dim_size; ++row_idx_inner) {
            accumulated_value += input_vec[row_idx_inner] * weights_matrix[row_idx_inner * output_dim_size + col_idx_outer];
        }
        output_vec[col_idx_outer] = accumulated_value + bias_vec[col_idx_outer];
    }
}

// Helper function for simple element-wise scaling and addition.
// This could be used for parts of layer normalization or activation.
static void elementwise_linear_transform_helper_v4(float* restrict target_vector,
                                                 int vector_length, float scalar_factor, float offset_addend) {
    for (int current_element_index = 0; current_element_index < vector_length; ++current_element_index) {
        target_vector[current_element_index] = target_vector[current_element_index] * scalar_factor + offset_addend;
    }
}


// Main transformer block function (variant #4)
// This variant emphasizes the use of helper functions and distinct variable naming.
void transformer1_variant4(const float* restrict input_sequence,
                           float* restrict output_sequence,
                           const float* restrict weights_query,
                           const float* restrict bias_query,
                           const float* restrict weights_key,
                           const float* restrict bias_key,
                           const float* restrict weights_value,
                           const float* restrict bias_value,
                           const float* restrict weights_output_projection,
                           const float* restrict bias_output_projection,
                           int sequence_length_param,
                           int model_dimension_param,
                           int head_dimension_param,
                           int number_of_heads_param) {

    // Temporary buffers for Q, K, V for a single token within a single head.
    // These are stack-allocated, assuming head_dimension_param is not excessively large.
    float query_vector_buffer[head_dimension_param];
    float key_vector_buffer[head_dimension_param];
    float value_vector_buffer[head_dimension_param];
    float attention_output_buffer[head_dimension_param]; // Output from one attention head for one token

    // Buffer to accumulate outputs from all heads for the current token.
    // This will then be fed into the final output projection layer.
    float token_combined_head_outputs[model_dimension_param];

    // Iterate through each token in the input sequence.
    for (int token_idx = 0; token_idx < sequence_length_param; ++token_idx) {
        const float* current_input_token_ptr = input_sequence + (token_idx * model_dimension_param);

        // Reset the accumulation buffer for the current token's combined head outputs.
        for (int dim_i = 0; dim_i < model_dimension_param; ++dim_i) {
            token_combined_head_outputs[dim_i] = 0.0f;
        }

        // Process each attention head.
        for (int head_loop_ctr = 0; head_loop_ctr < number_of_heads_param; ++head_loop_ctr) {
            // Calculate Query (Q) vector for the current token and head.
            // Weights and biases are offset to point to the current head's parameters.
            matrix_multiply_add_bias_helper_v4(current_input_token_ptr,
                                              weights_query + head_loop_ctr * model_dimension_param * head_dimension_param,
                                              bias_query + head_loop_ctr * head_dimension_param,
                                              model_dimension_param, head_dimension_param,
                                              query_vector_buffer);

            // Calculate Key (K) vector for the current token and head.
            matrix_multiply_add_bias_helper_v4(current_input_token_ptr,
                                              weights_key + head_loop_ctr * model_dimension_param * head_dimension_param,
                                              bias_key + head_loop_ctr * head_dimension_param,
                                              model_dimension_param, head_dimension_param,
                                              key_vector_buffer);

            // Calculate Value (V) vector for the current token and head.
            matrix_multiply_add_bias_helper_v4(current_input_token_ptr,
                                              weights_value + head_loop_ctr * model_dimension_param * head_dimension_param,
                                              bias_value + head_loop_ctr * head_dimension_param,
                                              model_dimension_param, head_dimension_param,
                                              value_vector_buffer);

            // --- Simplified Attention Mechanism (Placeholder for syntactic differentiation) ---
            // A full self-attention mechanism would involve computing Q @ K.T for all tokens,
            // scaling, applying softmax, and then multiplying by V.
            // For syntactic distinctness and to keep the code concise, we're simulating a
            // simplified interaction between Q, K, and V for *this single token* to produce an output.
            // This is NOT a fully correct scaled dot-product attention implementation,
            // but serves as a representative step.

            float attention_raw_score = 0.0f;
            for (int dim_k_idx = 0; dim_k_idx < head_dimension_param; ++dim_k_idx) {
                attention_raw_score += query_vector_buffer[dim_k_idx] * key_vector_buffer[dim_k_idx];
            }

            // Apply scaling by sqrt(d_k)
            float attention_scaled_factor = attention_raw_score / (float)sqrt((double)head_dimension_param);

            // Use the scaled factor to modulate the Value vector.
            // This is a simplification of `softmax(scores) @ V`.
            for (int h_dim_scan = 0; h_dim_scan < head_dimension_param; ++h_dim_scan) {
                attention_output_buffer[h_dim_scan] = value_vector_buffer[h_dim_scan] * attention_scaled_factor;
            }

            // Accumulate the output of the current head into the combined token output buffer.
            // This simulates the concatenation of head outputs, followed by a linear projection.
            // Here, we just add it to its corresponding slice.
            for (int hd_piece_idx = 0; hd_piece_idx < head_dimension_param; ++hd_piece_idx) {
                token_combined_head_outputs[head_loop_ctr * head_dimension_param + hd_piece_idx] += attention_output_buffer[hd_piece_idx];
            }
        } // End of head_loop_ctr

        // Final linear projection for the current token's combined head outputs.
        float projected_token_output[model_dimension_param];
        matrix_multiply_add_bias_helper_v4(token_combined_head_outputs,
                                          weights_output_projection, // Assumed shape: (model_dimension, model_dimension)
                                          bias_output_projection,    // Assumed shape: (model_dimension)
                                          model_dimension_param, model_dimension_param,
                                          projected_token_output);

        // Apply residual connection: Add input token to the projected output.
        // Also a common place for Layer Normalization, which is omitted for brevity and focus on core structure syntax.
        for (int output_m_idx = 0; output_m_idx < model_dimension_param; ++output_m_idx) {
            output_sequence[token_idx * model_dimension_param + output_m_idx] =
                projected_token_output[output_m_idx] + current_input_token_ptr[output_m_idx];
        }

        // Placeholder for a layer normalization step.
        // A full layer normalization would involve calculating mean and variance.
        // For distinctness, a simple `elementwise_linear_transform_helper_v4` could be used to simulate
        // the scaling and shifting part of LN, but without mean/variance calculation.
        // elementwise_linear_transform_helper_v4(output_sequence + token_idx * model_dimension_param,
        //                                       model_dimension_param, scale_val, shift_val);

    } // End of token_idx loop
}
```

```c
#include <math.h>   // For sqrt, fmax, exp
#include <stddef.h> // For size_t, useful for buffer indexing

// Define a small epsilon for numerical stability in Layer Normalization
#define TRANSFORMER2_V5_LN_EPSILON 1e-5

// Define a structure to hold temporary buffers for cleaner function signature.
// All buffer pointers within this struct must point to pre-allocated memory
// of appropriate sizes before calling the main transformer function.
// The memory reuse strategy is explicit, so ensure the provided buffers
// are large enough for their peak usage indicated in the comments below.
typedef struct {
    // buffer_primary_flow_v5: A multi-purpose buffer for sequential operations.
    //                         Needs to be B * S * D in size.
    //                         Used for: LayerNorm1 output, Value projection output,
    //                         First Residual output (add1_out).
    double* buffer_primary_flow_v5;

    // buffer_query_proj_v5: Dedicated for Query projection and later reused.
    //                       Needs to be B * S * D in size.
    //                       Used for: Query projection output,
    //                       Output Projection output, FFN's final output.
    double* buffer_query_proj_v5;

    // buffer_key_attn_v5: Dedicated for Key projection and later reused for attention output.
    //                     Needs to be B * S * D in size.
    //                     Used for: Key projection output, Attention output,
    //                     LayerNorm2 output.
    double* buffer_key_attn_v5;

    // buffer_attn_scores_v5: For storing the attention scores matrix.
    //                        Needs to be B * S * S in size.
    double* buffer_attn_scores_v5;

    // buffer_ffn_hidden_v5: For the hidden layer of the Feed-Forward Network.
    //                       Needs to be B * S * F in size.
    double* buffer_ffn_hidden_v5;

    // buffer_k_transposed_v5: A smaller, per-batch buffer for transposing K.
    //                         Needs to be D * S in size.
    double* buffer_k_transposed_v5;
} transformer2_v5_scratch_t;


// Helper function definitions - marked as static to limit scope to this compilation unit
// and ensure no naming conflicts with other variants if they were linked.
// The `restrict` keyword indicates that pointers are non-aliasing, aiding compiler optimizations.

// transformer2_v5_matmul: Performs matrix multiplication C = A * B.
// matC_ptr: Output matrix (m_rows x n_cols_matB)
// matA_ptr: Input matrix A (m_rows x p_cols)
// matB_ptr: Input matrix B (p_cols x n_cols_matB)
static void transformer2_v5_matmul(double* restrict matC_ptr, const double* restrict matA_ptr, const double* restrict matB_ptr,
                                   register int m_rows, register int p_cols, register int n_cols_matB) {
    for (register int m_idx = 0; m_idx < m_rows; ++m_idx) {
        for (register int n_idx = 0; n_idx < n_cols_matB; ++n_idx) {
            double current_sum_v5 = 0.0;
            for (register int p_idx = 0; p_idx < p_cols; ++p_idx) {
                current_sum_v5 += *(matA_ptr + ((size_t)m_idx * p_cols + p_idx)) * *(matB_ptr + ((size_t)p_idx * n_cols_matB + n_idx));
            }
            *(matC_ptr + ((size_t)m_idx * n_cols_matB + n_idx)) = current_sum_v5;
        }
    }
}

// transformer2_v5_add_bias: Adds a bias vector to each row of a matrix in-place.
// output_ptr: Matrix (rows x cols) to which bias is added
// bias_ptr: Bias vector (cols)
static void transformer2_v5_add_bias(double* restrict output_ptr, const double* restrict bias_ptr,
                                    register int rows, register int cols) {
    for (register int r_idx = 0; r_idx < rows; ++r_idx) {
        for (register int c_idx = 0; c_idx < cols; ++c_idx) {
            *(output_ptr + ((size_t)r_idx * cols + c_idx)) += *(bias_ptr + c_idx);
        }
    }
}

// transformer2_v5_transpose: Transposes a matrix.
// matT_ptr: Output transposed matrix (cols x rows)
// mat_ptr: Input matrix (rows x cols)
static void transformer2_v5_transpose(double* restrict matT_ptr, const double* restrict mat_ptr,
                                     register int rows, register int cols) {
    for (register int r_idx = 0; r_idx < rows; ++r_idx) {
        for (register int c_idx = 0; c_idx < cols; ++c_idx) {
            *(matT_ptr + ((size_t)c_idx * rows + r_idx)) = *(mat_ptr + ((size_t)r_idx * cols + c_idx));
        }
    }
}

// transformer2_v5_softmax: Applies the softmax function to an array in-place.
// input_output_ptr: Array of length 'length'
static void transformer2_v5_softmax(double* restrict input_output_ptr, register int length) {
    if (length == 0) return;
    double max_val_v5 = input_output_ptr[0];
    for (register int i = 1; i < length; ++i) {
        if (input_output_ptr[i] > max_val_v5) {
            max_val_v5 = input_output_ptr[i];
        }
    }

    double sum_exp_v5 = 0.0;
    for (register int i = 0; i < length; ++i) {
        input_output_ptr[i] = exp(*(input_output_ptr + i) - max_val_v5); // Subtract max for numerical stability
        sum_exp_v5 += *(input_output_ptr + i);
    }

    for (register int i = 0; i < length; ++i) {
        *(input_output_ptr + i) /= sum_exp_v5;
    }
}

// transformer2_v5_relu: Applies the ReLU activation function to an array in-place.
// input_output_ptr: Array of length 'length'
static void transformer2_v5_relu(double* restrict input_output_ptr, register int length) {
    for (register int i = 0; i < length; ++i) {
        *(input_output_ptr + i) = fmax(0.0, *(input_output_ptr + i));
    }
}

// transformer2_v5_layernorm: Performs Layer Normalization on a sequence of feature vectors.
// output_ptr: Output array (seq_len_param * feature_dim_param)
// input_ptr: Input array (seq_len_param * feature_dim_param)
// gamma_ptr: Scaling parameter vector (feature_dim_param)
// beta_ptr: Shifting parameter vector (feature_dim_param)
// seq_len_param: The number of vectors in the sequence for the current batch element
// feature_dim_param: The dimensionality of each feature vector
static void transformer2_v5_layernorm(double* restrict output_ptr, const double* restrict input_ptr,
                                      const double* restrict gamma_ptr, const double* restrict beta_ptr,
                                      register int seq_len_param, register int feature_dim_param) {
    for (register int s_idx = 0; s_idx < seq_len_param; ++s_idx) {
        double current_mean_v5 = 0.0;
        double current_variance_v5 = 0.0;
        const double* sequence_start_v5 = input_ptr + ((size_t)s_idx * feature_dim_param);

        for (register int f_idx = 0; f_idx < feature_dim_param; ++f_idx) {
            current_mean_v5 += *(sequence_start_v5 + f_idx);
        }
        current_mean_v5 /= feature_dim_param;

        for (register int f_idx = 0; f_idx < feature_dim_param; ++f_idx) {
            double diff_v5 = *(sequence_start_v5 + f_idx) - current_mean_v5;
            current_variance_v5 += diff_v5 * diff_v5;
        }
        current_variance_v5 /= feature_dim_param;

        double std_dev_v5 = sqrt(current_variance_v5 + TRANSFORMER2_V5_LN_EPSILON);
        double inv_std_dev_v5 = 1.0 / std_dev_v5;

        double* output_start_v5 = output_ptr + ((size_t)s_idx * feature_dim_param);
        for (register int f_idx = 0; f_idx < feature_dim_param; ++f_idx) {
            double normalized_val_v5 = (*(sequence_start_v5 + f_idx) - current_mean_v5) * inv_std_dev_v5;
            *(output_start_v5 + f_idx) = normalized_val_v5 * *(gamma_ptr + f_idx) + *(beta_ptr + f_idx);
        }
    }
}

// transformer2_v5_scale_in_place: Multiplies an array by a scalar in-place.
// target_ptr: Array of length 'length'
// scalar_val: Value to multiply by
// length: Length of the array
static void transformer2_v5_scale_in_place(double* restrict target_ptr, double scalar_val, register int length) {
    for (register int i = 0; i < length; ++i) {
        *(target_ptr + i) *= scalar_val;
    }
}

// transformer2_v5_elementwise_add: Performs element-wise addition of two arrays.
// result_ptr: Output array (length)
// a_ptr: First input array (length)
// b_ptr: Second input array (length)
// length: Length of the arrays
static void transformer2_v5_elementwise_add(double* restrict result_ptr, const double* restrict a_ptr,
                                            const double* restrict b_ptr, register int length) {
    for (register int i = 0; i < length; ++i) {
        *(result_ptr + i) = *(a_ptr + i) + *(b_ptr + i);
    }
}


// Main transformer block function: transformer2_v5_forward
// This function implements a simplified single-head transformer block with pre-normalization.
//
// Parameters:
// output_v5_seq: Pointer to the output sequence tensor (B, S, D).
// input_v5_seq: Pointer to the input sequence tensor (B, S, D).
// Wq_v5, Bq_v5: Query projection weights (D,D) and biases (D).
// Wk_v5, Bk_v5: Key projection weights (D,D) and biases (D).
// Wv_v5, Bv_v5: Value projection weights (D,D) and biases (D).
// Wo_v5, Bo_v5: Output projection weights (D,D) and biases (D).
// W1_v5, B1_v5: FFN layer 1 weights (D,F) and biases (F).
// W2_v5, B2_v5: FFN layer 2 weights (F,D) and biases (D).
// gamma1_v5, beta1_v5: Layer Normalization 1 parameters (D).
// gamma2_v5, beta2_v5: Layer Normalization 2 parameters (D).
// scratch_v5: Pointer to a transformer2_v5_scratch_t struct containing necessary temporary buffers.
// batch_size_v5: Number of sequences in the batch (B).
// sequence_length_v5: Length of each sequence (S).
// model_dim_v5: Dimensionality of the model (D).
// ffn_dim_v5: Dimensionality of the FFN hidden layer (F).
void transformer2_v5_forward(
    double* restrict output_v5_seq,         // (B, S, D)
    const double* restrict input_v5_seq,    // (B, S, D)
    const double* restrict Wq_v5,           // (D, D)
    const double* restrict Bq_v5,           // (D)
    const double* restrict Wk_v5,           // (D, D)
    const double* restrict Bk_v5,           // (D)
    const double* restrict Wv_v5,           // (D, D)
    const double* restrict Bv_v5,           // (D)
    const double* restrict Wo_v5,           // (D, D)
    const double* restrict Bo_v5,           // (D)
    const double* restrict W1_v5,           // (D, F)
    const double* restrict B1_v5,           // (F)
    const double* restrict W2_v5,           // (F, D)
    const double* restrict B2_v5,           // (D)
    const double* restrict gamma1_v5,       // (D)
    const double* restrict beta1_v5,        // (D)
    const double* restrict gamma2_v5,       // (D)
    const double* restrict beta2_v5,        // (D)
    transformer2_v5_scratch_t* restrict scratch_v5, // Pointer to scratch buffers struct
    register int batch_size_v5,
    register int sequence_length_v5,
    register int model_dim_v5,
    register int ffn_dim_v5
) {
    // Determine effective dimensions for flattened batch-sequence processing
    const register int bs_dim_v5 = batch_size_v5 * sequence_length_v5;
    const register int s_dim_v5 = sequence_length_v5;
    const register int d_dim_v5 = model_dim_v5;
    const register int f_dim_v5 = ffn_dim_v5;

    // Assign internal pointers using the provided scratch buffers.
    // Memory reuse is carefully managed to ensure distinct active data is not overwritten.
    double* restrict current_norm1_out_v5 = scratch_v5->buffer_primary_flow_v5; // For LN1 output
    double* restrict current_query_v5 = scratch_v5->buffer_query_proj_v5;     // For Q projection
    double* restrict current_key_v5 = scratch_v5->buffer_key_attn_v5;         // For K projection
    double* restrict current_value_v5 = scratch_v5->buffer_primary_flow_v5;   // Reuses norm1_out_v5's space for V projection
                                                                           // (norm1_out_v5 is consumed by QKV projections)

    double* restrict current_attn_scores_v5 = scratch_v5->buffer_attn_scores_v5; // For QK^T scores
    double* restrict current_attn_output_v5 = scratch_v5->buffer_key_attn_v5;    // Reuses current_key_v5's space for attention output
                                                                           // (current_key_v5 is consumed after K^T is formed)

    double* restrict current_proj_output_v5 = scratch_v5->buffer_query_proj_v5;  // Reuses current_query_v5's space for output projection
                                                                           // (current_query_v5 is consumed after attention)
    double* restrict current_add1_out_v5 = scratch_v5->buffer_primary_flow_v5;   // Reuses current_value_v5's space for first residual add
                                                                           // (current_value_v5 is consumed after attention)

    double* restrict current_norm2_out_v5 = scratch_v5->buffer_key_attn_v5;      // Reuses current_attn_output_v5's space for LN2 output
                                                                           // (current_attn_output_v5 is consumed by output projection)

    double* restrict current_ffn_hidden_v5 = scratch_v5->buffer_ffn_hidden_v5; // For FFN hidden layer
    double* restrict current_ffn_output_v5 = scratch_v5->buffer_query_proj_v5; // Reuses current_proj_output_v5's space for FFN output
                                                                           // (current_proj_output_v5 is consumed by residual add)

    double* restrict current_k_transposed_v5_per_batch = scratch_v5->buffer_k_transposed_v5; // For K transpose (per batch)


    // Step 1: Layer Normalization 1 (Pre-Attention)
    // current_norm1_out_v5 = LayerNorm(input_v5_seq, gamma1_v5, beta1_v5)
    for (register int b_idx = 0; b_idx < batch_size_v5; ++b_idx) {
        transformer2_v5_layernorm(
            current_norm1_out_v5 + ((size_t)b_idx * s_dim_v5 * d_dim_v5),
            input_v5_seq + ((size_t)b_idx * s_dim_v5 * d_dim_v5),
            gamma1_v5, beta1_v5, s_dim_v5, d_dim_v5
        );
    }

    // Step 2: Query, Key, Value Projections
    // current_query_v5 = MatMul(current_norm1_out_v5, Wq_v5) + Bq_v5
    transformer2_v5_matmul(current_query_v5, current_norm1_out_v5, Wq_v5, bs_dim_v5, d_dim_v5, d_dim_v5);
    transformer2_v5_add_bias(current_query_v5, Bq_v5, bs_dim_v5, d_dim_v5);

    // current_key_v5 = MatMul(current_norm1_out_v5, Wk_v5) + Bk_v5
    transformer2_v5_matmul(current_key_v5, current_norm1_out_v5, Wk_v5, bs_dim_v5, d_dim_v5, d_dim_v5);
    transformer2_v5_add_bias(current_key_v5, Bk_v5, bs_dim_v5, d_dim_v5);

    // current_value_v5 = MatMul(current_norm1_out_v5, Wv_v5) + Bv_v5
    transformer2_v5_matmul(current_value_v5, current_norm1_out_v5, Wv_v5, bs_dim_v5, d_dim_v5, d_dim_v5);
    transformer2_v5_add_bias(current_value_v5, Bv_v5, bs_dim_v5, d_dim_v5);
    // current_norm1_out_v5 (in buffer_primary_flow_v5) is now fully consumed.

    // Step 3: Scaled Dot-Product Attention (Simplified single head approach per batch element)
    // Scale factor is 1/sqrt(D) as Dk is effectively D for a simplified single-head.
    const double scale_factor_v5 = 1.0 / sqrt((double)d_dim_v5); 

    for (register int b_idx = 0; b_idx < batch_size_v5; ++b_idx) {
        const double* q_batch_ptr = current_query_v5 + ((size_t)b_idx * s_dim_v5 * d_dim_v5);
        const double* k_batch_ptr = current_key_v5 + ((size_t)b_idx * s_dim_v5 * d_dim_v5);
        const double* v_batch_ptr = current_value_v5 + ((size_t)b_idx * s_dim_v5 * d_dim_v5);

        // Transpose K for the current batch: current_k_transposed_v5_per_batch = K[b_idx].T (S x D -> D x S)
        transformer2_v5_transpose(current_k_transposed_v5_per_batch, k_batch_ptr, s_dim_v5, d_dim_v5);

        // Compute attention scores: current_attn_scores_v5 = MatMul(Q[b_idx], K[b_idx].T) (S x D * D x S -> S x S)
        transformer2_v5_matmul(
            current_attn_scores_v5 + ((size_t)b_idx * s_dim_v5 * s_dim_v5),
            q_batch_ptr, current_k_transposed_v5_per_batch,
            s_dim_v5, d_dim_v5, s_dim_v5
        );

        // Scale attention scores
        transformer2_v5_scale_in_place(
            current_attn_scores_v5 + ((size_t)b_idx * s_dim_v5 * s_dim_v5),
            scale_factor_v5, s_dim_v5 * s_dim_v5
        );

        // Apply Softmax to each row of attention scores
        for (register int s_row_idx = 0; s_row_idx < s_dim_v5; ++s_row_idx) {
            transformer2_v5_softmax(
                current_attn_scores_v5 + ((size_t)b_idx * s_dim_v5 * s_dim_v5) + ((size_t)s_row_idx * s_dim_v5),
                s_dim_v5
            );
        }

        // Compute attention output: current_attn_output_v5 = MatMul(Attn_Weights, V[b_idx]) (S x S * S x D -> S x D)
        transformer2_v5_matmul(
            current_attn_output_v5 + ((size_t)b_idx * s_dim_v5 * d_dim_v5),
            current_attn_scores_v5 + ((size_t)b_idx * s_dim_v5 * s_dim_v5),
            v_batch_ptr,
            s_dim_v5, s_dim_v5, d_dim_v5
        );
    } // End batch loop for attention
    // current_query_v5, current_key_v5, current_value_v5 (and thus current_norm1_out_v5) are now consumed.

    // Step 4: Output Projection
    // current_proj_output_v5 = MatMul(current_attn_output_v5, Wo_v5) + Bo_v5
    transformer2_v5_matmul(current_proj_output_v5, current_attn_output_v5, Wo_v5, bs_dim_v5, d_dim_v5, d_dim_v5);
    transformer2_v5_add_bias(current_proj_output_v5, Bo_v5, bs_dim_v5, d_dim_v5);
    // current_attn_output_v5 (in buffer_key_attn_v5) is now consumed.

    // Step 5: Residual Connection 1 + Add
    // current_add1_out_v5 = input_v5_seq + current_proj_output_v5
    transformer2_v5_elementwise_add(current_add1_out_v5, input_v5_seq, current_proj_output_v5, bs_dim_v5 * d_dim_v5);
    // current_proj_output_v5 (in buffer_query_proj_v5) is now consumed.

    // Step 6: Layer Normalization 2 (Pre-FFN)
    // current_norm2_out_v5 = LayerNorm(current_add1_out_v5, gamma2_v5, beta2_v5)
    for (register int b_idx = 0; b_idx < batch_size_v5; ++b_idx) {
        transformer2_v5_layernorm(
            current_norm2_out_v5 + ((size_t)b_idx * s_dim_v5 * d_dim_v5),
            current_add1_out_v5 + ((size_t)b_idx * s_dim_v5 * d_dim_v5),
            gamma2_v5, beta2_v5, s_dim_v5, d_dim_v5
        );
    }

    // Step 7: Feed-Forward Network (FFN)
    // current_ffn_hidden_v5 = ReLU(MatMul(current_norm2_out_v5, W1_v5) + B1_v5)
    transformer2_v5_matmul(current_ffn_hidden_v5, current_norm2_out_v5, W1_v5, bs_dim_v5, d_dim_v5, f_dim_v5);
    transformer2_v5_add_bias(current_ffn_hidden_v5, B1_v5, bs_dim_v5, f_dim_v5);
    transformer2_v5_relu(current_ffn_hidden_v5, bs_dim_v5 * f_dim_v5);
    // current_norm2_out_v5 (in buffer_key_attn_v5) is now consumed.

    // current_ffn_output_v5 = MatMul(current_ffn_hidden_v5, W2_v5) + B2_v5
    transformer2_v5_matmul(current_ffn_output_v5, current_ffn_hidden_v5, W2_v5, bs_dim_v5, f_dim_v5, d_dim_v5);
    transformer2_v5_add_bias(current_ffn_output_v5, B2_v5, bs_dim_v5, d_dim_v5);
    // current_ffn_hidden_v5 (in buffer_ffn_hidden_v5) is now consumed.

    // Step 8: Residual Connection 2 + Add (Final Output)
    // output_v5_seq = current_add1_out_v5 + current_ffn_output_v5
    transformer2_v5_elementwise_add(output_v5_seq, current_add1_out_v5, current_ffn_output_v5, bs_dim_v5 * d_dim_v5);
    // current_add1_out_v5 (in buffer_primary_flow_v5) and current_ffn_output_v5 (in buffer_query_proj_v5) are consumed
    // by this final operation, leaving output_v5_seq ready.
}
```

```c
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> // For NULL

/**
 * @brief Enum defining the possible operational states of the orchestrator.
 */
typedef enum {
    ORCH_STATE_IDLE,        ///< The orchestrator is waiting for a command.
    ORCH_STATE_PREPARING,   ///< The orchestrator is setting up tasks/resources.
    ORCH_STATE_RUNNING,     ///< The orchestrator is actively managing tasks.
    ORCH_STATE_PAUSED,      ///< The orchestrator's operations are temporarily suspended.
    ORCH_STATE_COMPLETED,   ///< All managed tasks have finished successfully.
    ORCH_STATE_ERROR,       ///< An error occurred, operations are halted.
    ORCH_STATE_TOTAL_COUNT  ///< Internal: Total number of states (for array sizing, etc.)
} OrchestratorState_t;

/**
 * @brief Enum defining the types of events the orchestrator can react to.
 */
typedef enum {
    ORCH_EVENT_NONE,            ///< No specific event.
    ORCH_EVENT_START_REQUEST,   ///< Request to begin orchestration.
    ORCH_EVENT_PAUSE_REQUEST,   ///< Request to pause current operations.
    ORCH_EVENT_RESUME_REQUEST,  ///< Request to resume paused operations.
    ORCH_EVENT_STOP_REQUEST,    ///< Request to terminate all operations.
    ORCH_EVENT_TASK_DONE,       ///< Notification that a sub-task has completed successfully.
    ORCH_EVENT_TASK_FAILED,     ///< Notification that a sub-task has failed.
    ORCH_EVENT_CONFIG_UPDATE,   ///< Notification of a configuration change.
    ORCH_EVENT_TIMER_ELAPSED,   ///< Notification that an internal timer has elapsed.
    ORCH_EVENT_TOTAL_COUNT      ///< Internal: Total number of event types.
} OrchestratorEvent_t;

/**
 * @brief Structure holding the internal context and state of the orchestrator.
 */
typedef struct {
    OrchestratorState_t currentState; ///< The current state of the orchestrator.
    uint32_t activeTasksCount;        ///< Counter for currently active sub-tasks.
    uint32_t errorFlags;              ///< Bitmask for various error conditions.
    // Additional context data (e.g., configurations, task queues, timers) could be added here.
} OrchestratorContext_t;

// --- Static (internal) helper function prototypes ---
// These functions encapsulate the state transition logic for specific events.
static void orchestrator_handle_start_request(OrchestratorContext_t* const ctx);
static void orchestrator_handle_pause_request(OrchestratorContext_t* const ctx);
static void orchestrator_handle_resume_request(OrchestratorContext_t* const ctx);
static void orchestrator_handle_stop_request(OrchestratorContext_t* const ctx);
static void orchestrator_handle_task_done(OrchestratorContext_t* const ctx);
static void orchestrator_handle_task_failed(OrchestratorContext_t* const ctx);
static void orchestrator_handle_config_update(OrchestratorContext_t* const ctx);
static void orchestrator_handle_timer_elapsed(OrchestratorContext_t* const ctx);


/**
 * @brief Initializes the orchestrator context to its default state.
 *
 * @param ctx Pointer to the orchestrator context structure to be initialized.
 */
void orchestrator_init(OrchestratorContext_t* const ctx) {
    if (ctx != NULL) {
        ctx->currentState = ORCH_STATE_IDLE;
        ctx->activeTasksCount = 0;
        ctx->errorFlags = 0;
        // Initialize any other members of OrchestratorContext_t here
    }
}

/**
 * @brief Retrieves the current state of the orchestrator.
 *
 * @param ctx Pointer to the orchestrator context.
 * @return The current OrchestratorState_t, or ORCH_STATE_ERROR if context is invalid.
 */
OrchestratorState_t orchestrator_get_state(const OrchestratorContext_t* const ctx) {
    if (ctx != NULL) {
        return ctx->currentState;
    }
    return ORCH_STATE_ERROR; // Indicate an invalid context
}

/**
 * @brief Processes an incoming event, potentially causing a state transition
 *        and triggering actions within the orchestrator.
 *
 * This implementation uses a 'switch' statement on the `event` type. Each
 * event handler then contains 'if-else if' logic to determine the valid
 * state transitions based on the `currentState`. This structure provides
 * a distinct separation of event handling from state transition rules.
 *
 * @param ctx Pointer to the orchestrator context.
 * @param event The event to be processed by the orchestrator.
 * @return true if the event was recognized and processed (even if no state change occurred),
 *         false if the event was invalid or the context was NULL.
 */
bool orchestrator_process_event(OrchestratorContext_t* const ctx, const OrchestratorEvent_t event) {
    if (ctx == NULL || event >= ORCH_EVENT_TOTAL_COUNT || event == ORCH_EVENT_NONE) {
        return false; // Invalid context or event
    }

    // Dispatch to event-specific handlers
    switch (event) {
        case ORCH_EVENT_START_REQUEST:
            orchestrator_handle_start_request(ctx);
            break;
        case ORCH_EVENT_PAUSE_REQUEST:
            orchestrator_handle_pause_request(ctx);
            break;
        case ORCH_EVENT_RESUME_REQUEST:
            orchestrator_handle_resume_request(ctx);
            break;
        case ORCH_EVENT_STOP_REQUEST:
            orchestrator_handle_stop_request(ctx);
            break;
        case ORCH_EVENT_TASK_DONE:
            orchestrator_handle_task_done(ctx);
            break;
        case ORCH_EVENT_TASK_FAILED:
            orchestrator_handle_task_failed(ctx);
            break;
        case ORCH_EVENT_CONFIG_UPDATE:
            orchestrator_handle_config_update(ctx);
            break;
        case ORCH_EVENT_TIMER_ELAPSED:
            orchestrator_handle_timer_elapsed(ctx);
            break;
        default:
            // Should not be reached if event validation above is correct.
            return false;
    }
    return true; // Event was handled by a specific function
}

// --- Internal Helper Function Implementations ---

/**
 * @brief Handles the ORCH_EVENT_START_REQUEST event.
 * Transitions from IDLE or COMPLETED to PREPARING, or from ERROR to IDLE before PREPARING.
 *
 * @param ctx Pointer to the orchestrator context.
 */
static void orchestrator_handle_start_request(OrchestratorContext_t* const ctx) {
    if (ctx->currentState == ORCH_STATE_IDLE || ctx->currentState == ORCH_STATE_COMPLETED) {
        ctx->activeTasksCount = 1; // Example: Start with one initial task to prepare
        ctx->currentState = ORCH_STATE_PREPARING;
        // Logic to initiate preparation (e.g., signal other modules)
    } else if (ctx->currentState == ORCH_STATE_ERROR) {
        // Allow a restart from error by first clearing error and going to IDLE, then PREPARING
        ctx->errorFlags = 0;
        ctx->currentState = ORCH_STATE_IDLE;
        // Optionally, recursively call this function or re-evaluate the request
        orchestrator_handle_start_request(ctx); // Simpler for this example
    }
    // Ignore start request if already RUNNING or PAUSED
}

/**
 * @brief Handles the ORCH_EVENT_PAUSE_REQUEST event.
 * Transitions from RUNNING to PAUSED.
 *
 * @param ctx Pointer to the orchestrator context.
 */
static void orchestrator_handle_pause_request(OrchestratorContext_t* const ctx) {
    if (ctx->currentState == ORCH_STATE_RUNNING) {
        ctx->currentState = ORCH_STATE_PAUSED;
        // Logic to pause active sub-tasks
    }
    // Ignore pause request in other states (e.g., IDLE, PAUSED, ERROR)
}

/**
 * @brief Handles the ORCH_EVENT_RESUME_REQUEST event.
 * Transitions from PAUSED to RUNNING.
 *
 * @param ctx Pointer to the orchestrator context.
 */
static void orchestrator_handle_resume_request(OrchestratorContext_t* const ctx) {
    if (ctx->currentState == ORCH_STATE_PAUSED) {
        ctx->currentState = ORCH_STATE_RUNNING;
        // Logic to resume paused sub-tasks
    }
    // Ignore resume request in other states
}

/**
 * @brief Handles the ORCH_EVENT_STOP_REQUEST event.
 * Transitions from any active or error state to IDLE.
 *
 * @param ctx Pointer to the orchestrator context.
 */
static void orchestrator_handle_stop_request(OrchestratorContext_t* const ctx) {
    if (ctx->currentState == ORCH_STATE_RUNNING ||
        ctx->currentState == ORCH_STATE_PAUSED ||
        ctx->currentState == ORCH_STATE_PREPARING ||
        ctx->currentState == ORCH_STATE_ERROR) {
        ctx->activeTasksCount = 0; // Terminate all tasks
        ctx->errorFlags = 0;       // Clear any errors
        ctx->currentState = ORCH_STATE_IDLE;
        // Logic to signal all sub-tasks to stop
    }
    // Ignore stop request if already IDLE or COMPLETED
}

/**
 * @brief Handles the ORCH_EVENT_TASK_DONE event.
 * Decrements active task count and transitions based on completion status.
 *
 * @param ctx Pointer to the orchestrator context.
 */
static void orchestrator_handle_task_done(OrchestratorContext_t* const ctx) {
    if (ctx->currentState == ORCH_STATE_RUNNING || ctx->currentState == ORCH_STATE_PREPARING) {
        if (ctx->activeTasksCount > 0) {
            ctx->activeTasksCount--;
        }

        if (ctx->activeTasksCount == 0) {
            if (ctx->currentState == ORCH_STATE_PREPARING) {
                ctx->currentState = ORCH_STATE_RUNNING; // All prep done, start running
                // Logic to start main operational tasks
            } else if (ctx->currentState == ORCH_STATE_RUNNING) {
                ctx->currentState = ORCH_STATE_COMPLETED; // All main tasks done
                // Logic for finalization/reporting completion
            }
        }
    }
    // Ignore task done in other states (e.g., IDLE, PAUSED, ERROR)
}

/**
 * @brief Handles the ORCH_EVENT_TASK_FAILED event.
 * Transitions to ERROR state upon any sub-task failure.
 *
 * @param ctx Pointer to the orchestrator context.
 */
static void orchestrator_handle_task_failed(OrchestratorContext_t* const ctx) {
    if (ctx->currentState != ORCH_STATE_ERROR && ctx->currentState != ORCH_STATE_IDLE) {
        ctx->errorFlags |= (1U << 0); // Set a generic task failure flag
        ctx->currentState = ORCH_STATE_ERROR;
        // Logic to log error, notify other modules, or clean up
    }
    // If already in ERROR or IDLE, a task failed event might be ignored or handled differently.
}

/**
 * @brief Handles the ORCH_EVENT_CONFIG_UPDATE event.
 * Allows configuration updates in IDLE or PAUSED states.
 *
 * @param ctx Pointer to the orchestrator context.
 */
static void orchestrator_handle_config_update(OrchestratorContext_t* const ctx) {
    if (ctx->currentState == ORCH_STATE_IDLE || ctx->currentState == ORCH_STATE_PAUSED) {
        // Apply configuration changes here.
        // For example, if config data was part of OrchestratorContext_t, update it.
        // This might involve re-initializing parts of the orchestrator based on new config.
    } else if (ctx->currentState == ORCH_STATE_RUNNING) {
        // In a real system, some config updates might be allowed during RUNNING
        // if they are non-disruptive, or queued for a safe point.
        // For this example, we assume non-disruptive updates are not handled here.
    }
    // Ignore config updates in PREPARING, RUNNING (for simplicity), or ERROR states.
}

/**
 * @brief Handles the ORCH_EVENT_TIMER_ELAPSED event.
 * Can trigger timeouts or periodic actions based on the current state.
 *
 * @param ctx Pointer to the orchestrator context.
 */
static void orchestrator_handle_timer_elapsed(OrchestratorContext_t* const ctx) {
    if (ctx->currentState == ORCH_STATE_PREPARING) {
        // If a timer elapses during preparation, it might indicate a timeout.
        ctx->errorFlags |= (1U << 1); // Set a timeout error flag
        ctx->currentState = ORCH_STATE_ERROR;
        // Logic to handle preparation timeout
    } else if (ctx->currentState == ORCH_STATE_RUNNING) {
        // A timer might be used for periodic checks, heartbeats, or triggering next steps.
        // For example, if no task_done event received within a timeframe, trigger an error.
        // Or, trigger the next periodic task.
    }
    // Ignore timer events in other states, or implement state-specific timer actions.
}
```

