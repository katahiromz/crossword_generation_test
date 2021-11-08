#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <algorithm>
#include <utility>

typedef std::pair<int, int> pos_t;

namespace std {
    template <>
    struct hash<pos_t> {
        size_t operator()(const pos_t& pos) const {
            return pos.first ^ (pos.second * 128);
        }
    };
}

inline bool is_letter(uint8_t ch) {
    return (ch != ' ' && ch != '#' && ch != '?');
}

struct board_data_t {
    std::vector<uint8_t> m_data;

    board_data_t(int cx = 1, int cy = 1, uint8_t ch = ' ') {
        allocate(cx, cy, ch);
    }

    void allocate(int cx, int cy, uint8_t ch = ' ') {
        m_data.assign(cx * cy, ch);
    }

    void fill(uint8_t ch = ' ') {
        std::fill(m_data.begin(), m_data.end(), ch);
    }

    void replace(uint8_t chOld, uint8_t chNew) {
        std::replace(m_data.begin(), m_data.end(), chOld, chNew);
    }
};

struct board_t : board_data_t {
    int m_x0 = 0, m_y0 = 0;
    int m_cx = 0, m_cy = 0;

    board_t(int cx = 1, int cy = 1, uint8_t ch = ' ', int x0 = 0, int y0 = 0)
        : board_data_t(cx, cy, ch), m_cx(cx), m_cy(cy), m_x0(x0), m_y0(y0)
    {
    }
    board_t(const board_t& b) = default;
    board_t& operator=(const board_t& b) = default;

    // x, y: absolute coordinate
    uint8_t get_at(int x, int y) const {
        if (0 <= x && x < m_cx && 0 <= y && y < m_cy) {
            return m_data[y * m_cx + x];
        }
        return '?';
    }
    // x, y: absolute coordinate
    void set_at(int x, int y, uint8_t ch) {
        if (0 <= x && x < m_cx && 0 <= y && y < m_cy) {
            m_data[y * m_cx + x] = ch;
        }
    }

    // x: relative coordinate
    void ensure_x(int x) {
        if (x < m_x0) {
            grow_x0(m_x0 - x, '?');
        } else if (m_cx + m_x0 <= x) {
            grow_x1(x - (m_cy + m_y0) + 1, '?');
        }
    }
    // y: relative coordinate
    void ensure_y(int y) {
        if (y < m_y0) {
            grow_y0(m_y0 - y, '?');
        } else if (m_cy + m_y0 <= y) {
            grow_y1(y - (m_cy + m_y0) + 1, '?');
        }
    }
    // x, y: relative coordinate
    void ensure(int x, int y) {
        ensure_x(x);
        ensure_y(y);
    }

    // x, y: relative coordinate
    uint8_t get_on(int x, int y) const {
        assert(m_x0 <= 0);
        assert(m_y0 <= 0);
        return get_at(x - m_x0, y - m_y0);
    }
    // x, y: relative coordinate
    void set_on(int x, int y, uint8_t ch) {
        assert(m_x0 <= 0);
        assert(m_y0 <= 0);
        ensure(x, y);
        set_at(x - m_x0, y - m_y0, ch);
    }

    // x0: absolute coordinate
    void insert_x(int x0, int cx = 1, uint8_t ch = ' ') {
        assert(0 <= x0 && x0 <= m_cx);

        board_t data(m_cx + cx, m_cy, ch);

        for (int y = 0; y < m_cy; ++y) {
            for (int x = 0; x < m_cx; ++x) {
                auto ch = get_at(x, y);
                if (x < x0)
                    data.set_at(x, y, ch);
                else
                    data.set_at(x + cx, y, ch);
            }
        }

        m_data = std::move(data.m_data);
        m_cx += cx;
    }

    // y0: absolute coordinate
    void insert_y(int y0, int cy = 1, uint8_t ch = ' ') {
        assert(0 <= y0 && y0 <= m_cy);

        board_t data(m_cx, m_cy + cy, ch);

        for (int y = 0; y < m_cy; ++y) {
            for (int x = 0; x < m_cx; ++x) {
                auto ch = get_at(x, y);
                if (y < y0)
                    data.set_at(x, y, ch);
                else
                    data.set_at(x, y + cy, ch);
            }
        }

        m_data = std::move(data.m_data);
        m_cy += cy;
    }

    // x0: absolute coordinate
    void delete_x(int x0) {
        assert(0 <= x0 && x0 < m_cx);

        board_t data(m_cx - 1, m_cy, ' ');

        for (int y = 0; y < m_cy; ++y) {
            for (int x = 0; x < m_cx - 1; ++x) {
                if (x < x0)
                    data.set_at(x, y, get_at(x, y));
                else
                    data.set_at(x, y, get_at(x + 1, y));
            }
        }

        m_data = std::move(data.m_data);
        --m_cx;
    }

    // y0: absolute coordinate
    void delete_y(int y0) {
        assert(0 <= y0 && y0 < m_cy);

        board_t data(m_cx, m_cy - 1, ' ');

        for (int y = 0; y < m_cy - 1; ++y) {
            for (int x = 0; x < m_cx; ++x) {
                if (y < y0)
                    data.set_at(x, y, get_at(x, y));
                else
                    data.set_at(x, y, get_at(x, y + 1));
            }
        }

        m_data = std::move(data.m_data);
        --m_cy;
    }

    void grow_x0(int cx, uint8_t ch = ' ') {
        assert(cx > 0);
        insert_x(0, cx, ch);
        m_x0 -= cx;
    }
    void grow_x1(int cx, uint8_t ch = ' ') {
        assert(cx > 0);
        insert_x(m_cx, cx, ch);
    }

    void grow_y0(int cy, uint8_t ch = ' ') {
        assert(cy > 0);
        insert_y(0, cy, ch);
        m_y0 -= cy;
    }
    void grow_y1(int cy, uint8_t ch = ' ') {
        assert(cy > 0);
        insert_y(m_cy, cy, ch);
    }

    void trim_x() {
        bool found;
        int x, y;

        while (m_cx > 0) {
            found = false;
            x = 0;
            for (y = 0; y < m_cy; ++y) {
                uint8_t ch = get_at(x, y);
                if (is_letter(ch)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                delete_x(0);
                ++m_x0;
            } else {
                break;
            }
        }

        while (m_cx > 0) {
            found = false;
            x = m_cx - 1;
            for (y = 0; y < m_cy; ++y) {
                uint8_t ch = get_at(x, y);
                if (is_letter(ch)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                delete_x(m_cx - 1);
            } else {
                break;
            }
        }

        m_x0 = 0;
    }

    void trim_y() {
        bool found;
        int x, y;

        while (m_cy > 0) {
            found = false;
            y = 0;
            for (x = 0; x < m_cx; ++x) {
                uint8_t ch = get_at(x, y);
                if (is_letter(ch)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                delete_y(0);
            } else {
                break;
            }
        }

        while (m_cy > 0) {
            found = false;
            y = m_cy - 1;
            for (x = 0; x < m_cx; ++x) {
                uint8_t ch = get_at(x, y);
                if (is_letter(ch)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                delete_y(m_cy - 1);
            } else {
                break;
            }
        }

        m_y0 = 0;
    }

    void trim() {
        trim_y();
        trim_x();
    }

    void print() const {
        std::printf("dx:%d, dy:%d, cx:%d, cy:%d\n", m_x0, m_y0, m_cx, m_cy);
        for (int y = m_y0; y < m_y0 + m_cy; ++y) {
            printf("%3d: ", y);
            for (int x = m_x0; x < m_x0 + m_cx; ++x) {
                auto ch = get_on(x, y);
                putchar(ch);
            }
            std::printf("\n");
        }
        std::fflush(stdout);
    }

    static void unittest() {
        board_t b(3, 3, '#');
        b.insert_x(1, 1, '|');
        assert(b.get_at(0, 0) == '#');
        assert(b.get_at(1, 0) == '|');
        assert(b.get_at(2, 0) == '#');
        assert(b.get_at(3, 0) == '#');
        b.insert_y(1, 1, '-');
        assert(b.get_at(0, 0) == '#');
        assert(b.get_at(0, 1) == '-');
        assert(b.get_at(0, 2) == '#');
        assert(b.get_at(0, 3) == '#');
        b.delete_y(1);
        assert(b.get_at(0, 0) == '#');
        assert(b.get_at(0, 1) == '#');
        assert(b.get_at(0, 2) == '#');
        b.delete_x(1);
        assert(b.get_at(0, 0) == '#');
        assert(b.get_at(1, 0) == '#');
        assert(b.get_at(2, 0) == '#');
        b.grow_x0(1, '|');
        assert(b.get_on(-1, 1) == '|');
        assert(b.get_on(0, 1) == '#');
        assert(b.get_on(1, 1) == '#');
        assert(b.get_on(2, 1) == '#');
        assert(b.get_at(0, 1) == '|');
        assert(b.get_at(1, 1) == '#');
        assert(b.get_at(2, 1) == '#');
        assert(b.get_at(3, 1) == '#');
        b.delete_x(0);
        b.m_x0 = 0;
        b.grow_y0(1, '-');
        assert(b.get_on(1, -1) == '-');
        assert(b.get_on(1, 0) == '#');
        assert(b.get_on(1, 1) == '#');
        assert(b.get_on(1, 2) == '#');
        assert(b.get_at(1, 0) == '-');
        assert(b.get_at(1, 1) == '#');
        assert(b.get_at(1, 2) == '#');
        assert(b.get_at(1, 3) == '#');
        b.delete_y(0);
        b.m_y0 = 0;
        b.grow_x1(1, '|');
        assert(b.get_on(0, 1) == '#');
        assert(b.get_on(1, 1) == '#');
        assert(b.get_on(2, 1) == '#');
        assert(b.get_on(3, 1) == '|');
        b.delete_x(3);
        b.grow_y1(1, '-');
        assert(b.get_on(1, 0) == '#');
        assert(b.get_on(1, 1) == '#');
        assert(b.get_on(1, 2) == '#');
        assert(b.get_on(1, 3) == '-');
        b.delete_y(3);
        b.allocate(3, 3, '?');
        b.grow_x0(1, '?');
        b.set_on(1, 1, 'A');
        b.trim_x();
        b.trim_y();
        assert(b.get_at(0, 0) == 'A');
        b.set_on(2, 2, 'A');
        assert(b.get_at(0, 0) == 'A');
        assert(b.get_at(2, 2) == 'A');
        assert(b.get_on(0, 0) == 'A');
        assert(b.get_on(2, 2) == 'A');
        b.grow_x0(1, '|');
        assert(b.get_at(0, 0) == '|');
        assert(b.get_at(1, 0) == 'A');
        assert(b.get_at(2, 0) == '?');
        assert(b.get_at(3, 0) == '?');
        assert(b.get_on(-1, 0) == '|');
        assert(b.get_on(0, 0) == 'A');
        assert(b.get_on(1, 0) == '?');
        assert(b.get_on(2, 0) == '?');
        b.delete_x(0);
        b.m_x0 = 0;
        b.grow_y0(1, '-');
        assert(b.get_at(0, 0) == '-');
        assert(b.get_at(0, 1) == 'A');
        assert(b.get_at(0, 2) == '?');
        assert(b.get_at(0, 3) == '?');
        assert(b.get_on(0, -1) == '-');
        assert(b.get_on(0, 0) == 'A');
        assert(b.get_on(0, 1) == '?');
        assert(b.get_on(0, 2) == '?');
        b.delete_y(0);
        b.m_y0 = 0;
    }
};

struct generation_data_t {
    board_t m_board;
    std::unordered_set<std::string> m_words;
    std::unordered_set<pos_t> m_crossable_x, m_crossable_y;
};

struct candidate_t {
    int m_x = 0, m_y = 0;
    bool m_vertical = false;
    std::string m_pat, m_word;
};

bool g_generated = false;
bool g_canceled = false;
board_t g_solution;
std::mutex g_mutex;

void apply_candidate(generation_data_t& data, const candidate_t& cand) {
    auto& pat = cand.m_pat;
    int x = cand.m_x, y = cand.m_y;
    if (cand.m_vertical) {
        for (size_t ich = 0; ich < pat.size(); ++ich) {
            data.m_board.set_on(x, y + int(ich), pat[ich]);
        }
    } else {
        for (size_t ich = 0; ich < pat.size(); ++ich) {
            data.m_board.set_on(x + int(ich), y, pat[ich]);
        }
    }
    data.m_words.erase(cand.m_word);
}

std::vector<candidate_t>
get_candidates_x(const generation_data_t& data, int x, int y) {
    auto& board = data.m_board;
    std::vector<candidate_t> cands;

    for (auto& word : data.m_words) {
        uint8_t ch0 = board.get_on(x, y);
        for (size_t ich = 0; ich < word.size(); ++ich) {
            if (word[ich] != ch0)
                continue;

            int x0 = x - int(ich);
            int x1 = x0 + int(word.size());
            bool matched = true;
            if (matched) {
                uint8_t ch1 = board.get_on(x0 - 1, y);
                uint8_t ch2 = board.get_on(x1, y);
                if (is_letter(ch1) || ch1 == ' ') {
                    matched = false;
                } else if (is_letter(ch2) || ch2 == ' ') {
                    matched = false;
                }
            }
            if (matched) {
                for (size_t k = 0; k < word.size(); ++k) {
                    uint8_t ch3 = board.get_on(x0 + int(k), y);
                    if (ch3 != '?' && word[k] != ch3) {
                        matched = false;
                        break;
                    }
                }
            }
            if (matched) {
                cands.push_back({x0 - 1, y, true, '#' + word + '#', word});
            }
        }
    }

    return cands;
}

std::vector<candidate_t>
get_candidates_y(const generation_data_t& data, int x, int y) {
    auto& board = data.m_board;
    std::vector<candidate_t> cands;

    for (auto& word : data.m_words) {
        uint8_t ch0 = board.get_on(x, y);
        for (size_t ich = 0; ich < word.size(); ++ich) {
            if (word[ich] != ch0)
                continue;

            int y0 = y - int(ich);
            int y1 = y0 + int(word.size());
            bool matched = true;
            if (matched) {
                uint8_t ch1 = board.get_on(x, y0 - 1);
                uint8_t ch2 = board.get_on(x, y1);
                if (is_letter(ch1) || ch1 == ' ') {
                    matched = false;
                } else if (is_letter(ch2) || ch2 == ' ') {
                    matched = false;
                }
            }
            if (matched) {
                for (size_t k = 0; k < word.size(); ++k) {
                    uint8_t ch3 = board.get_on(x, y0 + int(k));
                    if (ch3 != '?' && word[k] != ch3) {
                        matched = false;
                        break;
                    }
                }
            }
            if (matched) {
                cands.push_back({x, y0 - 1, true, '#' + word + '#', word});
            }
        }
    }

    return cands;
}

bool generate_recurse(const generation_data_t& data) {
    if (g_canceled)
        return false;

    if (g_generated)
        return true;

    if (data.m_words.empty()) {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_generated = true;
        g_solution = data.m_board;
        g_solution.trim();
        g_solution.replace('?', '#');
        return true;
    }

    auto& board = data.m_board;
    generation_data_t new_data = data;

    auto crossable_x = new_data.m_crossable_x;
    auto crossable_y = new_data.m_crossable_y;

    if (crossable_x.empty() && crossable_y.empty())
        return false;

    std::vector<candidate_t> candidates;

    for (auto& cross : new_data.m_crossable_x) {
        auto cands = get_candidates_x(new_data, cross.first, cross.second);
        if (cands.empty()) {
            crossable_x.erase(cross);
        } else {
            candidates.insert(candidates.end(), cands.begin(), cands.end());
        }
    }

    for (auto& cross : new_data.m_crossable_y) {
        auto cands = get_candidates_y(new_data, cross.first, cross.second);
        if (cands.empty()) {
            crossable_y.erase(cross);
        } else {
            candidates.insert(candidates.end(), cands.begin(), cands.end());
        }
    }

    new_data.m_crossable_x = std::move(crossable_x);
    new_data.m_crossable_y = std::move(crossable_y);

    for (auto& cand : candidates) {
        generation_data_t copy(new_data);
        apply_candidate(copy, cand);
        if (generate_recurse(copy))
            return true;
    }

    return false;
}

bool generate(generation_data_t& data) {
    auto words = data.m_words;
    for (auto& word : data.m_words) {
        if (word.size() <= 1) {
            words.erase(word);
        }
    }

    if (words.empty())
        return false;

    auto word = *words.begin();
    auto& board = data.m_board;
    board = { int(word.size()) + 2, 1, '?' };
    board.set_on(0, 0, '#');
    for (int x = 0, y = 0, cx = int(word.size()); x < cx; ++x) {
        board.set_on(x + 1, y, word[x]);
        data.m_crossable_y.emplace(x + 1, y);
    }
    board.set_on(1 + int(word.size()), 0, '#');

    data.m_words = std::move(words);
    data.m_words.erase(word);

    if (!generate_recurse(data)) {
        return false;
    }

    return true;
}

bool do_generate(const std::unordered_set<std::string>& words) {
    g_generated = g_canceled = false;
    generation_data_t data;
    data.m_words = words;
    if (generate(data)) {
        g_solution.print();
        return true;
    }
    return false;
}

void unittest() {
    board_t::unittest();
    do_generate({"TEST", "EXAMPLE", "TEMPLE"});
}

int main(void) {
    unittest();
    return 0;
}
