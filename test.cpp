#include "crossword_generation.hpp"

int main(void) {
    board_t::unittest();
    auto t0 = std::time(NULL);
    generation_t::do_generate({
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
        });
    auto t1 = std::time(NULL);
    printf("%u\n", int(t1 - t0));
    return 0;
}
