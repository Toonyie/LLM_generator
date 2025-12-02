#include <math.h>

#define BLOCK_SEQ_LEN 10
#define BLOCK_MODEL_DIM 64
#define BLOCK_FF_DIM 256

void transformer1_block_variant3(
    float* restrict output_matrix,
    const float* restrict input_matrix,
    const float* restrict q_proj_weights,
    const float* restrict k_proj_weights,
    const float* restrict v_proj_weights,
    const float* restrict o_proj_weights,
    const float* restrict ff_layer1_weights,
    const float* restrict ff_layer2_weights)
{
    float query_vals[BLOCK_SEQ_LEN][BLOCK_MODEL_DIM];
    float key_vals[BLOCK_SEQ_LEN][BLOCK_MODEL_DIM];
    float value_vals[BLOCK_SEQ_LEN][BLOCK_MODEL_DIM];

    float attention_logits[BLOCK_SEQ_LEN][BLOCK_SEQ_LEN];
    float attention_probs[BLOCK_SEQ_LEN][BLOCK_SEQ_LEN];

    float attention_result_pre_proj[BLOCK_SEQ_LEN][BLOCK_MODEL_DIM];
    float attention_output_final[BLOCK_SEQ_LEN][BLOCK_MODEL_DIM];

    float post_attn_residual[BLOCK_SEQ_LEN][BLOCK_MODEL_DIM];
    float ff_intermediate_output[BLOCK_SEQ_LEN][BLOCK_FF_DIM];
    float ff_final_output[BLOCK_SEQ_LEN][BLOCK_MODEL_DIM];

    for (int current_seq_pos = 0; current_seq_pos < BLOCK_SEQ_LEN; ++current_seq_pos) {
        for (int output_feature_idx = 0; output_feature_idx < BLOCK_MODEL_DIM; ++output_feature_idx) {
            query_vals[current_seq_pos][output_feature_idx] = 0.0f;
            key_vals[current_seq_pos][output_feature_idx] = 0.0f;
            value_vals[current_seq_pos][output_feature_idx] = 0.0f;
            for (int input_feature_idx = 0; input_feature_idx < BLOCK_MODEL_DIM; ++input_feature_idx) {
                query_vals[current_seq_pos][output_feature_idx] += input_matrix[current_seq_pos * BLOCK_MODEL_DIM + input_feature_idx] * q_proj_weights[input_feature_idx * BLOCK_MODEL_DIM + output_feature_idx];
                key_vals[current_seq_pos][output_feature_idx] += input_matrix[current_seq_pos * BLOCK_MODEL_DIM + input_feature_idx] * k_proj_weights[input_feature_idx * BLOCK_MODEL_DIM + output_feature_idx];
                value_vals[current_seq_pos][output_feature_idx] += input_matrix[current_seq_pos * BLOCK_MODEL_DIM + input_feature_idx] * v_proj_weights[input_feature_idx * BLOCK_MODEL_DIM + output_feature_idx];
            }
        }
    }

    const float inv_sqrt_model_dim = 1.0f / sqrtf((float)BLOCK_MODEL_DIM);
    for (int query_pos_idx = 0; query_pos_idx < BLOCK_SEQ_LEN; ++query_pos_idx) {
        for (int key_pos_idx = 0; key_pos_idx < BLOCK_SEQ_LEN; ++key_pos_idx) {
            attention_logits[query_pos_idx][key_pos_idx] = 0.0f;
            for (int feature_elem_idx = 0; feature_elem_idx < BLOCK_MODEL_DIM; ++feature_elem_idx) {
                attention_logits[query_pos_idx][key_pos_idx] += query_vals[query_pos_idx][feature_elem_idx] * key_vals[key_pos_idx][feature_elem_idx];
            }
            attention_logits[query_pos_idx][key_pos_idx] *= inv_sqrt_model_dim;
        }
    }

    for (int softmax_row_idx = 0; softmax_row_idx < BLOCK_SEQ_LEN; ++softmax_row_idx) {
        float max_val_in_row = attention_logits[softmax_row_idx][0];
        for (int col_traverse = 1; col_traverse < BLOCK_SEQ_LEN; ++col_traverse) {
            if (attention_logits[softmax_row_idx][col_traverse] > max_val_in_row) {
                max_val_in_row = attention_logits[softmax_row_idx][col_traverse];
            }
        }

        float exponentiated_sum = 0.0f;
        for (int col_traverse = 0; col_traverse < BLOCK_SEQ_LEN; ++col_traverse) {
            attention_probs[softmax_row_idx][col_traverse] = expf(attention_logits[softmax_row_idx][col_traverse] - max_val_in_row);
            exponentiated_sum += attention_probs[softmax_row_idx][col_traverse];
        }

        float inv_exp_sum = 1.0f / exponentiated_sum;
        for (int col_traverse = 0; col_traverse < BLOCK_SEQ_LEN; ++col_traverse) {
            attention_probs[softmax_row_idx][col_traverse] *= inv_exp_sum;
        }
    }

    for (int output_seq_idx = 0; output_seq_idx < BLOCK_SEQ_LEN; ++output_seq_idx) {
        for (int output_dim_elem = 0; output_dim_elem < BLOCK_MODEL_DIM; ++output_dim_elem) {
            attention_result_pre_proj[output_seq_idx][output_dim_elem] = 0.0f;
            for (int value_source_idx = 0; value_source_idx < BLOCK_SEQ_LEN; ++value_source_idx) {
                attention_result_pre_proj[output_seq_idx][output_dim_elem] += attention_probs[output_seq_idx][value_source_idx] * value_vals[value_source_idx][output_dim_elem];
            }
        }
    }

    for (int row_val = 0; row_val < BLOCK_SEQ_LEN; ++row_val) {
        for (int col_val = 0; col_val < BLOCK_MODEL_DIM; ++col_val) {
            attention_output_final[row_val][col_val] = 0.0f;
            for (int k_projection_val = 0; k_projection_val < BLOCK_MODEL_DIM; ++k_projection_val) {
                attention_output_final[row_val][col_val] += attention_result_pre_proj[row_val][k_projection_val] * o_proj_weights[k_projection_val * BLOCK_MODEL_DIM + col_val];
            }
        }
    }

    for (int res_add_idx_r = 0; res_add_idx_r < BLOCK_SEQ_LEN; ++res_add_idx_r) {
        for (int res_add_idx_c = 0; res_add_idx_c < BLOCK_MODEL_DIM; ++res_add_idx_c) {
            post_attn_residual[res_add_idx_r][res_add_idx_c] = input_matrix[res_add_idx_r * BLOCK_MODEL_DIM + res_add_idx_c] + attention_output_final[res_add_idx_r][res_add_idx_c];
        }
    }

    for (int ff1_r_iter = 0; ff1_r_iter < BLOCK_SEQ_LEN; ++ff1_r_iter) {
        for (int ff1_c_iter = 0; ff1_c_iter < BLOCK_FF_DIM; ++ff1_c_iter) {
            ff_intermediate_output[ff1_r_iter][ff1_c_iter] = 0.0f;
            for (int ff1_k_iter = 0; ff1_k_iter < BLOCK_MODEL_DIM; ++ff1_k_iter) {
                ff_intermediate_output[ff1_r_iter][ff1_c_iter] += post_attn_residual[ff1_r_iter][ff1_k_iter] * ff_layer1_weights[ff1_k_iter * BLOCK_FF_DIM + ff1_c_iter];
            }
            ff_intermediate_output[ff1_r_iter][ff1_c_iter] = fmaxf(0.0f, ff_intermediate_output[ff1_r_iter][ff1_c_iter]);
        }
    }

    for (int ff2_r_iter = 0; ff2_r_iter < BLOCK_SEQ_LEN; ++ff2_r_iter) {
        for (int ff2_c_iter = 0; ff2_c_iter < BLOCK_MODEL_DIM; ++ff2_c_iter) {
            ff_final_output[ff2_r_iter][ff2_c_iter] = 0.0f;
            for (int ff2_k_iter = 0; ff2_k_iter < BLOCK_FF_DIM; ++ff2_k_iter) {
                ff_final_output[ff2_r_iter][ff2_c_iter] += ff_intermediate_output[ff2_r_iter][ff2_k_iter] * ff_layer2_weights[ff2_k_iter * BLOCK_MODEL_DIM + ff2_c_iter];
            }
        }
    }

    for (int final_row_proc = 0; final_row_proc < BLOCK_SEQ_LEN; ++final_row_proc) {
        for (int final_col_proc = 0; final_col_proc < BLOCK_MODEL_DIM; ++final_col_proc) {
            output_matrix[final_row_proc * BLOCK_MODEL_DIM + final_col_proc] = post_attn_residual[final_row_proc][final_col_proc] + ff_final_output[final_row_proc][final_col_proc];
        }
    }
}