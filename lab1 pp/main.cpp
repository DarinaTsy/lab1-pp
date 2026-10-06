#include <pthread.h>
#pragma comment(lib,"pthreadVCE2.lib")
#include <sched.h>
#include <semaphore.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <clocale>


const double A = 5.0;
const double B = 15.0;

double f(double x) {
    return (1.0 + x) / (std::pow(2.0 + 3.0 * x, 2.0) * std::sqrt(2.0 + x));
}

struct ThreadArgs {
    int start;
    int end;
    double a;
    double h;
};

void* left_rect_part(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    double sum = 0.0;
    for (int i = args->start; i < args->end; ++i) {
        double x = args->a + i * args->h;
        sum += f(x);
    }
    double* result = new double(sum);
    return static_cast<void*>(result);
}

double compute_integral(int n, int numThreads, double& elapsed) {
    double h = (B - A) / n;
    std::vector<pthread_t> threads(numThreads);
    std::vector<ThreadArgs> args(numThreads);

    int base = n / numThreads;
    int rem = n % numThreads;
    int start = 0;
    for (int i = 0; i < numThreads; ++i) {
        int chunk = base + (i < rem ? 1 : 0);
        args[i].start = start;
        args[i].end = start + chunk;
        args[i].a = A;
        args[i].h = h;
        start += chunk;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        pthread_create(&threads[i], nullptr, left_rect_part, &args[i]);
    }

    double total = 0.0;
    for (int i = 0; i < numThreads; ++i) {
        void* ret;
        pthread_join(threads[i], &ret);
        double* part = static_cast<double*>(ret);
        total += *part;
        delete part;
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration<double>(t2 - t1).count();

    return total * h;
}

int main() {
    std::setlocale(LC_ALL, "Russian");

    int n, mainThreads, maxThreads;
    std::cout << "Введите количество разбиений n: ";
    std::cin >> n;
    std::cout << "Введите количество потоков для основного вычисления: ";
    std::cin >> mainThreads;
    std::cout << "Введите максимальное количество потоков для тестирования: ";
    std::cin >> maxThreads;

    std::cout << "\nPOSIX Threads\n";
    double time1 = 0.0, res1 = 0.0;

    std::cout << std::setw(8) << "Потоки"
        << std::setw(12) << "Время(с)"
        << std::setw(15) << "Результат"
        << std::setw(12) << "Ускорение"
        << std::setw(20) << "Эффективность(%)\n";

    for (int t = 1; t <= maxThreads; ++t) {
        double elapsed;
        double res = compute_integral(n, t, elapsed);
        if (t == 1) { time1 = elapsed; res1 = res; }

        double speedup = time1 / elapsed;
        double efficiency = speedup / t * 100.0;

        std::cout << std::setw(8) << t
            << std::setw(12) << std::fixed << std::setprecision(6) << elapsed
            << std::setw(15) << std::setprecision(10) << res
            << std::setw(12) << std::setprecision(4) << speedup
            << std::setw(15) << std::setprecision(2) << efficiency << "\n";
    }

    double mainElapsed;
    double mainRes = compute_integral(n, mainThreads, mainElapsed);

    std::cout << "\nОсновное вычисление (" << mainThreads << " потоков):\n";
    std::cout << "Результат: " << std::setprecision(10) << mainRes << "\n";
    std::cout << "Время: " << std::fixed << std::setprecision(6) << mainElapsed << " с\n";
    std::cout << "Однопоточное время: " << time1 << " с, результат: " << res1 << "\n";
    std::cout << "Ускорение: " << time1 / mainElapsed << "\n";

    return 0;
}