#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <limits>

using namespace std;

struct Stats {
    double min;
    double max;
    double sum;
    int count;
};

int main() {
    ifstream file("measurements.txt");
    if (!file.is_open()) {
        cerr << "Error opening file" << endl;
        return 1;
    }

    map<string, Stats> data;
    string line;

    while (getline(file, line)) {
        istringstream iss(line);
        string place;
        double measurement;
        if (getline(iss, place, ';') && (iss >> measurement)) {
            auto& stats = data[place];
            if (stats.count == 0) {
                stats.min = stats.max = measurement;
            } else {
                stats.min = min(stats.min, measurement);
                stats.max = max(stats.max, measurement);
            }
            stats.sum += measurement;
            stats.count++;
        }
    }
    file.close();

    for (const auto& [place, stats] : data) {
        double average = stats.sum / stats.count;
        // cout << fixed << setprecision(10);
        cout << "Place: " << place << "\n"
                  << "Min: " << stats.min << "\n"
                  << "Max: " << stats.max << "\n"
                  << setprecision(10) << "Average: " << average << "\n" << endl;
    }

    return 0;
}
