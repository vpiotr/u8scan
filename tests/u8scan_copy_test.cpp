#include "../include/utest/utest.h"
#include "../include/u8scan/u8scan.h"
#include <string>
#include <vector>
#include <algorithm>

using namespace u8scan;

UTEST_FUNC_DEF2(CopyFunctions, BasicCopy) {
    // Test with mixed ASCII and UTF-8 content
    std::string input = "Hello 世界! Emoji: 🌍🚀 Numbers: 123";
    std::string result;
    
    // u8scan creates char range for UTF-8 string, copies all characters, outputs UTF-8 string
    u8scan::copy(input, std::back_inserter(result));
    
    // Verify all UTF-8 characters were properly processed and copied
    UTEST_ASSERT_STR_EQUALS(result.c_str(), input.c_str());
    UTEST_ASSERT_GT(result.length(), 20u); // Should be longer due to multi-byte UTF-8 chars
    
    // Test with pure UTF-8 content
    std::string utf8_input = "こんにちは世界 🎌 中文测试";
    std::string utf8_result;
    u8scan::copy(utf8_input, std::back_inserter(utf8_result));
    UTEST_ASSERT_STR_EQUALS(utf8_result.c_str(), utf8_input.c_str());
}

UTEST_FUNC_DEF2(CopyFunctions, CopyIf) {
    // Test with comprehensive UTF-8 content
    std::string input = "Hello123世界🌍Test456你好🚀End!";
    std::string letters, digits, utf8_chars, ascii_chars, emojis;
    
    // u8scan creates char range, applies predicates, outputs UTF-8 strings
    u8scan::copy_if(input, std::back_inserter(letters), predicates::is_alpha_ascii());
    u8scan::copy_if(input, std::back_inserter(digits), predicates::is_digit_ascii());
    u8scan::copy_if(input, std::back_inserter(utf8_chars), predicates::is_utf8());
    u8scan::copy_if(input, std::back_inserter(ascii_chars), predicates::is_ascii());
    u8scan::copy_if(input, std::back_inserter(emojis), predicates::is_emoji());
    
    // Verify predicate-based filtering worked correctly
    UTEST_ASSERT_STR_EQUALS(letters.c_str(), "HelloTestEnd");
    UTEST_ASSERT_STR_EQUALS(digits.c_str(), "123456");
    UTEST_ASSERT_STR_EQUALS(utf8_chars.c_str(), "世界🌍你好🚀"); // Non-ASCII UTF-8 characters and emojis
    UTEST_ASSERT_STR_EQUALS(ascii_chars.c_str(), "Hello123Test456End!");
    UTEST_ASSERT_STR_EQUALS(emojis.c_str(), "🌍🚀");
    
    // Test with edge case: empty result
    std::string no_numbers = "HelloWorld世界";
    std::string should_be_empty;
    u8scan::copy_if(no_numbers, std::back_inserter(should_be_empty), predicates::is_digit_ascii());
    UTEST_ASSERT_TRUE(should_be_empty.empty());
}

UTEST_FUNC_DEF2(CopyFunctions, CopyUntil) {
    // Test stopping at different character types in UTF-8 content
    std::string input = "Hello世界123🌍World";
    std::string until_digit, until_emoji, until_utf8;
    
    // u8scan processes UTF-8 char range, stops at predicate match, outputs UTF-8 string
    u8scan::copy_until(input, std::back_inserter(until_digit), predicates::is_digit_ascii());
    u8scan::copy_until(input, std::back_inserter(until_emoji), predicates::is_emoji());
    u8scan::copy_until(input, std::back_inserter(until_utf8), predicates::is_utf8());
    
    UTEST_ASSERT_STR_EQUALS(until_digit.c_str(), "Hello世界");      // Stops before "123"
    UTEST_ASSERT_STR_EQUALS(until_emoji.c_str(), "Hello世界123");   // Stops before "🌍"
    UTEST_ASSERT_STR_EQUALS(until_utf8.c_str(), "Hello");          // Stops before "世界"
    
    // Test with content that never matches predicate
    std::string all_letters = "HelloWorld";
    std::string copy_all;
    u8scan::copy_until(all_letters, std::back_inserter(copy_all), predicates::is_digit_ascii());
    UTEST_ASSERT_STR_EQUALS(copy_all.c_str(), "HelloWorld"); // Should copy everything
    
    // Test with immediate match
    std::string starts_with_digit = "123Hello";
    std::string copy_none;
    u8scan::copy_until(starts_with_digit, std::back_inserter(copy_none), predicates::is_digit_ascii());
    UTEST_ASSERT_TRUE(copy_none.empty()); // Should copy nothing
}

UTEST_FUNC_DEF2(CopyFunctions, CopyFrom) {
    // Test starting from different character types in UTF-8 content
    std::string input = "Hello世界123🌍World";
    std::string from_digit, from_emoji, from_utf8;
    
    // u8scan processes UTF-8 char range, starts at predicate match, outputs UTF-8 string
    u8scan::copy_from(input, std::back_inserter(from_digit), predicates::is_digit_ascii());
    u8scan::copy_from(input, std::back_inserter(from_emoji), predicates::is_emoji());
    u8scan::copy_from(input, std::back_inserter(from_utf8), predicates::is_utf8());
    
    UTEST_ASSERT_STR_EQUALS(from_digit.c_str(), "123🌍World");     // Starts from "123"
    UTEST_ASSERT_STR_EQUALS(from_emoji.c_str(), "🌍World");        // Starts from "🌍"
    UTEST_ASSERT_STR_EQUALS(from_utf8.c_str(), "世界123🌍World");  // Starts from "世界"
    
    // Test with content that never matches predicate
    std::string all_letters = "HelloWorld";
    std::string copy_none;
    u8scan::copy_from(all_letters, std::back_inserter(copy_none), predicates::is_digit_ascii());
    UTEST_ASSERT_TRUE(copy_none.empty()); // Should copy nothing
    
    // Test with match at beginning
    std::string starts_with_digit = "123Hello世界";
    std::string copy_all;
    u8scan::copy_from(starts_with_digit, std::back_inserter(copy_all), predicates::is_digit_ascii());
    UTEST_ASSERT_STR_EQUALS(copy_all.c_str(), "123Hello世界"); // Should copy everything
}

UTEST_FUNC_DEF2(CopyFunctions, CopyN) {
    // Test with UTF-8 content where character count != byte count
    std::string input = "Hello世界🌍Test"; // 10 characters, but more bytes
    std::string first_five, first_seven, all_chars, zero_chars;
    
    // u8scan processes UTF-8 char range, counts Unicode characters, outputs UTF-8 string
    u8scan::copy_n(input, std::back_inserter(first_five), 5);
    u8scan::copy_n(input, std::back_inserter(first_seven), 7);
    u8scan::copy_n(input, std::back_inserter(all_chars), 100); // More than available
    u8scan::copy_n(input, std::back_inserter(zero_chars), 0);
    
    UTEST_ASSERT_STR_EQUALS(first_five.c_str(), "Hello");        // First 5 characters
    UTEST_ASSERT_STR_EQUALS(first_seven.c_str(), "Hello世界");    // First 7 characters (includes UTF-8)
    UTEST_ASSERT_STR_EQUALS(all_chars.c_str(), input.c_str());   // All characters when requesting more than available
    UTEST_ASSERT_TRUE(zero_chars.empty());                      // Zero characters
    
    // Test with pure UTF-8 content
    std::string utf8_input = "你好世界测试";
    std::string first_three_utf8;
    u8scan::copy_n(utf8_input, std::back_inserter(first_three_utf8), 3);
    UTEST_ASSERT_STR_EQUALS(first_three_utf8.c_str(), "你好世");    // First 3 Unicode characters
}

UTEST_FUNC_DEF2(CopyFunctions, CopyWhile) {
    // Test continuing while different character types match in UTF-8 content
    std::string input = "123Hello世界🌍456";
    std::string digits_start, ascii_middle, utf8_content;
    
    // u8scan processes UTF-8 char range, continues while predicate matches, outputs UTF-8 string
    u8scan::copy_while(input, std::back_inserter(digits_start), predicates::is_digit_ascii());
    
    std::string middle_part = "Hello世界";
    u8scan::copy_while(middle_part, std::back_inserter(ascii_middle), predicates::is_alpha_ascii());
    
    std::string utf8_part = "世界你好";
    u8scan::copy_while(utf8_part, std::back_inserter(utf8_content), predicates::is_utf8());
    
    UTEST_ASSERT_STR_EQUALS(digits_start.c_str(), "123");        // Continues while digits
    UTEST_ASSERT_STR_EQUALS(ascii_middle.c_str(), "Hello");      // Continues while ASCII letters
    UTEST_ASSERT_STR_EQUALS(utf8_content.c_str(), "世界你好");    // Continues while UTF-8
    
    // Test with content that matches throughout
    std::string all_letters = "HelloWorld";
    std::string all_copied;
    u8scan::copy_while(all_letters, std::back_inserter(all_copied), predicates::is_alpha_ascii());
    UTEST_ASSERT_STR_EQUALS(all_copied.c_str(), "HelloWorld");
    
    // Test with immediate non-match
    std::string starts_with_digit = "123Hello";
    std::string copy_none;
    u8scan::copy_while(starts_with_digit, std::back_inserter(copy_none), predicates::is_alpha_ascii());
    UTEST_ASSERT_TRUE(copy_none.empty());
}

UTEST_FUNC_DEF2(CopyFunctions, EdgeCases) {
    // Test with empty string - u8scan should handle gracefully
    std::string empty = "";
    std::string empty_result;
    
    u8scan::copy(empty, std::back_inserter(empty_result));
    UTEST_ASSERT_TRUE(empty_result.empty());
    
    empty_result.clear();
    u8scan::copy_if(empty, std::back_inserter(empty_result), predicates::is_ascii());
    UTEST_ASSERT_TRUE(empty_result.empty());
    
    empty_result.clear();
    u8scan::copy_until(empty, std::back_inserter(empty_result), predicates::is_digit_ascii());
    UTEST_ASSERT_TRUE(empty_result.empty());
    
    empty_result.clear();
    u8scan::copy_from(empty, std::back_inserter(empty_result), predicates::is_alpha_ascii());
    UTEST_ASSERT_TRUE(empty_result.empty());
    
    empty_result.clear();
    u8scan::copy_n(empty, std::back_inserter(empty_result), 5);
    UTEST_ASSERT_TRUE(empty_result.empty());

    empty_result.clear();
    u8scan::copy_while(empty, std::back_inserter(empty_result), predicates::is_alpha_ascii());
    UTEST_ASSERT_TRUE(empty_result.empty());

    // Test copy_until when predicate never matches - should process entire UTF-8 string
    std::string mixed_content = "Hello世界🌍Test";
    std::string all_chars;
    u8scan::copy_until(mixed_content, std::back_inserter(all_chars), predicates::is_digit_ascii());
    UTEST_ASSERT_STR_EQUALS(all_chars.c_str(), "Hello世界🌍Test"); // Should copy all since no digit found
    
    // Test copy_from when predicate never matches - should output empty UTF-8 string
    std::string no_match_result;
    u8scan::copy_from(mixed_content, std::back_inserter(no_match_result), predicates::is_digit_ascii());
    UTEST_ASSERT_TRUE(no_match_result.empty()); // Should copy nothing since no digit found
    
    // Test with only UTF-8 characters (no ASCII)
    std::string pure_utf8 = "世界你好测试";
    std::string ascii_from_utf8;
    u8scan::copy_if(pure_utf8, std::back_inserter(ascii_from_utf8), predicates::is_ascii());
    UTEST_ASSERT_TRUE(ascii_from_utf8.empty()); // Should be empty since no ASCII chars
    
    std::string all_utf8;
    u8scan::copy_if(pure_utf8, std::back_inserter(all_utf8), predicates::is_utf8());
    UTEST_ASSERT_STR_EQUALS(all_utf8.c_str(), pure_utf8.c_str()); // Should copy all UTF-8 chars
}

UTEST_FUNC_DEF2(CopyFunctions, STLIntegration) {
    // Demonstrate u8scan char range -> STL algorithm -> u8scan predicate workflow
    std::string input = "Hello123世界🌍Test456你好🚀End!";
    
    // Create a char range using u8scan
    auto char_range = u8scan::make_char_range(input);
    
    // Example 1: Count different character types using STL with u8scan predicates
    auto ascii_count = std::count_if(char_range.begin(), char_range.end(),
                                    predicates::is_ascii());
    auto utf8_count = std::count_if(char_range.begin(), char_range.end(),
                                   predicates::is_utf8());
    auto emoji_count = std::count_if(char_range.begin(), char_range.end(),
                                    predicates::is_emoji());
    auto digit_count = std::count_if(char_range.begin(), char_range.end(),
                                    predicates::is_digit_ascii());
    
    UTEST_ASSERT_EQUALS(ascii_count, 19); // "Hello123Test456End!"
    UTEST_ASSERT_EQUALS(utf8_count, 6);   // "世界🌍你好🚀"
    UTEST_ASSERT_EQUALS(emoji_count, 2);  // "🌍🚀"
    UTEST_ASSERT_EQUALS(digit_count, 6);  // "123456"
    
    // Example 2: Find operations using STL with u8scan predicates
    auto first_digit = std::find_if(char_range.begin(), char_range.end(),
                                   predicates::is_digit_ascii());
    UTEST_ASSERT_TRUE(first_digit != char_range.end());
    UTEST_ASSERT_EQUALS(first_digit->codepoint, static_cast<uint32_t>('1'));
    
    auto first_emoji = std::find_if(char_range.begin(), char_range.end(),
                                   predicates::is_emoji());
    UTEST_ASSERT_TRUE(first_emoji != char_range.end());
    UTEST_ASSERT_EQUALS(first_emoji->codepoint, static_cast<uint32_t>(0x1F30D)); // 🌍
    
    // Example 3: Boolean predicates using STL with u8scan predicates
    auto has_emoji = std::any_of(char_range.begin(), char_range.end(),
                                predicates::is_emoji());
    UTEST_ASSERT_TRUE(has_emoji);
    
    auto all_ascii = std::all_of(char_range.begin(), char_range.end(),
                                predicates::is_ascii());
    UTEST_ASSERT_FALSE(all_ascii); // Contains UTF-8 characters
    
    // Example 4: Using u8scan copy functions to create filtered UTF-8 strings
    std::string utf8_only, ascii_only, digits_only, no_emojis;
    
    // These use u8scan's UTF-8 processing: char range -> predicate -> UTF-8 string
    u8scan::copy_if(input, std::back_inserter(utf8_only), predicates::is_utf8());
    u8scan::copy_if(input, std::back_inserter(ascii_only), predicates::is_ascii());
    u8scan::copy_if(input, std::back_inserter(digits_only), predicates::is_digit_ascii());
    
    // Create compound predicate: not emoji
    auto not_emoji = [](const CharInfo& ch) { 
        return !predicates::is_emoji()(ch); 
    };
    u8scan::copy_if(input, std::back_inserter(no_emojis), not_emoji);
    
    UTEST_ASSERT_STR_EQUALS(utf8_only.c_str(), "世界🌍你好🚀");
    UTEST_ASSERT_STR_EQUALS(ascii_only.c_str(), "Hello123Test456End!");
    UTEST_ASSERT_STR_EQUALS(digits_only.c_str(), "123456");
    UTEST_ASSERT_STR_EQUALS(no_emojis.c_str(), "Hello123世界Test456你好End!");
}

int main() {
    UTEST_PROLOG();
    UTEST_ENABLE_VERBOSE_MODE();
    
    // Copy function tests using u8scan framework
    UTEST_FUNC2(CopyFunctions, BasicCopy);
    UTEST_FUNC2(CopyFunctions, CopyIf);
    UTEST_FUNC2(CopyFunctions, CopyUntil);
    UTEST_FUNC2(CopyFunctions, CopyFrom);
    UTEST_FUNC2(CopyFunctions, CopyN);
    UTEST_FUNC2(CopyFunctions, CopyWhile);
    UTEST_FUNC2(CopyFunctions, EdgeCases);
    UTEST_FUNC2(CopyFunctions, STLIntegration);
    
    UTEST_EPILOG();
}
