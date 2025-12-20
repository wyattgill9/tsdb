#include <iostream>
#include <print>

#include <seastar/core/app-template.hh>
#include <seastar/core/reactor.hh>

#include "toolkit.hh"

struct DataPoint {
    u64 time;
    int data;
};

struct Series {
private:
    std::vector<DataPoint> points;

public:
    auto append(DataPoint point) {
        points.push_back(point);
    }

    auto get_range(u64 start_time, u64 end_time) -> std::vector<DataPoint> {
        std::vector<DataPoint> out;
        for(auto& point : points) {
            if(point.time >= start_time && point.time <= end_time) {
                out.push_back(point);
            }
        }
        return out;
    }
};

seastar::future<> core() {
    Series s;

    DataPoint d {
        .time = 100,
        .data = 5
    };

    DataPoint p {
        .time = 101,
        .data = 6
    };

    s.append(d);
    s.append(p);

    auto range = s.get_range(100, 101);

    for(auto& point : range) {
        std::println("{}", point.data);
    }

    return seastar::make_ready_future<>();
}

auto main(int argc, char** argv) -> int {
    seastar::app_template app;
    try {
        app.run(argc, argv, core);
    } catch(...) {
        std::cerr << "Couldn't start application: "
                  << std::current_exception() << "\n";
        return 1;
    }
    return 0;
}
