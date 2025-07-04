#include "../include/utest/utest.h"
#include "../include/u8scan/u8scan.h"
#include <vector>
#include <numeric>
#include <algorithm>

using namespace u8scan;

// Test STL algorithm compatibility
UTEST_FUNC_DEF2(U8ScanSTL, STLAlgorithmCompatibility) {
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
    std::size_t digit_count = static_cast<std::size_t>(std::count_if(range.begin(), range.end(), predicates::is_digit_ascii()));
    UTEST_ASSERT_EQUALS(3u, digit_count);  // "123"
    
    // Test with find_if
    auto digit_iter = std::find_if(range.begin(), range.end(), predicates::is_digit_ascii());
    UTEST_ASSERT_TRUE(digit_iter != range.end());
    UTEST_ASSERT_EQUALS('1', static_cast<char>(digit_iter->codepoint));
}

// Test STL transform algorithm
UTEST_FUNC_DEF2(U8ScanSTL, STLTransformAlgorithm) {
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
UTEST_FUNC_DEF2(U8ScanSTL, STLForEachAlgorithm) {
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
UTEST_FUNC_DEF2(U8ScanSTL, PredicateFunctions) {
    std::string input = u8"A1 世界";
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
UTEST_FUNC_DEF2(U8ScanSTL, CharIteratorFunctionality) {
    std::string input = u8"Hello 世界!";
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
    std::size_t char_count = static_cast<std::size_t>(std::distance(range.begin(), range.end()));
    UTEST_ASSERT_EQUALS(9u, char_count);  // H e l l o (space) 世 界 !
}

// Run all tests
int main() {
    UTEST_PROLOG();
    UTEST_ENABLE_VERBOSE_MODE();
    
    // STL algorithm compatibility tests
    UTEST_FUNC2(U8ScanSTL, STLAlgorithmCompatibility);
    UTEST_FUNC2(U8ScanSTL, STLTransformAlgorithm);
    UTEST_FUNC2(U8ScanSTL, STLForEachAlgorithm);
    UTEST_FUNC2(U8ScanSTL, PredicateFunctions);
    UTEST_FUNC2(U8ScanSTL, CharIteratorFunctionality);
    
    UTEST_EPILOG();
}
