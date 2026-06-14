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

#include <vector>
#include <algorithm>
#include <iostream>
#include <omp.h>

void parallelBucketSort(std::vector<int>& arr, size_t num_buckets) {
    size_t n = arr.size();
    if (n <= 1) return;

    // 1. Równoległe znajdowanie min/max
    int min_val = arr[0], max_val = arr[0];
    #pragma omp parallel
    {
        int local_min = arr[0];
        int local_max = arr[0];
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

    // 2. Inicjalizacja lokalnych kubełków
    size_t num_threads = (size_t)omp_get_max_threads();
    std::vector<std::vector<std::vector<int>>> local_buckets(num_threads, std::vector<std::vector<int>>(num_buckets));

    long long range = (long long)max_val - min_val;

    // 3. Równoległe rozdzielanie
    #pragma omp parallel
    {
        size_t tid = (size_t)omp_get_thread_num();
        #pragma omp for
        for (size_t i = 0; i < n; i++) {
            long long diff = (long long)arr[i] - min_val;
            size_t bucket_idx = (size_t)(diff * (num_buckets - 1) / range);
            
            // BEZPIECZNIK
            if (bucket_idx >= num_buckets) bucket_idx = num_buckets - 1;
            
            local_buckets[tid][bucket_idx].push_back(arr[i]);
        }
    }

    // 4. Scalanie i sortowanie (z optymalizacją pamięci)
    std::vector<std::vector<int>> global_buckets(num_buckets);
    
    #pragma omp parallel for schedule(dynamic)
    for (size_t b = 0; b < num_buckets; b++) {
        // Najpierw zliczamy rozmiar, by raz zaalokować pamięć (reserve)
        size_t total_size = 0;
        for (size_t t = 0; t < num_threads; t++) {
            total_size += local_buckets[t][b].size();
        }
        global_buckets[b].reserve(total_size);

        // Scalamy
        for (size_t t = 0; t < num_threads; t++) {
            global_buckets[b].insert(global_buckets[b].end(), 
                                     local_buckets[t][b].begin(), 
                                     local_buckets[t][b].end());
        }
        std::sort(global_buckets[b].begin(), global_buckets[b].end());
    }

    // 5. Zapis wyniku
    std::vector<size_t> start_idx(num_buckets, 0);
    for (size_t b = 1; b < num_buckets; b++) {
        start_idx[b] = start_idx[b - 1] + global_buckets[b - 1].size();
    }

    #pragma omp parallel for
    for (size_t b = 0; b < num_buckets; b++) {
        size_t idx = start_idx[b];
        for (int val : global_buckets[b]) {
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