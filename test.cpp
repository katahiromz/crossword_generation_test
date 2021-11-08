#include "crossword_generation.hpp"

int main(void) {
    board_t<wchar_t>::unittest();
    auto t0 = std::time(NULL);
    generation_t<wchar_t>::do_generate({
#if 0
L"ABBREVIATION",
L"ABDOMEN",
L"ABILITY",
L"ABOLITION",
L"ABSENCE",
L"ABUSE",
L"ACADEMY",
L"ACCELERATOR",
L"ACCENT",
L"ACCEPTANCE",
L"ACCESS",
L"ACCESSIBILITY",
L"ACCESSORIES",
L"ACCESSORY",
L"ACCIDENT",
L"ACCOMMODATION",
L"ACCOMPLISHMENT",
L"ACCORD",
L"ACCORDION",
L"ACCOUNT",
L"ACCOUNTANT",
L"ACCOUNTS",
L"ACCURACY",
L"ACHIEVEMENT",
L"ACID",
L"ACKNOWLEDGMENTS",
L"ACQUAINTANCE",
L"ACRE",
L"ACRES",
#else
L"USING",
L"WHICH",
L"SUPPORT",
L"INTEGRATED",
L"CMAKE",
L"HOW",
L"CAN",
L"TARGET",
L"WITHOUT",
L"WRITING",
L"FLAGS",
L"FOR",
L"EACH",
L"SPECIFIC",
L"COMPILERS",
L"CURRENT",
L"GLOBAL",
L"SETTING",
L"WORK",
#endif
        });
    auto t1 = std::time(NULL);
    printf("%u\n", int(t1 - t0));
    return 0;
}
