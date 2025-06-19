# U8SCAN - UTF-8 String Scanning Library

[![CI](https://github.com/vpiotr/u8scan/actions/workflows/ci.yml/badge.svg)](https://github.com/vpiotr/u8scan/actions/workflows/ci.yml)
[![C++](https://img.shields.io/badge/C%2B%2B-11%2F14%2F17%2F20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Header Only](https://img.shields.io/badge/header--only-yes-brightgreen.svg)]()

A lightweight, header-only C++ library for UTF-8 string processing with STL-compatible iterators and character scanning utilities.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [Usage Examples](#usage-examples)
- [API Reference](#api-reference)
- [Building and Testing](#building-and-testing)
- [Performance Considerations](#performance-considerations)
- [Contributing](#contributing)
- [License](#license)

## Overview

U8SCAN provides STL-friendly iterators, ranges, and utilities for processing UTF-8 and ASCII strings at the character level. The library enables efficient character-by-character processing of Unicode strings while remaining compatible with standard C++ algorithms.

## Features

- **STL-compatible iterators**: `CharIterator` and `CharRange` for seamless integration with standard algorithms
- **UTF-8 and ASCII scanning**: Efficient character-by-character processing with BOM detection
- **Character property predicates**: `is_ascii()`, `is_digit_ascii()`, `is_alpha_ascii()`, `is_alphanum_ascii()`, `is_lowercase_ascii()`, `is_uppercase_ascii()`, `is_whitespace_ascii()`, `is_emoji()`
- **Character conversion**: `to_lower_ascii()` and `to_upper_ascii()` for ASCII character case conversion
- **STL-like copy functions**: `copy()`, `copy_if()`, `copy_until()`, `copy_from()`, `copy_n()`, `copy_while()` for UTF-8 string filtering and processing
- **String length calculation**: `length()` for counting Unicode code points (characters), not bytes
- **High-performance scanning**: Custom character processing via `scan_utf8()` and `scan_ascii()`
- **String utilities**: `quoted_str()` for safe quoting and escaping, `transform_chars()` for string transformation

## Key Features at a Glance

```cpp
#include "u8scan/u8scan.h"
#include <algorithm>
#include <iostream>

int main() {
    // Fun example: Chinese restaurant menu items with UTF-8 string literals
    std::string menu_item = u8"🍜 麻婆豆腐 (Mapo Tofu) - ¥45.99 🌶️🌶️🌶️";
    auto char_range = u8scan::make_char_range(menu_item);
    
    // Count different character types using STL algorithms
    size_t ascii_count = std::count_if(char_range.begin(), char_range.end(), 
                                      u8scan::predicates::is_ascii());
    size_t utf8_count = std::count_if(char_range.begin(), char_range.end(), 
                                     u8scan::predicates::is_utf8());
    size_t emoji_count = std::count_if(char_range.begin(), char_range.end(), 
                                      u8scan::predicates::is_emoji());
    
    std::cout << "Menu: " << menu_item << std::endl;
    std::cout << "Characters: " << u8scan::length(menu_item) << std::endl; // Unicode code points (characters)
    std::cout << "Bytes: " << menu_item.length() << std::endl;            // Raw byte count
    std::cout << "ASCII characters: " << ascii_count << std::endl;     // Letters, numbers, punctuation
    std::cout << "UTF-8 characters: " << utf8_count << std::endl;      // Chinese characters + currency symbol
    std::cout << "Emoji characters: " << emoji_count << std::endl;     // Food and spice emojis
    
    // Extract just the price using copy functions
    std::string price_only;
    u8scan::copy_if(menu_item, std::back_inserter(price_only), 
        [](const u8scan::CharInfo& info) {
            return u8scan::predicates::is_digit_ascii()(info) || 
                   info.codepoint == '.' || info.codepoint == 0x00A5; // Include ¥ symbol
        });
    std::cout << "Price extracted: " << price_only << std::endl;       // "¥45.99"
    
    // Transform English text to uppercase (preserves Chinese characters and emojis)
    std::string upper_menu;
    u8scan::transform_chars(menu_item, std::back_inserter(upper_menu), 
        [](const u8scan::CharInfo& info) {
            return static_cast<char>(u8scan::to_upper_ascii(info));
        });
    std::cout << "Uppercase: " << upper_menu << std::endl;
    
    return 0;
}
```

## Why U8SCAN?

- 🚀 **Header-only** - Just include `u8scan.h` and start using
- 📦 **Zero dependencies** - Works with standard C++ library only
- 🔧 **Highly customizable** - Custom character processing functions
- 🎯 **STL-compatible** - Works seamlessly with standard algorithms
- 🛡️ **UTF-8 safe** - Properly handles multi-byte characters and validation
- ✅ **Well tested** - Comprehensive test suite included
- ⚡ **Performance-focused** - Separate optimized paths for ASCII and UTF-8

## Quick Start

```cpp
#include "u8scan/u8scan.h"
#include <iostream>
#include <algorithm>

int main() {
    std::string input = u8"Hello 世界! 123";
    
    // Create a character range for STL algorithm compatibility
    auto range = u8scan::make_char_range(input);
    
    // Count digits using STL algorithms
    int digit_count = std::count_if(range.begin(), range.end(), 
                                   u8scan::predicates::is_digit_ascii());
    std::cout << "Digit count: " << digit_count << std::endl;  // Output: 3
    
    // Remove digits using custom scanning
    std::string result = u8scan::scan_utf8(input, 
        [](const u8scan::CharInfo& info, const char* data) {
            if (u8scan::predicates::is_digit_ascii()(info)) {
                return u8scan::ProcessResult(u8scan::ScanAction::IGNORE);
            }
            return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
        });
    
    std::cout << "Without digits: " << result << std::endl;  // u8"Hello 世界! "
    
    return 0;
}
```

## Installation

U8SCAN is a header-only library. Simply:

1. Copy `include/u8scan/u8scan.h` to your project
2. Include the header: `#include "u8scan/u8scan.h"`
3. Start using: `u8scan::make_char_range(str)`, `u8scan::scan_utf8()`, etc.

### Requirements

- C++11 or later
- Standard library support for `<string>`, `<algorithm>`, `<functional>`, `<iterator>`

### Using Git

```bash
# Clone the repository
git clone https://github.com/vpiotr/u8scan.git

# Include in your project
#include "u8scan/u8scan.h"
```

## Usage Examples

### STL Algorithm Integration

```cpp
#include "u8scan/u8scan.h"
#include <algorithm>
#include <iostream>

void stl_examples() {
    std::string input = u8"Hello 世界! Test 123.";
    auto char_range = u8scan::make_char_range(input);
    
    // Count different character types
    auto ascii_count = std::count_if(char_range.begin(), char_range.end(), 
                                    u8scan::predicates::is_ascii());
    auto digit_count = std::count_if(char_range.begin(), char_range.end(), 
                                    u8scan::predicates::is_digit_ascii());
    auto lower_count = std::count_if(char_range.begin(), char_range.end(), 
                                    u8scan::predicates::is_lowercase_ascii());
    
    std::cout << "ASCII: " << ascii_count << ", Digits: " << digit_count 
              << ", Lowercase: " << lower_count << std::endl;
    
    // Find first UTF-8 character
    auto first_utf8 = std::find_if(char_range.begin(), char_range.end(), 
                                  u8scan::predicates::is_utf8());
    if (first_utf8 != char_range.end()) {
        std::cout << "First UTF-8 char at position: " << first_utf8->start_pos << std::endl;
    }
    
    // Validate all characters
    bool all_valid = std::all_of(char_range.begin(), char_range.end(), 
                                u8scan::predicates::is_valid());
    std::cout << "All characters valid: " << (all_valid ? "YES" : "NO") << std::endl;
}
```

### Character Processing and Scanning

```cpp
#include "u8scan/u8scan.h"

void scanning_examples() {
    std::string input = u8"Hello 世界! 123";
    
    // Convert to uppercase (ASCII only)
    std::string uppercase = u8scan::scan_utf8(input, 
        [](const u8scan::CharInfo& info, const char* data) {
            if (info.is_ascii && info.codepoint >= 'a' && info.codepoint <= 'z') {
                char upper = static_cast<char>(u8scan::to_upper_ascii(info));
                return u8scan::ProcessResult(u8scan::ScanAction::REPLACE, std::string(1, upper));
            }
            return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
        });
    
    // Filter out non-alphabetic characters
    std::string letters_only = u8scan::scan_utf8(input,
        [](const u8scan::CharInfo& info, const char* data) {
            if (info.is_ascii && !u8scan::predicates::is_alpha_ascii()(info) && 
                !u8scan::predicates::is_whitespace_ascii()(info)) {
                return u8scan::ProcessResult(u8scan::ScanAction::IGNORE);
            }
            return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
        });
    
    std::cout << "Uppercase: " << uppercase << std::endl;
    std::cout << "Letters only: " << letters_only << std::endl;
}
```

### String Transformation

```cpp
#include "u8scan/u8scan.h"
#include <vector>

void transformation_examples() {
    std::string input = "Hello World! 123";
    
    // Transform using STL-compatible approach
    std::vector<char> result_chars;
    u8scan::transform_chars(input, std::back_inserter(result_chars),
        [](const u8scan::CharInfo& info) -> char {
            // Convert to lowercase
            return static_cast<char>(u8scan::to_lower_ascii(info));
        });
    
    std::string lowercase(result_chars.begin(), result_chars.end());
    std::cout << "Lowercase: " << lowercase << std::endl;
    
    // String quoting with custom delimiters
    std::string original = u8"Text with \"quotes\" and 世界";
    std::string quoted = u8scan::quoted_str(original, '[', ']', '\\');
    std::cout << "Custom quoted: " << quoted << std::endl;
}
```

### Emoji Detection and Processing

```cpp
#include "u8scan/u8scan.h"
#include <algorithm>

void emoji_examples() {
    std::string input = "Hello 🌍 World! 🚀 Unicode text 😊 with emoji.";
    auto char_range = u8scan::make_char_range(input);
    
    // Count emoji characters
    size_t emoji_count = std::count_if(char_range.begin(), char_range.end(), 
                                      u8scan::predicates::is_emoji());
    std::cout << "Found " << emoji_count << " emoji characters" << std::endl;
    
    // Find and list all emoji positions
    std::cout << "Emoji positions: ";
    for (auto it = char_range.begin(); it != char_range.end(); ++it) {
        if (u8scan::predicates::is_emoji()(*it)) {
            std::cout << it->start_pos << " ";
        }
    }
    std::cout << std::endl;
    
    // Remove all emoji from string
    std::string no_emoji = u8scan::scan_utf8(input,
        [](const u8scan::CharInfo& info, const char* data) {
            if (u8scan::predicates::is_emoji()(info)) {
                return u8scan::ProcessResult(u8scan::ScanAction::IGNORE);
            }
            return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
        });
    std::cout << "Without emoji: " << no_emoji << std::endl;
    
    // Replace emoji with placeholder
    std::string emoji_replaced = u8scan::scan_utf8(input,
        [](const u8scan::CharInfo& info, const char* data) {
            if (u8scan::predicates::is_emoji()(info)) {
                return u8scan::ProcessResult(u8scan::ScanAction::REPLACE, "[emoji]");
            }
            return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
        });
    std::cout << "Emoji replaced: " << emoji_replaced << std::endl;
}
```

### Character Information and Properties

```cpp
#include "u8scan/u8scan.h"

void character_info_examples() {
    std::string input = u8"A世界!";
    
    // Get character information at specific positions
    auto char_info = u8scan::get_char_info(input, 0);  // 'A'
    std::cout << "Character 'A': codepoint=" << char_info.codepoint 
              << ", bytes=" << char_info.byte_count 
              << ", is_ascii=" << char_info.is_ascii << std::endl;
    
    char_info = u8scan::get_char_info(input, 1);  // '世'
    std::cout << "Character '世': codepoint=" << char_info.codepoint 
              << ", bytes=" << char_info.byte_count 
              << ", is_ascii=" << char_info.is_ascii << std::endl;
    
    // Use character predicates
    auto range = u8scan::make_char_range(input);
    for (const auto& info : range) {
        if (u8scan::predicates::is_ascii()(info)) {
            std::cout << "ASCII character: " << static_cast<char>(info.codepoint) << std::endl;
        } else {
            std::cout << "UTF-8 character at position: " << info.start_pos << std::endl;
        }
    }
}
```

### BOM (Byte Order Mark) Handling

```cpp
#include "u8scan/u8scan.h"

void bom_handling_examples() {
    // String with UTF-8 BOM
    std::string input_with_bom = "\xEF\xBB\xBFHello World";
    
    // scan_utf8 automatically skips BOM by default
    std::string processed = u8scan::scan_utf8(input_with_bom,
        [](const u8scan::CharInfo& info, const char* data) {
            return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
        });
    
    std::cout << "Processed (BOM skipped): " << processed << std::endl; // "Hello World"
    
    // For advanced BOM handling, use ScanConfig
    u8scan::ScanConfig config;
    config.bom_action = u8scan::BOMAction::COPY;  // Keep BOM in output
    
    std::string with_bom = u8scan::scan_string(input_with_bom,
        [](const u8scan::CharInfo& info, const char* data) {
            return u8scan::ProcessResult(u8scan::ScanAction::COPY_TO_OUTPUT);
        }, config);
}
```

## API Reference

### Core Types

#### `CharInfo`

Struct containing information about a UTF-8 character:

```cpp
struct CharInfo {
    std::size_t start_pos;      // Starting position in the string
    std::size_t byte_count;     // Number of bytes (1-4 for UTF-8)
    uint32_t codepoint;         // Unicode codepoint
    bool is_ascii;              // True if ASCII character (< 0x80)
    bool is_valid_utf8;         // True if valid UTF-8 sequence
    bool is_bom;                // True if BOM character
};
```

#### `ProcessResult`

Result returned by character processing functions:

```cpp
struct ProcessResult {
    ScanAction action;          // What to do with the character
    std::string replacement;    // Used when action is REPLACE
    
    ProcessResult(ScanAction a = ScanAction::COPY_TO_OUTPUT);
    ProcessResult(ScanAction a, const std::string& repl);
};
```

#### `ScanAction`

Actions that can be taken during character processing:

- `COPY_TO_OUTPUT` - Copy character to output as-is
- `REPLACE` - Replace character with provided replacement
- `IGNORE` - Skip this character  
- `STOP_SCANNING` - Stop the scanning process

### Core Functions

#### `scan_utf8(input, processor)`

Main UTF-8 scanning function with automatic BOM handling:

```cpp
template<typename Processor>
std::string scan_utf8(const std::string& input, Processor processor);
```

#### `scan_ascii(input, processor)`

High-performance ASCII-only scanning:

```cpp
template<typename Processor>  
std::string scan_ascii(const std::string& input, Processor processor);
```

#### `make_char_range(str [, utf8_mode, validate])`

Creates STL-compatible character range:

```cpp
CharRange make_char_range(const std::string& str, bool utf8_mode = true, bool validate = true);
```

#### `transform_chars(input, output_iter, operation)`

STL-style character transformation:

```cpp
template<typename OutputIterator, typename UnaryOperation>
OutputIterator transform_chars(const std::string& input, OutputIterator result, UnaryOperation op);
```

#### `quoted_str(input [, start_delim, end_delim, escape])`

String quoting and escaping utility:

```cpp
std::string quoted_str(const std::string& input, char start_delim = '"', 
                      char end_delim = '"', char escape = '\\');
```

#### `length(input, utf8_mode, validate)`

Calculates the length of a UTF-8 string in Unicode code points (characters), not bytes:

```cpp
std::size_t length(const std::string& input, bool utf8_mode = true, bool validate = true);
```

**Parameters:**
- `input`: The UTF-8 string to measure
- `utf8_mode`: Whether to use UTF-8 mode (true) or ASCII mode (false), defaults to true  
- `validate`: Whether to validate UTF-8 sequences, defaults to true

**Returns:** The number of Unicode code points (characters) in the string

**Examples:**

```cpp
// ASCII strings
std::string ascii = "Hello World";
size_t length = u8scan::length(ascii);  // Returns 11

// UTF-8 strings with multi-byte characters
std::string chinese = u8"世界";  // 2 characters, 6 bytes
size_t char_count = u8scan::length(chinese);  // Returns 2 (not 6)

// Mixed ASCII and UTF-8
std::string mixed = u8"Hello 世界!";  // 9 characters total
size_t total = u8scan::length(mixed);  // Returns 9

// Emoji characters
std::string emoji = u8"🌍🚀";  // 2 emoji, 8 bytes
size_t emoji_count = u8scan::length(emoji);  // Returns 2 (not 8)

// Complex example from restaurant menu
std::string menu_item = u8"宫保鸡丁 (Kung Pao Chicken) ¥28.00 🌶️🌶️";
size_t item_length = u8scan::length(menu_item);  // Returns 35 characters

// Compare with std::string::length() which counts bytes
size_t byte_count = menu_item.length();  // Returns more than 35 bytes

// ASCII mode - counts bytes instead of characters
size_t byte_mode = u8scan::length(chinese, false);  // Returns 6 (bytes)
size_t char_mode = u8scan::length(chinese, true);   // Returns 2 (characters)
```

This function is essential for:
- **Text layout**: Calculating display width for formatting
- **String truncation**: Cutting strings at character boundaries  
- **Validation**: Ensuring string length limits are character-based
- **Internationalization**: Proper text handling across languages

### Character Predicates

All predicates in the `u8scan::predicates` namespace return `std::function<bool(const CharInfo&)>`:

- `is_ascii()` - ASCII character (< 0x80)
- `is_utf8()` - UTF-8 multi-byte character
- `is_valid()` - Valid UTF-8 sequence
- `is_digit_ascii()` - ASCII digit (0-9)
- `is_alpha_ascii()` - ASCII letter (A-Z, a-z)
- `is_alphanum_ascii()` - ASCII alphanumeric character (A-Z, a-z, 0-9)
- `is_lowercase_ascii()` - ASCII lowercase letter (a-z)
- `is_uppercase_ascii()` - ASCII uppercase letter (A-Z)
- `is_whitespace_ascii()` - ASCII whitespace (space, tab, newline, carriage return)
- `is_emoji()` - Unicode emoji character (based on Unicode emoji ranges)
- `has_codepoint(codepoint)` - Specific Unicode codepoint
- `in_range(min_cp, max_cp)` - Codepoint in range

### Character Conversion Functions

#### `to_lower_ascii(info)`

Converts ASCII character to lowercase:

```cpp
uint32_t to_lower_ascii(const CharInfo& info);
```

#### `to_upper_ascii(info)`

Converts ASCII character to uppercase:

```cpp
uint32_t to_upper_ascii(const CharInfo& info);
```

### STL-like Copy Functions

U8SCAN provides STL-compatible copy functions that work directly with UTF-8 strings:

#### `copy(input, output_iter)`

Copies all characters from a UTF-8 string:

```cpp
template<typename OutputIt>
OutputIt copy(const std::string& input, OutputIt result);
```

#### `copy_if(input, output_iter, predicate)`

Copies characters that match a predicate:

```cpp
template<typename OutputIt, typename Predicate>
OutputIt copy_if(const std::string& input, OutputIt result, Predicate pred);

// Example: Copy only ASCII letters
std::string input = u8"Hello123世界!";
std::string letters;
u8scan::copy_if(input, std::back_inserter(letters), u8scan::predicates::is_alpha_ascii());
// Result: "Hello"

// Example: Copy only alphanumeric characters
std::string alphanum_only;
u8scan::copy_if(input, std::back_inserter(alphanum_only), u8scan::predicates::is_alphanum_ascii());
// Result: "Hello123"
```

#### `copy_until(input, output_iter, predicate)`

Copies characters until a predicate matches:

```cpp
template<typename OutputIt, typename Predicate>
OutputIt copy_until(const std::string& input, OutputIt result, Predicate pred);

// Example: Copy until hitting a digit
std::string input = u8"Hello123世界";
std::string result;
u8scan::copy_until(input, std::back_inserter(result), u8scan::predicates::is_digit_ascii());
// Result: "Hello"
```

#### `copy_from(input, output_iter, predicate)`

Copies characters starting from when a predicate first matches:

```cpp
template<typename OutputIt, typename Predicate>
OutputIt copy_from(const std::string& input, OutputIt result, Predicate pred);

// Example: Copy from first digit onwards
std::string input = u8"Hello123世界";
std::string result;
u8scan::copy_from(input, std::back_inserter(result), u8scan::predicates::is_digit_ascii());
// Result: u8"123世界"
```

#### `copy_n(input, output_iter, n)`

Copies the first N Unicode characters:

```cpp
template<typename OutputIt>
OutputIt copy_n(const std::string& input, OutputIt result, size_t n);

// Example: Copy first 3 characters (Unicode-aware)
std::string input = u8"Hello世界";
std::string result;
u8scan::copy_n(input, std::back_inserter(result), 3);
// Result: "Hel" (3 Unicode characters, not bytes)
```

#### `copy_while(input, output_iter, predicate)`

Copies characters while a predicate continues to match:

```cpp
template<typename OutputIt, typename Predicate>
OutputIt copy_while(const std::string& input, OutputIt result, Predicate pred);

// Example: Copy while characters are digits
std::string input = "123Hello";
std::string result;
u8scan::copy_while(input, std::back_inserter(result), u8scan::predicates::is_digit_ascii());
// Result: "123"
```

### STL Integration

#### `CharIterator`

STL-compatible forward iterator for UTF-8 characters:

```cpp
class CharIterator {
    // Standard iterator interface
    using iterator_category = std::forward_iterator_tag;
    using value_type = CharInfo;
    // ... standard iterator operations
};
```

#### `CharRange`

Range object for use with STL algorithms:

```cpp
class CharRange {
public:
    CharIterator begin() const;
    CharIterator end() const;
    std::size_t size() const;
    bool empty() const;
};
```

## Building and Testing

### Prerequisites

- C++11 compatible compiler (GCC 4.8+, Clang 3.4+, MSVC 2015+)
- CMake 3.10 or later (for building tests and demos)
- Standard C++ library

### Building with CMake

```bash
# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Or use the provided scripts
chmod +x ../rebuild.sh ../run_tests.sh ../run_demos.sh
cd .. && ./rebuild.sh
```

### Running Tests

```bash
# Using CMake (from build directory)
cd build && make test
# Or
cd build && ctest

# Using provided scripts
./run_tests.sh

# Or run directly
./build/bin/u8scan_scanning_test
./build/bin/u8scan_stl_test
./build/bin/u8scan_copy_test
./build/bin/u8scan_emoji_test
```

### Running Demos

```bash
# Using provided scripts
./run_demos.sh

# Or run directly
./build/bin/u8scan_scanning_demo
./build/bin/u8scan_stl_demo
```

### Build Options

Available CMake options:

- `U8SCAN_BUILD_TESTS` - Build test executables (default: ON)
- `U8SCAN_BUILD_DEMOS` - Build demo executables (default: ON)  
- `U8SCAN_BUILD_DOCS` - Build documentation with Doxygen (default: OFF)

Example:

```bash
cmake -DU8SCAN_BUILD_DOCS=ON -DU8SCAN_BUILD_TESTS=OFF ..
```

### Using U8SCAN in Your CMake Project

After installation:

```cmake
find_package(u8scan REQUIRED)
target_link_libraries(your_target PRIVATE u8scan::u8scan)
```

Or include directly:

```cmake
add_subdirectory(path/to/u8scan)
target_link_libraries(your_target PRIVATE u8scan::u8scan)
```

### Project Structure

```
u8scan/
├── include/
│   └── u8scan/
│       └── u8scan.h            # Main header file
├── tests/
│   ├── u8scan_scanning_test.cpp # Scanning functionality tests
│   ├── u8scan_stl_test.cpp      # STL integration tests
│   ├── u8scan_copy_test.cpp     # Copy functions tests
│   └── u8scan_emoji_test.cpp    # Emoji detection tests
├── demos/
│   ├── u8scan_scanning_demo.cpp # Basic scanning examples
│   ├── u8scan_stl_demo.cpp      # STL algorithm examples
│   └── multi_module/            # Multi-module project demo
├── docs/                        # Documentation (Doxygen)
├── cmake/                       # CMake configuration files
├── build/                       # Build output directory
├── CMakeLists.txt              # Main CMake configuration
├── rebuild.sh                  # Build script
├── run_tests.sh               # Test runner script
├── run_demos.sh               # Demo runner script
└── README.md                  # This file
```

## Performance Considerations

U8SCAN is designed for performance and flexibility:

1. **Zero overhead for ASCII**: ASCII-only processing uses optimized `scan_ascii()` function
2. **UTF-8 validation**: Optional validation can be disabled for better performance when input is trusted
3. **STL compatibility**: Direct integration with standard algorithms without additional overhead
4. **Memory efficiency**: Minimal memory allocations, iterator-based processing
5. **BOM handling**: Efficient BOM detection and handling without string copying

### Performance Tips

- Use `scan_ascii()` for ASCII-only strings when possible
- Disable UTF-8 validation with `make_char_range(str, true, false)` for trusted input
- Use STL algorithms with character ranges for maximum performance
- Prefer `transform_chars()` over manual character processing loops

### Benchmarks

Typical performance characteristics (relative to manual character iteration):

- **ASCII scanning**: ~98-100% (minimal overhead)
- **UTF-8 scanning**: ~92-95% (includes validation and multi-byte handling)  
- **STL algorithms**: ~95-98% (iterator overhead)
- **Character predicates**: ~99-100% (inlined function calls)

## Contributing

Contributions are welcome! Please follow these guidelines:

1. **Code Style**: Follow existing style (4 spaces, clear naming)
2. **Testing**: Add tests for new features in appropriate test files
3. **Documentation**: Update README and inline documentation
4. **Compatibility**: Maintain C++11 compatibility

### Development Workflow

```bash
# Clone and build
git clone https://github.com/vpiotr/u8scan.git
cd u8scan
./rebuild.sh

# Run tests
./run_tests.sh

# Run demos
./run_demos.sh

# Make changes and test again
# ... edit files ...
./rebuild.sh && ./run_tests.sh
```

### Adding New Features

1. Add implementation to `include/u8scan/u8scan.h`
2. Add tests to appropriate test file:
   - Core scanning: `tests/u8scan_scanning_test.cpp`
   - STL integration: `tests/u8scan_stl_test.cpp`
3. Add examples to demos if applicable
4. Update README.md with usage examples

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by modern C++ string processing best practices
- Uses STL-compatible design patterns for seamless integration
- Thanks to the C++ community for UTF-8 processing guidance

---

**U8SCAN** - Efficient UTF-8 string scanning and processing for modern C++.
