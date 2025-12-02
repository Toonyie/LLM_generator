unsigned long long generate_key_for_block_v3(unsigned int block_identifier) {
    unsigned long long current_key_seed = (unsigned long long)block_identifier * 0x9E3779B97F4A7C15ULL; // Golden ratio for 64-bit
    unsigned long long shuffled_key = (current_key_seed ^ (current_key_seed >> 30)) * 0xBF58476D1CE4E5B9ULL;
    unsigned long long final_key_value = (shuffled_key ^ (shuffled_key >> 27)) * 0x94D049BB133111EBULL;
    return final_key_value ^ (final_key_value >> 31);
}