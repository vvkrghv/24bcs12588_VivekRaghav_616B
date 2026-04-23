#include <emscripten/bind.h>
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>
#include <cmath>

using namespace emscripten;

// ─── Result struct ───────────────────────────────────────────────
struct SortMetrics {
    double time;
    int    comparisons;
    int    swaps;
    bool   stable;
    std::string complexity_best;
    std::string complexity_avg;
    std::string complexity_worst;
    std::string complexity_space;
};

// ─── Merge Sort ──────────────────────────────────────────────────
static void mergeHelper(std::vector<double>& arr, int l, int m, int r,
                        int& comps, int& swps) {
    std::vector<double> L(arr.begin()+l, arr.begin()+m+1);
    std::vector<double> R(arr.begin()+m+1, arr.begin()+r+1);
    int i=0, j=0, k=l;
    while (i<(int)L.size() && j<(int)R.size()) {
        comps++;
        if (L[i] <= R[j]) { arr[k++]=L[i++]; }
        else               { arr[k++]=R[j++]; swps++; }
    }
    while (i<(int)L.size()) arr[k++]=L[i++];
    while (j<(int)R.size()) arr[k++]=R[j++];
}
static void mergeSort(std::vector<double>& arr, int l, int r, int& c, int& s) {
    if (l >= r) return;
    int m = (l+r)/2;
    mergeSort(arr, l, m, c, s);
    mergeSort(arr, m+1, r, c, s);
    mergeHelper(arr, l, m, r, c, s);
}

// ─── Quick Sort ──────────────────────────────────────────────────
static int qpartition(std::vector<double>& arr, int lo, int hi, int& c, int& s) {
    double pivot = arr[hi];
    int i = lo - 1;
    for (int j=lo; j<hi; j++) {
        c++;
        if (arr[j] <= pivot) { i++; std::swap(arr[i], arr[j]); s++; }
    }
    std::swap(arr[i+1], arr[hi]); s++;
    return i+1;
}
static void quickSort(std::vector<double>& arr, int lo, int hi, int& c, int& s) {
    if (lo >= hi) return;
    int p = qpartition(arr, lo, hi, c, s);
    quickSort(arr, lo, p-1, c, s);
    quickSort(arr, p+1, hi, c, s);
}

// ─── Heap Sort ───────────────────────────────────────────────────
static void heapify(std::vector<double>& arr, int n, int i, int& c, int& s) {
    int largest = i, l = 2*i+1, r = 2*i+2;
    if (l<n) { c++; if (arr[l]>arr[largest]) largest=l; }
    if (r<n) { c++; if (arr[r]>arr[largest]) largest=r; }
    if (largest != i) { std::swap(arr[i], arr[largest]); s++; heapify(arr,n,largest,c,s); }
}
static void heapSort(std::vector<double>& arr, int& c, int& s) {
    int n = arr.size();
    for (int i=n/2-1; i>=0; i--) heapify(arr, n, i, c, s);
    for (int i=n-1; i>0; i--) {
        std::swap(arr[0], arr[i]); s++;
        heapify(arr, i, 0, c, s);
    }
}

// ─── Counting Sort ───────────────────────────────────────────────
static void countingSort(std::vector<double>& arr, int& c, int& s) {
    if (arr.empty()) return;
    int mn = (int)std::floor(*std::min_element(arr.begin(), arr.end()));
    int mx = (int)std::ceil( *std::max_element(arr.begin(), arr.end()));
    std::vector<int> count(mx - mn + 1, 0);
    for (auto v : arr) { count[(int)std::round(v) - mn]++; s++; }
    c = (int)arr.size();
    int idx = 0;
    for (int v=mn; v<=mx; v++) {
        while (count[v-mn]-- > 0) { arr[idx++] = (double)v; s++; }
    }
}

// ─── Radix Sort ──────────────────────────────────────────────────
static void countSortByExp(std::vector<double>& arr, int exp, int& s) {
    int n = arr.size();
    std::vector<double> output(n);
    int count[10] = {0};
    for (int i=0; i<n; i++) { count[((int)std::round(std::abs(arr[i]))/exp)%10]++; s++; }
    for (int i=1; i<10; i++) count[i]+=count[i-1];
    for (int i=n-1; i>=0; i--) {
        int d = ((int)std::round(std::abs(arr[i]))/exp)%10;
        output[--count[d]] = arr[i]; s++;
    }
    for (int i=0; i<n; i++) arr[i]=output[i];
}
static void radixSort(std::vector<double>& arr, int& c, int& s) {
    if (arr.empty()) return;
    int mx = 0;
    for (auto v : arr) mx = std::max(mx, (int)std::round(std::abs(v)));
    c = (int)arr.size();
    for (int exp=1; mx/exp>0; exp*=10) countSortByExp(arr, exp, s);
}

// ─── Bucket Sort ─────────────────────────────────────────────────
static void insSort(std::vector<double>& b, int& c, int& s) {
    for (int i=1; i<(int)b.size(); i++) {
        double key = b[i]; int j=i-1;
        while (j>=0 && (c++, b[j]>key)) { b[j+1]=b[j]; j--; s++; }
        b[j+1] = key;
    }
}
static void bucketSort(std::vector<double>& arr, int& c, int& s) {
    if (arr.empty()) return;
    double mn = *std::min_element(arr.begin(), arr.end());
    double mx = *std::max_element(arr.begin(), arr.end());
    int n = arr.size();
    double range = mx - mn + 1e-9;
    std::vector<std::vector<double>> buckets(n);
    for (auto v : arr) {
        int bi = (int)(((v-mn)/range)*n);
        if (bi >= n) bi = n-1;
        buckets[bi].push_back(v); s++;
    }
    int idx = 0;
    for (auto& b : buckets) {
        insSort(b, c, s);
        for (auto v : b) arr[idx++] = v;
    }
}

// ─── Dispatcher ──────────────────────────────────────────────────
SortMetrics analyzeSort(std::vector<double> data, std::string algo) {
    int c=0, s=0;
    auto t0 = std::chrono::high_resolution_clock::now();

    if      (algo == "Merge Sort")    mergeSort(data, 0, (int)data.size()-1, c, s);
    else if (algo == "Quick Sort")    quickSort(data, 0, (int)data.size()-1, c, s);
    else if (algo == "Heap Sort")     heapSort(data, c, s);
    else if (algo == "Counting Sort") countingSort(data, c, s);
    else if (algo == "Radix Sort")    radixSort(data, c, s);
    else if (algo == "Bucket Sort")   bucketSort(data, c, s);

    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1-t0).count();

    bool stable = (algo=="Merge Sort"||algo=="Counting Sort"||algo=="Radix Sort"||algo=="Bucket Sort");

    std::string best, avg, worst, space;
    if      (algo=="Merge Sort")    { best="O(n log n)"; avg="O(n log n)"; worst="O(n log n)"; space="O(n)"; }
    else if (algo=="Quick Sort")    { best="O(n log n)"; avg="O(n log n)"; worst="O(n²)";      space="O(log n)"; }
    else if (algo=="Heap Sort")     { best="O(n log n)"; avg="O(n log n)"; worst="O(n log n)"; space="O(1)"; }
    else if (algo=="Counting Sort") { best="O(n + k)";   avg="O(n + k)";   worst="O(n + k)";   space="O(k)"; }
    else if (algo=="Radix Sort")    { best="O(nk)";      avg="O(nk)";      worst="O(nk)";      space="O(n + k)"; }
    else if (algo=="Bucket Sort")   { best="O(n + k)";   avg="O(n + k)";   worst="O(n²)";      space="O(n)"; }

    return { ms, c, s, stable, best, avg, worst, space };
}

// ─── Bindings ────────────────────────────────────────────────────
EMSCRIPTEN_BINDINGS(engine) {
    register_vector<double>("VectorDouble");

    value_object<SortMetrics>("SortMetrics")
        .field("time",             &SortMetrics::time)
        .field("comparisons",      &SortMetrics::comparisons)
        .field("swaps",            &SortMetrics::swaps)
        .field("stable",           &SortMetrics::stable)
        .field("complexity_best",  &SortMetrics::complexity_best)
        .field("complexity_avg",   &SortMetrics::complexity_avg)
        .field("complexity_worst", &SortMetrics::complexity_worst)
        .field("complexity_space", &SortMetrics::complexity_space);

    function("analyzeSort", &analyzeSort);
}
