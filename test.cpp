//#define SINGLETHREADDEBUG
//#define NO_RANDOM

#include "crossword_generation.hpp"

std::unordered_set<std::string> s_words;

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

#if 1
void do_test2(void) {
    using namespace crossword_generation;
    reset();

    board_t<char, true> board(6, 6, '?');
    board.m_data =
"?????#"
"?#?#?#"
"?#????"
"????#?"
"#?#?#?"
"#?????";

    for (int i = 0; i < 5; ++i) {
        reset();
        non_add_block_t<char>::do_generate(board, s_words);
        wait_for_threads(1, 10);
        if (s_generated) {
            s_mutex.lock();
            non_add_block_t<char>::s_solution.print();
            s_mutex.unlock();
            break;
        }
        else {
            std::printf("failed\n");
        }
    }
}
#endif

int main(int argc, char **argv) {
    std::srand(DWORD(::GetTickCount64()) ^ ::GetCurrentThreadId());

    using namespace crossword_generation;
    board_t<char, false>::unittest();

    if (argc > 1) {
        s_words.clear();
        if (argc == 2) {
            if (!load_dict(argv[1], s_words)) {
                std::fprintf(stderr, "ERROR: cannot load file '%s'\n", argv[1]);
                return EXIT_FAILURE;
            }
        } else {
            for (int iarg = 1; iarg < argc; ++iarg) {
                s_words.insert(argv[iarg]);
            }
        }
    }

    if (s_words.empty()) {
        auto name = "dict.txt";
        if (!load_dict(name, s_words)) {
            std::fprintf(stderr, "ERROR: cannot load file '%s'\n", name );
            return EXIT_FAILURE;
        }
    }

    auto t0 = std::time(NULL);
#if 0
    do_test1();
#else
    do_test2();
#endif
    auto t1 = std::time(NULL);
    printf("%u\n", int(t1 - t0));

    return 0;
}
