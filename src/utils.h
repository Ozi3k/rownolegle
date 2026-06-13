#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <time.h>

unsigned int* load_binary_data(const char* filename, size_t* num_elements);
double get_time_diff_sec(struct timespec start, struct timespec end);

#endif // UTILS_H