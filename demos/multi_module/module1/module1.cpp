#include "u8scan/u8scan.h"
#include <string>
#include <iostream>
#include <cassert>

// Define a test function using standard assertions
bool test_module1_basic_utf8_handling() {
    // Test basic scanning functionality
    std::string input = u8"Hello 世界!";
    std::string output;
    
    // Scan and count characters
    int total_chars = 0;
    int ascii_chars = 0;
    int utf8_multibyte_chars = 0;
    
    u8scan::scan_utf8(input, [&](const u8scan::CharInfo& info, const char* data) {
        output.append(data, info.byte_count);
        total_chars++;
        if (info.is_ascii) {
            ascii_chars++;
        } else if (info.is_valid_utf8 && info.byte_count > 1) {
            utf8_multibyte_chars++;
        }
        return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
    });
    
    // Verify results
    assert(output == input);
    assert(total_chars == 8);  // H, e, l, l, o, <space>, 世, 界, !
    assert(ascii_chars == 6);  // H, e, l, l, o, <space>, !
    assert(utf8_multibyte_chars == 2);  // 世, 界
    
    // Test the new is_lowercase_ascii and is_uppercase_ascii predicates
    std::string case_test = "AbCdEf";
    auto range = u8scan::make_char_range(case_test);
    
    int lowercase_count = static_cast<int>(std::count_if(range.begin(), range.end(), u8scan::predicates::is_lowercase_ascii()));
    int uppercase_count = static_cast<int>(std::count_if(range.begin(), range.end(), u8scan::predicates::is_uppercase_ascii()));
    
    assert(lowercase_count == 3);  // b, d, f
    assert(uppercase_count == 3);  // A, C, E
    (void)lowercase_count; (void)uppercase_count;  // Suppress unused warnings in release builds
    
    // Test the to_lower_ascii and to_upper_ascii functions
    u8scan::CharInfo uppercase_a = *(u8scan::make_char_range("A").begin());
    u8scan::CharInfo lowercase_a = *(u8scan::make_char_range("a").begin());
    u8scan::CharInfo utf8_char = *(u8scan::make_char_range("世").begin());
    
    assert(u8scan::to_lower_ascii(uppercase_a) == 'a');
    assert(u8scan::to_lower_ascii(lowercase_a) == 'a');
    assert(u8scan::to_lower_ascii(utf8_char) == utf8_char.codepoint);  // Non-ASCII unchanged
    
    assert(u8scan::to_upper_ascii(uppercase_a) == 'A');
    assert(u8scan::to_upper_ascii(lowercase_a) == 'A');
    assert(u8scan::to_upper_ascii(utf8_char) == utf8_char.codepoint);  // Non-ASCII unchanged
    (void)uppercase_a; (void)lowercase_a; (void)utf8_char;  // Suppress unused warnings in release builds
    
    return true;
}

// Function that processes UTF-8 strings using u8scan
std::string module1_process_utf8() {
    std::string result;
    
    // Process a simple ASCII string
    std::string ascii_input = "Hello World 123";
    result += "ASCII input: " + ascii_input + "\n";
    
    // Count digits
    int digit_count = 0;
    u8scan::scan_utf8(ascii_input, [&digit_count](const u8scan::CharInfo& info, const char*) {
        if (u8scan::predicates::is_digit_ascii()(info)) {
            digit_count++;
        }
        return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
    });
    result += "Digit count: " + std::to_string(digit_count) + "\n";
    
    // Remove digits
    std::string no_digits;
    u8scan::scan_utf8(ascii_input, [&no_digits](const u8scan::CharInfo& info, const char* data) {
        if (!u8scan::predicates::is_digit_ascii()(info)) {
            no_digits.append(data, info.byte_count);
        }
        return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
    });
    result += "Without digits: " + no_digits + "\n";
    
    // Process a UTF-8 string
    std::string utf8_input = u8"Hello 世界!";
    result += "\nUTF-8 input: " + utf8_input + "\n";
    
    // Count characters by type
    int total = 0, ascii = 0, utf8_mb = 0;
    u8scan::scan_utf8(utf8_input, [&](const u8scan::CharInfo& info, const char*) {
        total++;
        if (info.is_ascii) ascii++;
        if (info.is_valid_utf8 && info.byte_count > 1) utf8_mb++;
        return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
    });
    
    result += "Total characters: " + std::to_string(total) + "\n";
    result += "ASCII characters: " + std::to_string(ascii) + "\n";
    result += "UTF-8 multi-byte: " + std::to_string(utf8_mb) + "\n";
    
    // Count lowercase letters using the new predicate
    auto char_range = u8scan::make_char_range(ascii_input);
    int lowercase_count = static_cast<int>(std::count_if(char_range.begin(), char_range.end(), u8scan::predicates::is_lowercase_ascii()));
    result += "\nLowercase letters count (using predicate): " + std::to_string(lowercase_count) + "\n";
    
    // Extract only lowercase letters
    std::string lowercase_only;
    u8scan::scan_utf8(ascii_input, [&lowercase_only](const u8scan::CharInfo& info, const char* data) {
        if (u8scan::predicates::is_lowercase_ascii()(info)) {
            lowercase_only.append(data, info.byte_count);
        }
        return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
    });
    result += "Lowercase letters only: " + lowercase_only + "\n";
    
    // Using to_lower_ascii_str function to convert text to lowercase (simplified)
    std::string text_lower;
    u8scan::scan_utf8(u8"Hello WORLD 123 世界!", [&text_lower](const u8scan::CharInfo& info, const char* /*data*/) {
        text_lower.append(u8scan::to_lower_ascii_str(info));
        return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
    });
    result += "Text to lowercase: " + text_lower + "\n";
    
    return result;
}

// Function to run the test
bool module1_run_test() {
    std::cout << "Running Module1 basic UTF-8 handling test..." << std::endl;
    bool success = test_module1_basic_utf8_handling();
    if (success) {
        std::cout << "Module1 test passed!" << std::endl;
    } else {
        std::cout << "Module1 test failed!" << std::endl;
    }
    return success;
}
