void transformer1_block(const float* block_input, float* block_output, size_t sequence_length, size_t embedding_dimension) {
    size_t total_elements = sequence_length * embedding_dimension;

    // Declare intermediate buffers on the stack (assuming stack size allows for typical transformer dims)
    float mha_intermediate_result[total_elements];
    float ffn_intermediate_result[total_elements];
    float residual_sum_mha[total_elements]; // Buffer for MHA + Input
    float residual_sum_ffn[total_elements]; // Buffer for FFN + MHA_normed

    // Forward pass through Multi-Head Attention
    // Assumes transformer1_multi_head_attention takes input, writes to output
    transformer1_multi_head_attention(block_input, mha_intermediate_result, sequence_length, embedding_dimension);

    // Add & Normalize 1: Add residual connection and apply layer normalization
    // Residual connection: block_input + mha_intermediate_result
    for (size_t i = 0; i < total_elements; ++i) {
        residual_sum_mha[i] = block_input[i] + mha_intermediate_result[i];
    }
    transformer1_layer_norm(residual_sum_mha, sequence_length, embedding_dimension); // Normalize the summed output

    // Forward pass through Feed-Forward Network
    // Assumes transformer1_feed_forward_network takes input (normalized MHA output), writes to output
    transformer1_feed_forward_network(residual_sum_mha, ffn_intermediate_result, sequence_length, embedding_dimension);

    // Add & Normalize 2: Add residual connection and apply layer normalization
    // Residual connection: residual_sum_mha + ffn_intermediate_result
    for (size_t i = 0; i < total_elements; ++i) {
        residual_sum_ffn[i] = residual_sum_mha[i] + ffn_intermediate_result[i];
    }
    transformer1_layer_norm(residual_sum_ffn, sequence_length, embedding_dimension); // Normalize the summed output

    // The final result of this transformer block is stored in block_output
    for (size_t i = 0; i < total_elements; ++i) {
        block_output[i] = residual_sum_ffn[i];
    }
}

// Dummy declarations for assumed helper functions (implementations would be elsewhere)
// These are required for the above code to be syntactically valid C.
void transformer1_layer_norm(float* data, size_t seq_len, size_t embed_dim) {
    // Placeholder for layer normalization logic
    // (e.g., mean subtraction, variance division, scaling, and shifting)
}

void transformer1_multi_head_attention(const float* input_data, float* output_data, size_t seq_len, size_t embed_dim) {
    // Placeholder for multi-head attention logic
    // (e.g., query, key, value projections, dot product attention, concatenation, linear projection)
    for (size_t i = 0; i < seq_len * embed_dim; ++i) {
        output_data[i] = input_data[i] * 0.5f; // Example operation
    }
}

void transformer1_feed_forward_network(const float* input_data, float* output_data, size_t seq_len, size_t embed_dim) {
    // Placeholder for feed-forward network logic
    // (e.g., two linear transformations with an activation function in between)
    for (size_t i = 0; i < seq_len * embed_dim; ++i) {
        output_data[i] = input_data[i] * 2.0f; // Example operation
    }
}