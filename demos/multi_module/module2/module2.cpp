#include "u8scan/u8scan.h"
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>

// Define a test function using standard assertions
bool test_module2_advanced_utf8_handling() {
    // Test STL algorithm compatibility with CharRange
    std::string input = "Hello 世界! 123 🌍";
    auto range = u8scan::make_char_range(input);
    
    // Count digits using STL algorithms
    int digit_count = std::count_if(range.begin(), range.end(), u8scan::predicates::is_digit_ascii());
    assert(digit_count == 3);  // 1, 2, 3
    (void)digit_count;  // Suppress unused warning in release builds
    
    // Check if it contains multi-byte UTF-8
    bool has_utf8 = std::any_of(range.begin(), range.end(), 
        [](const u8scan::CharInfo& info) { return info.is_valid_utf8 && info.byte_count > 1; });
    assert(has_utf8 == true);
    (void)has_utf8;  // Suppress unused warning in release builds
    
    // Find first emoji
    auto emoji_it = std::find_if(range.begin(), range.end(), u8scan::predicates::is_emoji());
    assert(emoji_it != range.end());
    assert(emoji_it->byte_count == 4);  // Emoji (🌍) is 4 bytes
    (void)emoji_it;  // Suppress unused warning in release builds
    
    // Test the new is_lowercase_ascii and is_uppercase_ascii predicates
    std::string test_str = "AbCdEf123";
    auto test_range = u8scan::make_char_range(test_str);
    
    int lowercase_count = std::count_if(test_range.begin(), test_range.end(), u8scan::predicates::is_lowercase_ascii());
    int uppercase_count = std::count_if(test_range.begin(), test_range.end(), u8scan::predicates::is_uppercase_ascii());
    
    assert(lowercase_count == 3);  // b, d, f
    assert(uppercase_count == 3);  // A, C, E
    (void)lowercase_count; (void)uppercase_count;  // Suppress unused warnings in release builds
    
    return true;
}

// Custom class demonstrating u8scan usage
class UTF8TextProcessor {
private:
    std::string text_;
public:
    UTF8TextProcessor(const std::string& text) : text_(text) {}
    
    std::string to_uppercase() const {
        std::string result;
        u8scan::scan_utf8(text_, [&result](const u8scan::CharInfo& info, const char* /*data*/) {
            result.append(u8scan::to_upper_ascii_str(info));
            return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
        });
        return result;
    }
    
    std::string to_lowercase() const {
        std::string result;
        u8scan::scan_utf8(text_, [&result](const u8scan::CharInfo& info, const char* /*data*/) {
            result.append(u8scan::to_lower_ascii_str(info));
            return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
        });
        return result;
    }
    
    std::string to_quoted(char quote_char = '"', char escape_char = '\\') const {
        return u8scan::quoted_str(text_, quote_char, quote_char, escape_char);
    }
    
    std::map<std::string, int> analyze() const {
        std::map<std::string, int> stats;
        stats["total"] = 0;
        stats["ascii"] = 0;
        stats["utf8_multibyte"] = 0;
        stats["digits"] = 0;
        stats["spaces"] = 0;
        
        u8scan::scan_utf8(text_, [&stats](const u8scan::CharInfo& info, const char*) {
            stats["total"]++;
            
            if (info.is_ascii) {
                stats["ascii"]++;
                
                if (u8scan::predicates::is_digit_ascii()(info)) {
                    stats["digits"]++;
                }
                
                if (u8scan::predicates::is_whitespace_ascii()(info)) {
                    stats["spaces"]++;
                }
            } else if (info.is_valid_utf8 && info.byte_count > 1) {
                stats["utf8_multibyte"]++;
            }
            
            return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
        });
        
        return stats;
    }
};

// Function that uses u8scan to process complex UTF-8 strings
std::string module2_process_complex_utf8() {
    std::string result;
    
    // Process UTF-8 text with advanced features
    UTF8TextProcessor processor("Hello 世界! 123 🌍 Test");
    
    result += "Original text: Hello 世界! 123 🌍 Test\n";
    result += "Uppercase: " + processor.to_uppercase() + "\n";
    result += "Lowercase: " + processor.to_lowercase() + "\n";
    result += "Quoted: " + processor.to_quoted() + "\n";
    
    // Text statistics
    auto stats = processor.analyze();
    result += "\nText statistics:\n";
    result += "  Total characters: " + std::to_string(stats["total"]) + "\n";
    result += "  ASCII characters: " + std::to_string(stats["ascii"]) + "\n";
    result += "  UTF-8 multi-byte: " + std::to_string(stats["utf8_multibyte"]) + "\n";
    result += "  Digits: " + std::to_string(stats["digits"]) + "\n";
    result += "  Spaces: " + std::to_string(stats["spaces"]) + "\n";
    
    // STL algorithm example
    std::string sample = "Example with UTF-8: 世界 and emoji 🌍";
    auto range = u8scan::make_char_range(sample);
    
    result += "\nSTL Algorithm example:\n";
    result += "  Input: " + sample + "\n";
    
    // Find multi-byte characters
    std::vector<u8scan::CharInfo> multibyte_chars;
    std::copy_if(range.begin(), range.end(), 
                 std::back_inserter(multibyte_chars),
                 [](const u8scan::CharInfo& info) { return info.byte_count > 1; });
    
    result += "  Multi-byte characters found: " + std::to_string(multibyte_chars.size()) + "\n";
    
    // Find first UTF-8 character
    auto first_utf8 = std::find_if(range.begin(), range.end(),
                                 [](const u8scan::CharInfo& info) { return info.byte_count > 1; });
    
    if (first_utf8 != range.end()) {
        result += "  First UTF-8 character at position: " + 
                  std::to_string(std::distance(range.begin(), first_utf8)) + "\n";
        result += "  Byte count: " + std::to_string(first_utf8->byte_count) + "\n";
    }
    
    // Using is_lowercase_ascii and is_uppercase_ascii predicates
    result += "\nCase analysis using predicates:\n";
    
    // Count lowercase and uppercase letters
    int lowercase_count = std::count_if(range.begin(), range.end(), u8scan::predicates::is_lowercase_ascii());
    int uppercase_count = std::count_if(range.begin(), range.end(), u8scan::predicates::is_uppercase_ascii());
    
    result += "  Lowercase letters (ASCII): " + std::to_string(lowercase_count) + "\n";
    result += "  Uppercase letters (ASCII): " + std::to_string(uppercase_count) + "\n";
    
    // Find first lowercase and uppercase letters
    auto first_lowercase = std::find_if(range.begin(), range.end(), u8scan::predicates::is_lowercase_ascii());
    auto first_uppercase = std::find_if(range.begin(), range.end(), u8scan::predicates::is_uppercase_ascii());
    
    if (first_lowercase != range.end()) {
        result += "  First lowercase letter: '" + std::string(1, static_cast<char>(first_lowercase->codepoint)) + 
                  "' at position " + std::to_string(std::distance(range.begin(), first_lowercase)) + "\n";
    }
    
    if (first_uppercase != range.end()) {
        result += "  First uppercase letter: '" + std::string(1, static_cast<char>(first_uppercase->codepoint)) + 
                  "' at position " + std::to_string(std::distance(range.begin(), first_uppercase)) + "\n";
    }
    
    return result;
}

// Function to run the test
bool module2_run_test() {
    std::cout << "Running Module2 advanced UTF-8 handling test..." << std::endl;
    bool success = test_module2_advanced_utf8_handling();
    if (success) {
        std::cout << "Module2 test passed!" << std::endl;
    } else {
        std::cout << "Module2 test failed!" << std::endl;
    }
    return success;
}
