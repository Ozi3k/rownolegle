#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>   
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

void sequentialBucketSort(std::vector<int>& arr, size_t num_buckets) {
    size_t n = arr.size();
    if (n <= 1) return;

    // 1. Znajdowanie min/max
    int min_val = arr[0];
    int max_val = arr[0];
    for (size_t i = 1; i < n; i++) {
        if (arr[i] < min_val) min_val = arr[i];
        if (arr[i] > max_val) max_val = arr[i];
    }

    if (min_val == max_val) return;

    // 2. Inicjalizacja kubełków (z optymalizacją pamięci)
    std::vector<std::vector<int>> buckets(num_buckets);
    long long range = (long long)max_val - min_val;

    size_t expected_size = (n / num_buckets) * 1.2;
    for (size_t i = 0; i < num_buckets; ++i) {
        buckets[i].reserve(expected_size);
    }

    // 3. Rozdzielanie
    for (size_t i = 0; i < n; i++) {
        int bucket_idx = (int)( ((long long)arr[i] - min_val) * (num_buckets - 1) / range );
        buckets[bucket_idx].push_back(arr[i]);
    }

    // 4. Sortowanie i zapis wyniku
    size_t idx = 0;
    for (size_t b = 0; b < num_buckets; b++) {
        std::sort(buckets[b].begin(), buckets[b].end());
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

        auto startCounting = std::chrono::high_resolution_clock::now(); 
        
        sequentialBucketSort(data, num_buckets);
        
        auto stopVal = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = stopVal - startCounting;
        
        std::cout << "Czas wykonania: " << elapsed.count() << " s" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Wystapil blad krytyczny: " << e.what() << "\n";
        return 1;
    }

    return 0;
}