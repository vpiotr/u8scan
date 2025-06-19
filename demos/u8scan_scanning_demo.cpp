#include "../include/u8scan/u8scan.h"
#include <iostream>

using namespace u8scan;

/**
 * @brief Demo: Simplified usage
 */
void demo_simplified_usage() {
    std::cout << "\n=== Simplified Usage Demo ===" << std::endl;
    
    std::string input = "Hello 世界! 123";
    std::cout << "Input: " << input << std::endl;
    
    // Use the simplified scan_utf8 function
    auto filter_processor = [](const CharInfo& info, const char*) -> ProcessResult {
        if (info.is_ascii && predicates::is_digit_ascii()(info)) {
            return ProcessResult(ScanAction::IGNORE);  // Remove digits
        }
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    std::string no_digits = scan_utf8(input, filter_processor);
    std::cout << "Without digits: " << no_digits << std::endl;
    
    // Use the simplified scan_ascii function for ASCII-only processing
    auto upper_processor = [](const CharInfo& info, const char*) -> ProcessResult {
        if (predicates::is_lowercase_ascii()(info)) {
            char upper = static_cast<char>(info.codepoint - 32);
            return ProcessResult(ScanAction::REPLACE, std::string(1, upper));
        }
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    std::string ascii_only = "hello world";
    std::string uppercase = scan_ascii(ascii_only, upper_processor);
    std::cout << "ASCII uppercase: " << uppercase << std::endl;
}

int main() {
    try {
        demo_simplified_usage();
        
        std::cout << "=== Scanning Demos Completed Successfully ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
