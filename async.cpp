#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <omp.h>
#include <fstream>

std::vector<int> load(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Nie udalo sie otworzyc pliku: " + filepath);
    }

    std::streamsize size_in_bytes = file.tellg();
    file.seekg(0, std::ios::beg);

    size_t num_elements = size_in_bytes / sizeof(int);
    std::vector<int> buffer(num_elements);

    if (!file.read(reinterpret_cast<char*>(buffer.data()), num_elements * sizeof(int))) {
        throw std::runtime_error("Blad odczytu pliku z danymi wejsciowymi.");
    }

    return buffer;
}

void parallelBucketSort(std::vector<int>& arr, size_t num_buckets) {
    size_t n = arr.size();
    if (n <= 1) return;

    int min_val = arr[0], max_val = arr[0];
    #pragma omp parallel
    {
        int local_min = min_val;
        int local_max = max_val;
        #pragma omp for nowait
        for (size_t i = 0; i < n; i++) {
            if (arr[i] < local_min) local_min = arr[i];
            if (arr[i] > local_max) local_max = arr[i];
        }
        #pragma omp critical
        {
            if (local_min < min_val) min_val = local_min;
            if (local_max > max_val) max_val = local_max;
        }
    }
    if (min_val == max_val) return;

    size_t num_threads = omp_get_max_threads();
    std::vector<std::vector<std::vector<int>>> local_buckets(num_threads, std::vector<std::vector<int>>(num_buckets));

    // PREALOKACJA dla wątków lokalnych
    size_t expected_local_size = (n / num_buckets / num_threads) * 1.5; // margines 50% ze względu na odchylenia między wątkami
    for (size_t t = 0; t < num_threads; ++t) {
        for (size_t b = 0; b < num_buckets; ++b) {
            local_buckets[t][b].reserve(expected_local_size);
        }
    }

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        long long range = (long long)max_val - min_val;
        #pragma omp for
        for (size_t i = 0; i < n; i++) {
            // POPRAWKA KRYTYCZNA: Bezpośrednie rzutowanie arr[i] by uniknąć przepełnienia (overflow)
            int bucket_idx = (int)( ((long long)arr[i] - min_val) * (num_buckets - 1) / range );
            local_buckets[tid][bucket_idx].push_back(arr[i]);
        }
    }

    std::vector<std::vector<int>> buckets(num_buckets);
    
    // PREALOKACJA dla kubełków globalnych
    size_t expected_global_size = (n / num_buckets) * 1.2;
    for (int b = 0; b < num_buckets; ++b) {
        buckets[b].reserve(expected_global_size);
    }

    #pragma omp parallel for
    for (size_t b = 0; b < num_buckets; b++) {
        for (size_t t = 0; t < num_threads; t++) {
            buckets[b].insert(buckets[b].end(), local_buckets[t][b].begin(), local_buckets[t][b].end());
        }
    }

    #pragma omp parallel for schedule(dynamic)
    for (size_t b = 0; b < num_buckets; b++) {
        std::sort(buckets[b].begin(), buckets[b].end());
    }

    std::vector<size_t> start_idx(num_buckets, 0);
    for (size_t b = 1; b < num_buckets; b++) {
        start_idx[b] = start_idx[b - 1] + buckets[b - 1].size();
    }

    #pragma omp parallel for
    for (size_t b = 0; b < num_buckets; b++) {
        int idx = start_idx[b];
        for (int val : buckets[b]) {
            arr[idx++] = val;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Blad uzycia. Oczekiwano: " << argv[0] << " <sciezka_do_pliku> <liczba_kubelkow>\n";
        return 1;
    }

    std::string filepath = argv[1];
    size_t num_buckets;
    
    try {
        num_buckets = std::stoi(argv[2]);
        if (num_buckets < 1) throw std::invalid_argument("Liczba kubelkow musi byc dodatnia.");
    } catch (const std::exception& e) {
        std::cerr << "Blad parsowania liczby kubelkow: " << e.what() << "\n";
        return 1;
    }

    try {
        std::vector<int> data = load(filepath);

        double startCouting = omp_get_wtime(); 
        
        parallelBucketSort(data, num_buckets);
        
        double stopVal = omp_get_wtime();
        
        std::cout << "Czas wykonania: " << stopVal - startCouting << " s" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Wystapil blad krytyczny: " << e.what() << "\n";
        return 1;
    }

    return 0;
}