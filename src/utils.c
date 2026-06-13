#include "utils.h"
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

unsigned int* load_binary_data(const char* filename, size_t* num_elements) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Błąd otwarcia pliku");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    *num_elements = file_size / sizeof(unsigned int);
    unsigned int* data = (unsigned int*)malloc(file_size);
    if (!data) {
        perror("Błąd alokacji pamięci na dane wejściowe");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size_t read_elements = fread(data, sizeof(unsigned int), *num_elements, file);
    if (read_elements != *num_elements) {
        perror("Błąd odczytu danych");
        free(data);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
    return data;
}

double get_time_diff_sec(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}