// #include <seastar/core/app-template.hh>
// #include <seastar/core/reactor.hh>

#include "toolkit.hh"

#include <algorithm>
#include <ctime>
#include <exception>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <utility>
#include <time.h>

namespace tsdb {

struct Time {
    i64 ns; // ns sense unix epoch (UTC)

    constexpr auto hour() -> i64 {
        return ns / 3'600'000'000'000LL;
    };

    static constexpr auto from_hour(i64 h) -> Time {
        return Time { h * 3'600'000'000'000LL };
    }

    
    constexpr auto operator<=>(const Time&) const = default;
};

inline Time now() {
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    return Time {
        ts.tv_sec * 1'000'000'000LL + ts.tv_nsec
    };
}

template<typename T>
struct TimeSeries {
private:
    std::vector<T> points;

public:
    template<typename U>
    auto append( U&& args ) -> void {
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
    // map (type) -> timeseries of (type)
    Time hour;
    std::tuple<TimeSeries<Ts>...> _series_map;

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
    // map (hour) -> (Segment); Segmant -> map (type) -> (list of points of type)
    // std::unordered_map<u64, Segment<Ts...>> _segment_map;
    std::vector<Segment<Ts...>> _segment_map; // sorted

public:
    template<typename U>
    auto append( U&& data_point ) {
        using T = std::remove_cvref_t<U>;
    
        auto point_hour = Time::from_hour(data_point.time.hour());
    
        auto seg_it = std::lower_bound(
                _segment_map.begin(),
                _segment_map.end(),
                point_hour,
                [](const Segment<Ts...>& seg, const Time& t) {
                    return seg.hour < t;
                }
            );

        if(seg_it == _segment_map.end() || seg_it->hour > point_hour) {
            seg_it = _segment_map.insert(seg_it, Segment<Ts...>{
                .hour = point_hour,
                ._series_map = {}
            });
        }

        seg_it
            ->template get_series<T>()
            .append(std::forward<U>(data_point));
    }

    template<typename U>
    auto get_range( Time t1, Time t2 ) -> std::vector<U> {
        return {};
    }
};

};

struct SimpleDataPoint {
    sstring    name;
    tsdb::Time time;

    int        data;
};

auto main() -> int {
    tsdb::TSDB<SimpleDataPoint> t = {};

    SimpleDataPoint d {
        .name = "point_1",
        .time = tsdb::now(),
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
