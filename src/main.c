#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "bucket_sort.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Użycie: %s <plik_wejsciowy> [liczba_kubelkow]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* filename = argv[1];
    size_t num_buckets = (argc >= 3) ? atoi(argv[2]) : 10000;

    printf("Wczytywanie danych z %s...\n", filename);
    size_t n;
    unsigned int* data = load_binary_data(filename, &n);
    printf("Wczytano %zu elementow.\n", n);

    struct timespec start, end;

    printf("Rozpoczecie sortowania (sekwencyjnego)...\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    bucket_sort_sequential(data, n, num_buckets);
    
    clock_gettime(CLOCK_MONOTONIC, &end);

    double time_spent = get_time_diff_sec(start, end);
    printf("Czas sortowania: %.6f sekund\n", time_spent);

    free(data);
    return EXIT_SUCCESS;
}