#pragma once
#include <cstdio>
#include <cstdint>
#include <ctime>
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

typedef char xchar_t;
typedef std::basic_string<xchar_t> xstring_t;

struct candidate_t {
    int m_x = 0, m_y = 0;
    bool m_vertical = false;
    xstring_t m_word;
};

inline bool is_letter(xchar_t ch) {
    return (ch != ' ' && ch != '#' && ch != '?');
}

struct board_data_t {
    std::basic_string<xchar_t> m_data;

    board_data_t(int cx = 1, int cy = 1, xchar_t ch = ' ') {
        allocate(cx, cy, ch);
    }

    void allocate(int cx, int cy, xchar_t ch = ' ') {
        m_data.assign(cx * cy, ch);
    }

    void fill(xchar_t ch = ' ') {
        std::fill(m_data.begin(), m_data.end(), ch);
    }

    void replace(xchar_t chOld, xchar_t chNew) {
        std::replace(m_data.begin(), m_data.end(), chOld, chNew);
    }
};

struct board_t : board_data_t {
    int m_x0 = 0, m_y0 = 0;
    int m_cx = 0, m_cy = 0;

    board_t(int cx = 1, int cy = 1, xchar_t ch = ' ', int x0 = 0, int y0 = 0)
        : board_data_t(cx, cy, ch), m_cx(cx), m_cy(cy), m_x0(x0), m_y0(y0)
    {
    }
    board_t(const board_t& b) = default;
    board_t& operator=(const board_t& b) = default;

    // x, y: absolute coordinate
    xchar_t get_at(int x, int y) const {
        if (0 <= x && x < m_cx && 0 <= y && y < m_cy) {
            return m_data[y * m_cx + x];
        }
        return '?';
    }
    // x, y: absolute coordinate
    void set_at(int x, int y, xchar_t ch) {
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
    xchar_t get_on(int x, int y) const {
        assert(m_x0 <= 0);
        assert(m_y0 <= 0);
        return get_at(x - m_x0, y - m_y0);
    }
    // x, y: relative coordinate
    void set_on(int x, int y, xchar_t ch) {
        assert(m_x0 <= 0);
        assert(m_y0 <= 0);
        set_at(x - m_x0, y - m_y0, ch);
    }

    // x0: absolute coordinate
    void insert_x(int x0, int cx = 1, xchar_t ch = ' ') {
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
    void insert_y(int y0, int cy = 1, xchar_t ch = ' ') {
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

    void grow_x0(int cx, xchar_t ch = ' ') {
        assert(cx > 0);
        insert_x(0, cx, ch);
        m_x0 -= cx;
    }
    void grow_x1(int cx, xchar_t ch = ' ') {
        assert(cx > 0);
        insert_x(m_cx, cx, ch);
    }

    void grow_y0(int cy, xchar_t ch = ' ') {
        assert(cy > 0);
        insert_y(0, cy, ch);
        m_y0 -= cy;
    }
    void grow_y1(int cy, xchar_t ch = ' ') {
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
                xchar_t ch = get_at(x, y);
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
                xchar_t ch = get_at(x, y);
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
                xchar_t ch = get_at(x, y);
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
                xchar_t ch = get_at(x, y);
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
        xchar_t ch1, ch2;
        ch1 = get_on(x - 1, y);
        ch2 = get_on(x + 1, y);
        if (ch1 == '?' || ch2 == '?')
            return true;
        return false;
    }
    bool is_crossable_y(int x, int y) const {
        assert(is_letter(get_on(x, y)));
        xchar_t ch1, ch2;
        ch1 = get_on(x, y - 1);
        ch2 = get_on(x, y + 1);
        if (ch1 == '?' || ch2 == '?')
            return true;
        return false;
    }

    bool must_be_cross(int x, int y) const {
        assert(is_letter(get_on(x, y)));
        xchar_t ch1, ch2;
        ch1 = get_on(x - 1, y);
        ch2 = get_on(x + 1, y);
        bool flag1 = (is_letter(ch1) || is_letter(ch2));
        ch1 = get_on(x, y - 1);
        ch2 = get_on(x, y + 1);
        bool flag2 = (is_letter(ch1) || is_letter(ch2));
        return flag1 && flag2;
    }

    void apply_size(const candidate_t& cand) {
        auto& word = cand.m_word;
        int x = cand.m_x, y = cand.m_y;
        if (cand.m_vertical) {
            ensure(x, y - 1);
            ensure(x, y + int(word.size()));
        }
        else {
            ensure(x - 1, y);
            ensure(x + int(word.size()), y);
        }
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
        b.ensure(2, 2);
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

struct generation_t {
    inline static bool s_generated = false;
    inline static bool s_canceled = false;
    inline static board_t s_solution;
    inline static std::mutex s_mutex;
    board_t m_board;
    std::unordered_set<xstring_t> m_words, m_dict;
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
                int y0 = y + int(ich);
                m_board.set_on(x, y0, word[ich]);
                m_crossable_y.erase({x, y0});
                if (m_board.is_crossable_x(x, y0))
                    m_crossable_x.insert({ x, y0 });
            }
        } else {
            m_board.ensure(x - 1, y);
            m_board.ensure(x + int(word.size()), y);
            m_board.set_on(x - 1, y, '#');
            m_board.set_on(x + int(word.size()), y, '#');
            for (size_t ich = 0; ich < word.size(); ++ich) {
                int x0 = x + int(ich);
                m_board.set_on(x0, y, word[ich]);
                m_crossable_x.erase({x0, y});
                if (m_board.is_crossable_y(x0, y))
                    m_crossable_y.insert({ x0, y });
            }
        }
    }

    std::vector<candidate_t>
    get_candidates_x(int x, int y) const {
        std::vector<candidate_t> cands;

        xchar_t ch0 = m_board.get_on(x, y);
        assert(is_letter(ch0));

        xchar_t ch1 = m_board.get_on(x - 1, y);
        xchar_t ch2 = m_board.get_on(x + 1, y);
        if (!is_letter(ch1) && !is_letter(ch2)) {
            xchar_t sz[2] = { ch0, 0 };
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
                    xchar_t ch1 = m_board.get_on(x0 - 1, y);
                    xchar_t ch2 = m_board.get_on(x1, y);
                    if (is_letter(ch1) || ch1 == ' ') {
                        matched = false;
                    } else if (is_letter(ch2) || ch2 == ' ') {
                        matched = false;
                    }
                }
                if (matched) {
                    for (size_t k = 0; k < word.size(); ++k) {
                        xchar_t ch3 = m_board.get_on(x0 + int(k), y);
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

        xchar_t ch0 = m_board.get_on(x, y);
        assert(is_letter(ch0));

        xchar_t ch1 = m_board.get_on(x, y - 1);
        xchar_t ch2 = m_board.get_on(x, y + 1);
        if (!is_letter(ch1) && !is_letter(ch2)) {
            xchar_t sz[2] = { ch0, 0 };
            cands.push_back({ x, y, true, sz });
        }

        for (auto& word : m_words) {
            for (size_t ich = 0; ich < word.size(); ++ich) {
                if (word[ich] != ch0)
                    continue;

                int y0 = y - int(ich);
                int y1 = y0 + int(word.size());
                bool matched = true;
                if (matched) {
                    xchar_t ch1 = m_board.get_on(x, y0 - 1);
                    xchar_t ch2 = m_board.get_on(x, y1);
                    if (is_letter(ch1) || ch1 == ' ') {
                        matched = false;
                    } else if (is_letter(ch2) || ch2 == ' ') {
                        matched = false;
                    }
                }
                if (matched) {
                    for (size_t k = 0; k < word.size(); ++k) {
                        xchar_t ch3 = m_board.get_on(x, y0 + int(k));
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

    bool fixup_candidates(std::vector<candidate_t>& candidates) {
        std::vector<candidate_t> cands;
        std::unordered_set<pos_t> positions;
        for (auto& cand : candidates) {
            if (cand.m_word.size() == 1) {
                cands.push_back(cand);
                positions.insert( {cand.m_x, cand.m_y} );
            }
        }
        for (auto& cand : candidates) {
            if (cand.m_word.size() != 1) {
                if (positions.count(pos_t(cand.m_x, cand.m_y)) == 0)
                    return false;
            }
        }
        for (auto& cand : cands) {
            apply_candidate(cand);
        }
        return true;
    }

    bool generate_recurse() {
        if (s_canceled)
            return false;

        if (s_generated)
            return true;

        if (m_crossable_x.empty() && m_crossable_y.empty())
            return false;

        std::vector<candidate_t> candidates;

        for (auto& cross : m_crossable_x) {
            auto cands = get_candidates_x(cross.first, cross.second);
            if (cands.empty()) {
                if (m_board.must_be_cross(cross.first, cross.second))
                    return false;
            } else if (cands.size() == 1 && cands[0].m_word.size() == 1) {
                if (m_board.must_be_cross(cross.first, cross.second))
                    return false;
            } else {
                candidates.insert(candidates.end(), cands.begin(), cands.end());
            }
        }

        for (auto& cross : m_crossable_y) {
            auto cands = get_candidates_y(cross.first, cross.second);
            if (cands.empty()) {
                if (m_board.must_be_cross(cross.first, cross.second))
                    return false;
            } else if (cands.size() == 1 && cands[0].m_word.size() == 1) {
                if (m_board.must_be_cross(cross.first, cross.second))
                    return false;
            } else {
                candidates.insert(candidates.end(), cands.begin(), cands.end());
            }
        }

        if (m_words.empty()) {
            if (fixup_candidates(candidates)) {
                board_t board0 = m_board;
                board0.trim();
                board0.replace('?', '#');
                if (is_solution(board0)) {
                    std::lock_guard<std::mutex> lock(s_mutex);
                    s_generated = true;
                    s_solution = board0;
                    return true;
                }
            }
            return s_generated;
        }

        for (auto& cand : candidates) {
            generation_t copy(*this);
            copy.apply_candidate(cand);
            if (copy.generate_recurse())
                return true;
        }

        return false;
    }

    bool generate() {
        auto words = m_words;
        if (words.empty())
            return false;

        for (auto& word : m_words) {
            if (word.size() <= 1) {
                words.erase(word);
            }
        }

        if (words.empty())
            return false;

        auto word = *words.begin();
        candidate_t cand = { 0, 0, false, word };
        apply_candidate(cand);
        if (!generate_recurse())
            return false;

        std::lock_guard<std::mutex> lock(s_mutex);
        s_solution.print();
        return true;
    }

    bool is_solution(const board_t& board) const {
        for (int y = board.m_y0; y < board.m_y0 + board.m_cy; ++y) {
            for (int x = board.m_x0; x < board.m_x0 + board.m_cx; ++x) {
                auto ch = board.get_on(x, y);
                if (!is_letter(ch) && ch != '#')
                    return false;
            }
        }

        std::unordered_set<xstring_t> words;

        for (int y = board.m_y0; y < board.m_y0 + board.m_cy; ++y) {
            for (int x = board.m_x0; x < board.m_x0 + board.m_cx - 1; ++x) {
                auto ch0 = board.get_on(x, y);
                auto ch1 = board.get_on(x + 1, y);
                xstring_t word;
                word += ch0;
                word += ch1;
                if (is_letter(ch0) && is_letter(ch1)) {
                    ++x;
                    for (;;) {
                        ++x;
                        ch1 = board.get_on(x, y);
                        if (!is_letter(ch1))
                            break;
                        word += ch1;
                    }
                    if (words.count(word) > 0 || m_dict.count(word) == 0) {
                        return false;
                    }
                    words.insert(word);
                }
            }
        }

        for (int x = board.m_x0; x < board.m_x0 + board.m_cx; ++x) {
            for (int y = board.m_y0; y < board.m_y0 + board.m_cy - 1; ++y) {
                auto ch0 = board.get_on(x, y);
                auto ch1 = board.get_on(x, y + 1);
                xstring_t word;
                word += ch0;
                word += ch1;
                if (is_letter(ch0) && is_letter(ch1)) {
                    ++y;
                    for (;;) {
                        ++y;
                        ch1 = board.get_on(x, y);
                        if (!is_letter(ch1))
                            break;
                        word += ch1;
                    }
                    if (words.count(word) > 0 || m_dict.count(word) == 0) {
                        return false;
                    }
                    words.insert(word);
                }
            }
        }

        return words.size() == m_dict.size();
    }

    static bool do_generate(const std::unordered_set<xstring_t>& words) {
        generation_t data;
        data.m_words = data.m_dict = words;
        return data.generate();
    }
};
