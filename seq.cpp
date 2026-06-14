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

void bucketSort(std::vector<int>& arr, int num_buckets) {
    int n = arr.size();
    if (n <= 1) return;

    int min_val = arr[0], max_val = arr[0];
    {
        int local_min = min_val;
        int local_max = max_val;
        for (int i = 0; i < n; i++) {
            if (arr[i] < local_min) local_min = arr[i];
            if (arr[i] > local_max) local_max = arr[i];
        }

        if (local_min < min_val) min_val = local_min;
        if (local_max > max_val) max_val = local_max;
    }
    if (min_val == max_val) return;

    int num_threads = omp_get_max_threads();
    std::vector<std::vector<std::vector<int>>> local_buckets(num_threads, std::vector<std::vector<int>>(num_buckets));

    int tid = omp_get_thread_num();
    long long range = (long long)max_val - min_val;

    for (int i = 0; i < n; i++) {
        int bucket_idx = (int)(((long long)(arr[i] - min_val) * (num_buckets - 1)) / range);
        local_buckets[tid][bucket_idx].push_back(arr[i]);
    }

    std::vector<std::vector<int>> buckets(num_buckets);
    for (int b = 0; b < num_buckets; b++) {
        for (int t = 0; t < num_threads; t++) {
            buckets[b].insert(buckets[b].end(), local_buckets[t][b].begin(), local_buckets[t][b].end());
        }
    }

    for (int b = 0; b < num_buckets; b++) {
        std::sort(buckets[b].begin(), buckets[b].end());
    }

    std::vector<int> start_idx(num_buckets, 0);
    for (int b = 1; b < num_buckets; b++) {
        start_idx[b] = start_idx[b - 1] + buckets[b - 1].size();
    }

    for (int b = 0; b < num_buckets; b++) {
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
    int num_buckets;
    
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
        
        bucketSort(data, num_buckets);
        
        double stopVal = omp_get_wtime();
        
        std::cout << "Czas wykonania: " << stopVal - startCouting << " s" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Wystapil blad krytyczny: " << e.what() << "\n";
        return 1;
    }

    return 0;
}