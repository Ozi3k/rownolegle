#!/bin/bash -l
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH -p standard
#SBATCH -J bucket_sort_benchmark
#SBATCH -o benchmark_output.txt

# Katalog roboczy (zakładamy, że jesteśmy w katalogu z Makefile)
WORK_DIR=$(pwd)
DATA_FILE="$WORK_DIR/dane_testowe.bin"

# Rozmiar generowanych danych: 2048 bloków po 1MB = 2GB danych
DATA_SIZE_MB=2048

echo "=== Rozpoczęcie zadania: $(date) ==="

# Kompilacja programu
echo "Kompilowanie źródeł..."
rm -f bucket_sort_seq bucket_sort_async
gcc -Wall -Wextra -O3 -std=gnu11 src/main.c src/bucket_sort.c src/utils.c -o bucket_sort_seq

# gcc -Wall -Wextra -O3 -std=gnu11 -fopenmp src/main.c src/bucket_sort.c src/utils.c -o bucket_sort_async

# Generowanie pliku testowego używając /dev/urandom
# Generujemy czyste losowe bajty, co w C zostanie zinterpretowane jako liczby całkowite.
echo "Generowanie pliku danych ($DATA_SIZE_MB MB)..."
dd if=/dev/urandom of="$DATA_FILE" bs=1M count=$DATA_SIZE_MB status=progress

# Weryfikacja czy plik istnieje i rozmiar
if [ -f "$DATA_FILE" ]; then
    echo "Plik wygenerowany pomyślnie."
else
    echo "Błąd generowania danych."
    exit 1
fi

# Uruchomienie wersji sekwencyjnej
echo "Uruchamianie programu sekwencyjnego..."
# Jeśli docelowo będziesz testował OpenMP, OMP_NUM_THREADS może zostać nadpisane tutaj:
export OMP_NUM_THREADS=1 
./bucket_sort_seq "$DATA_FILE" 10000

# Czyszczenie danych po benchmarku
echo "Usuwanie pliku wejściowego..."
rm -f "$DATA_FILE"

echo "=== Zakończenie zadania: $(date) ==="