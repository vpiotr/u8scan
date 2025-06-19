#include "../include/u8scan/u8scan.h"
#include <iostream>
#include <iomanip>

using namespace u8scan;

/**
 * @brief Demo: String access functions (at, empty, front, back)
 */
void demo_string_access() {
    std::cout << "=== String Access Functions Demo ===" << std::endl;
    
    // Test with mixed content
    std::string mixed = u8"Hello 世界! 🌍 Test";
    std::cout << "Text: " << mixed << std::endl;
    std::cout << "Length: " << length(mixed) << " characters" << std::endl;
    std::cout << "Empty: " << (empty(mixed) ? "YES" : "NO") << std::endl;
    
    // Demonstrate at() function
    std::cout << "\n--- Character Access with at() ---" << std::endl;
    for (size_t i = 0; i < length(mixed); ++i) {
        auto char_info = at(mixed, i);
        std::cout << "at(" << i << "): ";
        if (char_info.is_ascii) {
            std::cout << "'" << static_cast<char>(char_info.codepoint) << "'";
        } else {
            std::cout << "U+" << std::hex << std::uppercase << char_info.codepoint << std::dec;
        }
        std::cout << " (" << char_info.byte_count << " bytes)" << std::endl;
    }
    
    // Demonstrate front() and back()
    std::cout << "\n--- First and Last Characters ---" << std::endl;
    auto first_char = front(mixed);
    auto last_char = back(mixed);
    
    std::cout << "First character: ";
    if (first_char.is_ascii) {
        std::cout << "'" << static_cast<char>(first_char.codepoint) << "'";
    } else {
        std::cout << "U+" << std::hex << std::uppercase << first_char.codepoint << std::dec;
    }
    std::cout << std::endl;
    
    std::cout << "Last character: ";
    if (last_char.is_ascii) {
        std::cout << "'" << static_cast<char>(last_char.codepoint) << "'";
    } else {
        std::cout << "U+" << std::hex << std::uppercase << last_char.codepoint << std::dec;
    }
    std::cout << std::endl;
}

/**
 * @brief Demo: BOM handling
 */
void demo_bom_handling() {
    std::cout << "\n=== BOM Handling Demo ===" << std::endl;
    
    // String with BOM
    std::string with_bom = "\xEF\xBB\xBF" + std::string(u8"Hello 世界!");
    std::string without_bom = u8"Hello 世界!";
    
    std::cout << "String with BOM:" << std::endl;
    std::cout << "  Length: " << length(with_bom) << " characters (BOM excluded)" << std::endl;
    std::cout << "  Empty: " << (empty(with_bom) ? "YES" : "NO") << std::endl;
    std::cout << "  First char: ";
    try {
        auto first = front(with_bom);
        std::cout << "'" << static_cast<char>(first.codepoint) << "'" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "String without BOM:" << std::endl;
    std::cout << "  Length: " << length(without_bom) << " characters" << std::endl;
    std::cout << "  Empty: " << (empty(without_bom) ? "YES" : "NO") << std::endl;
    std::cout << "  First char: ";
    try {
        auto first = front(without_bom);
        std::cout << "'" << static_cast<char>(first.codepoint) << "'" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // BOM-only string
    std::string bom_only = "\xEF\xBB\xBF";
    std::cout << "BOM-only string:" << std::endl;
    std::cout << "  Length: " << length(bom_only) << " characters" << std::endl;
    std::cout << "  Empty: " << (empty(bom_only) ? "YES" : "NO") << std::endl;
}

/**
 * @brief Demo: Empty string handling
 */
void demo_empty_strings() {
    std::cout << "\n=== Empty String Handling Demo ===" << std::endl;
    
    std::string empty_str = "";
    std::cout << "Truly empty string:" << std::endl;
    std::cout << "  Length: " << length(empty_str) << " characters" << std::endl;
    std::cout << "  Empty: " << (empty(empty_str) ? "YES" : "NO") << std::endl;
    
    // Test front() and back() with empty string
    std::cout << "  front() test: ";
    try {
        front(empty_str);
        std::cout << "Got character (unexpected)" << std::endl;
    } catch (const std::out_of_range& e) {
        std::cout << "Correctly threw exception: " << e.what() << std::endl;
    }
    
    std::cout << "  back() test: ";
    try {
        back(empty_str);
        std::cout << "Got character (unexpected)" << std::endl;
    } catch (const std::out_of_range& e) {
        std::cout << "Correctly threw exception: " << e.what() << std::endl;
    }
    
    std::cout << "  at(0) test: ";
    try {
        at(empty_str, 0);
        std::cout << "Got character (unexpected)" << std::endl;
    } catch (const std::out_of_range& e) {
        std::cout << "Correctly threw exception: " << e.what() << std::endl;
    }
}

/**
 * @brief Demo: Single character strings
 */
void demo_single_character() {
    std::cout << "\n=== Single Character Demo ===" << std::endl;
    
    // ASCII single character
    std::string single_ascii = "A";
    std::cout << "Single ASCII character '" << single_ascii << "':" << std::endl;
    std::cout << "  Length: " << length(single_ascii) << std::endl;
    std::cout << "  at(0): '" << static_cast<char>(at(single_ascii, 0).codepoint) << "'" << std::endl;
    std::cout << "  front(): '" << static_cast<char>(front(single_ascii).codepoint) << "'" << std::endl;
    std::cout << "  back(): '" << static_cast<char>(back(single_ascii).codepoint) << "'" << std::endl;
    std::cout << "  Same char: " << (front(single_ascii).codepoint == back(single_ascii).codepoint ? "YES" : "NO") << std::endl;
    
    // UTF-8 single character
    std::string single_utf8 = u8"世";
    std::cout << "Single UTF-8 character '" << single_utf8 << "':" << std::endl;
    std::cout << "  Length: " << length(single_utf8) << std::endl;
    std::cout << "  at(0): U+" << std::hex << std::uppercase << at(single_utf8, 0).codepoint << std::dec << std::endl;
    std::cout << "  front(): U+" << std::hex << std::uppercase << front(single_utf8).codepoint << std::dec << std::endl;
    std::cout << "  back(): U+" << std::hex << std::uppercase << back(single_utf8).codepoint << std::dec << std::endl;
    std::cout << "  Same char: " << (front(single_utf8).codepoint == back(single_utf8).codepoint ? "YES" : "NO") << std::endl;
    
    // Emoji single character
    std::string single_emoji = u8"🌍";
    std::cout << "Single emoji character '" << single_emoji << "':" << std::endl;
    std::cout << "  Length: " << length(single_emoji) << std::endl;
    std::cout << "  Bytes: " << single_emoji.length() << std::endl;
    std::cout << "  at(0): U+" << std::hex << std::uppercase << at(single_emoji, 0).codepoint << std::dec << std::endl;
    std::cout << "  front(): U+" << std::hex << std::uppercase << front(single_emoji).codepoint << std::dec << std::endl;
    std::cout << "  back(): U+" << std::hex << std::uppercase << back(single_emoji).codepoint << std::dec << std::endl;
    std::cout << "  Same char: " << (front(single_emoji).codepoint == back(single_emoji).codepoint ? "YES" : "NO") << std::endl;
}

/**
 * @brief Demo: ASCII vs UTF-8 mode comparison
 */
void demo_ascii_vs_utf8_mode() {
    std::cout << "\n=== ASCII vs UTF-8 Mode Demo ===" << std::endl;
    
    std::string text = u8"Hello 世界";
    
    std::cout << "Text: " << text << std::endl;
    std::cout << "Byte length: " << text.length() << std::endl;
    
    std::cout << "\nUTF-8 Mode (true):" << std::endl;
    std::cout << "  Character length: " << length(text, true) << std::endl;
    std::cout << "  First char: '" << static_cast<char>(front(text, true).codepoint) << "'" << std::endl;
    std::cout << "  Last char: U+" << std::hex << std::uppercase << back(text, true).codepoint << std::dec << std::endl;
    
    std::cout << "\nASCII Mode (false):" << std::endl;
    std::cout << "  Character length: " << length(text, false) << std::endl;
    std::cout << "  First char: '" << static_cast<char>(front(text, false).codepoint) << "'" << std::endl;
    std::cout << "  Last char: 0x" << std::hex << std::uppercase << back(text, false).codepoint << std::dec << std::endl;
}

int main() {
    try {
        demo_string_access();
        demo_bom_handling();
        demo_empty_strings();
        demo_single_character();
        demo_ascii_vs_utf8_mode();
        
        std::cout << "\n=== Access Functions Demo Completed Successfully ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
