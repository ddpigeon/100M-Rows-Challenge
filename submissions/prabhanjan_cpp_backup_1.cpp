#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <iomanip>
#include <iostream>
#include <cstring>
#include <chrono>
#include <set>
#include <thread>
using ll = long long;
const int numproc = 4;

typedef struct temperature_data {
    ll sum;
    int max_temp;
    int min_temp;
    int count;
} tdata;

typedef struct mmap_range {
    const char* start;
    const char* end;
} mmap_range;

using ufm = boost::unordered::unordered_flat_map<std::string, tdata>;
ufm temps;
//std::set<ll> lc;
//std::mutex lc_mutex;

void process_range(mmap_range a, ufm &local_temps) {
    // longest line is 28 for me so I'll just take say 35 chars and parse(?)
    
    const char* startpt = a.start;
    //std::cout << *startpt;
    //std::cout << "Thread processing range from " << (ll) startpt << " to " << (ll) a.end << "\n";
    int ctr = 0;
    while (startpt < a.end) {
        const char* semipos = static_cast<const char*>(memchr(startpt, ';', 35));
        const char* lineend = static_cast<const char*>(memchr(semipos, '\n', 7));
        //std::lock_guard<std::mutex> lock(lc_mutex);
        //lc.insert((ll) lineend);

        std::string city(startpt, semipos - startpt);
        int temperature = (atoi(semipos+1) * 10);
        int decimal_point = *(lineend-1) - '0';
        if (temperature > 0) temperature += decimal_point;
        else temperature -= decimal_point;

        if (local_temps.find(city) == local_temps.end()) {
            local_temps[city] = {temperature, temperature, temperature, 1};
        }
        else {
            local_temps[city].count++;
            local_temps[city].sum += temperature;
            if (local_temps[city].max_temp < temperature) local_temps[city].max_temp = temperature;
            if (local_temps[city].min_temp > temperature) local_temps[city].min_temp = temperature;
        }

        startpt = lineend + 1;
    }
    //std::cout << "Done\n";
}

void parallel_process(std::array<mmap_range, numproc> &ranges) {
    std::vector<std::jthread> workers(numproc);
    std::vector<ufm> temp_maps(numproc);

    for (int i = 0; i < numproc; i++) {
        workers[i] = std::jthread([&, idx=i]() {process_range(ranges[idx], temp_maps[idx]);}); 
    }
    workers.clear();


    for (ufm &tmap: temp_maps) {
        for (const auto& [key, val] : tmap) {
            if (temps.find(key) == temps.end()) {
                temps.emplace(key, val);
            }
            else {
                temps[key].count += val.count;
                temps[key].sum += val.sum;
                temps[key].max_temp = std::max(temps[key].max_temp, val.max_temp);
                temps[key].min_temp = std::min(temps[key].min_temp, val.min_temp);
            }
        }
    }
}

int main() {
    auto begintime = std::chrono::high_resolution_clock().now();

    boost::iostreams::mapped_file mmap_data("measurements.txt", boost::iostreams::mapped_file::readonly);
    auto f = mmap_data.const_data();
    auto sz = mmap_data.size();
    auto l = f + sz;
    std::array<mmap_range, numproc> ranges;
    ranges[0].start = f;
    ranges[numproc - 1].end = l;

    for (int i = 1; i < numproc; i++) {
        auto st = f + (sz * i / numproc);
        ranges[i-1].end = static_cast<const char*>(memchr(st, '\n', l-st));
        ranges[i].start = ranges[i-1].end + 1;
    }

    parallel_process(ranges);

    ll tcount = 0;

    for (const auto& [place, stats] : temps) {
        tcount += stats.count;
        double average = (double) stats.sum / (double) (10 * stats.count);
        std::cout << "Place: " << place << "\n"
                  << "Min: " << stats.min_temp / 10 << "." << abs(stats.min_temp) % 10 << "\n"
                  << "Max: " << stats.max_temp / 10 << "." << abs(stats.max_temp) % 10 << "\n"
                  << std::setprecision(10) << "Average: " << stats.sum << " / " << stats.count << "\n\n";
    }

    auto endtime = std::chrono::high_resolution_clock().now();
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(endtime - begintime);

    std::cout << "time = " << total_time.count() << '\n';
    std::cout << tcount << '\n';
}
