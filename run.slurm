#!/bin/bash -l
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH -p standard
#SBATCH -J bucket_sort_benchmark
#SBATCH -o benchmark_output.txt

# Katalog roboczy i ścieżka pliku
WORK_DIR=$(pwd)
DATA_FILE="$WORK_DIR/dane_testowe.bin"
BUCKET_COUNT=10000

# Tablica z rozmiarami do przetestowania (5 GB = 5120 MB, 10 GB = 10240 MB)
SIZES_MB=(5120 10240)

echo "=== Rozpoczęcie zadania: $(date) ==="

# 1. Kompilacja obu wersji
echo "Kompilowanie źródeł (wersja sekwencyjna i równoległa)..."
rm -f bucket_sort_seq bucket_sort_async

g++ -Wall -Wextra -O3 -std=gnu11 \
    seq.cpp \
    -o bucket_sort_seq

g++ -Wall -Wextra -O3 -std=gnu11 -fopenmp \
    async.cpp \
    -o bucket_sort_async

if [[ ! -x bucket_sort_seq || ! -x bucket_sort_async ]]; then
    echo "Błąd kompilacji. Przerywam działanie."
    exit 1
fi

# 2. Główna pętla testowa
for DATA_SIZE_MB in "${SIZES_MB[@]}"; do
    
    echo ""
    echo "=========================================================="
    echo "=== ROZPOCZĘCIE CYKLU DLA ROZMIARU: ${DATA_SIZE_MB} MB ==="
    echo "=========================================================="

    # Generowanie danych
    echo "Generowanie pliku danych (${DATA_SIZE_MB} MB)..."
    dd if=/dev/urandom of="$DATA_FILE" bs=1M count="$DATA_SIZE_MB" status=progress

    if [ ! -f "$DATA_FILE" ]; then
        echo "Błąd generowania danych dla rozmiaru ${DATA_SIZE_MB} MB."
        exit 1
    fi

    # Testowanie wersji sekwencyjnej (baseline)
    echo "--- Uruchamianie programu sekwencyjnego (${DATA_SIZE_MB} MB) ---"
    export OMP_NUM_THREADS=1 
    ./bucket_sort_seq "$DATA_FILE" $BUCKET_COUNT

    # Testowanie wersji równoległej dla 1 do 16 wątków
    echo "--- Uruchamianie programu równoległego (${DATA_SIZE_MB} MB) ---"
    for THREAD_COUNT in {1..16}; do
        echo " -> Test dla OMP_NUM_THREADS=${THREAD_COUNT}"
        OMP_NUM_THREADS=$THREAD_COUNT ./bucket_sort_async "$DATA_FILE" $BUCKET_COUNT
    done

    # Czyszczenie pliku przed kolejną iteracją
    echo "Usuwanie pliku wejściowego (${DATA_SIZE_MB} MB)..."
    rm -f "$DATA_FILE"

done

echo "=== Zakończenie zadania: $(date) ==="