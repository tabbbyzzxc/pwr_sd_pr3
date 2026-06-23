#include <iostream>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <string>

enum HashType { DIVISION, MULTIPLICATION, CUSTOM };

struct Node {
    int key;
    Node* next;
};

struct HashTable {
    Node** buckets;
    int size;
    HashType hashType;

    HashTable(int sz, HashType ht) {
        size = sz;
        hashType = ht;
        buckets = new Node*[size];
        for (int i = 0; i < size; i++)
            buckets[i] = nullptr;
    }

    ~HashTable() {
        for (int i = 0; i < size; i++) {
            Node* cur = buckets[i];
            while (cur) {
                Node* tmp = cur;
                cur = cur->next;
                delete tmp;
            }
        }
        delete[] buckets;
    }

    int hash(int key) {
        if (hashType == DIVISION) {
            int h = key % size;
            return h < 0 ? h + size : h;
        }
        else if (hashType == MULTIPLICATION) {
            double A = 0.6180339887;
            double val = key * A;
            val = val - (long long)val;
            if (val < 0) val += 1.0;
            return (int)(size * val);
        }
        else {
            unsigned int h = (unsigned int)key;
            h = ((h >> 16) ^ h) * 0x45d9f3b;
            h = ((h >> 16) ^ h) * 0x45d9f3b;
            h = (h >> 16) ^ h;
            return h % size;
        }
    }

    void insert(int key) {
        int idx = hash(key);

        Node* cur = buckets[idx];
        while (cur) {
            if (cur->key == key)
                return;
            cur = cur->next;
        }

        Node* node = new Node;
        node->key = key;
        node->next = buckets[idx];
        buckets[idx] = node;
    }

    void remove(int key) {
        int idx = hash(key);

        if (!buckets[idx])
            return;

        if (buckets[idx]->key == key) {
            Node* tmp = buckets[idx];
            buckets[idx] = buckets[idx]->next;
            delete tmp;
            return;
        }

        Node* prev = buckets[idx];
        Node* cur = prev->next;
        while (cur) {
            if (cur->key == key) {
                prev->next = cur->next;
                delete cur;
                return;
            }
            prev = cur;
            cur = cur->next;
        }
    }
};

int main() {
    const int N = 100000;
    const int TABLE_SIZE = 100003; // prosta liczba (prime)

    std::string names[] = {"Division", "Multiplication", "Custom (bit-mix)"};
    HashType types[] = {DIVISION, MULTIPLICATION, CUSTOM};

    for (int t = 0; t < 3; t++) {
        std::cout << "=== Hash function: " << names[t] << " ===" << std::endl;

        // --- Best case: kluczy = 0, 1, 2, ... (idealnie leżą po bucketach dla division) ---
        {
            std::vector<int> keys(N);
            for (int i = 0; i < N; i++)
                keys[i] = i;

            HashTable ht(TABLE_SIZE, types[t]);

            auto t1 = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < N; i++)
                ht.insert(keys[i]);
            auto t2 = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < N; i++)
                ht.remove(keys[i]);
            auto t3 = std::chrono::high_resolution_clock::now();

            long long ins = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            long long rem = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
            std::cout << "  Best case:    insert = " << ins << " us,  remove = " << rem << " us" << std::endl;
        }

        // --- Average case: pseudolosowe dany ---
        {
            srand(42);
            std::vector<int> keys(N);
            for (int i = 0; i < N; i++)
                keys[i] = rand();

            HashTable ht(TABLE_SIZE, types[t]);

            auto t1 = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < N; i++)
                ht.insert(keys[i]);
            auto t2 = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < N; i++)
                ht.remove(keys[i]);
            auto t3 = std::chrono::high_resolution_clock::now();

            long long ins = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            long long rem = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
            std::cout << "  Average case: insert = " << ins << " us,  remove = " << rem << " us" << std::endl;
        }

        // --- Worst case: wszystkie kluczy = krotne TABLE_SIZE, wszystkie lecą do bucketa 0 ---
        {
            int WORST_N = 10000; // pomniej, inaczej O(n^2) się za bardzo przeciągnie
            std::vector<int> keys(WORST_N);
            for (int i = 0; i < WORST_N; i++)
                keys[i] = i * TABLE_SIZE; // wszystko dadzą hash = 0 dla division

            HashTable ht(TABLE_SIZE, types[t]);

            auto t1 = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < WORST_N; i++)
                ht.insert(keys[i]);
            auto t2 = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < WORST_N; i++)
                ht.remove(keys[i]);
            auto t3 = std::chrono::high_resolution_clock::now();

            long long ins = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            long long rem = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
            std::cout << "  Worst case:   insert = " << ins << " us,  remove = " << rem
                      << " us  (N=" << WORST_N << ")" << std::endl;
        }

        std::cout << std::endl;
    }

    return 0;
}
