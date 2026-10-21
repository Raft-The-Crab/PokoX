#include "poko/core/Result.hpp"
#include <cassert>
#include <string>

using namespace poko::core;

void test_result_success() {
    Result<int> result(42);
    assert(result.is_ok());
    assert(!result.is_error());
    assert(result.value() == 42);
}

void test_result_error() {
    Result<int> result(Error(ErrorCode::InvalidArgument, "test error"));
    assert(!result.is_ok());
    assert(result.is_error());
    assert(result.error().code() == ErrorCode::InvalidArgument);
    assert(result.error().message() == "test error");
}

void test_result_value_or() {
    Result<int> success(42);
    assert(success.value_or(100) == 42);
    
    Result<int> failure(Error(ErrorCode::ResourceNotFound, "not found"));
    assert(failure.value_or(100) == 100);
}

void test_error_to_string() {
    Error err1(ErrorCode::InvalidArgument, "test");
    std::string str1 = err1.to_string();
    assert(str1.find("InvalidArgument") != std::string::npos);
    assert(str1.find("test") != std::string::npos);
    
    Error err2(ErrorCode::Success);
    assert(err2.is_success());
    assert(err2.to_string() == "Success");
}

void test_result_string_type() {
    Result<std::string> result("hello");
    assert(result.is_ok());
    assert(result.value() == "hello");
}
