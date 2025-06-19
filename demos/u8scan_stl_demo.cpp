#include "../include/u8scan/u8scan.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <numeric>

using namespace u8scan;

/**
 * @brief Demo: STL algorithms with string iterators
 */
void demo_stl_algorithms() {
    std::cout << "=== STL Algorithms Demo ===" << std::endl;
    
    std::string input = "Hello 世界! 123 🌍 Test.";
    std::cout << "Input: " << input << std::endl;
    
    auto char_range = make_char_range(input);
    
    // 1. std::all_of - check if all characters are valid UTF-8
    bool all_valid = std::all_of(char_range.begin(), char_range.end(), predicates::is_valid());
    std::cout << "All characters valid UTF-8: " << (all_valid ? "YES" : "NO") << std::endl;
    
    // 2. std::any_of - check if any characters are ASCII
    bool has_ascii = std::any_of(char_range.begin(), char_range.end(), predicates::is_ascii());
    std::cout << "Contains ASCII characters: " << (has_ascii ? "YES" : "NO") << std::endl;
    
    // 3. std::none_of - check if no characters are invalid
    bool no_invalid = std::none_of(char_range.begin(), char_range.end(), 
        [](const CharInfo& info) { return !info.is_valid_utf8; });
    std::cout << "No invalid characters: " << (no_invalid ? "YES" : "NO") << std::endl;
    
    // 4. std::count_if - count digits
    auto digit_count = std::count_if(char_range.begin(), char_range.end(), predicates::is_digit_ascii());
    std::cout << "Digit count: " << digit_count << std::endl;
    
    // 5. std::count_if - count UTF-8 multi-byte characters
    auto utf8_count = std::count_if(char_range.begin(), char_range.end(), predicates::is_utf8());
    std::cout << "UTF-8 multi-byte character count: " << utf8_count << std::endl;
    
    // 6. std::find_if - find first non-ASCII character
    auto first_utf8 = std::find_if(char_range.begin(), char_range.end(), predicates::is_utf8());
    if (first_utf8 != char_range.end()) {
        std::cout << "First UTF-8 character at position: " << first_utf8->start_pos 
                  << ", codepoint: U+" << std::hex << std::uppercase << first_utf8->codepoint 
                  << std::dec << std::endl;
    }
    
    std::cout << std::endl;
}

/**
 * @brief Demo: STL-only approach
 */
void demo_stl_only_approach() {
    std::cout << "\n=== STL-Only Approach Demo ===" << std::endl;
    
    std::string input = "Hello 世界! Test 123.";
    std::cout << "Input: " << input << std::endl;
    
    // 1. Use transform_chars to convert to uppercase (ASCII only)
    std::vector<char> uppercase_chars;
    transform_chars(input, std::back_inserter(uppercase_chars), [](const CharInfo& info) -> char {
        if (predicates::is_lowercase_ascii()(info)) {
            return static_cast<char>(info.codepoint - 32);  // Convert to uppercase
        }
        return static_cast<char>(info.codepoint);  // Return as-is for non-lowercase ASCII
    });
    
    std::string uppercase_result(uppercase_chars.begin(), uppercase_chars.end());
    std::cout << "Uppercase (ASCII only): " << uppercase_result << std::endl;
    
    // 2. STL-only quoted_str implementation
    std::string quoted_result = quoted_str(input, '[', ']', '\\');
    std::cout << "Quoted with custom delimiters: " << quoted_result << std::endl;
    
    // 3. Pure STL algorithms for counting and filtering
    auto char_range = make_char_range(input);
    
    auto ascii_count = std::count_if(char_range.begin(), char_range.end(), predicates::is_ascii());
    auto utf8_count = std::count_if(char_range.begin(), char_range.end(), predicates::is_utf8());
    auto digit_count = std::count_if(char_range.begin(), char_range.end(), predicates::is_digit_ascii());
    
    std::cout << "Character analysis:" << std::endl;
    std::cout << "  ASCII characters: " << ascii_count << std::endl;
    std::cout << "  UTF-8 multi-byte: " << utf8_count << std::endl;
    std::cout << "  Digits: " << digit_count << std::endl;
    
    // 4. STL algorithms for validation
    bool all_valid = std::all_of(char_range.begin(), char_range.end(), predicates::is_valid());
    bool has_whitespace = std::any_of(char_range.begin(), char_range.end(), predicates::is_whitespace_ascii());
    
    std::cout << "Validation:" << std::endl;
    std::cout << "  All characters valid: " << (all_valid ? "YES" : "NO") << std::endl;
    std::cout << "  Contains whitespace: " << (has_whitespace ? "YES" : "NO") << std::endl;
}

/**
 * @brief Demo: Alternative quoted_str using STL algorithms
 */
void demo_stl_quoted_str() {
    std::cout << "\n=== STL Algorithm-based quoted_str Demo ===" << std::endl;
    
    std::vector<std::string> test_strings = {
        "simple",
        "with\"quotes",
        "with\\escape",
        "Hello 世界!",
        "emoji 🌍 test",
        ""
    };
    
    for (const auto& str : test_strings) {
        std::cout << "Original: '" << str << "'" << std::endl;
        
        // Standard quoted_str 
        std::string standard_quoted = quoted_str(str);
        std::cout << "Standard:  " << standard_quoted << std::endl;
        
        // Custom quoted_str
        std::string custom_quoted = quoted_str(str, '<', '>', '/');
        std::cout << "Custom:    " << custom_quoted << std::endl;
        
        std::cout << std::endl;
    }
}

int main() {
    try {
        demo_stl_algorithms();
        demo_stl_only_approach();
        demo_stl_quoted_str();
        
        std::cout << "=== All STL Demos Completed Successfully ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
