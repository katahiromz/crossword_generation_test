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

template <typename T_CHAR>
inline void mstr_trim(std::basic_string<T_CHAR>& str, const T_CHAR *spaces)
{
    typedef std::basic_string<T_CHAR> string_type;
    size_t i = str.find_first_not_of(spaces);
    size_t j = str.find_last_not_of(spaces);
    if ((i == string_type::npos) || (j == string_type::npos))
    {
        str.clear();
    }
    else
    {
        str = str.substr(i, j - i + 1);
    }
}

bool load_dict(const char *filename, std::unordered_set<std::string>& dict) {
    if (FILE *fp = fopen(filename, "r")) {
        char buf[256];
        while (fgets(buf, 256, fp)) {
            std::string str = buf;
            mstr_trim(str, " \t\r\n");
            dict.insert(str);
        }
        return true;
    }
    return false;
}

void do_test1(void) {
    using namespace crossword_generation;
    std::string nonconnected;
    if (!check_connectivity<char>(s_words, nonconnected)) {
        std::printf("check_connectivity failed: %s\n\n", nonconnected.c_str());
    } else {
        reset();
        from_words_t<char, false>::do_generate(s_words);
        wait_for_threads(1);
        if (s_generated) {
            s_mutex.lock();
            from_words_t<char, false>::s_solution.print();
            s_mutex.unlock();
        } else {
            std::printf("failed\n");
        }
    }
}

int main(int argc, char **argv) {
    std::srand(::GetTickCount() ^ ::GetCurrentThreadId());

    using namespace crossword_generation;
    board_t<char, false>::unittest();

    if (argc > 1) {
        s_words.clear();
        if (argc == 2) {
            if (!load_dict(argv[1], s_words)) {
                std::fprintf(stderr, "ERROR: cannot load file '%s'\n", argv[1]);
            }
        } else {
            for (int iarg = 1; iarg < argc; ++iarg) {
                s_words.insert(argv[iarg]);
            }
        }
    }

    auto t0 = std::time(NULL);
    do_test1();
    auto t1 = std::time(NULL);
    printf("%u\n", int(t1 - t0));

    return 0;
}
