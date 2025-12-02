#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct transformer1_Config_v5;
struct transformer1_State_v5;

typedef void (*transformer1_AttentionFunc_v5)(
    const float *query_block,
    const float *key_block,
    const float *value_block,
    float *output_block,
    int seq_len,
    int model_dim,
    int num_heads,
    void* attention_context
);

static void transformer1_dummy_self_attention_v5(
    const float *query_block,
    const float *key_block,
    const float *value_block,
    float *output_block,
    int seq_len,
    int model_dim,
    int num_heads,
    void* attention_context
) {
    for (int i = 0; i < seq_len * model_dim; ++i) {
        output_block[i] = value_block[i] * 0.5f;
    }
}

typedef struct transformer1_Config_v5 {
    int model_dim;
    int num_heads;
    int seq_len;
    transformer1_AttentionFunc_v5 attention_impl;
    void* attention_ctx;
} transformer1_Config_v5;

typedef union transformer1_AttentionIntermediate_v5 {
    struct {
        float *q_proj;
        float *k_proj;
        float *v_proj;
    } qkv;
    float *attention_output;
} transformer1_AttentionIntermediate_v5;

typedef struct transformer1_State_v5 {
    float *attention_memory_block;
    size_t attention_memory_block_size_bytes;

    transformer1_AttentionIntermediate_v5 intermediate_attention_data;

    float *ffn_intermediate_buffer;
    size_t ffn_intermediate_size_bytes;

    float *qkv_weights;
    float *qkv_bias;
    float *output_weights;
    float *output_bias;
    float *ffn1_weights;
    float *ffn1_bias;
    float *ffn2_weights;
    float *ffn2_bias;
} transformer1_State_v5;

int transformer1_init_v5(transformer1_Config_v5 *config, transformer1_State_v5 *state) {
    if (!config || !state || !config->attention_impl) {
        return -1;
    }
    memset(state, 0, sizeof(transformer1_State_v5));

    size_t float_size = sizeof(float);
    size_t model_dim_val = (size_t)config->model_dim;
    size_t model_dim_sq = model_dim_val * model_dim_val;
    size_t seq_len_model_dim = (size_t)config->seq_len * model_dim_val;

    state->attention_memory_block_size_bytes = 3 * seq_len_model_dim * float_size;
    state->attention_memory_block = (float *)malloc(state->attention_memory_block_size_bytes);
    if (!state->attention_memory_block) return -1;

    state->intermediate_attention_data.qkv.q_proj = state->attention_memory_block;
    state->intermediate_attention_data.qkv.k_proj = state->attention_memory_block + seq_len_model_dim;
    state->intermediate_attention_data.qkv.v_proj = state->attention_memory_block + 2 * seq_len_model_dim;

    state->ffn_intermediate_size_bytes = (size_t)config->seq_len * (4 * model_dim_val) * float_size;
    state->ffn_intermediate_buffer = (float *)malloc(state->ffn_intermediate_size_bytes);
    if (!state->ffn_intermediate_buffer) {
        free(state->attention_memory_block);
        return -1;
    }

    #define MALLOC_CHECK(ptr_name, size_val) do { state->ptr_name = (float*)malloc(size_val); if (!state->ptr_name) goto fail_init; } while(0)
    MALLOC_CHECK(qkv_weights, 3 * model_dim_sq * float_size);
    MALLOC_CHECK(qkv_bias, 3 * model_dim_val * float_size);
    MALLOC_CHECK(output_weights, model_dim_sq * float_size);
    MALLOC_CHECK(output_bias, model_dim_val * float_size);
    MALLOC_CHECK(ffn1_weights, model_dim_val * (4 * model_dim_val) * float_size);
    MALLOC_CHECK(ffn1_bias, (4 * model_dim_val) * float_size);
    MALLOC_CHECK(ffn2_weights, (4 * model_dim_val) * model_dim_val * float_size);
    MALLOC_CHECK(ffn2_bias, model_dim_val * float_size);
    #undef MALLOC_CHECK

    memset(state->qkv_weights, 0, 3 * model_dim_sq * float_size);
    memset(state->qkv_bias, 0, 3 * model_dim_val * float_size);

    return 0;

fail_init:
    if (state->attention_memory_block) free(state->attention_memory_block);
    if (state->ffn_intermediate_buffer) free(state->ffn_intermediate_buffer);
    if (state->qkv_weights) free(state->qkv_weights);
    if (state->qkv_bias) free(state->qkv_bias);
    if (state->output_weights) free(state->output_weights);
    if (state->output_bias) free(state->output_bias);
    if (state->ffn1_weights) free(state->ffn1_weights);
    if (state->ffn1_bias) free(state->ffn1_bias);
    if (state->ffn2_weights) free(state->ffn2_weights);
    if (state->ffn2_bias) free(state->ffn2_bias);
    memset(state, 0, sizeof(transformer1_State_v5));
    return -1;
}

int transformer1_process_v5(const transformer1_Config_v5 *config, transformer1_State_v5 *state, const float *input, float *output) {
    if (!config || !state || !input || !output || !config->attention_impl) {
        return -1;
    }

    size_t seq_len_model_dim = (size_t)config->seq_len * config->model_dim;

    float *q_proj = state->intermediate_attention_data.qkv.q_proj;
    float *k_proj = state->intermediate_attention_data.qkv.k_proj;
    float *v_proj = state->intermediate_attention_data.qkv.v_proj;

    for (size_t i = 0; i < seq_len_model_dim; ++i) {
        q_proj[i] = input[i] * 1.0f + state->qkv_bias[i % config->model_dim];
        k_proj[i] = input[i] * 1.1f + state->qkv_bias[i % config->model_dim + config->model_dim];
        v_proj[i] = input[i] * 1.2f + state->qkv_bias[i % config->model_dim + 2 * config->model_dim];
    }

    config->attention_impl(q_proj, k_proj, v_proj, state->attention_memory_block,
                            config->seq_len, config->model_dim, config->num_heads, config->attention_ctx);

    float *attention_output_result = state->attention_memory_block;

    memcpy(state->ffn_intermediate_buffer, attention_output_result, seq_len_model_dim * sizeof(float));

    for (size_t i = 0; i < seq_len_model_dim; ++i) {
        output[i] = state->ffn_intermediate_buffer[i] * 0.7f + state->ffn2_bias[i % config->model_dim];
    }

    return 0;
}

void transformer1_deinit_v5(transformer1_State_v5 *state) {
    if (!state) return;

    if (state->attention_memory_block) {
        free(state->attention_memory_block);
        state->attention_memory_block = NULL;
    }

    if (state->ffn_intermediate_buffer) {
        free(state->ffn_intermediate_buffer);
        state->ffn_intermediate_buffer = NULL;
    }

    if (state->qkv_weights) free(state->qkv_weights); state->qkv_weights = NULL;
    if (state->qkv_bias) free(state->qkv_bias); state->qkv_bias = NULL;
    if (state->output_weights) free(state->output_weights); state->output_weights = NULL;
    if (state->output_bias) free(state->output_bias); state->output_bias = NULL;
    if (state->ffn1_weights) free(state->ffn1_weights); state->ffn1_weights = NULL;
    if (state->ffn1_bias) free(state->ffn1_bias); state->ffn1_bias = NULL;
    if (state->ffn2_weights) free(state->ffn2_weights); state->ffn2_weights = NULL;
    if (state->ffn2_bias) free(state->ffn2_bias); state->ffn2_bias = NULL;

    memset(state, 0, sizeof(transformer1_State_v5));
}