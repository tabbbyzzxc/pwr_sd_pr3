"""
Benchmarking hash table operations: insert and delete
with different collision resolution strategies:
  - Linear Probing (Próbkowanie liniowe)
  - Quadratic Probing (Próbkowanie kwadratowe)
  - Double Hashing (Haszowanie podwójne)

Fill levels: 25%, 50%, 75%
"""

import matplotlib.pyplot as plt
import matplotlib
import numpy as np
import random
import os

matplotlib.rcParams['font.size'] = 11

# ─── Hash Table Implementation ───────────────────────────────────────────────

class HashTable:
    EMPTY = None
    DELETED = "__DELETED__"

    def __init__(self, size=1009):
        self.size = size
        self.table = [self.EMPTY] * size
        self.count = 0

    def _hash1(self, key):
        return key % self.size

    def _hash2(self, key):
        # For double hashing: must return non-zero value
        return 1 + (key % (self.size - 1))

    def _probe_linear(self, key, i):
        return (self._hash1(key) + i) % self.size

    def _probe_quadratic(self, key, i):
        return (self._hash1(key) + i * i) % self.size

    def _probe_double(self, key, i):
        return (self._hash1(key) + i * self._hash2(key)) % self.size

    def insert(self, key, method='linear'):
        """Insert key and return the number of probes (comparisons)."""
        probe_func = {
            'linear': self._probe_linear,
            'quadratic': self._probe_quadratic,
            'double': self._probe_double,
        }[method]

        probes = 0
        for i in range(self.size):
            probes += 1
            idx = probe_func(key, i)
            if self.table[idx] is self.EMPTY or self.table[idx] == self.DELETED:
                self.table[idx] = key
                self.count += 1
                return probes
        return probes  # table full

    def delete(self, key, method='linear'):
        """Delete key and return the number of probes (comparisons)."""
        probe_func = {
            'linear': self._probe_linear,
            'quadratic': self._probe_quadratic,
            'double': self._probe_double,
        }[method]

        probes = 0
        for i in range(self.size):
            probes += 1
            idx = probe_func(key, i)
            if self.table[idx] is self.EMPTY:
                return probes  # not found
            if self.table[idx] == key:
                self.table[idx] = self.DELETED
                self.count -= 1
                return probes
        return probes


# ─── Benchmark Functions ─────────────────────────────────────────────────────

TABLE_SIZE = 1009  # prime number for better distribution

def run_insert_benchmark(fill_percent, num_inserts=100, trials=50):
    """
    Benchmark inserting `num_inserts` elements into a hash table
    that is already filled to `fill_percent`%.
    Returns dict with average probes per method.
    """
    prefill_count = int(TABLE_SIZE * fill_percent / 100)
    methods = ['linear', 'quadratic', 'double']
    results = {m: [] for m in methods}

    for _ in range(trials):
        # Generate unique keys for prefill + inserts
        all_keys = random.sample(range(1, 10 * TABLE_SIZE), prefill_count + num_inserts)
        prefill_keys = all_keys[:prefill_count]
        insert_keys = all_keys[prefill_count:]

        for method in methods:
            ht = HashTable(TABLE_SIZE)
            # Prefill the table
            for k in prefill_keys:
                ht.insert(k, method)

            # Benchmark inserts
            total_probes = 0
            for k in insert_keys:
                total_probes += ht.insert(k, method)
            results[method].append(total_probes / num_inserts)

    return {m: np.mean(results[m]) for m in methods}


def run_delete_benchmark(fill_percent, num_deletes=100, trials=50):
    """
    Benchmark deleting `num_deletes` elements from a hash table
    that is filled to `fill_percent`%.
    Returns dict with average probes per method.
    """
    prefill_count = int(TABLE_SIZE * fill_percent / 100)
    methods = ['linear', 'quadratic', 'double']
    results = {m: [] for m in methods}

    for _ in range(trials):
        all_keys = random.sample(range(1, 10 * TABLE_SIZE), prefill_count)
        # Pick keys to delete from the ones already in the table
        delete_keys = random.sample(all_keys, min(num_deletes, len(all_keys)))

        for method in methods:
            ht = HashTable(TABLE_SIZE)
            for k in all_keys:
                ht.insert(k, method)

            total_probes = 0
            for k in delete_keys:
                total_probes += ht.delete(k, method)
            results[method].append(total_probes / len(delete_keys))

    return {m: np.mean(results[m]) for m in methods}


def run_detailed_benchmark(operation, fill_percent, trials=50):
    """
    Run benchmark for multiple batch sizes (10, 50, 100, 500, 1000).
    Returns a dict: {method: [avg_probes_per_batch_size, ...]}
    """
    batch_sizes = [10, 50, 100, 500, 1000]
    methods = ['linear', 'quadratic', 'double']
    results = {m: [] for m in methods}

    for n in batch_sizes:
        if operation == 'insert':
            r = run_insert_benchmark(fill_percent, num_inserts=n, trials=trials)
        else:
            r = run_delete_benchmark(fill_percent, num_deletes=n, trials=trials)
        for m in methods:
            results[m].append(round(r[m], 3))

    return batch_sizes, results


# ─── Plotting Functions ──────────────────────────────────────────────────────

METHOD_LABELS = {
    'linear': 'Próbkowanie liniowe',
    'quadratic': 'Próbkowanie kwadratowe',
    'double': 'Haszowanie podwójne',
}

COLORS = {
    'linear': '#e74c3c',
    'quadratic': '#3498db',
    'double': '#2ecc71',
}


def plot_table(batch_sizes, results, title, fig_num, save_path):
    """Create a table figure."""
    fig, ax = plt.subplots(figsize=(10, 2.5))
    ax.axis('off')
    ax.set_title(f'Rysunek {fig_num}: {title}', fontsize=12, fontweight='bold', pad=20)

    col_labels = [str(n) for n in batch_sizes]
    row_labels = [METHOD_LABELS[m] for m in ['linear', 'quadratic', 'double']]
    cell_data = [results[m] for m in ['linear', 'quadratic', 'double']]

    table = ax.table(
        cellText=cell_data,
        rowLabels=row_labels,
        colLabels=col_labels,
        cellLoc='center',
        rowLoc='center',
        loc='center',
    )
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1.2, 1.8)

    # Style header
    for j in range(len(col_labels)):
        table[(0, j)].set_facecolor('#34495e')
        table[(0, j)].set_text_props(color='white', fontweight='bold')

    # Style row labels
    for i in range(len(row_labels)):
        table[(i + 1, -1)].set_facecolor('#ecf0f1')
        table[(i + 1, -1)].set_text_props(fontweight='bold')

    # Color rows
    row_colors = ['#fadbd8', '#d6eaf8', '#d5f5e3']
    for i in range(len(row_labels)):
        for j in range(len(col_labels)):
            table[(i + 1, j)].set_facecolor(row_colors[i])

    plt.tight_layout()
    plt.savefig(save_path, dpi=150, bbox_inches='tight')
    plt.close()
    print(f"  Saved: {save_path}")


def plot_chart(batch_sizes, results, title, fig_num, ylabel, save_path):
    """Create a bar chart figure."""
    fig, ax = plt.subplots(figsize=(10, 6))

    x = np.arange(len(batch_sizes))
    width = 0.25
    methods = ['linear', 'quadratic', 'double']

    for i, method in enumerate(methods):
        bars = ax.bar(
            x + i * width,
            results[method],
            width,
            label=METHOD_LABELS[method],
            color=COLORS[method],
            edgecolor='white',
            linewidth=0.5,
        )
        # Add value labels on bars
        for bar, val in zip(bars, results[method]):
            ax.text(
                bar.get_x() + bar.get_width() / 2,
                bar.get_height() + 0.02 * max(max(results[m]) for m in methods),
                f'{val:.2f}',
                ha='center', va='bottom', fontsize=8, fontweight='bold',
            )

    ax.set_xlabel('Liczba operacji', fontsize=12)
    ax.set_ylabel(ylabel, fontsize=12)
    ax.set_title(f'Rysunek {fig_num}: {title}', fontsize=13, fontweight='bold')
    ax.set_xticks(x + width)
    ax.set_xticklabels(batch_sizes)
    ax.legend(loc='upper left', fontsize=10)
    ax.grid(axis='y', alpha=0.3)
    ax.set_axisbelow(True)

    plt.tight_layout()
    plt.savefig(save_path, dpi=150, bbox_inches='tight')
    plt.close()
    print(f"  Saved: {save_path}")


# ─── Main ────────────────────────────────────────────────────────────────────

def main():
    output_dir = os.path.join(os.path.dirname(__file__), 'wykresy')
    os.makedirs(output_dir, exist_ok=True)

    random.seed(42)
    np.random.seed(42)

    fill_levels = [25, 50, 75]
    fig_num = 1

    # ── INSERT benchmarks ──
    for fill in fill_levels:
        print(f"\n{'='*60}")
        print(f"  Wstawianie — zapełnienie {fill}%")
        print(f"{'='*60}")

        batch_sizes, results = run_detailed_benchmark('insert', fill)

        # Table
        plot_table(
            batch_sizes, results,
            f'Wyniki wstawiania do tablicy haszującej zapełnionej w {fill}%',
            fig_num,
            os.path.join(output_dir, f'rys{fig_num}_tablica_insert_{fill}.png'),
        )
        fig_num += 1

        # Chart
        plot_chart(
            batch_sizes, results,
            f'Wstawianie danych do tablicy haszującej zapełnionej w {fill}%',
            fig_num,
            'Średnia liczba porównań na operację',
            os.path.join(output_dir, f'rys{fig_num}_wykres_insert_{fill}.png'),
        )
        fig_num += 1

    # ── DELETE benchmarks ──
    for fill in fill_levels:
        print(f"\n{'='*60}")
        print(f"  Usuwanie — zapełnienie {fill}%")
        print(f"{'='*60}")

        batch_sizes, results = run_detailed_benchmark('delete', fill)

        # Table
        plot_table(
            batch_sizes, results,
            f'Wyniki usuwania z tablicy haszującej zapełnionej w {fill}%',
            fig_num,
            os.path.join(output_dir, f'rys{fig_num}_tablica_delete_{fill}.png'),
        )
        fig_num += 1

        # Chart
        plot_chart(
            batch_sizes, results,
            f'Usuwanie danych z tablicy haszującej zapełnionej w {fill}%',
            fig_num,
            'Średnia liczba porównań na operację',
            os.path.join(output_dir, f'rys{fig_num}_wykres_delete_{fill}.png'),
        )
        fig_num += 1

    print(f"\n✅ Wszystkie wykresy zapisane w: {output_dir}")
    print(f"   Łącznie wygenerowano {fig_num - 1} rysunków.")


if __name__ == '__main__':
    main()
