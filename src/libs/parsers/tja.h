#pragma once

#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <deque>
#include <filesystem>
#include <functional>
#include <limits>
#include <map>
#include <optional>
#include <regex>
#include <string>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;

struct Modifiers {
    bool auto_play = false;
    int speed = 10;
    bool display = false;
    bool inverse = false;
    int random = 0;
    int subdiff = 0;
    bool skip = false;
};

enum class NoteType : int {
    BARLINE = 0,
    DON = 1,
    KAT = 2,
    DON_L = 3,
    KAT_L = 4,
    ROLL_HEAD = 5,
    ROLL_HEAD_L = 6,
    BALLOON_HEAD = 7,
    TAIL = 8,
    KUSUDAMA = 9
};

enum class ScrollType : int {
    NMSCROLL = 0,
    BMSCROLL = 1,
    HBSCROLL = 2
};

struct TimelineObject {
    double start_time;
    double end_time;

    std::optional<double> bpm;
    std::optional<std::string> branch_params;
    std::optional<bool> gogo_time;
    std::optional<bool> section_reset;

    std::optional<double> judge_pos_x;
    std::optional<double> judge_pos_y;
    std::optional<double> delta_x;
    std::optional<double> delta_y;

    std::optional<std::string> lyric;

};

class Note {
public:
    NoteType type;
    double hit_ms;
    double load_ms;
    double unload_ms;
    double bpm;
    double scroll_x;
    double scroll_y;
    std::optional<double> sudden_appear_ms;
    std::optional<double> sudden_moving_ms;
    bool display;
    int index;
    int moji;
    bool is_branch_start;
    // Drumroll specific
    std::optional<int> color;
    float roll_intensity = 0.0f;
    // Balloon specific
    std::optional<int> count;
    std::optional<bool> popped;

    Note() : type(NoteType::BARLINE), hit_ms(0.0f), load_ms(0.0f), unload_ms(0.0f),
             bpm(0.0f), scroll_x(0.0f), scroll_y(0.0f),
             display(true), index(0), moji(0),
             is_branch_start(false) {}

    virtual ~Note() = default;

    bool operator<(const Note& other) const {
        return hit_ms < other.hit_ms;
    }
    bool operator<=(const Note& other) const {
        return hit_ms <= other.hit_ms;
    }
    bool operator>(const Note& other) const {
        return hit_ms > other.hit_ms;
    }
    bool operator>=(const Note& other) const {
        return hit_ms >= other.hit_ms;
    }
    bool operator==(const Note& other) const {
        return hit_ms == other.hit_ms && index == other.index;
    }

    size_t hash() const {
        size_t h = 0;
        auto combine = [&](size_t v) {
            h ^= v + 0x9e3779b9 + (h << 6) + (h >> 2);
        };
        combine(std::hash<double>{}(bpm));
        combine(std::hash<double>{}(hit_ms));
        combine(std::hash<double>{}(scroll_x));
        combine(std::hash<double>{}(scroll_y));
        combine(std::hash<int>{}(static_cast<int>(type)));
        return h;
    }
};

namespace std {
    template<>
    struct hash<Note> {
        size_t operator()(const Note& note) const {
            return note.hash();
        }
    };
}

struct CompareNotes {
    bool operator()(const Note& a, const Note& b) const { return a < b; }

    template<typename T, typename U>
    bool operator()(const T& a, const U& b) const {
        return a.hit_ms < b.hit_ms;
    }
};

struct TempoPoint {
    double ms;
    double beat;
    double bpm;  // 0 while a #DELAY holds the beat still
};

// Chart time to beat count, for #BMSCROLL / #HBSCROLL where notes scroll by beat
struct TempoMap {
    std::vector<TempoPoint> points;

    double beat_at(double ms) const {
        if (points.empty()) return 0.0;
        auto it = std::upper_bound(points.begin(), points.end(), ms,
            [](double v, const TempoPoint& p) { return v < p.ms; });
        const TempoPoint& p = it == points.begin() ? *it : *std::prev(it);
        return p.beat + (ms - p.ms) * p.bpm / 60000.0;
    }

    // The stretch of chart time around `ms` during which the beat count stays within [lo, hi]
    // (`ms` itself must be inside). A negative BPM runs the beat count back, so a chart can pass
    // the same beat many times; only the pass around `ms` counts.
    std::pair<double, double> ms_span_within(double ms, double lo, double hi) const {
        constexpr double inf = std::numeric_limits<double>::infinity();
        if (points.empty()) return {-inf, inf};
        auto it = std::upper_bound(points.begin(), points.end(), ms,
            [](double v, const TempoPoint& p) { return v < p.ms; });
        size_t seg = it == points.begin() ? 0 : size_t(it - points.begin()) - 1;
        // Chart times at which segment i's beat line is inside [lo, hi]
        auto inside = [&](size_t i) -> std::pair<double, double> {
            const TempoPoint& p = points[i];
            if (p.bpm == 0.0) return (p.beat >= lo && p.beat <= hi) ? std::make_pair(-inf, inf) : std::make_pair(inf, -inf);
            double a = p.ms + (lo - p.beat) * 60000.0 / p.bpm;
            double b = p.ms + (hi - p.beat) * 60000.0 / p.bpm;
            return {std::min(a, b), std::max(a, b)};
        };
        double first = ms;
        for (size_t i = seg + 1; i-- > 0;) {
            double from = i == 0 ? -inf : points[i].ms;
            double enter = inside(i).first;
            if (enter > from) { first = std::min(enter, first); break; }
            first = from;
        }
        double last = ms;
        for (size_t i = seg; i < points.size(); i++) {
            double to = i + 1 < points.size() ? points[i + 1].ms : inf;
            double leave = inside(i).second;
            if (leave < to) { last = std::max(leave, last); break; }
            last = to;
        }
        return {first, last};
    }
};

struct NoteList {
    std::deque<Note> notes;
    std::deque<TimelineObject> timeline;
    ScrollType scroll_type = ScrollType::NMSCROLL;
    TempoMap tempo_map;

    NoteList operator+(const NoteList& other) const {
        NoteList result;
        result.scroll_type = scroll_type;
        result.tempo_map = tempo_map;
        result.notes = notes;
        result.notes.insert(result.notes.end(),
                                 other.notes.begin(),
                                 other.notes.end());

        result.timeline = timeline;
        result.timeline.insert(result.timeline.end(),
                              other.timeline.begin(),
                              other.timeline.end());

        return result;
    }

    NoteList& operator+=(const NoteList& other) {
        notes.insert(notes.end(),
                         other.notes.begin(),
                         other.notes.end());

        timeline.insert(timeline.end(),
                       other.timeline.begin(),
                       other.timeline.end());
        return *this;
    }
};

struct CourseData {
    double level = 0;
    std::vector<int> balloon;
    std::vector<int> scoreinit;
    double scorediff = 0;
    bool is_branching = false;
};

struct TJAMetadata {
    std::map<std::string, std::string> title = {{"en", ""}};
    std::map<std::string, std::string> subtitle = {{"en", ""}};
    bool subtitle_full_display = false;
    std::string genre = "";
    fs::path wave;
    double demostart = 0.0f;
    double offset = 0.0f;
    double bpm = 120.0f;
    fs::path bgmovie;
    double movieoffset = 0.0f;
    fs::path preimage;
    std::string scene_preset = "";
    std::map<int, CourseData> course_data;
};

struct TJAEXData {
    bool new_audio = false;
    bool old_audio = false;
    bool limited_time = false;
};

struct ParserState {
    double time_signature = 4.0f / 4.0f;
    double bpm = 120.0f;
    double scroll_x_modifier = 1.0f;
    double scroll_y_modifier = 0.0f;
    ScrollType scroll_type = ScrollType::NMSCROLL;
    bool barline_display = true;
    std::deque<Note>* curr_note_list = nullptr;
    std::deque<TimelineObject>* curr_timeline = nullptr;
    double index = 0;
    std::vector<int> balloons;
    size_t balloon_cursor = 0;
    double balloon_index = 0;
    size_t branch_balloon_cursor = 0;
    bool barline_added = false;
    double sudden_appear = 0.0f;
    double sudden_moving = 0.0f;
    double judge_pos_x = 0.0f;
    double judge_pos_y = 0.0f;
    bool is_branching = false;
    bool is_section_start = false;
    double start_branch_ms = 0.0f;
    double start_branch_bpm = 120.0f;
    double start_branch_time_sig = 4.0f / 4.0f;
    double start_branch_x_scroll = 1.0f;
    double start_branch_y_scroll = 0.0f;
    bool start_branch_barline = false;
    double branch_balloon_index = 0;
    std::optional<Note> section_bar;
};

enum class Interval {
    UNKNOWN = 0,
    QUARTER = 1,
    EIGHTH = 2,
    TWELFTH = 3,
    SIXTEENTH = 4,
    TWENTYFOURTH = 6,
    THIRTYSECOND = 8
};

class TJAParser {
public:
    static const std::map<int, std::string> DIFFS;

    TJAParser() = default;

    TJAParser(const std::filesystem::path& path, int start_delay = 0, int player_num = 0);

    std::filesystem::path file_path;
    TJAMetadata metadata;
    TJAEXData ex_data;
    bool scroll_disabled = false;

    void get_metadata();
    std::string get_difficulty_name() {
        auto it = DIFFS.find(last_diff);
        return it != DIFFS.end() ? it->second : "";
    }

    using CommandHandler = std::function<void(const std::string&, ParserState&)>;

    std::tuple<NoteList, std::deque<NoteList>, std::deque<NoteList>, std::deque<NoteList>>
    notes_to_position(int diff);
    std::string get_song_hash();
    std::string get_diff_hash(int difficulty);

private:
    double start_ms;
    double current_ms;
    int last_diff = -1;
    NoteList master_notes;
    int player_num;
    std::string encoding;
    std::vector<std::string> data;
    std::deque<NoteList> branch_m;
    std::deque<NoteList> branch_e;
    std::deque<NoteList> branch_n;
    static const std::regex complex_number_regex;

    std::vector<std::string> read_file_lines(const std::filesystem::path& path,
                                             const std::string& encoding);

    static std::string to_lower(const std::string& str);
    static std::string split_after_colon(const std::string& str);
    static std::string join_after_colon(const std::string& str);
    static void replace_all(std::string& str, const std::string& from, const std::string& to);
    static std::string trim(const std::string& str);
    static std::vector<int> parse_balloon_data(const std::string& data);

    std::vector<std::vector<std::string>> data_to_notes(int diff);

    Note* get_note_ptr(Note& variant);
    void add_tempo_point(double bpm);

    void set_branch_params(std::vector<TimelineObject>& bar_list, std::string branch_params,
                          std::optional<Note> section_bar);

    std::map<std::string, CommandHandler> build_command_registry();
    std::vector<std::pair<std::string, CommandHandler>> cached_cmds;

    void handle_MEASURE(const std::string& value, ParserState& state);
    void handle_SCROLL(const std::string& value, ParserState& state);
    void handle_BPMCHANGE(const std::string& value, ParserState& state);
    void handle_GOGOSTART(const std::string& value, ParserState& state);
    void handle_GOGOEND(const std::string& value, ParserState& state);
    void handle_DELAY(const std::string& value, ParserState& state);
    void handle_BARLINEOFF(const std::string& value, ParserState& state);
    void handle_BARLINEON(const std::string& value, ParserState& state);
    void handle_BRANCHSTART(const std::string& value, ParserState& state);
    void handle_BRANCHEND(const std::string& value, ParserState& state);
    void handle_N(const std::string& value, ParserState& state);
    void handle_E(const std::string& value, ParserState& state);
    void handle_M(const std::string& value, ParserState& state);
    void handle_SECTION(const std::string& value, ParserState& state);
    void handle_NMSCROLL(const std::string& value, ParserState& state);
    void handle_BMSCROLL(const std::string& value, ParserState& state);
    void handle_HBSCROLL(const std::string& value, ParserState& state);
    void handle_SUDDEN(const std::string& value, ParserState& state);
    void handle_JPOSSCROLL(const std::string& part, ParserState& state);
    void handle_LYRIC(const std::string& value, ParserState& state);

    Note add_bar(ParserState& state);
    Note add_note(char item, ParserState& state);
};

double get_ms_per_measure(double bpm_val, double time_sig);
int calculate_base_score(const NoteList& notes);
std::string test_encodings(const std::filesystem::path& file_path);
std::string strip_comments(const std::string& code);

Interval get_note_interval_type(double interval_ms, double bpm, double time_sig = 4.0);
std::vector<std::pair<int, int>> find_streams(const std::deque<Note>& modded_notes, Interval interval_type);
void modifier_moji(NoteList& notes);
void modifier_speed(NoteList& notes, float value);
void modifier_display(NoteList& notes);
void modifier_inverse(NoteList& notes);
void modifier_random(NoteList& notes, int value);
void apply_modifiers(NoteList& notes, const Modifiers& modifiers);
