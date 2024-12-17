#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <climits>
#include <random>
#include <omp.h>
#include <chrono>
#include <windows.h>
#include <psapi.h>
#define NOMINMAX



#undef min
#undef max

#include <stdexcept>


constexpr int k = 3;                // k-mer ũ��
constexpr int NUM_HASHES = 100;     // MinHash ���� ����

// �޸� ��뷮 ��� �Լ�
void print_memory_usage() {
    PROCESS_MEMORY_COUNTERS memInfo;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &memInfo, sizeof(memInfo))) {
        std::cout << "Memory Usage: " << memInfo.PeakWorkingSetSize / 1024 << " KB\n";
    }
    else {
        std::cerr << "Failed to retrieve memory usage information.\n";
    }
}

// ���̳ʸ� ���Ͽ��� ���� ���� �б�
std::string load_reference(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Error opening binary file: " + filename);
    }

    size_t length;
    file.read(reinterpret_cast<char*>(&length), sizeof(length));
    std::string sequence(length, '\0');
    file.read(&sequence[0], length);

    return sequence;
}

// ���̳ʸ� ���Ͽ��� ���� �б�
std::vector<std::string> load_reads(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Error opening paired reads binary file: " + filename);
    }

    std::vector<std::string> reads;
    while (file) {
        size_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        if (file.eof()) break;

        std::string read_data(size, '\0');
        file.read(&read_data[0], size);

        reads.push_back(read_data);
    }
    return reads;
}

// k-mer ����
std::unordered_set<std::string> extract_kmers(const std::string& sequence, int k) {
    std::unordered_set<std::string> kmers;
    for (size_t i = 0; i <= sequence.size() - k; ++i) {
        kmers.insert(sequence.substr(i, k));
    }
    return kmers;
}

// MinHash ���� ����
std::vector<int> compute_minhash(const std::unordered_set<std::string>& kmers, const std::vector<std::function<int(const std::string&)>>& hash_functions) {
    std::vector<int> signature(hash_functions.size(), INT_MAX);

    for (const auto& kmer : kmers) {
        for (size_t i = 0; i < hash_functions.size(); ++i) {
            signature[i] = std::min(signature[i], hash_functions[i](kmer));
        }
    }

    return signature;
}

// �ؽ� �Լ� ����
std::vector<std::function<int(const std::string&)>> generate_hash_functions(int num_hashes) {
    std::vector<std::function<int(const std::string&)>> hash_functions;
    std::mt19937 rng(42); // ������ �õ�� ������ ��� ����
    std::uniform_int_distribution<int> dist(1, INT_MAX);

    for (int i = 0; i < num_hashes; ++i) {
        int a = dist(rng);
        int b = dist(rng);
        hash_functions.push_back([a, b](const std::string& kmer) {
            std::hash<std::string> hasher;
            return a * static_cast<int>(hasher(kmer)) + b;
            });
    }

    return hash_functions;
}

// MinHash ���� �� ���絵 ���
float compute_similarity(const std::vector<int>& signature1, const std::vector<int>& signature2) {
    int matches = 0;
    for (size_t i = 0; i < signature1.size(); ++i) {
        if (signature1[i] == signature2[i]) {
            matches++;
        }
    }
    return static_cast<float>(matches) / signature1.size();
}

// Main �Լ�
int main() {
    try {
        auto start_time = std::chrono::high_resolution_clock::now();

        // ���� ���� �� ���� ������ �ε�
        std::string reference_genome = load_reference("reference_sequence.bin");
        auto reads = load_reads("paired_reads_with_errors.bin");

        std::cout << "Reference sequence length: " << reference_genome.size() << '\n';
        std::cout << "Number of reads: " << reads.size() << '\n';

        // k-mer ���� �� MinHash ���� ����
        auto reference_kmers = extract_kmers(reference_genome, k);
        auto hash_functions = generate_hash_functions(NUM_HASHES);
        auto reference_signature = compute_minhash(reference_kmers, hash_functions);

        // �� ����� ���� ���� �� ���絵 ���
        float total_similarity = 0.0f;

        for (const auto& read : reads) {
            auto read_kmers = extract_kmers(read, k);
            auto read_signature = compute_minhash(read_kmers, hash_functions);
            float similarity = compute_similarity(reference_signature, read_signature);
            total_similarity += similarity;
        }

        // ��� ���絵 ���
        std::cout << "Average similarity: " << (total_similarity / reads.size()) * 100 << "%\n";

        auto end_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::cout << "Execution Time: " << elapsed_time << " ms\n";

        // �޸� ��뷮 ���
        print_memory_usage();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
