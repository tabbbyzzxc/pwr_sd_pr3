/*
 * Struktury Danych - Projekt 3: Tablice haszujace
 * Adresowanie otwarte z trzema metodami probkowania:
 *   1. Probkowanie liniowe
 *   2. Probkowanie kwadratowe
 *   3. Haszowanie podwojne
 *
 * Operacje: wstawianie (insert) i usuwanie (delete)
 * Poziomy zapelnienia: 25%, 50%, 75%
 * Pomiar: srednia liczba porownaw (prob) na operacje
 */

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <numeric>
#include <string>
#include <iomanip>
#include <set>

// ─── Stale ──────────────────────────────────────────────────────────────────

const int TABLE_SIZE = 1009;  // liczba pierwsza
const int TRIALS = 50;

// ─── Typ metody probkowania ─────────────────────────────────────────────────

enum ProbeMethod { LINEAR, QUADRATIC, DOUBLE_HASH };

// ─── Tablica haszujaca z adresowaniem otwartym ─────────────────────────────

class HashTable {
public:
    static const int EMPTY   = -1;
    static const int DELETED = -2;

    int* table;
    int size;
    int count;

    HashTable(int sz = TABLE_SIZE) : size(sz), count(0) {
        table = new int[size];
        for (int i = 0; i < size; i++)
            table[i] = EMPTY;
    }

    ~HashTable() {
        delete[] table;
    }

    // Pierwsza funkcja haszujaca (dzielenie)
    int hash1(int key) const {
        int h = key % size;
        return h < 0 ? h + size : h;
    }

    // Druga funkcja haszujaca (dla podwojnego haszowania)
    int hash2(int key) const {
        int h = 1 + (((key % (size - 1)) + (size - 1)) % (size - 1));
        return h;
    }

    // Funkcja probkujaca — zwraca indeks dla i-tej proby
    int probe(int key, int i, ProbeMethod method) const {
        switch (method) {
            case LINEAR:
                return (hash1(key) + i) % size;
            case QUADRATIC:
                return (hash1(key) + i * i) % size;
            case DOUBLE_HASH:
                return (hash1(key) + i * hash2(key)) % size;
        }
        return 0;
    }

    // Wstawianie — zwraca liczbe prob (porownan)
    int insert(int key, ProbeMethod method) {
        int probes = 0;
        for (int i = 0; i < size; i++) {
            probes++;
            int idx = probe(key, i, method);
            if (table[idx] == EMPTY || table[idx] == DELETED) {
                table[idx] = key;
                count++;
                return probes;
            }
        }
        return probes;  // tablica pelna
    }

    // Usuwanie — zwraca liczbe prob (porownan)
    int remove(int key, ProbeMethod method) {
        int probes = 0;
        for (int i = 0; i < size; i++) {
            probes++;
            int idx = probe(key, i, method);
            if (table[idx] == EMPTY) {
                return probes;  // nie znaleziono
            }
            if (table[idx] == key) {
                table[idx] = DELETED;
                count--;
                return probes;
            }
        }
        return probes;
    }
};

// ─── Generacja unikalnych kluczy ────────────────────────────────────────────

std::vector<int> generateUniqueKeys(int count, int maxVal = TABLE_SIZE * 10) {
    std::set<int> keySet;
    while ((int)keySet.size() < count) {
        keySet.insert(rand() % maxVal + 1);
    }
    return std::vector<int>(keySet.begin(), keySet.end());
}

// ─── Benchmark wstawiania ───────────────────────────────────────────────────

double benchmarkInsert(int fillPercent, int numInserts, ProbeMethod method) {
    int prefillCount = TABLE_SIZE * fillPercent / 100;
    double totalAvg = 0.0;

    for (int t = 0; t < TRIALS; t++) {
        std::vector<int> allKeys = generateUniqueKeys(prefillCount + numInserts);

        // Losowe potasowanie
        for (int i = (int)allKeys.size() - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            std::swap(allKeys[i], allKeys[j]);
        }

        HashTable ht(TABLE_SIZE);

        // Prefill tablicy
        for (int i = 0; i < prefillCount; i++)
            ht.insert(allKeys[i], method);

        // Benchmark wstawiania
        int totalProbes = 0;
        for (int i = prefillCount; i < prefillCount + numInserts; i++)
            totalProbes += ht.insert(allKeys[i], method);

        totalAvg += (double)totalProbes / numInserts;
    }

    return totalAvg / TRIALS;
}

// ─── Benchmark usuwania ─────────────────────────────────────────────────────

double benchmarkDelete(int fillPercent, int numDeletes, ProbeMethod method) {
    int prefillCount = TABLE_SIZE * fillPercent / 100;
    int actualDeletes = std::min(numDeletes, prefillCount);
    double totalAvg = 0.0;

    for (int t = 0; t < TRIALS; t++) {
        std::vector<int> allKeys = generateUniqueKeys(prefillCount);

        // Losowe potasowanie
        for (int i = (int)allKeys.size() - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            std::swap(allKeys[i], allKeys[j]);
        }

        HashTable ht(TABLE_SIZE);
        for (int i = 0; i < prefillCount; i++)
            ht.insert(allKeys[i], method);

        // Wybierz klucze do usuniecia (pierwsze actualDeletes)
        int totalProbes = 0;
        for (int i = 0; i < actualDeletes; i++)
            totalProbes += ht.remove(allKeys[i], method);

        totalAvg += (double)totalProbes / actualDeletes;
    }

    return totalAvg / TRIALS;
}

// ─── Nazwy metod ────────────────────────────────────────────────────────────

std::string methodName(ProbeMethod m) {
    switch (m) {
        case LINEAR:      return "Probkowanie liniowe    ";
        case QUADRATIC:   return "Probkowanie kwadratowe ";
        case DOUBLE_HASH: return "Haszowanie podwojne    ";
    }
    return "";
}

// ─── Main ───────────────────────────────────────────────────────────────────

int main() {
    srand(42);

    ProbeMethod methods[] = {LINEAR, QUADRATIC, DOUBLE_HASH};
    int batchSizes[] = {10, 50, 100, 500, 1000};
    int fillLevels[] = {25, 50, 75};

    // ── INSERT benchmarks ──
    for (int fill : fillLevels) {
        std::cout << "\n" << std::string(65, '=') << std::endl;
        std::cout << "  WSTAWIANIE — zapelnienie " << fill << "%" << std::endl;
        std::cout << std::string(65, '=') << std::endl;

        // Naglowek tabeli
        std::cout << std::left << std::setw(26) << "  Metoda";
        for (int n : batchSizes)
            std::cout << std::setw(10) << n;
        std::cout << std::endl;
        std::cout << "  " << std::string(63, '-') << std::endl;

        for (ProbeMethod m : methods) {
            std::cout << "  " << methodName(m);
            for (int n : batchSizes) {
                double avg = benchmarkInsert(fill, n, m);
                std::cout << std::fixed << std::setprecision(3)
                          << std::setw(10) << avg;
            }
            std::cout << std::endl;
        }
    }

    // ── DELETE benchmarks ──
    for (int fill : fillLevels) {
        std::cout << "\n" << std::string(65, '=') << std::endl;
        std::cout << "  USUWANIE — zapelnienie " << fill << "%" << std::endl;
        std::cout << std::string(65, '=') << std::endl;

        std::cout << std::left << std::setw(26) << "  Metoda";
        for (int n : batchSizes)
            std::cout << std::setw(10) << n;
        std::cout << std::endl;
        std::cout << "  " << std::string(63, '-') << std::endl;

        for (ProbeMethod m : methods) {
            std::cout << "  " << methodName(m);
            for (int n : batchSizes) {
                double avg = benchmarkDelete(fill, n, m);
                std::cout << std::fixed << std::setprecision(3)
                          << std::setw(10) << avg;
            }
            std::cout << std::endl;
        }
    }

    std::cout << "\n Wszystkie benchmarki zakonczone pomyslnie." << std::endl;

    return 0;
}
