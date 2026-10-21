#include <iostream>

// External test functions
void test_result_success();
void test_result_error();
void test_result_value_or();
void test_error_to_string();
void test_result_string_type();

void test_logger_singleton();
void test_logger_level_filtering();
void test_scoped_log_level();
void test_logger_thread_safety();

void test_entity_id_default();
void test_entity_id_construct();
void test_entity_id_equality();
void test_entity_id_comparison();
void test_entity_id_hash();
void test_entity_id_unordered_map();
void test_entity_id_invalid_static();

int main() {
    std::cout << "Running PokoCore Tests..." << std::endl;
    
    // Result tests
    test_result_success();
    test_result_error();
    test_result_value_or();
    test_error_to_string();
    test_result_string_type();
    std::cout << "Result tests passed" << std::endl;
    
    // Logger tests
    test_logger_singleton();
    test_logger_level_filtering();
    test_scoped_log_level();
    test_logger_thread_safety();
    std::cout << "Logger tests passed" << std::endl;
    
    // EntityId tests
    test_entity_id_default();
    test_entity_id_construct();
    test_entity_id_equality();
    test_entity_id_comparison();
    test_entity_id_hash();
    test_entity_id_unordered_map();
    test_entity_id_invalid_static();
    std::cout << "EntityId tests passed" << std::endl;
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
