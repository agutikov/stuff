#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <format>
#include <numeric>
#include <string>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <vector>

// Fast, variadic printf replacement using write() system call and std::format
template<typename... Args>
void printF(std::format_string<Args...> fmt, Args&&... args)
{
    // Format the string using std::format
    std::string formatted = std::format(fmt, std::forward<Args>(args)...);

    // Write directly to stdout (file descriptor 1)
    write(STDOUT_FILENO, formatted.data(), formatted.size());
}

// Global application configuration
struct AppConfig
{
    long iterations = 1000;     // Number of nanosleep calls to perform
    long nanoseconds = 1000000; // Number of nanoseconds to sleep in each call
    int worker_threads = 0;     // Number of CPU-intensive worker threads
};

// Print usage information
void print_usage(const char* program_name)
{
    printF("Usage: {} <iterations> <nanoseconds> [worker_threads]\n", program_name);
    printF("  iterations     - Number of nanosleep calls to perform\n");
    printF("  nanoseconds    - Number of nanoseconds to sleep in each call\n");
    printF("  worker_threads - Number of CPU-intensive worker threads (default: 0)\n");
}

// Parse and validate command line arguments
bool parse_arguments(int argc, char* argv[], AppConfig& config)
{
    if (argc < 3 || argc > 4) {
        print_usage(argv[0]);
        return false;
    }

    config.iterations = std::atol(argv[1]);
    config.nanoseconds = std::atol(argv[2]);

    // Parse optional worker_threads argument
    config.worker_threads = 0; // Default value
    if (argc >= 4) {
        config.worker_threads = std::atoi(argv[3]);
    }

    if (config.iterations <= 0 || config.nanoseconds < 0 || config.worker_threads < 0) {
        printF("Error: iterations must be positive, nanoseconds and worker_threads must be non-negative\n");
        print_usage(argv[0]);
        return false;
    }

    return true;
}

// CPU-intensive worker function
void cpu_worker(std::atomic<bool>& should_stop)
{
    // Dummy variables for computation
    double result = 0.0;

    // Keep doing meaningless calculations until told to stop
    while (!should_stop.load(std::memory_order_relaxed)) {
        // CPU-intensive operations that are hard to optimize away
        for (int i = 0; i < 10000; ++i) {
            result += std::sin(result + i) * std::cos(result * i);
            result = std::fmod(result, 10.0); // Keep result bounded
        }
    }

    // Use the result to prevent the compiler from optimizing away the loop
    volatile double dummy = result;
}

// Structure to hold information about interrupted calls
struct InterruptionStats
{
    int count = 0;                 // Number of interrupted calls
    double total_remaining_ns = 0; // Total remaining time in nanoseconds
};

// Structure to hold benchmark results
struct BenchmarkResult
{
    std::vector<double> durations;   // Individual duration measurements
    InterruptionStats interruptions; // Statistics about interrupted calls
};

// Run the benchmark and collect measurements
BenchmarkResult run_benchmark(const AppConfig& config)
{
    BenchmarkResult result;

    // Prepare timespec structure for nanosleep
    struct timespec req, rem;
    req.tv_sec = config.nanoseconds / 1000000000;
    req.tv_nsec = config.nanoseconds % 1000000000;

    // Reserve space for duration measurements
    result.durations.reserve(config.iterations);

    // Run the benchmark
    for (long i = 0; i < config.iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        int ret = nanosleep(&req, &rem);

        auto end = std::chrono::high_resolution_clock::now();

        if (ret != 0) {
            // Collect interruption statistics
            result.interruptions.count++;
            double remaining_ns = rem.tv_sec * 1e9 + rem.tv_nsec;
            result.interruptions.total_remaining_ns += remaining_ns;
        }

        // Calculate duration in nanoseconds
        std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        result.durations.push_back(duration.count());
    }

    return result;
}

// Structure to hold statistics
struct Statistics
{
    double mean = 0.0;
    double min = 0.0;
    double p50 = 0.0;
    double p90 = 0.0;
    double p95 = 0.0;
    double p99 = 0.0;
    double max = 0.0;
    double overhead = 0.0;

    // Interruption statistics
    int interruption_count = 0;
    double total_remaining_ns = 0.0;
    double avg_remaining_ns = 0.0;
    double interruption_rate = 0.0; // Percentage of calls interrupted
};

// Calculate statistics from benchmark results
Statistics calculate_statistics(const BenchmarkResult& benchmark, long nanoseconds)
{
    Statistics stats;
    const auto& durations = benchmark.durations;
    const auto& interruptions = benchmark.interruptions;

    // Calculate mean using std::accumulate
    double sum = std::accumulate(durations.begin(), durations.end(), 0.0);
    stats.mean = sum / durations.size();

    // Create a copy for percentile calculations
    auto sorted_durations = durations;
    std::sort(sorted_durations.begin(), sorted_durations.end());

    // Calculate percentiles
    size_t size = sorted_durations.size();
    stats.min = sorted_durations.front();
    stats.p50 = sorted_durations[size * 50 / 100];
    stats.p90 = sorted_durations[size * 90 / 100];
    stats.p95 = sorted_durations[size * 95 / 100];
    stats.p99 = sorted_durations[size * 99 / 100];
    stats.max = sorted_durations.back();

    // Calculate overhead
    stats.overhead = stats.mean - nanoseconds;

    // Add interruption statistics
    stats.interruption_count = interruptions.count;
    stats.total_remaining_ns = interruptions.total_remaining_ns;
    stats.avg_remaining_ns = interruptions.count > 0 ? interruptions.total_remaining_ns / interruptions.count : 0;
    stats.interruption_rate = 100.0 * interruptions.count / size;

    return stats;
}

// Print results using printF
void print_results(const Statistics& stats, const AppConfig& config)
{
    printF("\nResults (all times in nanoseconds):\n");
    printF("  Requested sleep time: {:.2f}\n", static_cast<double>(config.nanoseconds));
    printF("  Worker threads:       {}\n", config.worker_threads);
    printF("  Average actual time:  {:.2f}\n", stats.mean);
    printF("  Percentiles:\n");
    printF("    Min:                {:.2f}\n", stats.min);
    printF("    50th (median):      {:.2f}\n", stats.p50);
    printF("    90th:               {:.2f}\n", stats.p90);
    printF("    95th:               {:.2f}\n", stats.p95);
    printF("    99th:               {:.2f}\n", stats.p99);
    printF("    Max:                {:.2f}\n", stats.max);
    printF("  Overhead:             {:.2f}\n", stats.overhead);

    // Print interruption statistics
    printF("\nInterruption Statistics:\n");
    printF("  Interrupted calls:    {}\n", stats.interruption_count);
    printF("  Interruption rate:    {:.2f}%\n", stats.interruption_rate);
    if (stats.interruption_count > 0) {
        printF("  Total remaining time: {:.2f} ns\n", stats.total_remaining_ns);
        printF("  Avg remaining time:   {:.2f} ns\n", stats.avg_remaining_ns);
    }
}

int main(int argc, char* argv[])
{
    // Parse command line arguments
    AppConfig config;
    if (!parse_arguments(argc, argv, config)) {
        return 1;
    }

    printF("Running {} iterations with {} nanoseconds sleep time...\n", config.iterations, config.nanoseconds);

    // Start worker threads if requested
    std::atomic<bool> should_stop{ false };
    std::vector<std::jthread> workers;

    if (config.worker_threads > 0) {
        printF("Starting {} CPU-intensive worker threads...\n", config.worker_threads);
        workers.reserve(config.worker_threads);

        for (int i = 0; i < config.worker_threads; ++i) {
            // Using std::jthread instead of std::thread (C++20)
            // std::jthread automatically joins on destruction
            workers.emplace_back([&should_stop]() { cpu_worker(should_stop); });
        }
    }

    // Run the benchmark
    BenchmarkResult benchmark = run_benchmark(config);

    // Stop worker threads
    if (config.worker_threads > 0) {
        printF("Stopping worker threads...\n");
        should_stop.store(true, std::memory_order_relaxed);

        // No need to manually join threads when using std::jthread
        // They will be automatically joined when the vector is destroyed
    }

    // Calculate statistics
    Statistics stats = calculate_statistics(benchmark, config.nanoseconds);

    // Print results
    print_results(stats, config);

    return 0;
}
