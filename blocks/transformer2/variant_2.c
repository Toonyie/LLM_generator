#include <math.h> // For sqrtf
#include <stddef.h> // For size_t, and potential use with restrict

// Define a configuration structure for the transformer block.
// This allows passing various parameters and (pointers to) weights easily.
typedef struct {
    int sequence_length; // Number of tokens in the sequence
    int model_dimension; // Dimensionality of the model (d_model)
    int number_of_heads; // Number of attention heads (not explicitly used in this simplified variant)
    float dropout_rate;  // Dropout probability (not implemented in this simplified version)

    // Placeholder for actual weight pointers.
    // In a full implementation, these would point to allocated memory for weights/biases.
    // For this simplified example, assume these pointers are valid for the d_model size.
    float* attention_qkv_weights;    // Example: For Query, Key, Value linear transformations
    float* attention_output_weights; // Example: For attention output projection
    float* ffn_layer1_weights;       // Example: First layer of feed-forward network
    float* ffn_layer2_weights;       // Example: Second layer of feed-forward network
    float* norm1_gamma;              // Layer norm scale for first sub-layer (attention)
    float* norm1_beta;               // Layer norm bias for first sub-layer (attention)
    float* norm2_gamma;              // Layer norm scale for second sub-layer (feed-forward)
    float* norm2_beta;               // Layer norm bias for second sub-layer (feed-forward)
} TransformerBlockConfig_v2;

// Helper function for Layer Normalization.
// This variant uses a `static` keyword and takes gamma/beta explicitly as const pointers.
static void apply_layer_normalization_v2(
    float* restrict data_to_normalize, // Input/Output buffer for features of a single token
    const float* restrict gamma,       // Scaling parameter for LayerNorm
    const float* restrict beta,        // Bias parameter for LayerNorm
    int feature_size                   // Dimension of the features (d_model)
) {
    float sum_val = 0.0f;
    float sum_sq_val = 0.0f;

    // Calculate mean
    for (int idx = 0; idx < feature_size; ++idx) {
        sum_val += data_to_normalize[idx];
    }
    float mean_val = sum_val / feature_size;

    // Calculate variance
    for (int idx = 0; idx < feature_size; ++idx) {
        float diff_val = data_to_normalize[idx] - mean_val;
        sum_sq_val += diff_val * diff_val;
    }
    float variance_val = sum_sq_val / feature_size;

    // Apply normalization
    float inv_std_dev_val = 1.0f / sqrtf(variance_val + 1e-5f); // Add epsilon for numerical stability

    for (int idx = 0; idx < feature_size; ++idx) {
        data_to_normalize[idx] = (data_to_normalize[idx] - mean_val) * inv_std_dev_val;
        data_to_normalize[idx] = data_to_normalize[idx] * gamma[idx] + beta[idx];
    }
}

// Main function for the transformer block, variant #2.
// This variant takes a configuration struct pointer and uses explicit input/output pointers,
// and utilizes pre-LayerNorm architecture.
void execute_transformer_block_v2(
    const float* restrict input_sequence_ptr,        // Input feature sequence (seq_len * d_model)
    float* restrict output_sequence_ptr,             // Output feature sequence (seq_len * d_model)
    const TransformerBlockConfig_v2* restrict block_config // Configuration and weight pointers
) {
    const int seq_len_v2 = block_config->sequence_length;
    const int d_model_v2 = block_config->model_dimension;
    const size_t total_elements = (size_t)seq_len_v2 * d_model_v2;

    // Allocate temporary buffers on the stack using Variable Length Arrays (VLAs).
    // These buffers are used to store intermediate results for the entire sequence.
    float buffer_attn_output[total_elements]; // Stores output of attention sub-layer
    float buffer_ffn_output[total_elements];  // Stores output of feed-forward sub-layer
    
    // A small buffer for processing features of a single token during LayerNorm
    float single_token_features_buf[d_model_v2];

    // --- First Sub-layer: Multi-Head Attention + Add & Norm ---
    // (Using a Pre-LayerNorm design)

    // Step 1a: Apply Layer Normalization to the input sequence (token by token)
    // The normalized input is then fed into the attention mechanism.
    for (int i = 0; i < seq_len_v2; ++i) {
        // Copy features for current token into a mutable buffer
        for (int j = 0; j < d_model_v2; ++j) {
            single_token_features_buf[j] = input_sequence_ptr[i * d_model_v2 + j];
        }
        // Normalize the current token's features
        apply_layer_normalization_v2(
            single_token_features_buf,
            block_config->norm1_gamma,
            block_config->norm1_beta,
            d_model_v2
        );
        // Copy normalized features to the temporary buffer for attention input
        for (int j = 0; j < d_model_v2; ++j) {
            buffer_attn_output[i * d_model_v2 + j] = single_token_features_buf[j];
        }
    }

    // Step 1b: Simulate Multi-Head Attention operation (highly simplified placeholder)
    // In a real transformer, this involves complex matrix multiplications and softmax.
    // Here, we just perform a placeholder transformation on `buffer_attn_output`.
    for (size_t k = 0; k < total_elements; ++k) {
        // Example: A weighted blend of normalized input and original input to simulate attention output
        buffer_attn_output[k] = buffer_attn_output[k] * 0.55f + input_sequence_ptr[k] * 0.45f;
    }

    // Step 1c: First Residual Connection (Original Input + Attention Output)
    // The result serves as the input to the next sub-layer, before its own normalization.
    for (size_t k = 0; k < total_elements; ++k) {
        buffer_ffn_output[k] = input_sequence_ptr[k] + buffer_attn_output[k];
    }


    // --- Second Sub-layer: Feed-Forward Network + Add & Norm ---

    // Step 2a: Apply Layer Normalization to the output of the first residual connection
    // (token by token) before feeding it to the FFN.
    for (int i = 0; i < seq_len_v2; ++i) {
        // Copy features for current token into a mutable buffer
        for (int j = 0; j < d_model_v2; ++j) {
            single_token_features_buf[j] = buffer_ffn_output[i * d_model_v2 + j];
        }
        // Normalize the current token's features
        apply_layer_normalization_v2(
            single_token_features_buf,
            block_config->norm2_gamma,
            block_config->norm2_beta,
            d_model_v2
        );
        // Copy normalized features to a temporary buffer, preparing for FFN input
        for (int j = 0; j < d_model_v2; ++j) {
            buffer_attn_output[i * d_model_v2 + j] = single_token_features_buf[j]; // Reusing buffer_attn_output
        }
    }

    // Step 2b: Simulate Feed-Forward Network operation (highly simplified placeholder)
    // FFN typically involves two linear layers with an activation in between.
    // We apply a placeholder transformation on the normalized input from the previous step.
    for (size_t k = 0; k < total_elements; ++k) {
        buffer_ffn_output[k] = buffer_attn_output[k] * 1.2f; // Example: A simple scaling
    }

    // Step 2c: Second Residual Connection (Output of first Add&Norm + FFN Output)
    // This produces the final output of the entire transformer block.
    for (size_t k = 0; k < total_elements; ++k) {
        output_sequence_ptr[k] = buffer_attn_output[k] + buffer_ffn_output[k];
    }
}