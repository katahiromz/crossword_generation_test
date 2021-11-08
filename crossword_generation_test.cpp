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
            grow_x1(x - (m_cx + m_x0) + 1, '?');
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

    bool is_crossable_x(int x, int y) const {
        assert(is_letter(get_on(x, y)));
        uint8_t ch1, ch2;
        ch1 = get_on(x - 1, y);
        ch2 = get_on(x + 1, y);
        if (ch1 == '?' || ch2 == '?')
            return true;
        return false;
    }
    bool is_crossable_y(int x, int y) const {
        assert(is_letter(get_on(x, y)));
        uint8_t ch1, ch2;
        ch1 = get_on(x, y - 1);
        ch2 = get_on(x, y + 1);
        if (ch1 == '?' || ch2 == '?')
            return true;
        return false;
    }

    bool must_be_cross(int x, int y) const {
        assert(is_letter(get_on(x, y)));
        uint8_t ch1, ch2;
        ch1 = get_on(x - 1, y);
        ch2 = get_on(x + 1, y);
        bool flag1 = (is_letter(ch1) || is_letter(ch2));
        ch1 = get_on(x, y - 1);
        ch2 = get_on(x, y + 1);
        bool flag2 = (is_letter(ch1) || is_letter(ch2));
        return flag1 && flag2;
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

struct candidate_t {
    int m_x = 0, m_y = 0;
    bool m_vertical = false;
    std::string m_word;
};

struct generation_t {
    inline static bool m_generated = false;
    inline static bool m_canceled = false;
    inline static board_t m_solution;
    inline static std::mutex m_mutex;
    board_t m_board;
    std::unordered_set<std::string> m_words;
    std::unordered_set<pos_t> m_crossable_x, m_crossable_y;

    void apply_candidate(const candidate_t& cand) {
        auto& word = cand.m_word;
        m_words.erase(word);
        int x = cand.m_x, y = cand.m_y;
        if (cand.m_vertical) {
            m_board.ensure(x, y - 1);
            m_board.ensure(x, y + int(word.size()));
            m_board.set_on(x, y - 1, '#');
            m_board.set_on(x, y + int(word.size()), '#');
            for (size_t ich = 0; ich < word.size(); ++ich) {
                m_board.set_on(x, y + int(ich), word[ich]);
                if (m_board.is_crossable_x(x, y + int(ich)))
                    m_crossable_x.insert({ x, y + int(ich) });
            }
        } else {
            m_board.ensure(x - 1, y);
            m_board.ensure(x + int(word.size()), y);
            m_board.set_on(x - 1, y, '#');
            m_board.set_on(x + int(word.size()), y, '#');
            for (size_t ich = 0; ich < word.size(); ++ich) {
                m_board.set_on(x + int(ich), y, word[ich]);
                if (m_board.is_crossable_y(x + int(ich), y))
                    m_crossable_y.insert({ x + int(ich), y });
            }
        }
    }

    std::vector<candidate_t>
    get_candidates_x(int x, int y) const {
        std::vector<candidate_t> cands;

        uint8_t ch0 = m_board.get_on(x, y);
        assert(is_letter(ch0));

        uint8_t ch1 = m_board.get_on(x - 1, y);
        uint8_t ch2 = m_board.get_on(x + 1, y);
        if (!is_letter(ch1) && !is_letter(ch2)) {
            char sz[2] = { char(ch0), 0 };
            cands.push_back({ x, y, false, sz });
        }

        for (auto& word : m_words) {
            for (size_t ich = 0; ich < word.size(); ++ich) {
                if (word[ich] != ch0)
                    continue;

                int x0 = x - int(ich);
                int x1 = x0 + int(word.size());
                bool matched = true;
                if (matched) {
                    uint8_t ch1 = m_board.get_on(x0 - 1, y);
                    uint8_t ch2 = m_board.get_on(x1, y);
                    if (is_letter(ch1) || ch1 == ' ') {
                        matched = false;
                    } else if (is_letter(ch2) || ch2 == ' ') {
                        matched = false;
                    }
                }
                if (matched) {
                    for (size_t k = 0; k < word.size(); ++k) {
                        uint8_t ch3 = m_board.get_on(x0 + int(k), y);
                        if (ch3 != '?' && word[k] != ch3) {
                            matched = false;
                            break;
                        }
                    }
                }
                if (matched) {
                    cands.push_back({x0, y, false, word});
                }
            }
        }

        return cands;
    }

    std::vector<candidate_t>
    get_candidates_y(int x, int y) const {
        std::vector<candidate_t> cands;

        uint8_t ch0 = m_board.get_on(x, y);
        assert(is_letter(ch0));

        uint8_t ch1 = m_board.get_on(x, y - 1);
        uint8_t ch2 = m_board.get_on(x, y + 1);
        if (!is_letter(ch1) && !is_letter(ch2)) {
            char sz[2] = { char(ch0), 0 };
            cands.push_back({ x, y - 1, true, sz });
        }

        for (auto& word : m_words) {
            for (size_t ich = 0; ich < word.size(); ++ich) {
                if (word[ich] != ch0)
                    continue;

                int y0 = y - int(ich);
                int y1 = y0 + int(word.size());
                bool matched = true;
                if (matched) {
                    uint8_t ch1 = m_board.get_on(x, y0 - 1);
                    uint8_t ch2 = m_board.get_on(x, y1);
                    if (is_letter(ch1) || ch1 == ' ') {
                        matched = false;
                    } else if (is_letter(ch2) || ch2 == ' ') {
                        matched = false;
                    }
                }
                if (matched) {
                    for (size_t k = 0; k < word.size(); ++k) {
                        uint8_t ch3 = m_board.get_on(x, y0 + int(k));
                        if (ch3 != '?' && word[k] != ch3) {
                            matched = false;
                            break;
                        }
                    }
                }
                if (matched) {
                    cands.push_back({x, y0, true, word});
                }
            }
        }

        return cands;
    }

    bool generate_recurse() {
        if (m_canceled)
            return false;

        if (m_generated)
            return true;

        if (m_words.empty()) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_generated = true;
            m_solution = m_board;
            m_solution.trim();
            m_solution.replace('?', '#');
            return true;
        }

        auto& board = m_board;
        generation_t new_data = *this;

        auto crossable_x = new_data.m_crossable_x;
        auto crossable_y = new_data.m_crossable_y;

        if (crossable_x.empty() && crossable_y.empty())
            return false;

        std::vector<candidate_t> candidates;

        for (auto& cross : new_data.m_crossable_x) {
            auto cands = new_data.get_candidates_x(cross.first, cross.second);
            if (cands.empty()) {
                if (new_data.m_board.must_be_cross(cross.first, cross.second))
                    return false;
            } else {
                candidates.insert(candidates.end(), cands.begin(), cands.end());
            }
            crossable_x.erase(cross);
        }

        for (auto& cross : new_data.m_crossable_y) {
            auto cands = new_data.get_candidates_y(cross.first, cross.second);
            if (cands.empty()) {
                if (new_data.m_board.must_be_cross(cross.first, cross.second))
                    return false;
            } else {
                candidates.insert(candidates.end(), cands.begin(), cands.end());
            }
            crossable_y.erase(cross);
        }

        new_data.m_crossable_x = std::move(crossable_x);
        new_data.m_crossable_y = std::move(crossable_y);

        for (auto& cand : candidates) {
            generation_t copy(new_data);
            copy.apply_candidate(cand);
            if (copy.generate_recurse())
                return true;
        }

        return false;
    }

    bool generate() {
        auto words = m_words;
        for (auto& word : m_words) {
            if (word.size() <= 1) {
                words.erase(word);
            }
        }

        if (words.empty())
            return false;

        auto word = *words.begin();
        auto& board = m_board;
        board = { int(word.size()) + 2, 1, '?' };
        board.set_on(0, 0, '#');
        for (int x = 0, y = 0, cx = int(word.size()); x < cx; ++x) {
            board.set_on(x + 1, y, word[x]);
            m_crossable_y.emplace(x + 1, y);
        }
        board.set_on(1 + int(word.size()), 0, '#');
        board.print();

        m_words = std::move(words);
        m_words.erase(word);

        if (!generate_recurse()) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        m_solution.print();
        return true;
    }

    static bool do_generate(const std::unordered_set<std::string>& words) {
        generation_t data;
        data.m_words = words;
        return data.generate();
    }
};

void unittest() {
    board_t::unittest();
    generation_t::do_generate({"TEST", "EXAMPLE", "TEMPLE"});
}

int main(void) {
    unittest();
    return 0;
}
