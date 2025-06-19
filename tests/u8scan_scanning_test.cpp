#include "../include/utest/utest.h"
#include "../include/u8scan/u8scan.h"
#include <vector>

using namespace u8scan;

// STL alternative: U8ScanSTL::STLForEachAlgorithm (see tests/u8scan_stl_test.cpp)
// This test is covered by STL-based for_each and CharRange usage for character collection.
// Test basic ASCII scanning
UTEST_FUNC_DEF2(U8Scan, BasicASCIIScanning) {
    std::string input = "Hello World!";
    
    std::vector<CharInfo> chars;
    auto collector = [&chars](const CharInfo& info, const char* /*data*/) -> ProcessResult {
        chars.push_back(info);
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    std::string result = scan_string(input, collector);
    
    UTEST_ASSERT_STR_EQUALS(input.c_str(), result.c_str());
    UTEST_ASSERT_EQUALS(input.length(), chars.size());
    
    // Check all characters are ASCII
    for (const auto& char_info : chars) {
        UTEST_ASSERT_TRUE(char_info.is_ascii);
        UTEST_ASSERT_EQUALS(1u, char_info.byte_count);
        UTEST_ASSERT_TRUE(char_info.is_valid_utf8);
        UTEST_ASSERT_FALSE(char_info.is_bom);
    }
}

// Test UTF-8 multi-byte character scanning
UTEST_FUNC_DEF2(U8Scan, UTF8MultiByteScanning) {
    // STL alternative: U8ScanSTL::STLAlgorithmCompatibility (see tests/u8scan_stl_test.cpp)
    // This test is covered by STL-based count_if, find_if, and CharRange usage.
    
    std::string input = "Hello 世界!";
    
    std::vector<CharInfo> chars;
    auto collector = [&chars](const CharInfo& info, const char* /*data*/) -> ProcessResult {
        chars.push_back(info);
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    std::string result = scan_string(input, collector);
    
    UTEST_ASSERT_STR_EQUALS(input.c_str(), result.c_str());
    
    // Should have: H e l l o (space) 世 界 !
    UTEST_ASSERT_EQUALS(9u, chars.size());
    
    // Check specific characters
    UTEST_ASSERT_TRUE(chars[0].is_ascii); // 'H'
    UTEST_ASSERT_TRUE(chars[5].is_ascii); // ' ' (space)
    UTEST_ASSERT_FALSE(chars[6].is_ascii); // '世' - should be multi-byte
    UTEST_ASSERT_EQUALS(3u, chars[6].byte_count); // Chinese character is 3 bytes in UTF-8
    UTEST_ASSERT_FALSE(chars[7].is_ascii); // '界' - should be multi-byte  
    UTEST_ASSERT_EQUALS(3u, chars[7].byte_count);
    UTEST_ASSERT_TRUE(chars[8].is_ascii); // '!'
}

// Test BOM detection and handling
UTEST_FUNC_DEF2(U8Scan, BOMDetection) {
    // Create string with UTF-8 BOM
    std::string input_with_bom;
    input_with_bom += static_cast<char>(0xEF);
    input_with_bom += static_cast<char>(0xBB);
    input_with_bom += static_cast<char>(0xBF);
    input_with_bom += "Hello";
    
    // Test BOM IGNORE
    {
        ScanConfig config;
        config.bom_action = BOMAction::IGNORE;
        
        auto processor = [](const CharInfo& /*info*/, const char* /*data*/) -> ProcessResult {
            return ProcessResult(ScanAction::COPY_TO_OUTPUT);
        };
        
        std::string result = scan_string(input_with_bom, processor, config);
        UTEST_ASSERT_STR_EQUALS("Hello", result.c_str());
    }
    
    // Test BOM COPY
    {
        ScanConfig config;
        config.bom_action = BOMAction::COPY;
        
        auto processor = [](const CharInfo& /*info*/, const char* /*data*/) -> ProcessResult {
            return ProcessResult(ScanAction::COPY_TO_OUTPUT);
        };
        
        std::string result = scan_string(input_with_bom, processor, config);
        UTEST_ASSERT_EQUALS(8u, result.length()); // 3 BOM bytes + 5 "Hello"
    }
    
    // Test BOM CUSTOM
    {
        bool bom_handler_called = false;
        ScanConfig config;
        config.bom_action = BOMAction::CUSTOM;
        config.bom_handler = [&bom_handler_called](const BOMInfo& bom_info, const char* /*bom_data*/) -> std::string {
            bom_handler_called = true;
            UTEST_ASSERT_TRUE(bom_info.found);
            UTEST_ASSERT_EQUALS(3u, bom_info.size);
            return "[BOM]";
        };
        
        auto processor = [](const CharInfo& /*info*/, const char* /*data*/) -> ProcessResult {
            return ProcessResult(ScanAction::COPY_TO_OUTPUT);
        };
        
        std::string result = scan_string(input_with_bom, processor, config);
        UTEST_ASSERT_TRUE(bom_handler_called);
        UTEST_ASSERT_STR_EQUALS("[BOM]Hello", result.c_str());
    }
}

// Test character ignoring
UTEST_FUNC_DEF2(U8Scan, CharacterIgnoring) {
    // STL alternative: U8ScanSTL::PredicateFunctions (see tests/u8scan_stl_test.cpp)
    // This test is covered by STL-based count_if and CharRange usage for filtering.
    
    std::string input = "Hello World!";
    
    // Ignore all vowels
    auto vowel_filter = [](const CharInfo& info, const char* /*data*/) -> ProcessResult {
        if (info.is_ascii) {
            char c = static_cast<char>(info.codepoint);
            if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
                c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') {
                return ProcessResult(ScanAction::IGNORE);
            }
        }
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    std::string result = scan_string(input, vowel_filter);
    UTEST_ASSERT_STR_EQUALS("Hll Wrld!", result.c_str());
}

// Test character replacement
UTEST_FUNC_DEF2(U8Scan, CharacterReplacement) {
    // STL alternative: U8ScanSTL::STLTransformAlgorithm (see tests/u8scan_stl_test.cpp)
    // This test is covered by STL-based transform and CharRange usage for replacement.
    
    std::string input = "Hello World!";
    
    // Replace spaces with underscores
    auto space_replacer = [](const CharInfo& info, const char* /*data*/) -> ProcessResult {
        if (info.is_ascii && info.codepoint == ' ') {
            return ProcessResult(ScanAction::REPLACE, "_");
        }
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    std::string result = scan_string(input, space_replacer);
    UTEST_ASSERT_STR_EQUALS("Hello_World!", result.c_str());
}

// Test early termination
UTEST_FUNC_DEF2(U8Scan, EarlyTermination) {
    std::string input = "Hello World! More text here.";
    
    // Stop at exclamation mark
    auto early_stopper = [](const CharInfo& info, const char* /*data*/) -> ProcessResult {
        if (info.is_ascii && info.codepoint == '!') {
            return ProcessResult(ScanAction::STOP_SCANNING);
        }
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    std::string result = scan_string(input, early_stopper);
    UTEST_ASSERT_STR_EQUALS("Hello World", result.c_str()); // Should stop before '!'
}

// Test ASCII-only mode
UTEST_FUNC_DEF2(U8Scan, ASCIIOnlyMode) {
    // STL alternative: U8ScanSTL::PredicateFunctions (see tests/u8scan_stl_test.cpp)
    // This test is covered by STL-based count_if and CharRange usage for ASCII/UTF-8 detection.
    
    std::string input = "Hello 世界!";
    
    std::vector<CharInfo> chars;
    auto collector = [&chars](const CharInfo& info, const char* /*data*/) -> ProcessResult {
        chars.push_back(info);
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    std::string result = scan_string_ascii(input, collector);
    
    UTEST_ASSERT_STR_EQUALS(input.c_str(), result.c_str());
    
    // In ASCII mode, each byte is treated as a separate character
    UTEST_ASSERT_EQUALS(input.length(), chars.size());
    
    // All should be marked as ASCII (even UTF-8 bytes)
    for (const auto& char_info : chars) {
        UTEST_ASSERT_EQUALS(1u, char_info.byte_count);
    }
}

// Test output size limiting
UTEST_FUNC_DEF2(U8Scan, OutputSizeLimiting) {
    std::string input = "This is a long string";
    
    auto copy_processor = [](const CharInfo& /*info*/, const char* /*data*/) -> ProcessResult {
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    ScanConfig config;
    config.max_output_size = 10;
    
    std::string result = scan_string(input, copy_processor, config);
    
    UTEST_ASSERT_TRUE(result.length() <= 10);
    UTEST_ASSERT_STR_EQUALS("This is a ", result.c_str());
}

// Test invalid UTF-8 handling
UTEST_FUNC_DEF2(U8Scan, InvalidUTF8Handling) {
    // Create string with invalid UTF-8
    std::string input = "Valid";
    input += static_cast<char>(0xFF); // Invalid start byte
    input += "More";
    
    bool invalid_found = false;
    auto validator = [&invalid_found](const CharInfo& info, const char* /*data*/) -> ProcessResult {
        if (!info.is_valid_utf8) {
            invalid_found = true;
            return ProcessResult(ScanAction::REPLACE, "X");
        }
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    ScanConfig config;
    config.validate_utf8 = true;
    
    std::string result = scan_string(input, validator, config);
    
    UTEST_ASSERT_TRUE(invalid_found);
    UTEST_ASSERT_STR_EQUALS("ValidXMore", result.c_str());
}

// Test emoji handling (4-byte UTF-8)
UTEST_FUNC_DEF2(U8Scan, EmojiHandling) {
    // STL alternative: U8ScanSTL::PredicateFunctions (see tests/u8scan_stl_test.cpp)
    // This test is covered by STL-based count_if and CharRange usage for emoji/multibyte detection.
    
    std::string input = "Hello 🌍 World!";
    
    std::vector<CharInfo> chars;
    auto collector = [&chars](const CharInfo& info, const char* /*data*/) -> ProcessResult {
        chars.push_back(info);
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    std::string result = scan_string(input, collector);
    
    UTEST_ASSERT_STR_EQUALS(input.c_str(), result.c_str());
    
    // Find the emoji character
    bool emoji_found = false;
    for (const auto& char_info : chars) {
        if (predicates::is_emoji()(char_info)) {
            emoji_found = true;
            UTEST_ASSERT_TRUE(char_info.codepoint > 0x10000); // Should be in supplementary plane
            break;
        }
    }
    UTEST_ASSERT_TRUE(emoji_found);
}

// Test get_char_info function
UTEST_FUNC_DEF2(U8Scan, GetCharInfo) {
    // STL alternative: U8ScanSTL::CharIteratorFunctionality (see tests/u8scan_stl_test.cpp)
    // This test is covered by STL-based CharIterator and CharRange usage for position and increment.
    
    std::string input = "A世🌍";
    
    // Test ASCII character
    CharInfo info0 = u8scan::get_char_info(input, 0);
    UTEST_ASSERT_TRUE(info0.is_ascii);
    UTEST_ASSERT_EQUALS(1u, info0.byte_count);
    UTEST_ASSERT_EQUALS('A', static_cast<char>(info0.codepoint));
    
    // Test 3-byte UTF-8 character (世)
    CharInfo info1 = u8scan::get_char_info(input, 1);
    UTEST_ASSERT_FALSE(info1.is_ascii);
    UTEST_ASSERT_EQUALS(3u, info1.byte_count);
    UTEST_ASSERT_TRUE(info1.is_valid_utf8);
    
    // Test 4-byte UTF-8 character (🌍)
    CharInfo info4 = u8scan::get_char_info(input, 4);
    UTEST_ASSERT_FALSE(info4.is_ascii);
    UTEST_ASSERT_EQUALS(4u, info4.byte_count);
    UTEST_ASSERT_TRUE(info4.is_valid_utf8);
}

// Test empty string
UTEST_FUNC_DEF2(U8Scan, EmptyString) {
    // STL alternative: U8ScanSTL::STLForEachAlgorithm (see tests/u8scan_stl_test.cpp)
    // This test is covered by STL-based for_each and CharRange usage for empty string handling.
    
    std::string input = "";
    
    bool processor_called = false;
    auto processor = [&processor_called](const CharInfo& /*info*/, const char* /*data*/) -> ProcessResult {
        processor_called = true;
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    std::string result = scan_string(input, processor);
    
    UTEST_ASSERT_FALSE(processor_called);
    UTEST_ASSERT_STR_EQUALS("", result.c_str());
}

// Test BOM without content
UTEST_FUNC_DEF2(U8Scan, BOMOnly) {
    // Create string with only UTF-8 BOM
    std::string input;
    input += static_cast<char>(0xEF);
    input += static_cast<char>(0xBB);
    input += static_cast<char>(0xBF);
    
    bool bom_handler_called = false;
    ScanConfig config;
    config.bom_action = BOMAction::CUSTOM;
    config.bom_handler = [&bom_handler_called](const BOMInfo& bom_info, const char* /*bom_data*/) -> std::string {
        bom_handler_called = true;
        UTEST_ASSERT_TRUE(bom_info.found);
        return "[BOM]";
    };
    
    auto processor = [](const CharInfo& /*info*/, const char* /*data*/) -> ProcessResult {
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    std::string result = scan_string(input, processor, config);
    
    UTEST_ASSERT_TRUE(bom_handler_called);
    UTEST_ASSERT_STR_EQUALS("[BOM]", result.c_str());
}

// Test configuration disable BOM detection
UTEST_FUNC_DEF2(U8Scan, DisableBOMDetection) {
    // Create string with UTF-8 BOM
    std::string input;
    input += static_cast<char>(0xEF);
    input += static_cast<char>(0xBB);
    input += static_cast<char>(0xBF);
    input += "Hello";
    
    bool bom_handler_called = false;
    ScanConfig config;
    config.bom_action = BOMAction::IGNORE; // Disable BOM processing
    
    auto processor = [](const CharInfo& /*info*/, const char* /*data*/) -> ProcessResult {
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    std::string result = scan_string(input, processor, config);
    
    UTEST_ASSERT_FALSE(bom_handler_called); // BOM handler should not be called
    UTEST_ASSERT_STR_EQUALS("Hello", result.c_str()); // BOM should be skipped
}

// Test large replacement strings
UTEST_FUNC_DEF2(U8Scan, LargeReplacements) {
    // STL alternative: U8ScanSTL::STLTransformAlgorithm (see tests/u8scan_stl_test.cpp)
    // This test is covered by STL-based transform and CharRange usage for large replacements.
    
    std::string input = "a b c";
    
    // Replace single characters with large strings
    auto large_replacer = [](const CharInfo& info, const char* /*data*/) -> ProcessResult {
        if (info.is_ascii && info.codepoint != ' ') {
            return ProcessResult(ScanAction::REPLACE, "[REPLACED_WITH_LONG_STRING]");
        }
        return ProcessResult(ScanAction::COPY_TO_OUTPUT);
    };
    
    std::string result = scan_string(input, large_replacer);
    UTEST_ASSERT_STR_EQUALS("[REPLACED_WITH_LONG_STRING] [REPLACED_WITH_LONG_STRING] [REPLACED_WITH_LONG_STRING]", result.c_str());
}

// Test STL algorithm compatibility
UTEST_FUNC_DEF2(U8Scan, STLAlgorithmCompatibility) {
    std::string input = "Hello 123 World!";
    
    // Test with all_of predicate
    auto range = make_char_range(input);
    bool all_valid = std::all_of(range.begin(), range.end(), predicates::is_valid());
    UTEST_ASSERT_TRUE(all_valid);
    
    // Test with any_of predicate  
    bool has_digits = std::any_of(range.begin(), range.end(), predicates::is_digit_ascii());
    UTEST_ASSERT_TRUE(has_digits);
    
    // Test with none_of predicate
    bool no_invalid = std::none_of(range.begin(), range.end(), [](const CharInfo& info) {
        return !info.is_valid_utf8;
    });
    UTEST_ASSERT_TRUE(no_invalid);
    
    // Test with count_if
    std::size_t digit_count = std::count_if(range.begin(), range.end(), predicates::is_digit_ascii());
    UTEST_ASSERT_EQUALS(3u, digit_count);  // "123"
    
    // Test with find_if
    auto digit_iter = std::find_if(range.begin(), range.end(), predicates::is_digit_ascii());
    UTEST_ASSERT_TRUE(digit_iter != range.end());
    UTEST_ASSERT_EQUALS('1', static_cast<char>(digit_iter->codepoint));
}

// Test STL transform algorithm
UTEST_FUNC_DEF2(U8Scan, STLTransformAlgorithm) {
    std::string input = "hello";
    
    // Transform to collect all codepoints
    auto range = make_char_range(input);
    std::vector<uint32_t> codepoints;
    std::transform(range.begin(), range.end(), std::back_inserter(codepoints),
                   [](const CharInfo& info) { return info.codepoint; });
    
    UTEST_ASSERT_EQUALS(5u, codepoints.size());
    UTEST_ASSERT_EQUALS('h', static_cast<char>(codepoints[0]));
    UTEST_ASSERT_EQUALS('e', static_cast<char>(codepoints[1]));
    UTEST_ASSERT_EQUALS('l', static_cast<char>(codepoints[2]));
    UTEST_ASSERT_EQUALS('l', static_cast<char>(codepoints[3]));
    UTEST_ASSERT_EQUALS('o', static_cast<char>(codepoints[4]));
}

// Test STL for_each algorithm
UTEST_FUNC_DEF2(U8Scan, STLForEachAlgorithm) {
    std::string input = "Hello";
    
    std::string result;
    auto range = make_char_range(input);
    std::for_each(range.begin(), range.end(), [&result](const CharInfo& info) {
        if (info.is_ascii) {
            result += static_cast<char>(info.codepoint);
        }
    });
    
    UTEST_ASSERT_STR_EQUALS("Hello", result.c_str());
}

// Test predicate functions
UTEST_FUNC_DEF2(U8Scan, PredicateFunctions) {
    std::string input = "A1 世界";
    auto range = make_char_range(input);
    
    // Test is_ascii predicate
    auto ascii_chars = std::count_if(range.begin(), range.end(), predicates::is_ascii());
    UTEST_ASSERT_EQUALS(3u, ascii_chars);  // "A", "1", " "
    
    // Test is_digit_ascii predicate
    auto digits = std::count_if(range.begin(), range.end(), predicates::is_digit_ascii());
    UTEST_ASSERT_EQUALS(1u, digits);  // "1"
    
    // Test is_alpha_ascii predicate
    auto alphas = std::count_if(range.begin(), range.end(), predicates::is_alpha_ascii());
    UTEST_ASSERT_EQUALS(1u, alphas);  // "A"
    
    // Test is_alphanum_ascii predicate
    auto alphanums = std::count_if(range.begin(), range.end(), predicates::is_alphanum_ascii());
    UTEST_ASSERT_EQUALS(2u, alphanums);  // "A", "1"
    
    // Test is_whitespace_ascii predicate  
    auto whitespaces = std::count_if(range.begin(), range.end(), predicates::is_whitespace_ascii());
    UTEST_ASSERT_EQUALS(1u, whitespaces);  // " "
    
    // Test is_utf8 predicate (non-ASCII)
    auto utf8_chars = std::count_if(range.begin(), range.end(), predicates::is_utf8());
    UTEST_ASSERT_EQUALS(2u, utf8_chars);  // "世", "界"
}

// Test CharIterator functionality
UTEST_FUNC_DEF2(U8Scan, CharIteratorFunctionality) {
    std::string input = "Hello 世界!";
    auto range = make_char_range(input);
    
    // Test iterator increment
    auto it = range.begin();
    UTEST_ASSERT_EQUALS('H', static_cast<char>(it->codepoint));
    ++it;
    UTEST_ASSERT_EQUALS('e', static_cast<char>(it->codepoint));
    
    // Test iterator equality
    auto it2 = range.begin();
    UTEST_ASSERT_TRUE(it2 != it);
    ++it2;
    UTEST_ASSERT_TRUE(it2 == it);
    
    // Test range size
    std::size_t char_count = std::distance(range.begin(), range.end());
    UTEST_ASSERT_EQUALS(9u, char_count);  // H e l l o (space) 世 界 !
}

// Test UTF-8 string literals and various character encodings
UTEST_FUNC_DEF2(U8Scan, UTF8StringLiterals) {
    // Test various UTF-8 string literals
    
    // 1. Basic Latin and CJK characters
    std::string cjk_string = u8"Hello 世界! 🌍";
    auto cjk_range = make_char_range(cjk_string);
    
    // Count different character types
    auto ascii_count = std::count_if(cjk_range.begin(), cjk_range.end(), predicates::is_ascii());
    auto utf8_count = std::count_if(cjk_range.begin(), cjk_range.end(), predicates::is_utf8());
    auto emoji_count = std::count_if(cjk_range.begin(), cjk_range.end(), predicates::is_emoji());
    
    UTEST_ASSERT_EQUALS(8u, ascii_count);  // "Hello !"
    UTEST_ASSERT_EQUALS(3u, utf8_count);   // "世", "界", "🌍"
    UTEST_ASSERT_EQUALS(1u, emoji_count);  // "🌍"
    
    // 2. Accented characters (Latin Extended)
    std::string accented = u8"Café résumé naïve";
    auto accented_range = make_char_range(accented);
    auto accented_size = std::distance(accented_range.begin(), accented_range.end());
    UTEST_ASSERT_EQUALS(17u, accented_size);  // 17 Unicode characters
    
    // 3. Cyrillic script
    std::string cyrillic = u8"Привет мир";  // "Hello world" in Russian
    auto cyrillic_range = make_char_range(cyrillic);
    auto cyrillic_ascii = std::count_if(cyrillic_range.begin(), cyrillic_range.end(), predicates::is_ascii());
    auto cyrillic_utf8 = std::count_if(cyrillic_range.begin(), cyrillic_range.end(), predicates::is_utf8());
    
    UTEST_ASSERT_EQUALS(1u, cyrillic_ascii);  // Only the space character
    UTEST_ASSERT_EQUALS(9u, cyrillic_utf8);  // All Cyrillic characters
    
    // 4. Mixed scripts with numbers and punctuation
    std::string mixed = u8"Price: €25.99 (税込み)";  // Euro symbol + Japanese
    auto mixed_range = make_char_range(mixed);
    auto digit_count = std::count_if(mixed_range.begin(), mixed_range.end(), predicates::is_digit_ascii());
    auto alphanum_count = std::count_if(mixed_range.begin(), mixed_range.end(), predicates::is_alphanum_ascii());
    
    UTEST_ASSERT_EQUALS(4u, digit_count);     // "2", "5", "9", "9"
    UTEST_ASSERT_EQUALS(9u, alphanum_count);  // Same digits (no ASCII letters in this string)
    
    // 5. Test scanning with UTF-8 literals
    std::string scanned_result = scan_utf8(mixed, 
        [](const CharInfo& info, const char* /*data*/) {
            // Remove ASCII digits, keep everything else
            if (predicates::is_digit_ascii()(info)) {
                return ProcessResult(ScanAction::IGNORE);
            }
            return ProcessResult(ScanAction::COPY_TO_OUTPUT);
        });
    
    // Should remove ASCII digits but keep everything else
    UTEST_ASSERT_STR_EQUALS(u8"Price: €. (税込み)", scanned_result.c_str());
    
    // 6. Test character info for specific UTF-8 characters
    auto euro_info = get_char_info(u8"€", 0);  // Euro symbol
    UTEST_ASSERT_FALSE(euro_info.is_ascii);
    UTEST_ASSERT_TRUE(euro_info.is_valid_utf8);
    UTEST_ASSERT_EQUALS(3u, euro_info.byte_count);  // Euro is 3 bytes in UTF-8
    UTEST_ASSERT_EQUALS(0x20AC, euro_info.codepoint);  // Unicode codepoint for Euro
    
    auto chinese_info = get_char_info(u8"世", 0);  // Chinese character
    UTEST_ASSERT_FALSE(chinese_info.is_ascii);
    UTEST_ASSERT_TRUE(chinese_info.is_valid_utf8);
    UTEST_ASSERT_EQUALS(3u, chinese_info.byte_count);
    UTEST_ASSERT_EQUALS(0x4E16, chinese_info.codepoint);  // Unicode for "世"
}

// Note: String conversion functions (to_string, to_lower_ascii_str, to_upper_ascii_str) 
// are tested and verified to work correctly in the demo applications

// Run all tests
int main() {
    UTEST_PROLOG();
    UTEST_ENABLE_VERBOSE_MODE();
    
    // UTF-8 Scanner tests
    UTEST_FUNC2(U8Scan, BasicASCIIScanning);
    UTEST_FUNC2(U8Scan, UTF8MultiByteScanning);
    UTEST_FUNC2(U8Scan, BOMDetection);
    UTEST_FUNC2(U8Scan, CharacterIgnoring);
    UTEST_FUNC2(U8Scan, CharacterReplacement);
    UTEST_FUNC2(U8Scan, EarlyTermination);
    UTEST_FUNC2(U8Scan, ASCIIOnlyMode);
    UTEST_FUNC2(U8Scan, OutputSizeLimiting);
    UTEST_FUNC2(U8Scan, InvalidUTF8Handling);
    UTEST_FUNC2(U8Scan, EmojiHandling);
    UTEST_FUNC2(U8Scan, GetCharInfo);
    UTEST_FUNC2(U8Scan, EmptyString);
    UTEST_FUNC2(U8Scan, BOMOnly);
    UTEST_FUNC2(U8Scan, DisableBOMDetection);
    UTEST_FUNC2(U8Scan, LargeReplacements);
    UTEST_FUNC2(U8Scan, UTF8StringLiterals);
    
    // STL algorithm compatibility tests
    UTEST_FUNC2(U8Scan, STLAlgorithmCompatibility);
    UTEST_FUNC2(U8Scan, STLTransformAlgorithm);
    UTEST_FUNC2(U8Scan, STLForEachAlgorithm);
    UTEST_FUNC2(U8Scan, PredicateFunctions);
    UTEST_FUNC2(U8Scan, CharIteratorFunctionality);
    UTEST_FUNC2(U8Scan, STLAlgorithmCompatibility);
    UTEST_FUNC2(U8Scan, STLTransformAlgorithm);
    UTEST_FUNC2(U8Scan, STLForEachAlgorithm);
    UTEST_FUNC2(U8Scan, PredicateFunctions);
    UTEST_FUNC2(U8Scan, CharIteratorFunctionality);
    
    UTEST_EPILOG();
}
