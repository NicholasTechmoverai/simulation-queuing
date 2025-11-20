inter_arrival_time = {
    "time": [2, 3, 4, 5, 6],
    "probability": [0.15, 0.25, 0.20, 0.25, 0.15]
}

service_time = {
    "time": [2, 3, 4, 5, 6],
    "probability": [0.10, 0.25, 0.30, 0.20, 0.15]
}

def getRandomLCG(a, c, M, seed, count):
    nums = []
    x = seed
    for _ in range(count):
        x = (a * x + c) % M
        nums.append(x % 100)
    return nums

def createCumulativeProbability(probability):
    cumulative, total = [], 0.0
    for p in probability:
        total += p
        cumulative.append(round(total, 2))
    return cumulative

def createRange(cumulative_prob):
    ranges = []
    prev = 0
    for cp in cumulative_prob:
        upper = int(cp * 100) - 1
        ranges.append((prev, upper))
        prev = upper + 1
    return ranges

def getTimeFromRandomNumber(rn, ranges, values):
    for i, (low, high) in enumerate(ranges):
        if low <= rn <= high:
            return values[i]
    return values[-1]

def parse_time(t):
    if isinstance(t, int):
        return 0, t
    s = str(t)
    if "." in s:
        h, m = s.split(".")
        return int(h), int(m)
    return int(float(s)), 0

def addTime(t1, t2):
    h1, m1 = parse_time(t1)
    h2, m2 = parse_time(t2)

    total_min = m1 + m2
    extra_hr = total_min // 60
    final_min = total_min % 60

    final_hr = h1 + h2 + extra_hr
    return float(f"{final_hr}.{final_min:02d}")

def subtractTime_hours_decimal(t2, t1):
    h1, m1 = parse_time(t1)
    h2, m2 = parse_time(t2)

    t1_minutes = h1 * 60 + m1
    t2_minutes = h2 * 60 + m2

    diff = t2_minutes - t1_minutes
    return round(diff / 60, 2)

def simulate(sim_count, a, c, M, seed):
    rn_arrival = getRandomLCG(a, c, M, seed, sim_count)
    rn_service = getRandomLCG(a, c, M, seed + 1, sim_count)

    cum_IA = createCumulativeProbability(inter_arrival_time["probability"])
    range_IA = createRange(cum_IA)

    cum_ST = createCumulativeProbability(service_time["probability"])
    range_ST = createRange(cum_ST)

    IA_times = [getTimeFromRandomNumber(r, range_IA, inter_arrival_time["time"]) for r in rn_arrival]
    ST_times = [getTimeFromRandomNumber(r, range_ST, service_time["time"]) for r in rn_service]

    arrival_times = []
    service_start_times = []
    service_end_times = []
    patient_waiting_times = []
    system_idle_times = []

    arrival_times.append(addTime(7.00, IA_times[0]))

    for i in range(sim_count):
        if i > 0:
            arrival_times.append(addTime(arrival_times[i-1], IA_times[i]))

        if i == 0:
            service_start = arrival_times[0]
            system_idle_times.append(0)
        else:
            a_minutes = parse_time(arrival_times[i])[0]*60 + parse_time(arrival_times[i])[1]
            prev_end_minutes = parse_time(service_end_times[i-1])[0]*60 + parse_time(service_end_times[i-1])[1]
            
            if prev_end_minutes > a_minutes:
                service_start = service_end_times[i-1]
                system_idle_times.append(0)
            else:
                service_start = arrival_times[i]
                idle_time = subtractTime_hours_decimal(arrival_times[i], service_end_times[i-1])
                system_idle_times.append(round(idle_time, 2))

        service_start_times.append(service_start)

        wait_time = subtractTime_hours_decimal(service_start, arrival_times[i])
        patient_waiting_times.append(round(wait_time, 2))

        end_time = addTime(service_start, ST_times[i])
        service_end_times.append(end_time)

    print("\nSIMULATION TABLE")
    header = ("No", "RN(IA)", "IA", "Arrival", "RN(ST)", "ST", "Start", "End", "Wait(hrs)", "Idle(hrs)")
    print("{:>3} {:>8} {:>8} {:>9} {:>8} {:>8} {:>9} {:>9} {:>10} {:>10}".format(*header))
    print("-" * 95)

    for i in range(sim_count):
        print("{:3d} {:8d} {:8d} {:9} {:8d} {:8d} {:9} {:9} {:10.2f} {:10.2f}".format(
            i+1,
            rn_arrival[i],
            IA_times[i],
            arrival_times[i],
            rn_service[i],
            ST_times[i],
            service_start_times[i],
            service_end_times[i],
            patient_waiting_times[i],
            system_idle_times[i]
        ))

    total_wait = sum(patient_waiting_times)
    total_idle = sum(system_idle_times)
    avg_wait = total_wait / sim_count
    avg_idle = total_idle / sim_count
    
    print(f"\nTotal Patient Waiting Time: {total_wait:.2f} hours")
    print(f"Total System Idle Time: {total_idle:.2f} hours")
    print(f"Average Waiting Time per Patient: {avg_wait:.2f} hours")
    print(f"Average System Idle Time per Patient: {avg_idle:.2f} hours")

if __name__ == "__main__":
    seed = 25
    a = 241
    c = 59
    M = 10007 
    sim_count = 50
    simulate(sim_count, a, c, M, seed)