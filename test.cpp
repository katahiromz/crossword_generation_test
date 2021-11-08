#include "crossword_generation.hpp"

int main(void) {
    board_t::unittest();
    auto t0 = std::time(NULL);
    generation_t::do_generate({
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
    auto t1 = std::time(NULL);
    printf("%u\n", int(t1 - t0));
    return 0;
}
