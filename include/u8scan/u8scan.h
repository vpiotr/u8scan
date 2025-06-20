/**
 * @file u8scan.h
 * @brief STL-Compatible String Iterators for UTF-8 and ASCII strings
 *
 * This header provides STL-friendly iterators, ranges, and utilities for processing UTF-8 and ASCII strings at the character level.
 *
 * ## Features
 * - STL-compatible `CharIterator` and `CharRange` for use with standard algorithms
 * - Efficient UTF-8 and ASCII scanning, with BOM detection and handling
 * - Character property predicates (is_ascii, is_digit_ascii, is_alpha_ascii, is_alphanum_ascii, is_lowercase_ascii, is_uppercase_ascii, etc.)
 * - Character conversion functions (to_lower_ascii, to_upper_ascii) for ASCII characters
 * - High-performance transformation and filtering with `transform_chars()`
 * - STL-like copy functions: `copy()`, `copy_if()`, `copy_until()`, `copy_from()`, `copy_n()`, `copy_while()`
 * - String length calculation in Unicode code points with `length()`
 * - String access functions: `at()`, `empty()`, `front()`, `back()` with BOM-aware character-level access
 * - Custom character processing via `scan_utf8()` and `scan_ascii()`
 * - Utility: `quoted_str()` for safe quoting/escaping of strings
 *
 * ## Example Usage
 * @code
 * #include "u8scan/u8scan.h"
 * #include <algorithm>
 * #include <iostream>
 *
 * std::string input = "Hello 世界! 123 🌍 Test.";
 * auto char_range = u8scan::make_char_range(input);
 *
 * // Calculate string length in Unicode code points
 * size_t char_count = u8scan::length(input);  // Returns 21, not 28 bytes
 *
 * // Count ASCII and UTF-8 characters
 * size_t ascii_count = std::count_if(char_range.begin(), char_range.end(), u8scan::predicates::is_ascii());
 * size_t utf8_count = std::count_if(char_range.begin(), char_range.end(), u8scan::predicates::is_utf8());
 *
 * // Count lowercase and uppercase letters (ASCII only)
 * size_t lowercase_count = std::count_if(char_range.begin(), char_range.end(), u8scan::predicates::is_lowercase_ascii());
 * size_t uppercase_count = std::count_if(char_range.begin(), char_range.end(), u8scan::predicates::is_uppercase_ascii());
 *
 * // Count emoji characters
 * size_t emoji_count = std::count_if(char_range.begin(), char_range.end(), u8scan::predicates::is_emoji());
 *
 * // Transform to uppercase (ASCII only)
 * std::string upper;
 * u8scan::transform_chars(input, std::back_inserter(upper), [](const u8scan::CharInfo& info) {
 *     return static_cast<char>(u8scan::to_upper_ascii(info));
 * });
 *
 * // Transform to lowercase (ASCII only)
 * std::string lower;
 * u8scan::transform_chars(input, std::back_inserter(lower), [](const u8scan::CharInfo& info) {
 *     return static_cast<char>(u8scan::to_lower_ascii(info));
 * });
 *
 * // String access functions with BOM handling
 * if (!u8scan::empty(input)) {
 *     auto first_char = u8scan::front(input);      // Get first character
 *     auto last_char = u8scan::back(input);        // Get last character
 *     auto char_at_6 = u8scan::at(input, 6);       // Get character at index 6 (世)
 *     std::cout << "First: " << static_cast<char>(first_char.codepoint) << std::endl;
 *     std::cout << "Last: " << static_cast<char>(last_char.codepoint) << std::endl;
 *     std::cout << "At 6: U+" << std::hex << char_at_6.codepoint << std::endl;
 * }
 *
 * // Quoting/escaping
 * std::string quoted = u8scan::quoted_str("A\"B世界", '"', '"', '\\');
 * // Result: "A\"B世界"
 * @endcode
 */

#ifndef U8SCAN_H
#define U8SCAN_H

#include <string>
#include <functional>
#include <cstdint>
#include <iterator>
#include <algorithm>
#include <stdexcept>

namespace u8scan {

/**
 * @brief UTF-8 character information
 */
struct CharInfo {
    std::size_t start_pos;      ///< Starting position in the string
    std::size_t byte_count;     ///< Number of bytes in this character (1-4)
    uint32_t codepoint;         ///< Unicode codepoint (if valid UTF-8)
    bool is_ascii;              ///< True if this is an ASCII character (< 0x80)
    bool is_valid_utf8;         ///< True if this is a valid UTF-8 sequence
    bool is_bom;                ///< True if this is a BOM character
    
    CharInfo() : start_pos(0), byte_count(1), codepoint(0), is_ascii(true), is_valid_utf8(true), is_bom(false) {}
    
    // STL algorithm compatibility
    char operator*() const { return static_cast<char>(codepoint & 0x7F); } // ASCII part only
    bool operator==(const CharInfo& other) const { return codepoint == other.codepoint; }
    bool operator!=(const CharInfo& other) const { return !(*this == other); }
    bool operator<(const CharInfo& other) const { return codepoint < other.codepoint; }
};

/**
 * @brief BOM handling configuration
 */
enum class BOMAction {
    IGNORE,         ///< Skip BOM completely
    COPY,           ///< Copy BOM to output
    CUSTOM          ///< Call custom handler
};

/**
 * @brief BOM detection result
 */
struct BOMInfo {
    bool found;                 ///< True if BOM was detected
    std::size_t size;           ///< BOM size in bytes (usually 3 for UTF-8)
    BOMAction action_taken;     ///< Action that was taken
    
    BOMInfo() : found(false), size(0), action_taken(BOMAction::IGNORE) {}
};

/**
 * @brief Action to take for a character during scanning
 */
enum class ScanAction {
    COPY_TO_OUTPUT,     ///< Copy the character to output as-is
    REPLACE,            ///< Replace the character with provided replacement
    IGNORE,             ///< Skip this character (don't add to output)
    STOP_SCANNING       ///< Stop the scanning process
};

/**
 * @brief Result of character processing
 */
struct ProcessResult {
    ScanAction action;
    std::string replacement;    ///< Used when action is REPLACE
    
    ProcessResult(ScanAction a = ScanAction::COPY_TO_OUTPUT) : action(a) {}
    ProcessResult(ScanAction a, const std::string& repl) : action(a), replacement(repl) {}
};

/**
 * @brief Function type for processing characters during scanning
 */
using CharProcessor = std::function<ProcessResult(const CharInfo& info, const char* original_data)>;

/**
 * @brief Function type for BOM handling
 */
using BOMHandler = std::function<std::string(const BOMInfo& bom_info, const char* bom_data)>;

/**
 * @brief Simplified configuration for UTF-8 scanning
 */
struct ScanConfig {
    bool utf8_mode;             ///< UTF-8 mode (true) or ASCII mode (false)
    BOMAction bom_action;       ///< How to handle BOM
    BOMHandler bom_handler;     ///< Custom BOM handler (when bom_action == CUSTOM)
    bool validate_utf8;         ///< Validate UTF-8 sequences
    std::size_t max_output_size; ///< Maximum output size (0 = unlimited)
    
    ScanConfig() : utf8_mode(true), bom_action(BOMAction::IGNORE), validate_utf8(true), max_output_size(0) {}
};

/**
 * @brief Iterator for UTF-8 characters - enables STL algorithm compatibility
 */
class CharIterator {
private:
    const std::string* str_;
    std::size_t pos_;
    bool utf8_mode_;
    bool validate_;
    mutable CharInfo current_char_;
    mutable bool char_cached_;
    
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = CharInfo;
    using difference_type = std::ptrdiff_t;
    using pointer = const CharInfo*;
    using reference = const CharInfo&;
    
    CharIterator(const std::string* str, std::size_t pos, bool utf8_mode = true, bool validate = true)
        : str_(str), pos_(pos), utf8_mode_(utf8_mode), validate_(validate), char_cached_(false) {}
    
    const CharInfo& operator*() const {
        if (!char_cached_) {
            update_current_char();
            char_cached_ = true;
        }
        return current_char_;
    }
    
    const CharInfo* operator->() const {
        return &(operator*());
    }
    
    CharIterator& operator++() {
        if (!char_cached_) {
            update_current_char();
        }
        pos_ += current_char_.byte_count;
        char_cached_ = false;
        return *this;
    }
    
    CharIterator operator++(int) {
        CharIterator tmp = *this;
        ++(*this);
        return tmp;
    }
    
    bool operator==(const CharIterator& other) const {
        return str_ == other.str_ && pos_ == other.pos_;
    }
    
    bool operator!=(const CharIterator& other) const {
        return !(*this == other);
    }
    
    std::size_t position() const { return pos_; }
    
private:
    void update_current_char() const {
        current_char_ = get_char_info_impl(*str_, pos_, utf8_mode_, validate_);
    }
    
    static CharInfo get_char_info_impl(const std::string& input, std::size_t pos, bool utf8_mode, bool validate);
};

/**
 * @brief Simple UTF-8 character range for STL algorithms
 */
class CharRange {
private:
    const std::string* str_;
    std::size_t start_pos_;
    std::size_t end_pos_;
    bool utf8_mode_;
    bool validate_;
    
public:
    CharRange(const std::string& str, bool utf8_mode = true, bool validate = true, bool skip_bom = true)
        : str_(&str), start_pos_(0), end_pos_(str.length()), utf8_mode_(utf8_mode), validate_(validate) {
        if (skip_bom && str.length() >= 3 &&
            static_cast<unsigned char>(str[0]) == 0xEF &&
            static_cast<unsigned char>(str[1]) == 0xBB &&
            static_cast<unsigned char>(str[2]) == 0xBF) {
            start_pos_ = 3;  // Skip UTF-8 BOM
        }
    }
    
    CharRange(const std::string& str, std::size_t start, std::size_t end, bool utf8_mode = true, bool validate = true, bool skip_bom = true)
        : str_(&str), start_pos_(start), end_pos_(end), utf8_mode_(utf8_mode), validate_(validate) {
        if (skip_bom && start == 0 && str.length() >= 3 &&
            static_cast<unsigned char>(str[0]) == 0xEF &&
            static_cast<unsigned char>(str[1]) == 0xBB &&
            static_cast<unsigned char>(str[2]) == 0xBF) {
            start_pos_ = 3;  // Skip UTF-8 BOM
        }
    }
    
    CharIterator begin() const {
        return CharIterator(str_, start_pos_, utf8_mode_, validate_);
    }
    
    CharIterator end() const {
        return CharIterator(str_, end_pos_, utf8_mode_, validate_);
    }
    
    std::size_t size() const {
        return static_cast<std::size_t>(std::distance(begin(), end()));
    }
    
    bool empty() const {
        return start_pos_ >= end_pos_;
    }
};

// Core implementation functions (simplified and generic)
namespace details {

/**
 * @brief Core UTF-8 character info extraction (simplified)
 */
inline CharInfo extract_char_info(const std::string& input, std::size_t pos, bool utf8_mode, bool validate) {
    CharInfo info;
    info.start_pos = pos;
    info.is_bom = false;  // Initialize BOM flag
    
    if (pos >= input.length()) {
        info.is_valid_utf8 = false;
        info.byte_count = 1;  // Ensure at least 1 to prevent infinite loops
        return info;
    }
    
    unsigned char first_byte = static_cast<unsigned char>(input[pos]);
    
    if (!utf8_mode || first_byte < 0x80) {
        // ASCII mode or ASCII character
        info.is_ascii = true;
        info.byte_count = 1;
        info.codepoint = static_cast<uint32_t>(first_byte);
        info.is_valid_utf8 = true;
    } else {
        // UTF-8 multi-byte character
        info.is_ascii = false;
        
        // Determine byte count
        if ((first_byte & 0xE0) == 0xC0) info.byte_count = 2;
        else if ((first_byte & 0xF0) == 0xE0) info.byte_count = 3;
        else if ((first_byte & 0xF8) == 0xF0) info.byte_count = 4;
        else {
            info.byte_count = 1;  // Invalid UTF-8, treat as single byte
            info.is_valid_utf8 = false;
            info.codepoint = static_cast<uint32_t>(first_byte);
            return info;  // Return early for invalid UTF-8
        }
        
        // Validate and extract codepoint
        if (validate) {
            if (pos + info.byte_count > input.length()) {
                info.is_valid_utf8 = false;
                info.byte_count = 1;  // Prevent reading beyond string bounds
                info.codepoint = static_cast<uint32_t>(first_byte);
            } else {
                // Validate continuation bytes and extract codepoint
                info.codepoint = static_cast<uint32_t>(first_byte & ((1 << (7 - info.byte_count)) - 1));
                for (std::size_t i = 1; i < info.byte_count; ++i) {
                    unsigned char byte = static_cast<unsigned char>(input[pos + i]);
                    if ((byte & 0xC0) != 0x80) {
                        info.is_valid_utf8 = false;
                        info.byte_count = 1;  // Treat as single byte if invalid
                        info.codepoint = static_cast<uint32_t>(first_byte);
                        break;
                    }
                    info.codepoint = (info.codepoint << 6) | (byte & 0x3F);
                }
            }
        } else {
            // No validation, assume the UTF-8 is correct and extract codepoint optimistically
            if (pos + info.byte_count <= input.length()) {
                info.codepoint = static_cast<uint32_t>(first_byte & ((1 << (7 - info.byte_count)) - 1));
                for (std::size_t i = 1; i < info.byte_count; ++i) {
                    unsigned char byte = static_cast<unsigned char>(input[pos + i]);
                    info.codepoint = (info.codepoint << 6) | (byte & 0x3F);
                }
                info.is_valid_utf8 = true;
            } else {
                info.is_valid_utf8 = false;
                info.byte_count = 1;
                info.codepoint = static_cast<uint32_t>(first_byte);
            }
        }
    }
    
    return info;
}

/**
 * @brief BOM detection (simplified)
 */
inline BOMInfo detect_bom(const std::string& input) {
    BOMInfo bom;
    if (input.length() >= 3 &&
        static_cast<unsigned char>(input[0]) == 0xEF &&
        static_cast<unsigned char>(input[1]) == 0xBB &&
        static_cast<unsigned char>(input[2]) == 0xBF) {
        bom.found = true;
        bom.size = 3;
    }
    return bom;
}

} // namespace details

/**
 * @brief Returns the UTF-8 BOM (Byte Order Mark) string
 * @return The UTF-8 BOM sequence as a string
 * 
 * This utility function provides a consistent way to get the UTF-8 BOM
 * sequence for use in tests and string manipulation.
 * 
 * @example
 * @code
 * std::string text_with_bom = u8scan::bom_str() + "Hello World";
 * @endcode
 */
inline std::string bom_str() {
    return "\xEF\xBB\xBF";
}

/**
 * @brief Simplified and minimal UTF-8 scanner
 * Main entry point - automatically handles BOM and provides character-by-character processing
 */
template<typename Processor>
inline std::string scan_utf8(const std::string& input, Processor processor) {
    std::string result;
    BOMInfo bom_info = details::detect_bom(input);
    std::size_t pos = bom_info.found ? 3 : 0;  // Skip BOM if found
    
    while (pos < input.length()) {
        CharInfo char_info = details::extract_char_info(input, pos, true, true);
        if (pos >= input.length()) break;  // Safety check
        
        ProcessResult proc_result = processor(char_info, input.data() + pos);
        
        switch (proc_result.action) {
            case ScanAction::COPY_TO_OUTPUT:
                result.append(input, pos, char_info.byte_count);
                break;
            case ScanAction::REPLACE:
                result += proc_result.replacement;
                break;
            case ScanAction::IGNORE:
                break;
            case ScanAction::STOP_SCANNING:
                return result;
        }
        
        pos += char_info.byte_count;
    }
    
    return result;
}

/**
 * @brief ASCII-only simplified scanner for maximum performance
 */
template<typename Processor>
inline std::string scan_ascii(const std::string& input, Processor processor) {
    std::string result;
    for (std::size_t pos = 0; pos < input.length(); ++pos) {
        CharInfo char_info;
        char_info.start_pos = pos;
        char_info.byte_count = 1;
        char_info.codepoint = static_cast<uint32_t>(input[pos]);
        char_info.is_ascii = true;
        char_info.is_valid_utf8 = true;
        char_info.is_bom = false;
        
        ProcessResult proc_result = processor(char_info, input.data() + pos);
        
        switch (proc_result.action) {
            case ScanAction::COPY_TO_OUTPUT:
                result += input[pos];
                break;
            case ScanAction::REPLACE:
                result += proc_result.replacement;
                break;
            case ScanAction::IGNORE:
                break;
            case ScanAction::STOP_SCANNING:
                return result;
        }
    }
    return result;
}

/**
 * @brief Legacy compatibility function - uses the simplified scanner
 */
inline std::string scan_string(const std::string& input, CharProcessor processor, const ScanConfig& config = ScanConfig()) {
    std::string result;
    BOMInfo bom_info;
    std::size_t pos = 0;
    
    // Handle BOM if needed
    if (config.bom_action != BOMAction::IGNORE) {
        bom_info = details::detect_bom(input);
        if (bom_info.found) {
            bom_info.action_taken = config.bom_action;
            if (config.bom_action == BOMAction::COPY) {
                result.append(input, 0, 3);
            } else if (config.bom_action == BOMAction::CUSTOM && config.bom_handler) {
                std::string bom_result = config.bom_handler(bom_info, input.data());
                result += bom_result;
            }
            pos = 3; // Skip BOM in processing
        }
    } else {
        // Still detect BOM but ignore it
        bom_info = details::detect_bom(input);
        if (bom_info.found) {
            pos = 3; // Skip BOM
        }
    }
    
    // Main scanning loop
    while (pos < input.length()) {
        if (config.max_output_size > 0 && result.length() >= config.max_output_size) break;
        
        CharInfo char_info = details::extract_char_info(input, pos, config.utf8_mode, config.validate_utf8);
        if (pos >= input.length()) break;  // Safety check
        
        ProcessResult proc_result = processor(char_info, input.data() + pos);
        
        switch (proc_result.action) {
            case ScanAction::COPY_TO_OUTPUT:
                result.append(input, pos, char_info.byte_count);
                break;
            case ScanAction::REPLACE:
                result += proc_result.replacement;
                break;
            case ScanAction::IGNORE:
                break;
            case ScanAction::STOP_SCANNING:
                return result;
        }
        
        pos += char_info.byte_count;
    }
    
    return result;
}

/**
 * @brief High-performance ASCII-only scanning (legacy compatibility)
 */
inline std::string scan_string_ascii(const std::string& input, CharProcessor processor, std::size_t max_output_size = 0) {
    std::string result = scan_ascii(input, processor);
    if (max_output_size > 0 && result.length() > max_output_size) {
        result.resize(max_output_size);
    }
    return result;
}

/**
 * @brief Create a character range for STL algorithms
 */
inline CharRange make_char_range(const std::string& str, bool utf8_mode = true, bool validate = true, bool skip_bom = true) {
    return CharRange(str, utf8_mode, validate, skip_bom);
}

/**
 * @brief Create a character range with bounds
 */
inline CharRange make_char_range(const std::string& str, std::size_t start, std::size_t end, bool utf8_mode = true, bool validate = true, bool skip_bom = true) {
    return CharRange(str, start, end, utf8_mode, validate, skip_bom);
}

/**
 * @brief STL-only quoted_str implementation
 */
inline std::string quoted_str(const std::string& input, char start_delim = '"', char end_delim = '"', char escape = '\\') {
    std::string result;
    result.reserve(input.length() + 10);  // Pre-allocate for efficiency
    
    // Add start delimiter
    result += start_delim;
    
    auto char_range = make_char_range(input);
    
    // Use STL transform to process each character
    std::for_each(char_range.begin(), char_range.end(), [&](const CharInfo& info) {
        if (info.is_ascii) {
            char c = static_cast<char>(info.codepoint);
            if (c == start_delim || c == end_delim || c == escape) {
                result += escape;  // Add escape character
            }
            result += c;
        } else {
            // Add multi-byte UTF-8 character as-is 
            result.append(input, info.start_pos, info.byte_count);
        }
    });
    
    // Add end delimiter
    result += end_delim;
    return result;
}

/**
 * @brief Get information about a UTF-8 character at a specific position
 */
inline CharInfo get_char_info(const std::string& input, std::size_t pos, bool validate_utf8 = true) {
    return details::extract_char_info(input, pos, true, validate_utf8);
}

/**
 * @brief High-performance character transformation
 */
template<typename OutputIt, typename Transformer>
inline OutputIt transform_chars(const std::string& input, OutputIt result, Transformer transformer) {
    auto range = make_char_range(input);
    return std::transform(range.begin(), range.end(), result, transformer);
}

/**
 * @brief STL-like copy function - copies all characters
 */
template<typename OutputIt>
inline OutputIt copy(const std::string& input, OutputIt result) {
    auto range = make_char_range(input);
    for (const auto& char_info : range) {
        std::string char_str = input.substr(char_info.start_pos, char_info.byte_count);
        result = std::copy(char_str.begin(), char_str.end(), result);
    }
    return result;
}

/**
 * @brief STL-like copy_if - copies characters matching a predicate
 */
template<typename OutputIt, typename Predicate>
inline OutputIt copy_if(const std::string& input, OutputIt result, Predicate pred) {
    auto range = make_char_range(input);
    for (const auto& char_info : range) {
        if (pred(char_info)) {
            std::string char_str = input.substr(char_info.start_pos, char_info.byte_count);
            result = std::copy(char_str.begin(), char_str.end(), result);
        }
    }
    return result;
}

/**
 * @brief Copies characters until a predicate is met
 */
template<typename OutputIt, typename Predicate>
inline OutputIt copy_until(const std::string& input, OutputIt result, Predicate pred) {
    auto range = make_char_range(input);
    for (const auto& char_info : range) {
        if (pred(char_info)) {
            break;
        }
        std::string char_str = input.substr(char_info.start_pos, char_info.byte_count);
        result = std::copy(char_str.begin(), char_str.end(), result);
    }
    return result;
}

/**
 * @brief Copies characters from the first time a predicate is met
 */
template<typename OutputIt, typename Predicate>
inline OutputIt copy_from(const std::string& input, OutputIt result, Predicate pred) {
    auto range = make_char_range(input);
    auto start_it = std::find_if(range.begin(), range.end(), pred);
    for (auto it = start_it; it != range.end(); ++it) {
        std::string char_str = input.substr(it->start_pos, it->byte_count);
        result = std::copy(char_str.begin(), char_str.end(), result);
    }
    return result;
}

/**
 * @brief Copies a specified number of characters
 */
template<typename OutputIt>
inline OutputIt copy_n(const std::string& input, OutputIt result, size_t n) {
    auto range = make_char_range(input);
    auto it = range.begin();
    for (size_t i = 0; i < n && it != range.end(); ++i, ++it) {
        std::string char_str = input.substr(it->start_pos, it->byte_count);
        result = std::copy(char_str.begin(), char_str.end(), result);
    }
    return result;
}

/**
 * @brief Copies characters while a predicate is met
 */
template<typename OutputIt, typename Predicate>
inline OutputIt copy_while(const std::string& input, OutputIt result, Predicate pred) {
    auto range = make_char_range(input);
    for (const auto& char_info : range) {
        if (!pred(char_info)) {
            break;
        }
        std::string char_str = input.substr(char_info.start_pos, char_info.byte_count);
        result = std::copy(char_str.begin(), char_str.end(), result);
    }
    return result;
}

/**
 * @brief Calculate the length of a UTF-8 string in code points (characters)
 * @param input The UTF-8 string to measure
 * @param utf8_mode Whether to use UTF-8 mode (true) or ASCII mode (false), defaults to true
 * @param validate Whether to validate UTF-8 sequences, defaults to true
 * @return The number of Unicode code points (characters) in the string
 * 
 * This function counts the number of Unicode characters (code points) in a UTF-8 string,
 * not the number of bytes. For example:
 * - "Hello" returns 5 (5 ASCII characters)
 * - "世界" returns 2 (2 Chinese characters, even though they take 6 bytes)
 * - "🌍🚀" returns 2 (2 emoji characters, even though they take 8 bytes)
 * 
 * In ASCII mode, each byte is counted as one character.
 * In UTF-8 mode with validation disabled, malformed sequences are counted optimistically.
 * BOM is automatically detected and excluded from the count.
 * 
 * @code
 * std::string text = u8"Hello 世界! 🌍";
 * size_t length = u8scan::length(text);  // Returns 10 (not 16 bytes)
 * 
 * std::string ascii = "Hello World";
 * size_t ascii_length = u8scan::length(ascii, false);  // ASCII mode: returns 11
 * @endcode
 */
inline std::size_t length(const std::string& input, bool utf8_mode = true, bool validate = true) {
    // Detect and skip BOM if present
    BOMInfo bom_info = details::detect_bom(input);
    std::size_t start_pos = bom_info.found ? 3 : 0;
    
    auto range = make_char_range(input, start_pos, input.length(), utf8_mode, validate);
    return range.size();
}

/**
 * @brief Get character information at a specific index (character position, not byte position)
 * @param input The UTF-8 string to access
 * @param index The character index (0-based) to access
 * @param utf8_mode Whether to use UTF-8 mode (true) or ASCII mode (false), defaults to true
 * @param validate Whether to validate UTF-8 sequences, defaults to true
 * @return CharInfo for the character at the specified index
 * @throws std::out_of_range if index is beyond the string length in characters
 * 
 * This function provides character-level access to UTF-8 strings, similar to std::string::at()
 * but operating on character indices rather than byte indices.
 * 
 * @code
 * std::string text = u8"Hello 世界! 🌍";
 * auto char_info = u8scan::at(text, 6);  // Gets the first Chinese character '世'
 * assert(char_info.codepoint == 0x4E16);  // Unicode for '世'
 * @endcode
 */
inline CharInfo at(const std::string& input, std::size_t index, bool utf8_mode = true, bool validate = true) {
    // Detect and skip BOM if present
    BOMInfo bom_info = details::detect_bom(input);
    std::size_t start_pos = bom_info.found ? 3 : 0;
    
    auto range = make_char_range(input, start_pos, input.length(), utf8_mode, validate);
    auto it = range.begin();
    auto end_it = range.end();
    
    for (std::size_t i = 0; i < index && it != end_it; ++i, ++it) {
        // Advance iterator to the desired position
    }
    
    if (it == end_it) {
        throw std::out_of_range("Index out of range");
    }
    
    return *it;
}

/**
 * @brief Check if string is empty (ignoring BOM if present)
 * @param input The UTF-8 string to check
 * @param utf8_mode Whether to use UTF-8 mode (true) or ASCII mode (false), defaults to true
 * @param validate Whether to validate UTF-8 sequences, defaults to true
 * @return true if the string contains no characters (excluding BOM), false otherwise
 * 
 * This function checks if a UTF-8 string is empty at the character level.
 * A string containing only a BOM (Byte Order Mark) is considered empty.
 * 
 * @code
 * std::string empty_str = "";
 * std::string bom_only = "\xEF\xBB\xBF";  // UTF-8 BOM
 * std::string with_content = u8"Hello";
 * 
 * assert(u8scan::empty(empty_str) == true);
 * assert(u8scan::empty(bom_only) == true);   // BOM-only string is empty
 * assert(u8scan::empty(with_content) == false);
 * @endcode
 */
inline bool empty(const std::string& input, bool utf8_mode = true, bool validate = true) {
    // Detect and skip BOM if present
    BOMInfo bom_info = details::detect_bom(input);
    std::size_t start_pos = bom_info.found ? 3 : 0;
    
    auto range = make_char_range(input, start_pos, input.length(), utf8_mode, validate);
    return range.empty();
}

/**
 * @brief Get the first character from a UTF-8 string
 * @param input The UTF-8 string to access
 * @param utf8_mode Whether to use UTF-8 mode (true) or ASCII mode (false), defaults to true
 * @param validate Whether to validate UTF-8 sequences, defaults to true
 * @return CharInfo for the first character in the string
 * @throws std::out_of_range if the string is empty (excluding BOM)
 * 
 * This function returns the first character in a UTF-8 string, skipping any BOM if present.
 * 
 * @code
 * std::string text = u8"Hello 世界! 🌍";
 * auto first_char = u8scan::front(text);
 * assert(first_char.codepoint == 'H');
 * 
 * std::string with_bom = u8"\xEF\xBB\xBFHello";  // UTF-8 BOM + Hello
 * auto first_char_after_bom = u8scan::front(with_bom);
 * assert(first_char_after_bom.codepoint == 'H');  // BOM is skipped
 * @endcode
 */
inline CharInfo front(const std::string& input, bool utf8_mode = true, bool validate = true) {
    // Detect and skip BOM if present
    BOMInfo bom_info = details::detect_bom(input);
    std::size_t start_pos = bom_info.found ? 3 : 0;
    
    auto range = make_char_range(input, start_pos, input.length(), utf8_mode, validate);
    auto it = range.begin();
    
    if (it == range.end()) {
        throw std::out_of_range("String is empty");
    }
    
    return *it;
}

/**
 * @brief Get the last character from a UTF-8 string
 * @param input The UTF-8 string to access
 * @param utf8_mode Whether to use UTF-8 mode (true) or ASCII mode (false), defaults to true
 * @param validate Whether to validate UTF-8 sequences, defaults to true
 * @return CharInfo for the last character in the string
 * @throws std::out_of_range if the string is empty (excluding BOM)
 * 
 * This function returns the last character in a UTF-8 string.
 * 
 * @code
 * std::string text = u8"Hello 世界! 🌍";
 * auto last_char = u8scan::back(text);
 * assert(last_char.codepoint == 0x1F30D);  // Unicode for 🌍
 * @endcode
 */
inline CharInfo back(const std::string& input, bool utf8_mode = true, bool validate = true) {
    // Detect and skip BOM if present
    BOMInfo bom_info = details::detect_bom(input);
    std::size_t start_pos = bom_info.found ? 3 : 0;
    
    auto range = make_char_range(input, start_pos, input.length(), utf8_mode, validate);
    auto it = range.begin();
    auto end_it = range.end();
    
    if (it == end_it) {
        throw std::out_of_range("String is empty");
    }
    
    CharInfo last_char;
    while (it != end_it) {
        last_char = *it;
        ++it;
    }
    
    return last_char;
}

/**
 * @brief The `predicates` namespace provides a collection of predicate functions
 * suitable for use with STL algorithms.
 */
namespace predicates {

/**
 * @brief Check if character is ASCII
 * @return Predicate function that returns true for ASCII characters (codepoint < 0x80)
 */
inline std::function<bool(const CharInfo&)> is_ascii() {
    return [](const CharInfo& info) { return info.is_ascii; };
}

/**
 * @brief Check if character is UTF-8 multi-byte
 * @return Predicate function that returns true for UTF-8 multi-byte characters (codepoint >= 0x80)
 */
inline std::function<bool(const CharInfo&)> is_utf8() {
    return [](const CharInfo& info) { return !info.is_ascii; };
}

/**
 * @brief Check if character is valid UTF-8
 * @return Predicate function that returns true for valid UTF-8 sequences
 */
inline std::function<bool(const CharInfo&)> is_valid() {
    return [](const CharInfo& info) { return info.is_valid_utf8; };
}

/**
 * @brief Check if character has specific codepoint
 * @param codepoint The Unicode codepoint to match
 * @return Predicate function that returns true if character matches the codepoint
 */
inline std::function<bool(const CharInfo&)> has_codepoint(uint32_t codepoint) {
    return [codepoint](const CharInfo& info) { return info.codepoint == codepoint; };
}

/**
 * @brief Check if character codepoint is in range
 * @param min_cp Minimum codepoint (inclusive)
 * @param max_cp Maximum codepoint (inclusive)
 * @return Predicate function that returns true if character is in the specified range
 */
inline std::function<bool(const CharInfo&)> in_range(uint32_t min_cp, uint32_t max_cp) {
    return [min_cp, max_cp](const CharInfo& info) { 
        return info.codepoint >= min_cp && info.codepoint <= max_cp; 
    };
}

/**
 * @brief Check if character is ASCII digit
 * @return Predicate function that returns true for ASCII digits (0-9)
 */
inline std::function<bool(const CharInfo&)> is_digit_ascii() {
    return in_range('0', '9');
}

/**
 * @brief Check if character is ASCII letter
 * @return Predicate function that returns true for ASCII letters (A-Z, a-z)
 */
inline std::function<bool(const CharInfo&)> is_alpha_ascii() {
    return [](const CharInfo& info) {
        return (info.codepoint >= 'A' && info.codepoint <= 'Z') ||
               (info.codepoint >= 'a' && info.codepoint <= 'z');
    };
}

/**
 * @brief Check if character is ASCII alphanumeric
 * @return Predicate function that returns true for ASCII letters and digits (A-Z, a-z, 0-9)
 */
inline std::function<bool(const CharInfo&)> is_alphanum_ascii() {
    return [](const CharInfo& info) {
        return (info.codepoint >= 'A' && info.codepoint <= 'Z') ||
               (info.codepoint >= 'a' && info.codepoint <= 'z') ||
               (info.codepoint >= '0' && info.codepoint <= '9');
    };
}

/**
 * @brief Check if character is ASCII lowercase letter
 * @return Predicate function that returns true for ASCII lowercase letters (a-z)
 */
inline std::function<bool(const CharInfo&)> is_lowercase_ascii() {
    return [](const CharInfo& info) {
        return (info.codepoint >= 'a' && info.codepoint <= 'z');
    };
}

/**
 * @brief Check if character is ASCII uppercase letter
 * @return Predicate function that returns true for ASCII uppercase letters (A-Z)
 */
inline std::function<bool(const CharInfo&)> is_uppercase_ascii() {
    return [](const CharInfo& info) {
        return (info.codepoint >= 'A' && info.codepoint <= 'Z');
    };
}

/**
 * @brief Check if character is ASCII whitespace
 * @return Predicate function that returns true for ASCII whitespace characters (space, tab, newline, carriage return)
 * 
 * ASCII whitespace characters include:
 * - Space (U+0020)
 * - Tab (U+0009)
 * - Line feed (U+000A)
 * - Carriage return (U+000D)
 */
inline std::function<bool(const CharInfo&)> is_whitespace_ascii() {
    return [](const CharInfo& info) {
        return info.codepoint == ' ' || info.codepoint == '\t' || 
               info.codepoint == '\n' || info.codepoint == '\r';
    };
}

/**
 * @brief Check if character is an emoji
 * @return Predicate function that returns true for Unicode emoji characters
 * 
 * Detects emoji characters based on Unicode emoji ranges including:
 * - Emoticons (U+1F600-U+1F64F): 😀😃😄😁😆😅😂🤣😊😇🙂🙃😉😌😍🥰😘😗😙😚😋😛😝😜🤪🤨🧐🤓😎🤩
 * - Miscellaneous Symbols and Pictographs (U+1F300-U+1F5FF): 🌍🌎🌏🌐🗺🏔⛰🌋🗻🏕🏖🏜🏝🏞🏟🏛🏗🏘🏚🏠🏡
 * - Transport and Map Symbols (U+1F680-U+1F6FF): 🚀🛸🚁🚂🚃🚄🚅🚆🚇🚈🚉🚊🚝🚞🚋🚃🚄🚅🚆🚇🚈🚉
 * - Supplemental Symbols and Pictographs (U+1F900-U+1F9FF): 🤖🦄🦅🦆🦇🦈🦉🦊🦋🦌🦍🦎🦏🦐🦑🦒🦓🦔🦕🦖🦗
 * - Regional Indicator Symbols for flags (U+1F1E6-U+1F1FF): 🇦🇧🇨🇩🇪🇫🇬🇭🇮🇯🇰🇱🇲🇳🇴🇵🇶🇷🇸🇹🇺🇻🇼🇽🇾🇿
 * - Selected symbols from other ranges: ⭐❤️⚡☀️❄️✨⚽⬆️⬇️⬅️➡️
 * 
 * Note: Plain text symbols like © ® ™ are NOT considered emoji unless they appear with 
 * emoji variation selectors (U+FE0F). This function only detects the base emoji characters.
 * 
 * @code
 * auto range = u8scan::make_char_range("Hello 🌍 World! 🚀");
 * size_t emoji_count = std::count_if(range.begin(), range.end(), u8scan::predicates::is_emoji());
 * // emoji_count will be 2 (🌍 and 🚀)
 * @endcode
 */
inline std::function<bool(const CharInfo&)> is_emoji() {
    return [](const CharInfo& info) {
        uint32_t cp = info.codepoint;
        
        // Main emoji Unicode ranges based on Unicode Standard
        // Reference: https://unicode.org/emoji/charts/full-emoji-list.html
        
        // Emoticons (U+1F600-U+1F64F)
        if (cp >= 0x1F600 && cp <= 0x1F64F) return true;
        
        // Miscellaneous Symbols and Pictographs (U+1F300-U+1F5FF)
        if (cp >= 0x1F300 && cp <= 0x1F5FF) return true;
        
        // Transport and Map Symbols (U+1F680-U+1F6FF)
        if (cp >= 0x1F680 && cp <= 0x1F6FF) return true;
        
        // Supplemental Symbols and Pictographs (U+1F900-U+1F9FF)
        if (cp >= 0x1F900 && cp <= 0x1F9FF) return true;
        
        // Symbols and Pictographs Extended-A (U+1FA70-U+1FAFF)
        if (cp >= 0x1FA70 && cp <= 0x1FAFF) return true;
        
        // Regional Indicator Symbols (Flags) (U+1F1E6-U+1F1FF)
        if (cp >= 0x1F1E6 && cp <= 0x1F1FF) return true;
        
        // Miscellaneous Symbols (some emoji) (U+2600-U+26FF)
        // Common emoji in this range: ☀️ ⭐ ❤️ ⚡ ☔ ❄️ ⛄ ⚽ ⚾ ⛳ etc.
        if (cp >= 0x2600 && cp <= 0x26FF) {
            // More selective - only common emoji symbols
            return (cp >= 0x2600 && cp <= 0x2B55) || // Various symbols
                   (cp >= 0x2B50 && cp <= 0x2B55);   // Stars and other symbols
        }
        
        // Dingbats (some emoji) (U+2700-U+27BF)
        // Common emoji: ✂️ ✈️ ✉️ ✏️ ✒️ ✔️ ✖️ ✨ ✳️ ✴️ etc.
        if (cp >= 0x2700 && cp <= 0x27BF) {
            // More selective - only actual emoji
            return (cp >= 0x2702 && cp <= 0x2705) || // Scissors, airplane, etc.
                   (cp >= 0x2708 && cp <= 0x270F) || // Airplane, envelope, etc.
                   (cp >= 0x2712 && cp <= 0x2714) || // Writing implements, checkmark
                   (cp >= 0x2716 && cp <= 0x2716) || // Heavy multiplication X
                   (cp >= 0x271D && cp <= 0x271D) || // Latin cross
                   (cp >= 0x2721 && cp <= 0x2721) || // Star of David
                   (cp >= 0x2728 && cp <= 0x2728) || // Sparkles
                   (cp >= 0x2733 && cp <= 0x2734) || // Eight-pointed stars
                   (cp >= 0x2744 && cp <= 0x2744) || // Snowflake
                   (cp >= 0x2747 && cp <= 0x2747) || // Sparkle
                   (cp >= 0x274C && cp <= 0x274C) || // Cross mark
                   (cp >= 0x274E && cp <= 0x274E) || // Negative squared cross mark
                   (cp >= 0x2753 && cp <= 0x2755) || // Question marks
                   (cp >= 0x2757 && cp <= 0x2757) || // Heavy exclamation mark
                   (cp >= 0x2763 && cp <= 0x2764) || // Hearts
                   (cp >= 0x2795 && cp <= 0x2797) || // Plus/minus signs
                   (cp >= 0x27A1 && cp <= 0x27A1) || // Arrow
                   (cp >= 0x27B0 && cp <= 0x27B0) || // Curly loop
                   (cp >= 0x27BF && cp <= 0x27BF);   // Double curly loop
        }
        
        // Geometric Shapes (some emoji) (U+25A0-U+25FF)
        // Includes: ▶️ ◀️ 🔺 🔻 ⬆️ ⬇️ ⬅️ ➡️ etc.
        if (cp >= 0x25A0 && cp <= 0x25FF) {
            // Select only actual emoji shapes
            return (cp >= 0x25AA && cp <= 0x25AB) || // Small squares
                   (cp >= 0x25B6 && cp <= 0x25B6) || // Play button
                   (cp >= 0x25C0 && cp <= 0x25C0) || // Reverse button
                   (cp >= 0x25FB && cp <= 0x25FE);   // Various squares
        }
        
        // Arrows (some emoji) (U+2190-U+21FF)
        if (cp >= 0x2190 && cp <= 0x21FF) {
            // Select only common arrow emoji
            return (cp >= 0x2190 && cp <= 0x2199) || // Basic arrows
                   (cp >= 0x21A9 && cp <= 0x21AA);   // Hooked arrows
        }
        
        // Additional individual emoji characters scattered in other ranges
        // Note: © ® ™ are only emoji when followed by variation selector U+FE0F
        // In plain form they are considered text symbols, not emoji
        
        // Some individual symbols commonly used as emoji
        if (cp == 0x203C || cp == 0x2049) return true; // ‼️ ⁉️
        // Note: ™ (U+2122) is only emoji with variation selector, not in plain form
        if (cp == 0x2139) return true; // ℹ️
        if (cp == 0x2194 || cp == 0x2195) return true; // ↔️ ↕️
        if (cp == 0x2196 || cp == 0x2197 || cp == 0x2198 || cp == 0x2199) return true; // ↖️ ↗️ ↘️ ↙️
        if (cp == 0x21A9 || cp == 0x21AA) return true; // ↩️ ↪️
        if (cp == 0x231A || cp == 0x231B) return true; // ⌚ ⌛
        if (cp == 0x2328) return true; // ⌨️
        if (cp == 0x23CF) return true; // ⏏️
        if (cp >= 0x23E9 && cp <= 0x23F3) return true; // ⏩ ⏪ ⏫ ⏬ ⏭ ⏮ ⏯ ⏰ ⏱ ⏲ ⏳
        if (cp >= 0x23F8 && cp <= 0x23FA) return true; // ⏸ ⏹ ⏺
        if (cp == 0x24C2) return true; // Ⓜ️
        if (cp >= 0x25AA && cp <= 0x25AB) return true; // ▪️ ▫️
        if (cp == 0x25B6 || cp == 0x25C0) return true; // ▶️ ◀️
        if (cp >= 0x25FB && cp <= 0x25FE) return true; // ◻️ ◼️ ◽ ◾
        if (cp >= 0x2934 && cp <= 0x2935) return true; // ⤴️ ⤵️
        if (cp >= 0x2B05 && cp <= 0x2B07) return true; // ⬅️ ➡️ ⬇️
        if (cp == 0x2B1B || cp == 0x2B1C) return true; // ⬛ ⬜
        if (cp == 0x2B50 || cp == 0x2B55) return true; // ⭐ ⭕
        if (cp == 0x3030) return true; // 〰️
        if (cp == 0x303D) return true; // 〽️
        if (cp == 0x3297 || cp == 0x3299) return true; // ㊗️ ㊙️
        if (cp == 0x1F004) return true; // 🀄 (red dragon tile, commonly used in Mahjong)
        if (cp == 0x1F0CF) return true; // 🃏 (joker card, commonly used in playing cards)
        
        return false;
    };
}

} // namespace predicates

/**
 * @brief Converts an ASCII character to lowercase.
 * @param info The character information.
 * @return The lowercase version of the character if it's an ASCII letter (A-Z), otherwise returns the original codepoint.
 * 
 * This function only transforms ASCII uppercase letters (A-Z) to lowercase.
 * Non-ASCII characters and non-uppercase ASCII characters are returned unchanged.
 */
inline uint32_t to_lower_ascii(const CharInfo& info) {
    if (info.is_ascii && info.codepoint >= 'A' && info.codepoint <= 'Z') {
        return info.codepoint + ('a' - 'A');
    }
    return info.codepoint;
}

/**
 * @brief Converts an ASCII character to uppercase.
 * @param info The character information.
 * @return The uppercase version of the character if it's an ASCII letter (a-z), otherwise returns the original codepoint.
 * 
 * This function only transforms ASCII lowercase letters (a-z) to uppercase.
 * Non-ASCII characters and non-lowercase ASCII characters are returned unchanged.
 */
inline uint32_t to_upper_ascii(const CharInfo& info) {
    if (info.is_ascii && info.codepoint >= 'a' && info.codepoint <= 'z') {
        return info.codepoint - ('a' - 'A');
    }
    return info.codepoint;
}

/**
 * @brief Converts a Unicode codepoint to its UTF-8 string representation.
 * @param info The character information containing the codepoint.
 * @return The UTF-8 string representation of the codepoint.
 * 
 * This function converts any Unicode codepoint to its proper UTF-8 byte sequence.
 * For ASCII characters (< 0x80), it returns a single-byte string.
 * For non-ASCII characters, it returns the proper multi-byte UTF-8 sequence.
 */
inline std::string to_string(const CharInfo& info) {
    std::string result;
    uint32_t cp = info.codepoint;
    
    if (cp < 0x80) {
        // ASCII character
        result.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        // 2-byte UTF-8
        result.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        // 3-byte UTF-8
        result.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x110000) {
        // 4-byte UTF-8
        result.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    
    return result;
}

/**
 * @brief Converts a character to lowercase and returns as a UTF-8 string.
 * @param info The character information.
 * @return The lowercase version as a UTF-8 string.
 * 
 * For ASCII characters, this function converts uppercase letters to lowercase
 * and returns other ASCII characters unchanged as single-character strings.
 * For non-ASCII characters, it returns the original character as a UTF-8 string.
 */
inline std::string to_lower_ascii_str(const CharInfo& info) {
    if (info.is_ascii) {
        char c = static_cast<char>(to_lower_ascii(info));
        return std::string(1, c);
    } else {
        return to_string(info);
    }
}

/**
 * @brief Converts a character to uppercase and returns as a UTF-8 string.
 * @param info The character information.
 * @return The uppercase version as a UTF-8 string.
 * 
 * For ASCII characters, this function converts lowercase letters to uppercase
 * and returns other ASCII characters unchanged as single-character strings.
 * For non-ASCII characters, it returns the original character as a UTF-8 string.
 */
inline std::string to_upper_ascii_str(const CharInfo& info) {
    if (info.is_ascii) {
        char c = static_cast<char>(to_upper_ascii(info));
        return std::string(1, c);
    } else {
        return to_string(info);
    }
}

/**
 * @brief Checks if a string contains a UTF-8 BOM (Byte Order Mark)
 * @param input The input string to check
 * @return True if the string starts with a UTF-8 BOM, false otherwise
 */
inline bool has_bom(const std::string& input) {
    return details::detect_bom(input).found;
}

// Implementation for CharIterator
inline CharInfo CharIterator::get_char_info_impl(const std::string& input, std::size_t pos, bool utf8_mode, bool validate) {
    return details::extract_char_info(input, pos, utf8_mode, validate);
}

} // namespace u8scan

#endif // U8SCAN_H
