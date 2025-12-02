#include <stddef.h> // For size_t
#include <string.h> // For memcpy
#include <math.h>   // For sqrtf, expf

// Define some placeholder types for weights and biases
// In a real scenario, these would be properly sized arrays or matrices
typedef float* Matrix;
typedef float* Vector;

// --- Context Structure for Transformer2 ---
// This struct holds all parameters, intermediate buffers, and configuration
// for a single transformer2 block instance.

typedef struct {
    size_t sequence_length;
    size_t embed_dim;
    size_t num_heads;
    size_t ff_dim; // Feed-forward dimension

    // --- Parameters (Weights and Biases) ---
    // Self-Attention Layer
    Matrix q_proj_weight;  // Query projection weight (embed_dim x embed_dim)
    Vector q_proj_bias;    // Query projection bias (embed_dim)
    Matrix k_proj_weight;  // Key projection weight (embed_dim x embed_dim)
    Vector k_proj_bias;    // Key projection bias (embed_dim)
    Matrix v_proj_weight;  // Value projection weight (embed_dim x embed_dim)
    Vector v_proj_bias;    // Value projection bias (embed_dim)
    Matrix out_proj_weight; // Output projection weight (embed_dim x embed_dim)
    Vector out_proj_bias;   // Output projection bias (embed_dim)

    // Feed-Forward Layer
    Matrix ff1_weight;     // First FF layer weight (embed_dim x ff_dim)
    Vector ff1_bias;       // First FF layer bias (ff_dim)
    Matrix ff2_weight;     // Second FF layer weight (ff_dim x embed_dim)
    Vector ff2_bias;       // Second FF layer bias (embed_dim)

    // Layer Normalization Parameters
    Vector ln1_gamma;      // First LayerNorm gamma (embed_dim)
    Vector ln1_beta;       // First LayerNorm beta (embed_dim)
    Vector ln2_gamma;      // Second LayerNorm gamma (embed_dim)
    Vector ln2_beta;       // Second LayerNorm beta (embed_dim)

    // --- Buffers for intermediate computations ---
    // These would typically be dynamically allocated or pre-allocated pools
    // For this example, we use pointers assuming they are managed externally
    float* query_buffer;   // sequence_length * embed_dim
    float* key_buffer;     // sequence_length * embed_dim
    float* value_buffer;   // sequence_length * embed_dim
    float* attn_output_buffer; // sequence_length * embed_dim
    float* ff_hidden_buffer;   // sequence_length * ff_dim
    float* residual_buffer;    // sequence_length * embed_dim (for residual connections)
    float* layernorm_output_buffer; // sequence_length * embed_dim
    float* temp_buffer_1;      // General purpose temporary buffer (e.g., for attention scores)

    // Input and Output pointers for the current forward pass.
    // These are updated at the start of transformer2_process_block_v5.
    float* current_input_sequence; // Points to input_data or a normalized buffer
    float* current_output_sequence; // Points to output_data
} Transformer2_Context;


// --- Helper Functions (internal to the transformer2 block logic) ---
// These functions operate on the provided context, modifying its internal buffers.

// Placeholder for matrix multiplication (e.g., C = A * B + C_bias)
static void matrix_multiply_add_bias(float* result, const float* matrix_a, const float* matrix_b, const float* bias,
                                     size_t rows_a, size_t cols_a, size_t cols_b) {
    // This is a placeholder. A real implementation would perform GEMM.
    // For variant #5, we ensure the function signature and a minimal operation
    // that touches the 'result' buffer.
    size_t result_size = rows_a * cols_b;
    for (size_t i = 0; i < result_size; ++i) {
        result[i] = 0.0f; // Initialize
        if (bias) {
            result[i] += bias[i % cols_b]; // Dummy bias addition
        }
        // Dummy operation to simulate computation:
        // E.g., result[i] = matrix_a[i % (rows_a * cols_a)] * matrix_b[i % (cols_a * cols_b)] + (bias ? bias[i % cols_b] : 0.0f);
    }
}

// Placeholder for vector addition (e.g., result = v1 + v2)
static void vector_add(float* result, const float* vec1, const float* vec2, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        result[i] = vec1[i] + vec2[i];
    }
}

// Placeholder for scalar multiplication
static void vector_scale(float* vec, float scalar, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        vec[i] *= scalar;
    }
}

// Softmax computation for a vector
static void softmax_vector(float* vec, size_t len) {
    if (len == 0) return;

    float max_val = vec[0];
    for (size_t i = 1; i < len; ++i) {
        if (vec[i] > max_val) {
            max_val = vec[i];
        }
    }

    float sum_exp = 0.0f;
    for (size_t i = 0; i < len; ++i) {
        vec[i] = expf(vec[i] - max_val); // Subtract max_val for numerical stability
        sum_exp += vec[i];
    }

    if (sum_exp > 0.0f) {
        for (size_t i = 0; i < len; ++i) {
            vec[i] /= sum_exp;
        }
    } else {
        // Fallback for extremely small sums (e.g., all inputs were -infinity)
        // Or handle as an error condition. For now, distribute evenly.
        for (size_t i = 0; i < len; ++i) {
            vec[i] = 1.0f / (float)len;
        }
    }
}

// Layer Normalization
static void _transformer2_apply_layer_norm(Transformer2_Context* ctx, float* input, float* output,
                                           const Vector gamma, const Vector beta, size_t sequence_length) {
    size_t embed_dim = ctx->embed_dim;
    float epsilon = 1e-5f;

    for (size_t i = 0; i < sequence_length; ++i) {
        float mean = 0.0f;
        float variance = 0.0f;
        size_t offset = i * embed_dim;

        // Calculate mean
        for (size_t j = 0; j < embed_dim; ++j) {
            mean += input[offset + j];
        }
        mean /= (float)embed_dim;

        // Calculate variance
        for (size_t j = 0; j < embed_dim; ++j) {
            float diff = input[offset + j] - mean;
            variance += diff * diff;
        }
        variance /= (float)embed_dim;

        float std_dev_inv = (float)1.0 / sqrtf(variance + epsilon);

        // Normalize and scale/shift
        for (size_t j = 0; j < embed_dim; ++j) {
            output[offset + j] = gamma[j] * (input[offset + j] - mean) * std_dev_inv + beta[j];
        }
    }
}


// --- Main Transformer2 Block Processing Functions ---

static void _transformer2_self_attention_module(Transformer2_Context* ctx) {
    size_t seq_len = ctx->sequence_length;
    size_t embed_dim = ctx->embed_dim;
    size_t head_dim = embed_dim / ctx->num_heads; // Assuming embed_dim is divisible by num_heads

    // 1. Project Q, K, V from current_input_sequence
    // Q = input * Wq + Bq
    matrix_multiply_add_bias(ctx->query_buffer, ctx->current_input_sequence, ctx->q_proj_weight, ctx->q_proj_bias,
                             seq_len, embed_dim, embed_dim);
    // K = input * Wk + Bk
    matrix_multiply_add_bias(ctx->key_buffer, ctx->current_input_sequence, ctx->k_proj_weight, ctx->k_proj_bias,
                             seq_len, embed_dim, embed_dim);
    // V = input * Wv + Bv
    matrix_multiply_add_bias(ctx->value_buffer, ctx->current_input_sequence, ctx->v_proj_weight, ctx->v_proj_bias,
                             seq_len, embed_dim, embed_dim);

    // Dummy multi-head split and merge (simplified for this variant)
    // The matrix_multiply_add_bias function implicitly handles dimensions
    // For a real transformer, Q, K, V would be reshaped (seq_len, num_heads, head_dim)

    // 2. Compute Attention Scores (Q * K^T / sqrt(head_dim))
    // temp_buffer_1 holds attention scores (seq_len x seq_len)
    matrix_multiply_add_bias(ctx->temp_buffer_1, ctx->query_buffer, ctx->key_buffer, NULL, // No bias for QK^T
                             seq_len, embed_dim, embed_dim); // Assuming K is effectively transposed
    vector_scale(ctx->temp_buffer_1, (float)1.0 / sqrtf((float)head_dim), seq_len * seq_len); // Scaling

    // 3. Apply Softmax to scores
    for (size_t i = 0; i < seq_len; ++i) {
        softmax_vector(ctx->temp_buffer_1 + i * seq_len, seq_len); // Softmax across rows
    }

    // 4. Multiply with Values (Attention_Scores * V)
    // Store result in attn_output_buffer (seq_len x embed_dim)
    matrix_multiply_add_bias(ctx->attn_output_buffer, ctx->temp_buffer_1, ctx->value_buffer, NULL,
                             seq_len, seq_len, embed_dim);

    // 5. Output Projection
    // attn_output_buffer = attn_output_buffer * Wo + Bo
    matrix_multiply_add_bias(ctx->attn_output_buffer, ctx->attn_output_buffer, ctx->out_proj_weight, ctx->out_proj_bias,
                             seq_len, embed_dim, embed_dim);
}

static void _transformer2_feed_forward_module(Transformer2_Context* ctx) {
    size_t seq_len = ctx->sequence_length;
    size_t embed_dim = ctx->embed_dim;
    size_t ff_dim = ctx->ff_dim;

    // 1. First linear layer (input is from layernorm_output_buffer)
    // ff_hidden_buffer = layernorm_output_buffer * W1 + B1
    matrix_multiply_add_bias(ctx->ff_hidden_buffer, ctx->layernorm_output_buffer, ctx->ff1_weight, ctx->ff1_bias,
                             seq_len, embed_dim, ff_dim);

    // Apply Activation (e.g., ReLU or GELU)
    // For simplicity, using ReLU here
    for (size_t i = 0; i < seq_len * ff_dim; ++i) {
        if (ctx->ff_hidden_buffer[i] < 0) {
            ctx->ff_hidden_buffer[i] = 0;
        }
    }

    // 2. Second linear layer
    // layernorm_output_buffer = ff_hidden_buffer * W2 + B2 (re-using buffer)
    matrix_multiply_add_bias(ctx->layernorm_output_buffer, ctx->ff_hidden_buffer, ctx->ff2_weight, ctx->ff2_bias,
                             seq_len, ff_dim, embed_dim);
}


// --- Main Entry Point for the Transformer2 Block ---

// Function to initialize the context with dimensions and assign buffer/parameter pointers.
// In a real system, parameters would be loaded from a model file.
void transformer2_init_context(Transformer2_Context* ctx, size_t seq_len, size_t embed_dim, size_t num_heads, size_t ff_dim,
                               const float* q_w, const float* q_b, const float* k_w, const float* k_b, const float* v_w, const float* v_b,
                               const float* out_w, const float* out_b,
                               const float* ff1_w, const float* ff1_b, const float* ff2_w, const float* ff2_b,
                               const float* ln1_g, const float* ln1_b, const float* ln2_g, const float* ln2_b,
                               float* buffer_q, float* buffer_k, float* buffer_v,
                               float* buffer_attn_out, float* buffer_ff_hidden,
                               float* buffer_residual, float* buffer_layernorm_out,
                               float* buffer_tmp1) {

    ctx->sequence_length = seq_len;
    ctx->embed_dim = embed_dim;
    ctx->num_heads = num_heads;
    ctx->ff_dim = ff_dim;

    // Assign parameter pointers (casting away const for internal Matrix/Vector types if necessary,
    // assuming these are managed as read-only but typed as mutable pointers for function signatures)
    ctx->q_proj_weight = (Matrix)q_w;  ctx->q_proj_bias = (Vector)q_b;
    ctx->k_proj_weight = (Matrix)k_w;  ctx->k_proj_bias = (Vector)k_b;
    ctx->v_proj_weight = (Matrix)v_w;  ctx->v_proj_bias = (Vector)v_b;
    ctx->out_proj_weight = (Matrix)out_w; ctx->out_proj_bias = (Vector)out_b;

    ctx->ff1_weight = (Matrix)ff1_w; ctx->ff1_bias = (Vector)ff1_b;
    ctx->ff2_weight = (Matrix)ff2_w; ctx->ff2_bias = (Vector)ff2_b;

    ctx->ln1_gamma = (Vector)ln1_g; ctx->ln1_beta = (Vector)ln1_b;
    ctx->ln2_gamma = (Vector)ln2_g; ctx->ln2_beta = (Vector)ln2_b;

    // Assign buffer pointers
    ctx->query_buffer = buffer_q;
    ctx->key_buffer = buffer_k;
    ctx->value_buffer = buffer_v;
    ctx->attn_output_buffer = buffer_attn_out;
    ctx->ff_hidden_buffer = buffer_ff_hidden;
    ctx->residual_buffer = buffer_residual;
    ctx->layernorm_output_buffer = buffer_layernorm_out;
    ctx->temp_buffer_1 = buffer_tmp1;
}

// Main processing function for the transformer2 block.
// It takes a pointer to the initialized context and the input/output data.
// This structure (passing a full context struct to all operations) is a distinguishing
// feature for this variant #5.
void transformer2_process_block_v5(Transformer2_Context* ctx, const float* input_data, float* output_data) {
    // Set current input/output pointers within the context for this forward pass
    ctx->current_input_sequence = (float*)input_data; // Cast away const as internal buffers are mutable
    ctx->current_output_sequence = output_data;

    size_t seq_len = ctx->sequence_length;
    size_t embed_dim = ctx->embed_dim;
    size_t total_elements = seq_len * embed_dim;

    // 1. First Layer Normalization (Pre-Attention)
    // Save input for residual connection later
    memcpy(ctx->residual_buffer, ctx->current_input_sequence, total_elements * sizeof(float));
    _transformer2_apply_layer_norm(ctx, ctx->current_input_sequence, ctx->layernorm_output_buffer, ctx->ln1_gamma, ctx->ln1_beta, seq_len);
    // Update current_input_sequence to point to the normalized input for the attention module
    ctx->current_input_sequence = ctx->layernorm_output_buffer;

    // 2. Self-Attention Module
    _transformer2_self_attention_module(ctx);
    // Output of attention is in ctx->attn_output_buffer

    // 3. Residual Connection + Add (Input + Attention_Output)
    // Add the attention output to the original input (stored in residual_buffer)
    vector_add(ctx->layernorm_output_buffer, ctx->residual_buffer, ctx->attn_output_buffer, total_elements);
    // ctx->layernorm_output_buffer now holds the result after first residual connection

    // 4. Second Layer Normalization (Pre-Feed-Forward)
    // Save result for second residual connection later
    memcpy(ctx->residual_buffer, ctx->layernorm_output_buffer, total_elements * sizeof(float));
    _transformer2_apply_layer_norm(ctx, ctx->layernorm_output_buffer, ctx->temp_buffer_1, ctx->ln2_gamma, ctx->ln2_beta, seq_len);
    // Update layernorm_output_buffer to point to the normalized input for the FF module
    ctx->layernorm_output_buffer = ctx->temp_buffer_1;

    // 5. Feed-Forward Network
    _transformer2_feed_forward_module(ctx);
    // Output of FF is in ctx->layernorm_output_buffer

    // 6. Residual Connection + Add (After Layernorm + FF_Output)
    // Add the FF output to the previous residual (stored in residual_buffer)
    vector_add(ctx->current_output_sequence, ctx->residual_buffer, ctx->layernorm_output_buffer, total_elements);

    // The final output is now in ctx->current_output_sequence (which points to output_data)
}