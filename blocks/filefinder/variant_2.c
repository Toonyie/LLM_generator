typedef enum {
    FF_TYPE_UNKNOWN_V2 = 0,
    FF_TYPE_REGULAR_FILE_V2,
    FF_TYPE_DIRECTORY_V2,
    FF_TYPE_SYMLINK_V2,
    FF_TYPE_SPECIAL_V2
} FileFinderNodeType_V2;

typedef struct {
    const char* path_name_v2;
    unsigned long file_size_bytes_v2;
    unsigned int creation_time_v2;
    unsigned int last_access_time_v2;
    FileFinderNodeType_V2 node_type_v2;
    unsigned short permissions_v2;
} FileFinderResultItem_V2;

typedef int (*FileFinderCallbackFn_V2)(const FileFinderResultItem_V2* item, void* user_arg_v2);

void filefinder_execute_search_v2(const char* root_path_v2,
                                  const char* search_pattern_v2,
                                  int max_depth_v2,
                                  FileFinderCallbackFn_V2 result_callback_v2,
                                  void* user_data_v2);