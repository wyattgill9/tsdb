#include <iostream>

#include <seastar/core/app-template.hh>
#include <seastar/core/reactor.hh>

#include "toolkit.hh"

auto main(int argc, char** argv) -> i32 {
    seastar::app_template app;

    app.run(argc, argv, [] {
            std::cout << seastar::smp::count << "\n";
            return seastar::make_ready_future<>();
    });
}
