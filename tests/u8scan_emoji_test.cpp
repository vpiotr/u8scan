#include "../include/utest/utest.h"
#include "../include/u8scan/u8scan.h"
#include <vector>
#include <string>

using namespace u8scan;

// Test the is_emoji predicate with various emoji categories
UTEST_FUNC_DEF2(U8ScanEmoji, BasicEmojiDetection) {
    // Test basic emoji detection
    std::string input = "Hello 🌍 World 🚀!";
    auto range = make_char_range(input);
    
    int emoji_count = static_cast<int>(std::count_if(range.begin(), range.end(), predicates::is_emoji()));
    UTEST_ASSERT_EQUALS(2, emoji_count);  // 🌍 and 🚀
    
    // Find specific emojis
    bool found_earth = false;
    bool found_rocket = false;
    
    for (auto it = range.begin(); it != range.end(); ++it) {
        if (predicates::is_emoji()(*it)) {
            if (it->codepoint == 0x1F30D) {  // 🌍 Earth Globe Europe-Africa
                found_earth = true;
            } else if (it->codepoint == 0x1F680) {  // 🚀 Rocket
                found_rocket = true;
            }
        }
    }
    
    UTEST_ASSERT_TRUE(found_earth);
    UTEST_ASSERT_TRUE(found_rocket);
}

// Test emoji faces and expressions
UTEST_FUNC_DEF2(U8ScanEmoji, EmojiFaces) {
    std::vector<std::string> face_emojis = {
        "😀", "😃", "😄", "😁", "😆", "😅", "😂", "🤣",
        "😊", "😇", "🙂", "🙃", "😉", "😌", "😍", "🥰",
        "😘", "😗", "😙", "😚", "😋", "😛", "😝", "😜"
    };
    
    for (const auto& emoji : face_emojis) {
        auto range = make_char_range(emoji);
        auto it = range.begin();
        UTEST_ASSERT_TRUE(it != range.end());
        UTEST_ASSERT_TRUE(predicates::is_emoji()(*it));
    }
}

// Test common symbols that ARE emoji
UTEST_FUNC_DEF2(U8ScanEmoji, CommonEmojiSymbols) {
    std::vector<std::string> emoji_symbols = {
        "©️", "®️", "™️",  // Copyright/trademark symbols with emoji variation selector
        "⭐", "❤️", "⚡", "☀️", "❄️", "✨", "⚽",  // Common symbols that are emoji
        "➡️", "⬅️", "⬆️", "⬇️"  // Arrow symbols that are emoji
    };
    
    for (const auto& symbol : emoji_symbols) {
        auto range = make_char_range(symbol);
        int emoji_count = static_cast<int>(std::count_if(range.begin(), range.end(), predicates::is_emoji()));
        UTEST_ASSERT_TRUE(emoji_count >= 1);  // At least one emoji component
    }
}

// Test symbols and signs that should NOT be detected as emoji
UTEST_FUNC_DEF2(U8ScanEmoji, NonEmojiSymbols) {
    std::vector<std::string> non_emojis = {
        "©", "®", "™",  // Plain copyright/trademark (without variation selector)
        "℃", "℉", "€", "£", "¥", "₹", "₽",  // Currency and temperature symbols
        "α", "β", "γ", "π", "Ω", "∞", "≈", "≠", "±", "÷",  // Greek letters and math
        "∀", "∃", "∈",  // Mathematical logic symbols
        "世", "界",  // CJK ideographs
        "A", "a", "1", "!"  // Regular ASCII characters
    };
    
    for (const auto& symbol : non_emojis) {
        auto range = make_char_range(symbol);
        auto it = range.begin();
        if (it != range.end()) {
            UTEST_ASSERT_FALSE(predicates::is_emoji()(*it));
        }
    }
}

// Test animals and nature emojis
UTEST_FUNC_DEF2(U8ScanEmoji, AnimalsAndNature) {
    std::vector<std::string> animal_emojis = {
        "🐶", "🐱", "🐭", "🐹", "🐰", "🦊", "🐻", "🐼",
        "🐨", "🐯", "🦁", "🐮", "🐷", "🐸", "🐵", "🙈",
        "🌳", "🌲", "🌴", "🌿", "🍀", "🌺", "🌻", "🌹"
    };
    
    for (const auto& emoji : animal_emojis) {
        auto range = make_char_range(emoji);
        auto it = range.begin();
        UTEST_ASSERT_TRUE(it != range.end());
        UTEST_ASSERT_TRUE(predicates::is_emoji()(*it));
    }
}

// Test food and drink emojis
UTEST_FUNC_DEF2(U8ScanEmoji, FoodAndDrink) {
    std::vector<std::string> food_emojis = {
        "🍎", "🍊", "🍋", "🍌", "🍉", "🍇", "🍓", "🫐",
        "🍈", "🍒", "🍑", "🥭", "🍍", "🥥", "🥝", "🍅",
        "🍕", "🍔", "🍟", "🌭", "🥪", "🌮", "🌯", "🥙",
        "☕", "🍵", "🧃", "🥤", "🍶", "🍺", "🍻", "🥂"
    };
    
    for (const auto& emoji : food_emojis) {
        auto range = make_char_range(emoji);
        auto it = range.begin();
        UTEST_ASSERT_TRUE(it != range.end());
        UTEST_ASSERT_TRUE(predicates::is_emoji()(*it));
    }
}

// Test flag emojis
UTEST_FUNC_DEF2(U8ScanEmoji, Flags) {
    std::vector<std::string> flag_emojis = {
        "🇺🇸", "🇬🇧", "🇫🇷", "🇩🇪", "🇯🇵", "🇨🇳",
        "🇧🇷", "🇮🇳", "🇷🇺", "🇨🇦", "🇦🇺", "🇪🇸"
    };
    
    for (const auto& flag : flag_emojis) {
        auto range = make_char_range(flag);
        // Flags are sequences of Regional Indicator Symbols
        // Each flag consists of two regional indicator characters
        int emoji_count = static_cast<int>(std::count_if(range.begin(), range.end(), predicates::is_emoji()));
        UTEST_ASSERT_TRUE(emoji_count >= 1);  // At least one emoji component
    }
}

// Test transport and map symbols
UTEST_FUNC_DEF2(U8ScanEmoji, Transport) {
    std::vector<std::string> transport_emojis = {
        "🚗", "🚙", "🚌", "🚎", "🏎️", "🚓", "🚑", "🚒",
        "🚐", "🛻", "🚚", "🚛", "🚜", "🏍️", "🛵", "🚲",
        "✈️", "🛫", "🛬", "🚁", "🚟", "🚠", "🚡", "🛸"
    };
    
    for (const auto& emoji : transport_emojis) {
        auto range = make_char_range(emoji);
        auto it = range.begin();
        UTEST_ASSERT_TRUE(it != range.end());
        
        // Some transport emojis might be followed by variation selectors
        // Count total emoji characters in the sequence
        int emoji_count = static_cast<int>(std::count_if(range.begin(), range.end(), predicates::is_emoji()));
        UTEST_ASSERT_TRUE(emoji_count >= 1);
    }
}

// Test mathematical symbols (should NOT be emoji)
UTEST_FUNC_DEF2(U8ScanEmoji, MathematicalSymbols) {
    // These are 4-byte mathematical symbols that should NOT be detected as emoji
    std::vector<std::string> math_symbols = {
        "�", "𝕬",  // Mathematical script and double-struck A
        "�𝔸", "𝔹", "ℂ", "𝔻", "𝔼", "𝔽", "𝔾", "ℍ",  // Mathematical script letters
        "𝕀", "𝕁", "𝕂", "𝕃", "𝕄", "ℕ", "𝕆", "ℙ",  // Mathematical double-struck letters
        "ℚ", "ℝ", "𝕊", "𝕋", "𝕌", "𝕍", "𝕎", "𝕏"   // More mathematical symbols
    };
    
    for (const auto& symbol : math_symbols) {
        auto range = make_char_range(symbol);
        auto it = range.begin();
        if (it != range.end()) {
            UTEST_ASSERT_FALSE(predicates::is_emoji()(*it));
        }
    }
}

// Test miscellaneous symbols
UTEST_FUNC_DEF2(U8ScanEmoji, MiscellaneousSymbols) {
    std::vector<std::string> misc_emojis = {
        "⭐", "🌟", "💫", "✨", "🔥", "💧", "🌊", "💨",
        "❄️", "☃️", "⛄", "☀️", "🌤️", "⛅", "🌦️", "🌧️",
        "⚡", "🌈", "☂️", "☔", "⛱️", "🌍", "🌎", "🌏"
    };
    
    for (const auto& emoji : misc_emojis) {
        auto range = make_char_range(emoji);
        auto it = range.begin();
        UTEST_ASSERT_TRUE(it != range.end());
        
        // Count emoji characters (some might have variation selectors)
        int emoji_count = static_cast<int>(std::count_if(range.begin(), range.end(), predicates::is_emoji()));
        UTEST_ASSERT_TRUE(emoji_count >= 1);
    }
}

// Test specific emoji ranges and codepoints
UTEST_FUNC_DEF2(U8ScanEmoji, SpecificEmojiRanges) {
    struct EmojiTest {
        std::string emoji;
        uint32_t expected_codepoint;
        std::string description;
    };
    
    std::vector<EmojiTest> emoji_tests = {
        // Emoticons (U+1F600-U+1F64F)
        {"😀", 0x1F600, "Grinning face"},
        {"😍", 0x1F60D, "Smiling face with heart-eyes"},
        {"😢", 0x1F622, "Crying face"},
        {"🙏", 0x1F64F, "Folded hands"},
        
        // Miscellaneous Symbols and Pictographs (U+1F300-U+1F5FF)
        {"🌍", 0x1F30D, "Earth globe Europe-Africa"},
        {"🌟", 0x1F31F, "Glowing star"},
        {"🎉", 0x1F389, "Party popper"},
        {"🔥", 0x1F525, "Fire"},
        
        // Transport and Map Symbols (U+1F680-U+1F6FF)
        {"🚀", 0x1F680, "Rocket"},
        {"🚗", 0x1F697, "Automobile"},
        
        // Regional Indicator Symbols (Flags) (U+1F1E6-U+1F1FF)
        {"🇺", 0x1F1FA, "Regional indicator U"},
        {"🇸", 0x1F1F8, "Regional indicator S"},
        
        // Supplemental Symbols and Pictographs (U+1F900-U+1F9FF)
        {"🤖", 0x1F916, "Robot"},
        {"🦄", 0x1F984, "Unicorn"}
    };
    
    for (const auto& test : emoji_tests) {
        auto range = make_char_range(test.emoji);
        auto it = range.begin();
        UTEST_ASSERT_TRUE(it != range.end());
        UTEST_ASSERT_TRUE(predicates::is_emoji()(*it));
        UTEST_ASSERT_EQUALS(test.expected_codepoint, it->codepoint);
    }
}

// Test edge cases and boundaries
UTEST_FUNC_DEF2(U8ScanEmoji, EdgeCases) {
    // Test empty string
    std::string empty = "";
    auto range = make_char_range(empty);
    int emoji_count = static_cast<int>(std::count_if(range.begin(), range.end(), predicates::is_emoji()));
    UTEST_ASSERT_EQUALS(0, emoji_count);
    
    // Test ASCII only
    std::string ascii_only = "Hello World 123!";
    auto ascii_range = make_char_range(ascii_only);
    int ascii_emoji_count = static_cast<int>(std::count_if(ascii_range.begin(), ascii_range.end(), predicates::is_emoji()));
    UTEST_ASSERT_EQUALS(0, ascii_emoji_count);
    
    // Test mixed content
    std::string mixed = "Hello 🌍 World 123 🚀 Test!";
    auto mixed_range = make_char_range(mixed);
    int mixed_emoji_count = static_cast<int>(std::count_if(mixed_range.begin(), mixed_range.end(), predicates::is_emoji()));
    UTEST_ASSERT_EQUALS(2, mixed_emoji_count);
}

// Run all emoji tests
int main() {
    UTEST_PROLOG();
    UTEST_ENABLE_VERBOSE_MODE();
    
    // Emoji detection tests
    UTEST_FUNC2(U8ScanEmoji, BasicEmojiDetection);
    UTEST_FUNC2(U8ScanEmoji, EmojiFaces);
    UTEST_FUNC2(U8ScanEmoji, NonEmojiSymbols);
    UTEST_FUNC2(U8ScanEmoji, AnimalsAndNature);
    UTEST_FUNC2(U8ScanEmoji, FoodAndDrink);
    UTEST_FUNC2(U8ScanEmoji, Flags);
    UTEST_FUNC2(U8ScanEmoji, Transport);
    UTEST_FUNC2(U8ScanEmoji, MathematicalSymbols);
    UTEST_FUNC2(U8ScanEmoji, MiscellaneousSymbols);
    UTEST_FUNC2(U8ScanEmoji, SpecificEmojiRanges);
    UTEST_FUNC2(U8ScanEmoji, EdgeCases);
    
    UTEST_EPILOG();
}
