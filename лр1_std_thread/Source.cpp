#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <functional>
#include <clocale>

const double A = 5.0;
const double B = 15.0;

double f(double x) {
    return (1.0 + x) / (pow(2.0 + 3.0 * x, 2.0) * sqrt(2.0 + x));
}

// Параметры передаются по отдельности (не в структуре)
void left_rect_part(double a, double h, int start, int end, double& result) {
    double sum = 0.0;
    for (int i = start; i < end; ++i) {
        double x = a + i * h;
        sum += f(x);
    }
    result = sum;
}

double integral(int n, int numThreads, double& elapsed) {
    double h = (B - A) / n;
    std::vector<std::thread> threads;
    std::vector<double> partial(numThreads, 0.0);

    int base = n / numThreads;
    int rem = n % numThreads;
    int start = 0;

    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        int chunk = base + (i < rem ? 1 : 0);
        int end = start + chunk;
        threads.emplace_back(left_rect_part, A, h, start, end, std::ref(partial[i]));
        start = end;
    }
    for (auto& th : threads) {
        th.join();
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration<double>(t2 - t1).count();

    double total = 0.0;
    for (double v : partial) 
        total += v;
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

    std::cout << "\nstd::thread\n";
    double time1 = 0.0, res1 = 0.0;

    std::cout << std::setw(8) << "Потоки"
        << std::setw(12) << "Время(с)"
        << std::setw(15) << "Результат"
        << std::setw(12) << "Ускорение"
        << std::setw(20) << "Эффективность(%)\n";

    for (int t = 1; t <= maxThreads; ++t) {
        double elapsed;
        double res = integral(n, t, elapsed);
        if (t == 1) 
        { 
            time1 = elapsed; 
            res1 = res; 
        }

        double speedup = time1 / elapsed;
        double efficiency = speedup / t * 100.0;

        std::cout << std::setw(8) << t
            << std::setw(12) << std::fixed << std::setprecision(6) << elapsed
            << std::setw(15) << std::setprecision(10) << res
            << std::setw(12) << std::setprecision(4) << speedup
            << std::setw(15) << std::setprecision(2) << efficiency << "\n";
    }

    double mainElapsed;
    double mainRes = integral(n, mainThreads, mainElapsed);

    std::cout << "\nОсновное вычисление (" << mainThreads << " потоков):\n";
    std::cout << "Результат: " << std::setprecision(10) << mainRes << "\n";
    std::cout << "Время: " << std::fixed << std::setprecision(6) << mainElapsed << " с\n";
    std::cout << "Однопоточное время: " << time1 << " с, результат: " << res1 << "\n";
    std::cout << "Ускорение: " << time1 / mainElapsed << "\n";

    return 0;
}