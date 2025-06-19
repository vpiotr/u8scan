#include "../include/utest/utest.h"
#include "../include/u8scan/u8scan.h"
#include <stdexcept>
#include <string>
#include <iostream>
#include <iomanip>
#include <iostream>
#include <iomanip>

using namespace u8scan;

// Helper function to print compact byte dump with positions
void print_byte_dump(const std::string& input) {
    std::cout << "\n--- Byte Dump ---" << std::endl;
    
    // Print position row
    std::cout << "Pos: ";
    for (size_t i = 0; i < input.size(); ++i) {
        std::cout << std::setw(2) << std::setfill(' ') << i << " ";
    }
    std::cout << std::endl;
    
    // Print hex values row
    std::cout << "Hex: ";
    for (size_t i = 0; i < input.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << (static_cast<unsigned char>(input[i]) & 0xFF) << std::dec << " ";
    }
    std::cout << std::endl;
    
    // Reset formatting to defaults
    std::cout << std::setfill(' ') << std::setw(0);
}

// Helper function to add BOM only if not present
std::string with_bom(const std::string& input) {
    if (has_bom(input)) {
        return input;  // BOM already present
    }
    return bom_str() + input;
}

// Helper function to remove BOM if present
std::string skip_bom(const std::string& input) {
    if (has_bom(input)) {
        return input.substr(3);  // Skip the 3-byte BOM
    }
    return input;
}

// Test at() function with ASCII string
UTEST_FUNC_DEF2(U8ScanAccess, AtFunctionASCII) {
    std::string input = "Hello World";
    
    // Test accessing first character
    auto first_char = at(input, 0);
    UTEST_ASSERT_EQUALS('H', static_cast<char>(first_char.codepoint));
    UTEST_ASSERT_TRUE(first_char.is_ascii);
    UTEST_ASSERT_TRUE(first_char.is_valid_utf8);
    
    // Test accessing middle character
    auto space_char = at(input, 5);
    UTEST_ASSERT_EQUALS(' ', static_cast<char>(space_char.codepoint));
    
    // Test accessing last character
    auto last_char = at(input, 10);
    UTEST_ASSERT_EQUALS('d', static_cast<char>(last_char.codepoint));
    
    // Test out of range access
    bool caught_exception = false;
    try {
        at(input, 11);  // Beyond string length
    } catch (const std::out_of_range&) {
        caught_exception = true;
    }
    UTEST_ASSERT_TRUE(caught_exception);
}

// Test at() function with UTF-8 string
UTEST_FUNC_DEF2(U8ScanAccess, AtFunctionUTF8) {
    std::string input = u8"Hello 世界! 🌍";
    
    // Test accessing ASCII character
    auto h_char = at(input, 0);
    UTEST_ASSERT_EQUALS('H', static_cast<char>(h_char.codepoint));
    UTEST_ASSERT_TRUE(h_char.is_ascii);
    
    // Test accessing Chinese character
    auto chinese_char = at(input, 6);  // '世'
    UTEST_ASSERT_EQUALS(0x4E16u, chinese_char.codepoint);  // Unicode for '世'
    UTEST_ASSERT_FALSE(chinese_char.is_ascii);
    UTEST_ASSERT_TRUE(chinese_char.is_valid_utf8);
    UTEST_ASSERT_EQUALS(3u, chinese_char.byte_count);
    
    // Test accessing emoji character
    auto emoji_char = at(input, 10);  // 🌍
    UTEST_ASSERT_EQUALS(0x1F30Du, emoji_char.codepoint);  // Unicode for 🌍
    UTEST_ASSERT_FALSE(emoji_char.is_ascii);
    UTEST_ASSERT_TRUE(emoji_char.is_valid_utf8);
    UTEST_ASSERT_EQUALS(4u, emoji_char.byte_count);
}

// Test at() function with BOM
UTEST_FUNC_DEF2(U8ScanAccess, AtFunctionWithBOM) {
    std::string input = with_bom(std::string("Hello"));  // UTF-8 BOM + Hello
    
    // First accessible character should be 'H', not BOM
    auto first_char = at(input, 0);
    UTEST_ASSERT_EQUALS('H', static_cast<char>(first_char.codepoint));
    UTEST_ASSERT_TRUE(first_char.is_ascii);
}

// Test empty() function
UTEST_FUNC_DEF2(U8ScanAccess, EmptyFunction) {
    // Test truly empty string
    std::string empty_str = "";
    UTEST_ASSERT_TRUE(empty(empty_str));
    
    // Test BOM-only string (should be considered empty)
    std::string bom_only = bom_str();  // UTF-8 BOM
    UTEST_ASSERT_TRUE(empty(bom_only));
    
    // Test non-empty string
    std::string non_empty = "Hello";
    UTEST_ASSERT_FALSE(empty(non_empty));
    
    // Test BOM + content (should not be empty)
    std::string bom_with_content = with_bom(std::string("Hello"));
    UTEST_ASSERT_FALSE(empty(bom_with_content));
    
    // Test UTF-8 content
    std::string utf8_content = u8"世界";
    UTEST_ASSERT_FALSE(empty(utf8_content));
}

// Test front() function
UTEST_FUNC_DEF2(U8ScanAccess, FrontFunction) {
    // Test ASCII string
    std::string ascii_str = "Hello World";
    auto front_char = front(ascii_str);
    UTEST_ASSERT_EQUALS('H', static_cast<char>(front_char.codepoint));
    UTEST_ASSERT_TRUE(front_char.is_ascii);
    
    // Test UTF-8 string
    std::string utf8_str = u8"世界Hello";
    auto front_utf8 = front(utf8_str);
    UTEST_ASSERT_EQUALS(0x4E16u, front_utf8.codepoint);  // Unicode for '世'
    UTEST_ASSERT_FALSE(front_utf8.is_ascii);
    UTEST_ASSERT_EQUALS(3u, front_utf8.byte_count);
    
    // Test with BOM
    std::string bom_string = with_bom(std::string("Hello"));
    auto front_after_bom = front(bom_string);
    UTEST_ASSERT_EQUALS('H', static_cast<char>(front_after_bom.codepoint));
    
    // Test empty string exception
    std::string empty_str = "";
    bool caught_exception = false;
    try {
        front(empty_str);
    } catch (const std::out_of_range&) {
        caught_exception = true;
    }
    UTEST_ASSERT_TRUE(caught_exception);
    
    // Test BOM-only string exception
    std::string bom_only = bom_str();
    caught_exception = false;
    try {
        front(bom_only);
    } catch (const std::out_of_range&) {
        caught_exception = true;
    }
    UTEST_ASSERT_TRUE(caught_exception);
}

// Test back() function
UTEST_FUNC_DEF2(U8ScanAccess, BackFunction) {
    // Test ASCII string
    std::string ascii_str = "Hello World";
    auto back_char = back(ascii_str);
    UTEST_ASSERT_EQUALS('d', static_cast<char>(back_char.codepoint));
    UTEST_ASSERT_TRUE(back_char.is_ascii);
    
    // Test UTF-8 string
    std::string utf8_str = u8"Hello世界";
    auto back_utf8 = back(utf8_str);
    UTEST_ASSERT_EQUALS(0x754Cu, back_utf8.codepoint);  // Unicode for '界'
    UTEST_ASSERT_FALSE(back_utf8.is_ascii);
    UTEST_ASSERT_EQUALS(3u, back_utf8.byte_count);
    
    // Test emoji
    std::string emoji_str = u8"Hello 🌍";
    auto back_emoji = back(emoji_str);
    UTEST_ASSERT_EQUALS(0x1F30Du, back_emoji.codepoint);  // Unicode for 🌍
    UTEST_ASSERT_EQUALS(4u, back_emoji.byte_count);
    
    // Test single character
    std::string single_char = "A";
    auto single_back = back(single_char);
    UTEST_ASSERT_EQUALS('A', static_cast<char>(single_back.codepoint));
    
    // Test with BOM
    std::string bom_string = with_bom(std::string("Hello"));
    auto back_after_bom = back(bom_string);
    UTEST_ASSERT_EQUALS('o', static_cast<char>(back_after_bom.codepoint));
    
    // Test empty string exception
    std::string empty_str = "";
    bool caught_exception = false;
    try {
        back(empty_str);
    } catch (const std::out_of_range&) {
        caught_exception = true;
    }
    UTEST_ASSERT_TRUE(caught_exception);
}

// Test functions with ASCII mode
UTEST_FUNC_DEF2(U8ScanAccess, ASCIIMode) {
    std::string input = u8"Hello 世界";
    
    // In ASCII mode, multi-byte characters are treated as separate bytes
    auto char_info = at(input, 6, false);  // ASCII mode
    // This should get the first byte of the UTF-8 sequence for '世'
    UTEST_ASSERT_TRUE(char_info.is_ascii);  // In ASCII mode, each byte is treated as ASCII
    
    // Test length in ASCII vs UTF-8 mode
    std::size_t utf8_length = length(input, true);   // UTF-8 mode
    std::size_t ascii_length = length(input, false); // ASCII mode
    
    UTEST_ASSERT_EQUALS(8u, utf8_length);   // 6 ASCII + 2 Chinese characters
    UTEST_ASSERT_EQUALS(12u, ascii_length); // 6 ASCII + 6 bytes for Chinese characters
}

// Test edge cases
UTEST_FUNC_DEF2(U8ScanAccess, EdgeCases) {
    // Test single character strings
    std::string single_ascii = "A";
    UTEST_ASSERT_EQUALS('A', static_cast<char>(at(single_ascii, 0).codepoint));
    UTEST_ASSERT_EQUALS('A', static_cast<char>(front(single_ascii).codepoint));
    UTEST_ASSERT_EQUALS('A', static_cast<char>(back(single_ascii).codepoint));
    UTEST_ASSERT_FALSE(empty(single_ascii));
    
    std::string single_utf8 = u8"世";
    UTEST_ASSERT_EQUALS(0x4E16u, at(single_utf8, 0).codepoint);
    UTEST_ASSERT_EQUALS(0x4E16u, front(single_utf8).codepoint);
    UTEST_ASSERT_EQUALS(0x4E16u, back(single_utf8).codepoint);
    UTEST_ASSERT_FALSE(empty(single_utf8));
    
    // Test very long string access
    std::string long_str(1000, 'A');
    UTEST_ASSERT_EQUALS('A', static_cast<char>(at(long_str, 999).codepoint));
    UTEST_ASSERT_EQUALS('A', static_cast<char>(back(long_str).codepoint));
    UTEST_ASSERT_FALSE(empty(long_str));
}

// Test length() function with various inputs
UTEST_FUNC_DEF2(U8ScanAccess, LengthFunction) {
    // Test empty string
    std::string empty_str = "";
    UTEST_ASSERT_EQUALS(0u, length(empty_str));
    
    // Test ASCII string
    std::string ascii_str = "Hello World";
    UTEST_ASSERT_EQUALS(11u, length(ascii_str));
    
    // Test UTF-8 string with mixed characters
    std::string utf8_str = u8"Hello 世界!";
    UTEST_ASSERT_EQUALS(9u, length(utf8_str));  // 6 ASCII + 2 Chinese + 1 ASCII
    
    // Test single UTF-8 character
    std::string single_chinese = u8"世";
    UTEST_ASSERT_EQUALS(1u, length(single_chinese));
    
    // Test emoji
    std::string emoji_str = u8"🌍🌎🌏";
    UTEST_ASSERT_EQUALS(3u, length(emoji_str));  // 3 emoji characters
    
    // Test mixed ASCII and emoji
    std::string mixed_emoji = u8"Hello 🌍 World!";
    UTEST_ASSERT_EQUALS(14u, length(mixed_emoji));  // H e l l o (space) 🌍 (space) W o r l d !
}

// Test length() function with BOM
UTEST_FUNC_DEF2(U8ScanAccess, LengthWithBOM) {
    // Test BOM-only string (should be length 0)
    std::string bom_only = bom_str();  // UTF-8 BOM
    UTEST_ASSERT_EQUALS(0u, length(bom_only));
    
    // Test BOM + ASCII content
    std::string bom_ascii = with_bom(std::string("Hello"));
    UTEST_ASSERT_EQUALS(5u, length(bom_ascii));  // BOM should be excluded
    
    // Test BOM + UTF-8 content
    std::string bom_utf8 = with_bom(std::string(u8"世界"));
    UTEST_ASSERT_EQUALS(2u, length(bom_utf8));  // BOM should be excluded
    
    // Test BOM + mixed content
    std::string bom_mixed = with_bom(std::string(u8"Hello 世界!"));
    UTEST_ASSERT_EQUALS(9u, length(bom_mixed));  // BOM should be excluded
    
    // Compare with same content without BOM
    std::string no_bom = u8"Hello 世界!";
    UTEST_ASSERT_EQUALS(length(no_bom), length(bom_mixed));
}

// Test length() function in ASCII mode
UTEST_FUNC_DEF2(U8ScanAccess, LengthASCIIMode) {
    // Test UTF-8 string in both modes
    std::string utf8_str = u8"Hello 世界!";
    
    std::size_t utf8_length = length(utf8_str, true);   // UTF-8 mode
    std::size_t ascii_length = length(utf8_str, false); // ASCII mode
    
    UTEST_ASSERT_EQUALS(9u, utf8_length);   // 6 ASCII + 2 Chinese + 1 ASCII
    UTEST_ASSERT_EQUALS(13u, ascii_length); // 6 ASCII + 6 bytes for Chinese + 1 ASCII
    
    // Test emoji in both modes
    std::string emoji_str = u8"🌍";
    UTEST_ASSERT_EQUALS(1u, length(emoji_str, true));  // UTF-8 mode: 1 character
    UTEST_ASSERT_EQUALS(4u, length(emoji_str, false)); // ASCII mode: 4 bytes
    
    // Test BOM + UTF-8 in ASCII mode
    std::string bom_utf8 = with_bom(std::string(u8"世界"));
    UTEST_ASSERT_EQUALS(2u, length(bom_utf8, true));  // UTF-8 mode: BOM excluded, 2 chars
    UTEST_ASSERT_EQUALS(6u, length(bom_utf8, false)); // ASCII mode: BOM excluded, 6 bytes
}

// Test length() function with invalid UTF-8
UTEST_FUNC_DEF2(U8ScanAccess, LengthInvalidUTF8) {
    // Test string with invalid UTF-8 sequence
    std::string invalid_utf8 = "Hello\xFF\xFEWorld";
    
    // With validation enabled (default), invalid bytes should be handled gracefully
    std::size_t validated_length = length(invalid_utf8, true, true);
    UTEST_ASSERT_GT(validated_length, 0u);  // Should still return some length
    
    // With validation disabled, each byte is counted
    std::size_t unvalidated_length = length(invalid_utf8, true, false);
    UTEST_ASSERT_GT(unvalidated_length, 0u);  // Should still return some length
    
    // ASCII mode treats each byte separately
    std::size_t ascii_length = length(invalid_utf8, false, false);
    UTEST_ASSERT_EQUALS(12u, ascii_length);  // 5 + 2 + 5 = 12 bytes
}

// Test length() edge cases
UTEST_FUNC_DEF2(U8ScanAccess, LengthEdgeCases) {
    // Test very long string
    std::string long_ascii(10000, 'A');
    UTEST_ASSERT_EQUALS(10000u, length(long_ascii));
    
    // Test string with only spaces
    std::string spaces(100, ' ');
    UTEST_ASSERT_EQUALS(100u, length(spaces));
    
    // Test string with only multibyte characters
    std::string all_chinese = u8"世界世界世界";
    UTEST_ASSERT_EQUALS(6u, length(all_chinese, true));   // UTF-8 mode
    UTEST_ASSERT_EQUALS(18u, length(all_chinese, false)); // ASCII mode
    
    // Test single character at different positions
    for (char c = 'A'; c <= 'Z'; ++c) {
        std::string single_char(1, c);
        UTEST_ASSERT_EQUALS(1u, length(single_char));
    }
}

// Test comprehensive BOM handling for all access functions
UTEST_FUNC_DEF2(U8ScanAccess, ComprehensiveBOMTests) {
    // Test data: BOM + mixed UTF-8 content
    std::string bom_content = with_bom(u8"Hello 世界! 🌍");
    std::string no_bom_content = u8"Hello 世界! 🌍";
    
    // Test length() with and without BOM (should be same)
    UTEST_ASSERT_EQUALS(length(no_bom_content), length(bom_content));
    UTEST_ASSERT_EQUALS(11u, length(bom_content));  // H e l l o (space) 世 界 ! (space) 🌍
    
    // Test empty() with and without BOM
    UTEST_ASSERT_FALSE(empty(bom_content));
    UTEST_ASSERT_FALSE(empty(no_bom_content));
    
    // Test front() with and without BOM (should be same character)
    auto front_bom = front(bom_content);
    auto front_no_bom = front(no_bom_content);
    UTEST_ASSERT_EQUALS(front_no_bom.codepoint, front_bom.codepoint);
    UTEST_ASSERT_EQUALS('H', static_cast<char>(front_bom.codepoint));
    
    // Test back() with and without BOM (should be same character)
    auto back_bom = back(bom_content);
    auto back_no_bom = back(no_bom_content);
    UTEST_ASSERT_EQUALS(back_no_bom.codepoint, back_bom.codepoint);
    UTEST_ASSERT_EQUALS(0x1F30Du, back_bom.codepoint);  // 🌍
    
    // Test at() at various positions with and without BOM
    for (std::size_t i = 0; i < length(bom_content); ++i) {
        auto char_bom = at(bom_content, i);
        auto char_no_bom = at(no_bom_content, i);
        UTEST_ASSERT_EQUALS(char_no_bom.codepoint, char_bom.codepoint);
        UTEST_ASSERT_EQUALS(char_no_bom.is_ascii, char_bom.is_ascii);
        UTEST_ASSERT_EQUALS(char_no_bom.byte_count, char_bom.byte_count);
    }
    
    // Test accessing specific characters
    auto chinese_bom = at(bom_content, 6);     // '世'
    auto chinese_no_bom = at(no_bom_content, 6);
    UTEST_ASSERT_EQUALS(0x4E16u, chinese_bom.codepoint);
    UTEST_ASSERT_EQUALS(chinese_no_bom.codepoint, chinese_bom.codepoint);
    
    auto emoji_bom = at(bom_content, 10);      // '🌍'
    auto emoji_no_bom = at(no_bom_content, 10);
    UTEST_ASSERT_EQUALS(0x1F30Du, emoji_bom.codepoint);
    UTEST_ASSERT_EQUALS(emoji_no_bom.codepoint, emoji_bom.codepoint);
}

// Test BOM-only and empty string edge cases for all functions
UTEST_FUNC_DEF2(U8ScanAccess, BOMEdgeCases) {
    std::string bom_only = bom_str();  // UTF-8 BOM only
    std::string truly_empty = "";
    
    // Both should be considered empty
    UTEST_ASSERT_TRUE(empty(bom_only));
    UTEST_ASSERT_TRUE(empty(truly_empty));
    
    // Both should have length 0
    UTEST_ASSERT_EQUALS(0u, length(bom_only));
    UTEST_ASSERT_EQUALS(0u, length(truly_empty));
    
    // Both should throw exceptions for front(), back(), and at()
    bool front_exception_bom = false, front_exception_empty = false;
    bool back_exception_bom = false, back_exception_empty = false;
    bool at_exception_bom = false, at_exception_empty = false;
    
    try { front(bom_only); } catch (const std::out_of_range&) { front_exception_bom = true; }
    try { front(truly_empty); } catch (const std::out_of_range&) { front_exception_empty = true; }
    try { back(bom_only); } catch (const std::out_of_range&) { back_exception_bom = true; }
    try { back(truly_empty); } catch (const std::out_of_range&) { back_exception_empty = true; }
    try { at(bom_only, 0); } catch (const std::out_of_range&) { at_exception_bom = true; }
    try { at(truly_empty, 0); } catch (const std::out_of_range&) { at_exception_empty = true; }
    
    UTEST_ASSERT_TRUE(front_exception_bom);
    UTEST_ASSERT_TRUE(front_exception_empty);
    UTEST_ASSERT_TRUE(back_exception_bom);
    UTEST_ASSERT_TRUE(back_exception_empty);
    UTEST_ASSERT_TRUE(at_exception_bom);
    UTEST_ASSERT_TRUE(at_exception_empty);
    
    // Test different types of single characters with BOM
    std::string bom_ascii = with_bom(std::string("A"));
    std::string bom_chinese = with_bom(std::string(u8"世"));
    std::string bom_emoji = with_bom(std::string(u8"🌍"));
    
    // All should have length 1
    UTEST_ASSERT_EQUALS(1u, length(bom_ascii));
    UTEST_ASSERT_EQUALS(1u, length(bom_chinese));
    UTEST_ASSERT_EQUALS(1u, length(bom_emoji));
    
    // All should not be empty
    UTEST_ASSERT_FALSE(empty(bom_ascii));
    UTEST_ASSERT_FALSE(empty(bom_chinese));
    UTEST_ASSERT_FALSE(empty(bom_emoji));
    
    // front() and back() should return the same character (since length is 1)
    UTEST_ASSERT_EQUALS(front(bom_ascii).codepoint, back(bom_ascii).codepoint);
    UTEST_ASSERT_EQUALS(front(bom_chinese).codepoint, back(bom_chinese).codepoint);
    UTEST_ASSERT_EQUALS(front(bom_emoji).codepoint, back(bom_emoji).codepoint);
    
    // at(0) should return the same character as front() and back()
    UTEST_ASSERT_EQUALS(at(bom_ascii, 0).codepoint, front(bom_ascii).codepoint);
    UTEST_ASSERT_EQUALS(at(bom_chinese, 0).codepoint, front(bom_chinese).codepoint);
    UTEST_ASSERT_EQUALS(at(bom_emoji, 0).codepoint, front(bom_emoji).codepoint);
}

// Test comprehensive string analysis with UTF-8 string (without BOM)
UTEST_FUNC_DEF2(U8ScanAccess, StringAnalysisUTF8NoBOM) {
    // Using test string from UTF8StringLiterals
    std::string input = skip_bom(u8"Hello 世界! 🌍");
    
    // Write input string to console
    std::cout << "\n=== String Analysis Test: UTF-8 without BOM ===" << std::endl;
    std::cout << "Input string: " << input << std::endl;
    
    // Write string statistics
    std::cout << "\n--- String Statistics ---" << std::endl;
    std::cout << "Number of bytes: " << input.size() << std::endl;
    std::cout << "Length (characters): " << length(input) << std::endl;
    std::cout << "Contains BOM: " << (has_bom(input) ? "yes" : "no") << std::endl;
    
    // Write compact byte dump
    print_byte_dump(input);
    
    // Write character table
    std::cout << "\n--- Character Analysis ---" << std::endl;
    std::cout << "Pos | Character | is_ascii | is_utf8 | is_valid" << std::endl;
    std::cout << "----|-----------|----------|---------|----------" << std::endl;
    
    auto char_range = make_char_range(input);
    size_t pos = 0;
    for (auto it = char_range.begin(); it != char_range.end(); ++it, ++pos) {
        CharInfo info = *it;
        std::string char_str = to_string(info);
        std::cout << std::setw(3) << pos << " | " 
                  << std::setw(9) << char_str << " | "
                  << std::setw(8) << (info.is_ascii ? "true" : "false") << " | "
                  << std::setw(7) << (!info.is_ascii ? "true" : "false") << " | "
                  << std::setw(8) << (info.is_valid_utf8 ? "true" : "false") << std::endl;
    }
    
    // Perform assertions
    UTEST_ASSERT_EQUALS(18u, input.size());  // Byte count: "Hello 世界! 🌍"
    UTEST_ASSERT_EQUALS(11u, length(input)); // Character count: "Hello 世界! 🌍"
    UTEST_ASSERT_FALSE(has_bom(input));      // No BOM
    
    // Test specific characters
    auto h_char = at(input, 0);  // 'H'
    UTEST_ASSERT_TRUE(h_char.is_ascii);
    UTEST_ASSERT_TRUE(h_char.is_valid_utf8);
    
    auto chinese_char = at(input, 6);  // '世'
    UTEST_ASSERT_FALSE(chinese_char.is_ascii);
    UTEST_ASSERT_TRUE(chinese_char.is_valid_utf8);
    
    auto emoji_char = at(input, 10);  // '🌍'
    UTEST_ASSERT_FALSE(emoji_char.is_ascii);
    UTEST_ASSERT_TRUE(emoji_char.is_valid_utf8);
}

// Test comprehensive string analysis with UTF-8 string (with BOM)
UTEST_FUNC_DEF2(U8ScanAccess, StringAnalysisUTF8WithBOM) {
    // Using test string with explicit UTF-8 bytes to ensure cross-platform consistency
    std::string input = with_bom(u8"Hello 世界! 🌍");
    
    // Write input string to console
    std::cout << "\n=== String Analysis Test: UTF-8 with BOM ===" << std::endl;
    std::cout << "Input string: " << input << std::endl;
    
    // Write string statistics
    std::cout << "\n--- String Statistics ---" << std::endl;
    std::cout << "Number of bytes: " << input.size() << std::endl;
    std::cout << "Length (characters): " << length(input) << std::endl;
    std::cout << "Contains BOM: " << (has_bom(input) ? "yes" : "no") << std::endl;
    
    // Write compact byte dump
    print_byte_dump(input);
    
    // Write character table
    std::cout << "\n--- Character Analysis ---" << std::endl;
    std::cout << "Pos | Character | is_ascii | is_utf8 | is_valid" << std::endl;
    std::cout << "----|-----------|----------|---------|----------" << std::endl;
    
    auto char_range = make_char_range(input, true, true);  
    size_t pos = 0;
    size_t chars_processed = 0;
    for (auto it = char_range.begin(); it != char_range.end(); ++it, ++pos) {
        CharInfo info = *it;
        chars_processed++;
        std::string char_str = to_string(info);
        std::cout << std::setw(3) << pos << " | " 
                  << std::setw(9) << char_str << " | "
                  << std::setw(8) << (info.is_ascii ? "true" : "false") << " | "
                  << std::setw(7) << (!info.is_ascii ? "true" : "false") << " | "
                  << std::setw(8) << (info.is_valid_utf8 ? "true" : "false") << std::endl;
    }
    
    // Perform assertions
    UTEST_ASSERT_EQUALS(21u, input.size());  // Byte count (18 + 3 BOM bytes)
    UTEST_ASSERT_EQUALS(11u, length(input)); // Character count (BOM ignored): "Hello 世界! 🌍"
    UTEST_ASSERT_TRUE(has_bom(input));       // Has BOM
    UTEST_ASSERT_EQUALS(chars_processed, length(input)); // Should match character count
    
    // Test specific characters (BOM is ignored in at() function)
    auto h_char = at(input, 0);  // 'H' (first character after BOM)
    UTEST_ASSERT_TRUE(h_char.is_ascii);
    UTEST_ASSERT_TRUE(h_char.is_valid_utf8);
    
    auto chinese_char = at(input, 6);  // '世'
    UTEST_ASSERT_FALSE(chinese_char.is_ascii);
    UTEST_ASSERT_TRUE(chinese_char.is_valid_utf8);
    
    auto emoji_char = at(input, 10);  // '🌍'
    UTEST_ASSERT_FALSE(emoji_char.is_ascii);
    UTEST_ASSERT_TRUE(emoji_char.is_valid_utf8);
}

// Test comprehensive string analysis with ASCII-only string
UTEST_FUNC_DEF2(U8ScanAccess, StringAnalysisASCIIOnly) {
    // ASCII-only test string
    std::string input = "Hello World! 123";
    
    // Write input string to console
    std::cout << "\n=== String Analysis Test: ASCII-only ===" << std::endl;
    std::cout << "Input string: " << input << std::endl;
    
    // Write string statistics
    std::cout << "\n--- String Statistics ---" << std::endl;
    std::cout << "Number of bytes: " << input.size() << std::endl;
    std::cout << "Length (characters): " << length(input) << std::endl;
    std::cout << "Contains BOM: " << (has_bom(input) ? "yes" : "no") << std::endl;
    
    // Write compact byte dump
    print_byte_dump(input);
    
    // Write character table
    std::cout << "\n--- Character Analysis ---" << std::endl;
    std::cout << "Pos | Character | is_ascii | is_utf8 | is_valid" << std::endl;
    std::cout << "----|-----------|----------|---------|----------" << std::endl;
    
    auto char_range = make_char_range(input);
    size_t pos = 0;
    for (auto it = char_range.begin(); it != char_range.end(); ++it, ++pos) {
        CharInfo info = *it;
        std::string char_str = to_string(info);
        std::cout << std::setw(3) << pos << " | " 
                  << std::setw(9) << char_str << " | "
                  << std::setw(8) << (info.is_ascii ? "true" : "false") << " | "
                  << std::setw(7) << (!info.is_ascii ? "true" : "false") << " | "
                  << std::setw(8) << (info.is_valid_utf8 ? "true" : "false") << std::endl;
    }
    
    // Perform assertions
    UTEST_ASSERT_EQUALS(16u, input.size());   // Byte count
    UTEST_ASSERT_EQUALS(16u, length(input));  // Character count (same as bytes for ASCII)
    UTEST_ASSERT_FALSE(has_bom(input));       // No BOM
    
    // Test that all characters are ASCII and valid UTF-8
    for (auto it = char_range.begin(); it != char_range.end(); ++it) {
        CharInfo info = *it;
        UTEST_ASSERT_TRUE(info.is_ascii);
        UTEST_ASSERT_TRUE(info.is_valid_utf8);
    }
    
    // Test specific characters
    auto h_char = at(input, 0);  // 'H'
    UTEST_ASSERT_EQUALS('H', static_cast<char>(h_char.codepoint));
    UTEST_ASSERT_TRUE(h_char.is_ascii);
    
    auto space_char = at(input, 5);  // ' '
    UTEST_ASSERT_EQUALS(' ', static_cast<char>(space_char.codepoint));
    UTEST_ASSERT_TRUE(space_char.is_ascii);
    
    auto digit_char = at(input, 13);  // '1'
    UTEST_ASSERT_EQUALS('1', static_cast<char>(digit_char.codepoint));
    UTEST_ASSERT_TRUE(digit_char.is_ascii);
}

// Main test runner
int main() {
    UTEST_PROLOG();
    UTEST_ENABLE_VERBOSE_MODE();
    
    // Access function tests  
    UTEST_FUNC2(U8ScanAccess, AtFunctionASCII);
    UTEST_FUNC2(U8ScanAccess, AtFunctionUTF8);
    UTEST_FUNC2(U8ScanAccess, AtFunctionWithBOM);
    UTEST_FUNC2(U8ScanAccess, EmptyFunction);
    UTEST_FUNC2(U8ScanAccess, FrontFunction);
    UTEST_FUNC2(U8ScanAccess, BackFunction);
    UTEST_FUNC2(U8ScanAccess, ASCIIMode);
    UTEST_FUNC2(U8ScanAccess, EdgeCases);
    UTEST_FUNC2(U8ScanAccess, LengthFunction);
    UTEST_FUNC2(U8ScanAccess, LengthWithBOM);
    UTEST_FUNC2(U8ScanAccess, LengthASCIIMode);
    UTEST_FUNC2(U8ScanAccess, LengthInvalidUTF8);
    UTEST_FUNC2(U8ScanAccess, LengthEdgeCases);
    
    // Comprehensive BOM tests
    UTEST_FUNC2(U8ScanAccess, ComprehensiveBOMTests);
    UTEST_FUNC2(U8ScanAccess, BOMEdgeCases);
    
    // String analysis tests
    UTEST_FUNC2(U8ScanAccess, StringAnalysisUTF8NoBOM);
    UTEST_FUNC2(U8ScanAccess, StringAnalysisUTF8WithBOM);
    UTEST_FUNC2(U8ScanAccess, StringAnalysisASCIIOnly);
    
    UTEST_EPILOG();
}

