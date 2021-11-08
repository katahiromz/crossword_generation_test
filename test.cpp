#include "crossword_generation.hpp"

std::unordered_set<std::string> s_words = {
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
};

int main(int argc, char **argv) {
    std::srand(::GetTickCount() ^ ::GetCurrentThreadId());

    typedef char xchar_t;
    //typedef wchar_t xchar_t;

    using namespace crossword_generation;
    board_t<xchar_t>::unittest();

    if (argc > 1) {
        s_words.clear();
        for (int iarg = 1; iarg < argc; ++iarg) {
            s_words.insert(argv[iarg]);
        }
    }

    auto t0 = std::time(NULL);
    {
        if (!check_connectivity<xchar_t>(s_words)) {
            std::printf("check_connectivity failed\n");
        } else {
            const int RETRY_COUNT = 3;
            for (int i = 0; i < RETRY_COUNT; ++i) {
                generation_t<xchar_t>::do_generate_mt(s_words);
                if (generation_t<xchar_t>::s_generated) {
                    generation_t<xchar_t>::s_mutex.lock();
                    generation_t<xchar_t>::s_solution.print();
                    generation_t<xchar_t>::s_mutex.unlock();
                    break;
                } else {
                    std::printf("failed\n");
                }
            }
        }
    }
    auto t1 = std::time(NULL);
    printf("%u\n", int(t1 - t0));

    return 0;
}
