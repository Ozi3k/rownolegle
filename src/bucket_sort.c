#include "bucket_sort.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Funkcja pomocnicza dla qsort
static int compare_uint(const void* a, const void* b) {
    unsigned int arg1 = *(const unsigned int*)a;
    unsigned int arg2 = *(const unsigned int*)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

void bucket_sort_sequential(unsigned int* data, size_t n, size_t num_buckets) {
    if (n <= 1) return;

    // 1. Znalezienie min i max
    unsigned int min_val = data[0];
    unsigned int max_val = data[0];
    for (size_t i = 1; i < n; ++i) {
        if (data[i] < min_val) min_val = data[i];
        else if (data[i] > max_val) max_val = data[i];
    }

    if (min_val == max_val) return;

    // 2. Alokacja pamięci na liczniki rozmiarów kubełków (histogram)
    size_t* bucket_counts = (size_t*)calloc(num_buckets, sizeof(size_t));
    if (!bucket_counts) {
        perror("Błąd alokacji bucket_counts");
        exit(EXIT_FAILURE);
    }

    // Zakres dla każdego kubełka
    double range = (double)(max_val - min_val) / num_buckets;

    // 3. Zliczanie elementów w kubełkach
    for (size_t i = 0; i < n; ++i) {
        size_t b_idx = (size_t)((data[i] - min_val) / range);
        if (b_idx >= num_buckets) b_idx = num_buckets - 1; // Zabezpieczenie dla max_val
        bucket_counts[b_idx]++;
    }

    // 4. Obliczanie offsetów dla ciągłej tablicy wejściowej
    size_t* bucket_offsets = (size_t*)malloc(num_buckets * sizeof(size_t));
    if (!bucket_offsets) {
        perror("Błąd alokacji bucket_offsets");
        free(bucket_counts);
        exit(EXIT_FAILURE);
    }

    bucket_offsets[0] = 0;
    for (size_t i = 1; i < num_buckets; ++i) {
        bucket_offsets[i] = bucket_offsets[i - 1] + bucket_counts[i - 1];
    }

    // Kopia offsetów do iteracji przy wstawianiu
    size_t* current_offsets = (size_t*)malloc(num_buckets * sizeof(size_t));
    memcpy(current_offsets, bucket_offsets, num_buckets * sizeof(size_t));

    // 5. Rozrzucanie elementów do tymczasowej tablicy
    unsigned int* output = (unsigned int*)malloc(n * sizeof(unsigned int));
    if (!output) {
        perror("Błąd alokacji output");
        free(bucket_counts);
        free(bucket_offsets);
        free(current_offsets);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < n; ++i) {
        size_t b_idx = (size_t)((data[i] - min_val) / range);
        if (b_idx >= num_buckets) b_idx = num_buckets - 1;
        output[current_offsets[b_idx]++] = data[i];
    }

    // 6. Sortowanie poszczególnych kubełków i przepisywanie z powrotem
    for (size_t i = 0; i < num_buckets; ++i) {
        if (bucket_counts[i] > 0) {
            qsort(output + bucket_offsets[i], bucket_counts[i], sizeof(unsigned int), compare_uint);
        }
    }

    memcpy(data, output, n * sizeof(unsigned int));

    free(bucket_counts);
    free(bucket_offsets);
    free(current_offsets);
    free(output);
}