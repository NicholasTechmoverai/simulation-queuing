#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <string>
#include <sstream>

using namespace std;

struct TimeData {
    vector<int> time;
    vector<double> probability;
};

TimeData inter_arrival_time = {
    {2, 3, 4, 5, 6},
    {0.15, 0.25, 0.20, 0.25, 0.15}
};

TimeData service_time = {
    {2, 3, 4, 5, 6},
    {0.10, 0.25, 0.30, 0.20, 0.15}
};

vector<int> getRandomLCG(int a, int c, int M, int seed, int count) {
    vector<int> nums;
    int x = seed;
    for (int i = 0; i < count; i++) {
        x = (a * x + c) % M;
        nums.push_back(x % 100);
    }
    return nums;
}

vector<double> createCumulativeProbability(const vector<double>& probability) {
    vector<double> cumulative;
    double total = 0.0;
    for (double p : probability) {
        total += p;
        cumulative.push_back(round(total * 100) / 100.0);
    }
    return cumulative;
}

vector<pair<int, int>> createRange(const vector<double>& cumulative_prob) {
    vector<pair<int, int>> ranges;
    int prev = 0;
    for (double cp : cumulative_prob) {
        int upper = static_cast<int>(cp * 100) - 1;
        ranges.push_back(make_pair(prev, upper));
        prev = upper + 1;
    }
    return ranges;
}

int getTimeFromRandomNumber(int rn, const vector<pair<int, int>>& ranges, const vector<int>& values) {
    for (size_t i = 0; i < ranges.size(); i++) {
        if (rn >= ranges[i].first && rn <= ranges[i].second) {
            return values[i];
        }
    }
    return values.back();
}

pair<int, int> parse_time(double t) {
    int hours = static_cast<int>(t);
    int minutes = static_cast<int>((t - hours) * 100 + 0.5);
    return make_pair(hours, minutes);
}

double addTime(double t1, double t2) {
    auto [h1, m1] = parse_time(t1);
    auto [h2, m2] = parse_time(t2);

    int total_min = m1 + m2;
    int extra_hr = total_min / 60;
    int final_min = total_min % 60;

    int final_hr = h1 + h2 + extra_hr;
    return final_hr + final_min / 100.0;
}

double subtractTime_hours_decimal(double t2, double t1) {
    auto [h1, m1] = parse_time(t1);
    auto [h2, m2] = parse_time(t2);

    int t1_minutes = h1 * 60 + m1;
    int t2_minutes = h2 * 60 + m2;

    double diff = t2_minutes - t1_minutes;
    return round((diff / 60.0) * 100) / 100.0;
}

string format_time(double time) {
    auto [hours, minutes] = parse_time(time);
    stringstream ss;
    ss << hours << "." << setw(2) << setfill('0') << minutes;
    return ss.str();
}

void simulate(int sim_count, int a, int c, int M, int seed) {
    vector<int> rn_arrival = getRandomLCG(a, c, M, seed, sim_count);
    vector<int> rn_service = getRandomLCG(a, c, M, seed + 1, sim_count);

    vector<double> cum_IA = createCumulativeProbability(inter_arrival_time.probability);
    vector<pair<int, int>> range_IA = createRange(cum_IA);

    vector<double> cum_ST = createCumulativeProbability(service_time.probability);
    vector<pair<int, int>> range_ST = createRange(cum_ST);

    vector<int> IA_times;
    vector<int> ST_times;
    
    for (int rn : rn_arrival) {
        IA_times.push_back(getTimeFromRandomNumber(rn, range_IA, inter_arrival_time.time));
    }
    
    for (int rn : rn_service) {
        ST_times.push_back(getTimeFromRandomNumber(rn, range_ST, service_time.time));
    }

    vector<double> arrival_times;
    vector<double> service_start_times;
    vector<double> service_end_times;
    vector<double> patient_waiting_times;
    vector<double> system_idle_times;

    arrival_times.push_back(addTime(7.00, IA_times[0]));

    for (int i = 0; i < sim_count; i++) {
        if (i > 0) {
            arrival_times.push_back(addTime(arrival_times[i-1], IA_times[i]));
        }

        if (i == 0) {
            service_start_times.push_back(arrival_times[0]);
            system_idle_times.push_back(0);
        } else {
            auto [a_hours, a_minutes] = parse_time(arrival_times[i]);
            auto [prev_hours, prev_minutes] = parse_time(service_end_times[i-1]);
            
            int a_total_minutes = a_hours * 60 + a_minutes;
            int prev_total_minutes = prev_hours * 60 + prev_minutes;
            
            if (prev_total_minutes > a_total_minutes) {
                service_start_times.push_back(service_end_times[i-1]);
                system_idle_times.push_back(0);
            } else {
                service_start_times.push_back(arrival_times[i]);
                double idle_time = subtractTime_hours_decimal(arrival_times[i], service_end_times[i-1]);
                system_idle_times.push_back(round(idle_time * 100) / 100.0);
            }
        }

        double wait_time = subtractTime_hours_decimal(service_start_times[i], arrival_times[i]);
        patient_waiting_times.push_back(round(wait_time * 100) / 100.0);

        double end_time = addTime(service_start_times[i], ST_times[i]);
        service_end_times.push_back(end_time);
    }

    cout << "\nSIMULATION TABLE\n";
    cout << setw(3) << "No" << setw(9) << "RN(IA)" << setw(8) << "IA" 
         << setw(10) << "Arrival" << setw(9) << "RN(ST)" << setw(8) << "ST" 
         << setw(10) << "Start" << setw(10) << "End" 
         << setw(11) << "Wait(hrs)" << setw(11) << "Idle(hrs)" << endl;
    cout << string(95, '-') << endl;

    for (int i = 0; i < sim_count; i++) {
        cout << setw(3) << i+1 
             << setw(8) << rn_arrival[i] 
             << setw(8) << IA_times[i] 
             << setw(10) << format_time(arrival_times[i])
             << setw(8) << rn_service[i] 
             << setw(8) << ST_times[i] 
             << setw(10) << format_time(service_start_times[i])
             << setw(10) << format_time(service_end_times[i])
             << setw(11) << fixed << setprecision(2) << patient_waiting_times[i]
             << setw(11) << fixed << setprecision(2) << system_idle_times[i] << endl;
    }

    double total_wait = 0, total_idle = 0;
    for (int i = 0; i < sim_count; i++) {
        total_wait += patient_waiting_times[i];
        total_idle += system_idle_times[i];
    }
    
    double avg_wait = total_wait / sim_count;
    double avg_idle = total_idle / sim_count;
    
    cout << fixed << setprecision(2);
    cout << "\nTotal Patient Waiting Time: " << total_wait << " hours\n";
    cout << "Total System Idle Time: " << total_idle << " hours\n";
    cout << "Average Waiting Time per Patient: " << avg_wait << " hours\n";
    cout << "Average System Idle Time per Patient: " << avg_idle << " hours\n";
}

int main() {
    int seed = 427;
    int a = 50;
    int c = 40;
    int M = 1000;
    int sim_count = 78;
    
    simulate(sim_count, a, c, M, seed);
    
    return 0;
}