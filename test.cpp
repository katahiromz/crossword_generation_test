#include "crossword_generation.hpp"

int main(void) {
    typedef char xchar_t;
    //typedef wchar_t xchar_t;

    board_t<xchar_t>::unittest();
    auto t0 = std::time(NULL);
    const int RETRY_COUNT = 3;
    for (int i = 0; i < RETRY_COUNT; ++i)
    {
        generation_t<xchar_t>::do_generate_multithread({
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
        });
        if (generation_t<xchar_t>::s_generated) {
            generation_t<xchar_t>::s_mutex.lock();
            generation_t<xchar_t>::s_solution.print();
            generation_t<xchar_t>::s_mutex.unlock();
            break;
        } else {
            std::printf("failed\n");
        }
    }
    auto t1 = std::time(NULL);
    printf("%u\n", int(t1 - t0));
    return 0;
}
