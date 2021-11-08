#include "crossword_generation.hpp"

int main(void) {
    board_t<char>::unittest();
    auto t0 = std::time(NULL);
    generation_t<char>::do_generate({
#if 0
"ABBREVIATION",
"ABDOMEN",
"ABILITY",
"ABOLITION",
"ABSENCE",
"ABUSE",
"ACADEMY",
"ACCELERATOR",
"ACCENT",
"ACCEPTANCE",
"ACCESS",
"ACCESSIBILITY",
"ACCESSORIES",
"ACCESSORY",
"ACCIDENT",
"ACCOMMODATION",
"ACCOMPLISHMENT",
"ACCORD",
"ACCORDION",
"ACCOUNT",
"ACCOUNTANT",
"ACCOUNTS",
"ACCURACY",
"ACHIEVEMENT",
"ACID",
"ACKNOWLEDGMENTS",
"ACQUAINTANCE",
"ACRE",
"ACRES",
#else
"USING",
"WHICH",
"SUPPORT",
"INTEGRATED",
"CMAKE",
"HOW",
"CAN",
"TARGET",
"WITHOUT",
"WRITING",
"FLAGS",
"FOR",
"EACH",
"SPECIFIC",
"COMPILERS",
"CURRENT",
"GLOBAL",
"SETTING",
"WORK",
#endif
        });
    auto t1 = std::time(NULL);
    printf("%u\n", int(t1 - t0));
    return 0;
}
