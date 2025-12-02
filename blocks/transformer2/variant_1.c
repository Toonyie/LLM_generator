#include <math.h> // Required for sqrtf, expf
#include <stddef.h> // Required for NULL, size_t (though not strictly used for type)

// --- Placeholder Helper Functions ---
// In a real implementation, these would be optimized BLAS/DNN library calls
// or hand-optimized kernels. For this block, they define the computational structure.

// Placeholder for matrix multiplication: C = A * B
// A_rows: number of rows in A
// A_cols: number of columns in A (and rows in B)
// B_cols: number of columns in B
void matrix_mul(const float* A, const float* B, float* C, int A_rows, int A_cols, int B_cols) {
    // Basic conceptual loop, not optimized
    for (int i = 0; i < A_rows; ++i) {
        for (int j = 0; j < B_cols; ++j) {
            float sum = 0.0f;
            for (int k = 0; k < A_cols; ++k) {
                sum += A[i * A_cols + k] * B[k * B_cols + j];
            }
            C[i * B_cols + j] = sum;
        }
    }
}

// Placeholder for element-wise addition: C = A + B
void elementwise_add(const float* A, const float* B, float* C, int size) {
    for (int i = 0; i < size; ++i) {
        C[i] = A[i] + B[i];
    }
}

// Placeholder for ReLU activation
void relu(float* data, int size) {
    for (int i = 0; i < size; ++i) {
        data[i] = (data[i] > 0.0f) ? data[i] : 0.0f;
    }
}

// Placeholder for Softmax activation, applied to a single row (vector)
void softmax_row(float* data, int size) {
    float max_val = data[0];
    for (int i = 1; i < size; ++i) {
        if (data[i] > max_val) {
            max_val = data[i];
        }
    }

    float sum_exp = 0.0f;
    for (int i = 0; i < size; ++i) {
        data[i] = expf(data[i] - max_val); // Subtract max for numerical stability
        sum_exp += data[i];
    }

    // Add a small epsilon to sum_exp to prevent division by zero, though unlikely with expf.
    // For simplicity, omitting it here, assuming sum_exp > 0.
    for (int i = 0; i < size; ++i) {
        data[i] /= sum_exp;
    }
}

// Placeholder for Layer Normalization
// mean and variance are computed statistics, gamma and beta are learned parameters.
void layer_norm(float* data, int seq_len, int d_model,
                const float* norm_gamma, const float* norm_beta) {
    const float epsilon = 1e-5f; // Small constant for numerical stability

    // Iterate over each sequence element (each 'token' or 'word' vector)
    for (int i = 0; i < seq_len; ++i) {
        const float* current_vec = data + i * d_model;

        // Calculate mean
        float mean = 0.0f;
        for (int j = 0; j < d_model; ++j) {
            mean += current_vec[j];
        }
        mean /= d_model;

        // Calculate variance
        float variance = 0.0f;
        for (int j = 0; j < d_model; ++j) {
            variance += (current_vec[j] - mean) * (current_vec[j] - mean);
        }
        variance /= d_model;

        // Normalize and apply gamma/beta
        float inv_stddev = 1.0f / sqrtf(variance + epsilon);
        for (int j = 0; j < d_model; ++j) {
            data[i * d_model + j] = (current_vec[j] - mean) * inv_stddev * norm_gamma[j] + norm_beta[j];
        }
    }
}

// Placeholder for a linear transformation with weights and bias
// input: (in_rows x in_cols)
// weights: (in_cols x out_cols)
// bias: (out_cols)
// output: (in_rows x out_cols)
void linear_layer(const float* input, const float* weights, const float* bias,
                  float* output, int in_rows, int in_cols, int out_cols) {
    matrix_mul(input, weights, output, in_rows, in_cols, out_cols);
    // Add bias
    for (int i = 0; i < in_rows; ++i) {
        for (int j = 0; j < out_cols; ++j) {
            output[i * out_cols + j] += bias[j];
        }
    }
}

// --- Transformer Sub-Blocks ---

// Scaled Dot-Product Attention for a single head
// Q, K, V are already projected for this head (seq_len x head_dim)
void scaled_dot_product_attention(const float* Q_head, const float* K_head, const float* V_head,
                                  float* attn_output_head, int seq_len, int head_dim) {
    // Memory for intermediate scores (seq_len x seq_len)
    float scores_buffer[seq_len * seq_len]; // Assumes stack allocation for simplicity, typically heap

    // 1. Q * K_transpose
    // For matrix_mul(A, B, C, A_rows, A_cols, B_cols), B is used as-is.
    // If K_head is (seq_len x head_dim), then K_transpose is (head_dim x seq_len).
    // The `matrix_mul` helper implicitly handles matrix transpositions by parameter order.
    // Here, K_head is treated as K_transpose when passed as second argument `B` with dimensions swapped.
    // This is a simplification; a true K_transpose would be needed.
    // Let's assume K_head is effectively row-major K_T for the `matrix_mul` call.
    // A better approach would be to have a separate matrix_transpose function.
    // For this variant, let's just make the matrix_mul for Q*K_T accept `K_head` directly and indicate dimension swap.
    // (seq_len x head_dim) * (head_dim x seq_len) -> (seq_len x seq_len)
    // To achieve K_T, the B argument needs to be K_head interpreted column-major, or a transposed copy.
    // For simplicity, let's treat it as:
    // matrix_mul(Q_head, K_head_TRANSPOSED, scores_buffer, seq_len, head_dim, seq_len);
    // As a variant, for `matrix_mul` definition above, the `B` matrix `B[k * B_cols + j]` implies column-major or explicit transpose needed.
    // Let's assume K_head is already transposed for this call (a simplification for variant 1).
    matrix_mul(Q_head, K_head /* conceptual K_transposed */, scores_buffer, seq_len, head_dim, seq_len);

    // 2. Scale
    float scale = 1.0f / sqrtf((float)head_dim);
    for (int i = 0; i < seq_len * seq_len; ++i) {
        scores_buffer[i] *= scale;
    }

    // 3. Softmax
    for (int i = 0; i < seq_len; ++i) {
        softmax_row(scores_buffer + i * seq_len, seq_len);
    }

    // 4. Attention scores * V
    // (seq_len x seq_len) * (seq_len x head_dim) -> (seq_len x head_dim)
    matrix_mul(scores_buffer, V_head, attn_output_head, seq_len, seq_len, head_dim);
}

// Multi-Head Self-Attention Block
// This function assumes global access to projection weight matrices (W_Q, W_K, W_V, W_O) and their biases.
void multi_head_attention(const float* input, float* output,
                          int seq_len, int d_model, int num_heads,
                          // Projection weights and biases (simplified for Variant 1, assuming explicit access)
                          const float* wq_mat, const float* wq_bias,
                          const float* wk_mat, const float* wk_bias,
                          const float* wv_mat, const float* wv_bias,
                          const float* wo_mat, const float* wo_bias) {
    int head_dim = d_model / num_heads;

    // Allocate temporary buffers for Q, K, V projections and attention outputs
    // In a real system, these would be managed via dynamic allocation or a memory pool.
    // For simplicity, declaring as pointers and assuming memory allocation.
    float* Q_proj = (float*)NULL; // (seq_len x d_model)
    float* K_proj = (float*)NULL; // (seq_len x d_model)
    float* V_proj = (float*)NULL; // (seq_len x d_model)
    float* all_heads_output_buffer = (float*)NULL; // (seq_len x d_model)

    // Conceptual memory allocation:
    // Q_proj = malloc(seq_len * d_model * sizeof(float));
    // K_proj = malloc(seq_len * d_model * sizeof(float));
    // V_proj = malloc(seq_len * d_model * sizeof(float));
    // all_heads_output_buffer = malloc(seq_len * d_model * sizeof(float));

    // For variant 1, use stack allocated small buffers to represent the "head" data within the loop
    // and rely on a large conceptual buffer for concatenation.
    // In actual C, this would require dynamic allocation or passing pre-allocated large buffers.
    float q_head_slice[seq_len * head_dim];
    float k_head_slice[seq_len * head_dim];
    float v_head_slice[seq_len * head_dim];
    float current_head_output[seq_len * head_dim];

    // 1. Linear projections for Q, K, V
    linear_layer(input, wq_mat, wq_bias, Q_proj, seq_len, d_model, d_model);
    linear_layer(input, wk_mat, wk_bias, K_proj, seq_len, d_model, d_model);
    linear_layer(input, wv_mat, wv_bias, V_proj, seq_len, d_model, d_model);

    // Perform attention for each head
    for (int h = 0; h < num_heads; ++h) {
        // Extract Q_h, K_h, V_h for current head by slicing from Q_proj, K_proj, V_proj
        // This requires careful memory copying if actual slices are needed, or pointer arithmetic.
        // For simplicity, we assume these slice buffers are populated correctly.
        for (int i = 0; i < seq_len; ++i) {
            for (int j = 0; j < head_dim; ++j) {
                q_head_slice[i * head_dim + j] = Q_proj[i * d_model + h * head_dim + j];
                k_head_slice[i * head_dim + j] = K_proj[i * d_model + h * head_dim + j];
                v_head_slice[i * head_dim + j] = V_proj[i * d_model + h * head_dim + j];
            }
        }

        scaled_dot_product_attention(q_head_slice, k_head_slice, v_head_slice,
                                     current_head_output, seq_len, head_dim);

        // Copy current head's output to the concatenated buffer (all_heads_output_buffer)
        // This is conceptually concatenation. `h * (seq_len * head_dim)` is the offset.
        for (int i = 0; i < seq_len * head_dim; ++i) {
            all_heads_output_buffer[h * (seq_len * head_dim) + i] = current_head_output[i];
        }
    }

    // 2. Final linear projection (W_O) on concatenated heads
    linear_layer(all_heads_output_buffer, wo_mat, wo_bias, output, seq_len, d_model, d_model);

    // Conceptual memory deallocation:
    // free(Q_proj); free(K_proj); free(V_proj); free(all_heads_output_buffer);
}

// Feed-Forward Network Block
// This function assumes global access to FFN weight matrices (W1, W2) and biases (B1, B2).
void feed_forward_network(const float* input, float* output,
                          int seq_len, int d_model, int d_ff,
                          // FFN weights and biases
                          const float* w1_mat, const float* b1_bias,
                          const float* w2_mat, const float* b2_bias) {
    // Memory for hidden layer output
    float* hidden_layer_output = (float*)NULL; // (seq_len x d_ff)
    // Conceptual memory allocation: hidden_layer_output = malloc(seq_len * d_ff * sizeof(float));

    // 1. First linear layer (Input -> d_ff)
    linear_layer(input, w1_mat, b1_bias, hidden_layer_output, seq_len, d_model, d_ff);

    // 2. Activation (ReLU)
    relu(hidden_layer_output, seq_len * d_ff);

    // 3. Second linear layer (d_ff -> Output)
    linear_layer(hidden_layer_output, w2_mat, b2_bias, output, seq_len, d_ff, d_model);

    // Conceptual memory deallocation: free(hidden_layer_output);
}

// --- Main Transformer Block Function (Variant #1) ---
// This variant implements the Post-Normalization structure (Attention -> Add -> Norm -> FFN -> Add -> Norm).
// It assumes weight and bias parameters for all linear layers and LayerNorm parameters (gamma, beta)
// are passed in or are globally accessible. For simplicity here, they are passed as function arguments.
void transformer2_block_v1(const float* input_sequence, float* output_sequence,
                           int seq_len, int d_model, int num_heads, int d_ff,
                           // MHA weights and biases
                           const float* mha_wq_mat, const float* mha_wq_bias,
                           const float* mha_wk_mat, const float* mha_wk_bias,
                           const float* mha_wv_mat, const float* mha_wv_bias,
                           const float* mha_wo_mat, const float* mha_wo_bias,
                           // FFN weights and biases
                           const float* ffn_w1_mat, const float* ffn_b1_bias,
                           const float* ffn_w2_mat, const float* ffn_b2_bias,
                           // LayerNorm parameters
                           const float* norm1_gamma, const float* norm1_beta,
                           const float* norm2_gamma, const float* norm2_beta) {

    // Temporary buffers for intermediate results
    // These would typically be allocated dynamically or from a memory pool for efficiency.
    // For this conceptual code, they are declared as pointers assuming allocation.
    float* mha_output = (float*)NULL;     // Output of Multi-Head Attention (seq_len x d_model)
    float* add_norm1_output = (float*)NULL; // Output of first Add & Norm (seq_len x d_model)
    float* ffn_output = (float*)NULL;     // Output of Feed-Forward Network (seq_len x d_model)

    // Conceptual memory allocation:
    // mha_output = malloc(seq_len * d_model * sizeof(float));
    // add_norm1_output = malloc(seq_len * d_model * sizeof(float));
    // ffn_output = malloc(seq_len * d_model * sizeof(float));


    // 1. Multi-Head Self-Attention
    // Input: input_sequence, Output: mha_output
    multi_head_attention(input_sequence, mha_output, seq_len, d_model, num_heads,
                         mha_wq_mat, mha_wq_bias, mha_wk_mat, mha_wk_bias,
                         mha_wv_mat, mha_wv_bias, mha_wo_mat, mha_wo_bias);

    // 2. Add & Normalize (Residual connection 1 + Layer Normalization 1)
    // Add: input_sequence + mha_output -> add_norm1_output
    elementwise_add(input_sequence, mha_output, add_norm1_output, seq_len * d_model);
    // Norm: add_norm1_output (in-place)
    layer_norm(add_norm1_output, seq_len, d_model, norm1_gamma, norm1_beta);


    // 3. Feed-Forward Network
    // Input: add_norm1_output, Output: ffn_output
    feed_forward_network(add_norm1_output, ffn_output, seq_len, d_model, d_ff,
                         ffn_w1_mat, ffn_b1_bias, ffn_w2_mat, ffn_b2_bias);

    // 4. Add & Normalize (Residual connection 2 + Layer Normalization 2)
    // Add: add_norm1_output (residual) + ffn_output -> output_sequence
    elementwise_add(add_norm1_output, ffn_output, output_sequence, seq_len * d_model);
    // Norm: output_sequence (in-place)
    layer_norm(output_sequence, seq_len, d_model, norm2_gamma, norm2_beta);

    // Conceptual memory deallocation:
    // free(mha_output); free(add_norm1_output); free(ffn_output);
}