// #include <seastar/core/app-template.hh>
// #include <seastar/core/reactor.hh>

#include "toolkit.hh"

#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <utility>

namespace tsdb {

struct Time {
private:
    // year:    16 bits [0-15]   (allows 0-65535)
    // month:   4 bits  [16-19]  (allows 0-15,  we use 1-12)
    // day:     5 bits  [20-24]  (allows 0-31)
    // hour:    5 bits  [25-29]  (allows 0-31,  we use 0-23)
    // minute:  6 bits  [30-35]  (allows 0-63,  we use 0-59)
    // second:  6 bits  [36-41]  (allows 0-63,  we use 0-59)
    // weekday: 3 bits  [42-44]  (allows 0-7,   we use 1-7)
    // yearday: 9 bits  [45-53]  (allows 0-511, we use 1-366)
    // unused:  10 bits [54-63]

    u64 bit_time;

    // types: (position, width)
    static constexpr auto YEAR    = std::pair { 0, 16 };
    static constexpr auto MONTH   = std::pair {16,  4 };
    static constexpr auto DAY     = std::pair {20,  5 };
    static constexpr auto HOUR    = std::pair {25,  5 };
    static constexpr auto MINUTE  = std::pair {30,  6 };
    static constexpr auto SECOND  = std::pair {36,  6 };
    static constexpr auto WEEKDAY = std::pair {42,  3 };
    static constexpr auto YEARDAY = std::pair {45,  9 };

    [[nodiscard]] constexpr u64 get(std::pair<int, int> type) const {
        auto [pos, width] = type;
        return (bit_time >> pos) & ((1ULL << width) - 1);
    }

    constexpr u64 pack(std::pair<int, int> type, u64 val) const {
        auto [pos, width] = type;
        u64 mask = ((1ULL << width) - 1);
        return (val & mask) << pos;
    }

public:
    constexpr Time() : bit_time(0) {}
    constexpr Time(u64 raw) : bit_time(raw) {}
    
    constexpr Time(u16 y, u8 mon, u8 d, u8 h = 0, u8 min = 0, u8 s = 0) 
        : bit_time(
                pack(YEAR, y)
              | pack(MONTH, mon)
              | pack(DAY, d)
              | pack(HOUR, h)
              | pack(MINUTE, min)
              | pack(SECOND, s)
        ) {}

    [[nodiscard]] constexpr auto year   () const -> u64 { return get(YEAR   ); }
    [[nodiscard]] constexpr auto month  () const -> u64 { return get(MONTH  ); }
    [[nodiscard]] constexpr auto day    () const -> u64 { return get(DAY    ); }
    [[nodiscard]] constexpr auto hour   () const -> u64 { return get(HOUR   ); }
    [[nodiscard]] constexpr auto minute () const -> u64 { return get(MINUTE ); }
    [[nodiscard]] constexpr auto second () const -> u64 { return get(SECOND ); }
    [[nodiscard]] constexpr auto weekday() const -> u64 { return get(WEEKDAY); }
    [[nodiscard]] constexpr auto yearday() const -> u64 { return get(YEARDAY); }

    [[nodiscard]] constexpr auto inner() const -> u64 { return bit_time; }

    [[nodiscard]] friend constexpr auto operator<=>(const Time&, const Time&) = default;
};

template<typename T>
struct TimeSeries {
private:
    std::vector<T> points;

public:
    template<typename U>
    auto append(U&& args) -> void {
        points.emplace_back(std::forward<U>(args));
    }

    auto get_range( Time t1, Time t2 ) -> std::vector<T> {
        std::vector<T> out;
        for(auto& point : points) {
            if(point.time >= t1 && point.time <= t2) {
                out.push_back(point);
            }
        }
        return out;
    }
};

template<typename ...Ts>
struct Segment {
private:
    // map (type) -> timeseries of (type)
    std::tuple<TimeSeries<Ts>...> _series_map;

public:
    template<typename T>
    constexpr inline auto get_series() -> TimeSeries<T>& {
        return std::get<TimeSeries<T>>(_series_map);
    }
};

template<typename T>
concept isValidTimeSeries = requires (T t) {
    { t.time   } -> std::convertible_to<Time>;
};

template<isValidTimeSeries... Ts>
struct TSDB {
private:
    std::unordered_map<u64, Segment<Ts...>> _segment_map;

public:
    template<typename U>
    auto append(U&& data_point) {
        using T = std::remove_cvref_t<U>;
       
        Segment<Ts...>& seg = _segment_map.try_emplace(data_point.time.hour()).first->second;

        seg
            .template get_series<T>()
            .append(std::forward<U>(data_point));
    }

    template<typename U>
    auto get_range( Time t1, Time t2 ) -> std::vector<U> {
        return {};
    }
};

};

// u64 is just simple time idk
struct SimpleDataPoint {
    sstring    name;
    tsdb::Time time;

    i32        data;
};

auto main() -> int {
    tsdb::TSDB<SimpleDataPoint> t = {};

    SimpleDataPoint d {
        .name = "point_1",
        .time = 100,
        .data = 5,  
    };

    t.append(d);

    return 0;
}

// seastar::future<> core( {
//     Series s;

//     DataPoint d {
//         .time = 100,
//         .data = 5
//     };

//     DataPoint p {
//         .time = 101,
//         .data = 6
//     };

//     s.append(d);
//     s.append(p);

//     auto range = s.get_range(100, 101);

//     for(auto& point : range) {
//         std::println("{}", point.data);
//     }

//     return 0;
//     return seastar::make_ready_future<>();
// }

// auto main(int argc, char** argv) -> int {
//     seastar::app_template app;
//     try {
//         app.run(argc, argv, core);
//     } catch(...) {
//         std::cerr << "Couldn't start application: "
//                   << std::current_exception() << "\n";
//         return 1;
//     }
//     return 0;
// }
