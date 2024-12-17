//#define NOMINMAX
//#include <windows.h>
//#include <psapi.h>
//#undef min
//#undef max
//
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <vector>
//#include <unordered_map>
//#include <chrono>
//#include <algorithm>
//#include <stdexcept>
//#include <omp.h>
//
//// Suffix Array ���� �Լ�
//std::vector<int> build_suffix_array(const std::string& text) {
//    int n = text.size();
//    std::vector<int> suffix_array(n), rank(n), temp(n);
//
//    for (int i = 0; i < n; ++i) {
//        suffix_array[i] = i;
//        rank[i] = text[i];
//    }
//
//    for (int length = 1; length < n; length *= 2) {
//        auto comparator = [&](int a, int b) {
//            if (rank[a] != rank[b]) {
//                return rank[a] < rank[b];
//            }
//            int rank_a = (a + length < n) ? rank[a + length] : -1;
//            int rank_b = (b + length < n) ? rank[b + length] : -1;
//            return rank_a < rank_b;
//            };
//        std::sort(suffix_array.begin(), suffix_array.end(), comparator);
//
//        temp[suffix_array[0]] = 0;
//        for (int i = 1; i < n; ++i) {
//            temp[suffix_array[i]] = temp[suffix_array[i - 1]] + comparator(suffix_array[i - 1], suffix_array[i]);
//        }
//        rank = temp;
//    }
//
//    return suffix_array;
//}
//
//// �ִ� ���� ��ġ (MUM) ã��
//std::vector<int> find_mum(const std::string& reference, const std::string& query, const std::vector<int>& suffix_array) {
//    int ref_len = reference.size();
//    int query_len = query.size();
//    int lcp = 0;
//    std::vector<int> mums;
//
//    for (int i = 0; i < ref_len; ++i) {
//        int suffix_pos = suffix_array[i];
//        lcp = 0;
//        while (suffix_pos + lcp < ref_len && lcp < query_len && reference[suffix_pos + lcp] == query[lcp]) {
//            ++lcp;
//        }
//        if (lcp == query_len) {
//            mums.push_back(suffix_pos);
//        }
//    }
//
//    return mums;
//}
//
//// ���̳ʸ� ���Ͽ��� ���� ���� �б�
//std::string load_reference(const std::string& filename) {
//    std::ifstream file(filename, std::ios::binary);
//    if (!file) throw std::runtime_error("Failed to open reference file.");
//
//    size_t length;
//    file.read(reinterpret_cast<char*>(&length), sizeof(length));
//    if (!file || length == 0) throw std::runtime_error("Invalid reference file format.");
//
//    std::string reference(length, '\0');
//    file.read(&reference[0], length);
//
//    if (!file) throw std::runtime_error("Error reading reference sequence.");
//    return reference;
//}
//
//// ���̳ʸ� ���Ͽ��� ���� �б�
//std::vector<std::pair<std::string, std::string>> load_reads(const std::string& filename) {
//    std::ifstream file(filename, std::ios::binary);
//    if (!file) throw std::runtime_error("Failed to open reads file.");
//
//    std::vector<std::pair<std::string, std::string>> reads;
//
//    while (file) {
//        size_t size;
//        file.read(reinterpret_cast<char*>(&size), sizeof(size));
//        if (file.eof()) break;
//
//        std::string read(size, '\0');
//        file.read(&read[0], size);
//
//        auto tab_pos = read.find('\t');
//        if (tab_pos != std::string::npos) {
//            reads.emplace_back(read.substr(0, tab_pos), read.substr(tab_pos + 1));
//        }
//    }
//
//    return reads;
//}
//
//// ��Ȯ�� ���
//float calculate_accuracy(const std::string& reference, const std::string& reconstructed) {
//    int matches = 0;
//    for (size_t i = 0; i < reference.size(); ++i) {
//        if (reference[i] == reconstructed[i]) matches++;
//    }
//    return static_cast<float>(matches) / reference.size() * 100.0f;
//}
//
//// ���⼭�� �籸�� (�ֺ� Ȱ��)
//std::string reconstruct_sequence(const std::string& reference, const std::vector<std::pair<std::string, std::string>>& reads, const std::vector<int>& suffix_array) {
//    std::vector<std::unordered_map<char, int>> alignment(reference.size());
//    std::string reconstructed(reference.size(), 'N');
//
//    for (const auto& read_pair : reads) {
//        const std::string& forward_read = read_pair.first;
//        const std::string& reverse_read = read_pair.second;
//
//        auto forward_mums = find_mum(reference, forward_read, suffix_array);
//        auto reverse_mums = find_mum(reference, reverse_read, suffix_array);
//
//        for (int pos : forward_mums) {
//            for (size_t i = 0; i < forward_read.size(); ++i) {
//                if (pos + i < reference.size()) {
//                    alignment[pos + i][forward_read[i]]++;
//                }
//            }
//        }
//
//        for (int pos : reverse_mums) {
//            for (size_t i = 0; i < reverse_read.size(); ++i) {
//                if (pos + i < reference.size()) {
//                    alignment[pos + i][reverse_read[i]]++;
//                }
//            }
//        }
//    }
//
//    // �ֺ��� �̿��Ͽ� ���⼭�� �籸��
//    for (size_t i = 0; i < alignment.size(); ++i) {
//        if (!alignment[i].empty()) {
//            auto max_it = std::max_element(alignment[i].begin(), alignment[i].end(),
//                [](const auto& a, const auto& b) { return a.second < b.second; });
//            reconstructed[i] = max_it->first;
//        }
//    }
//
//    return reconstructed;
//}
//
//// �޸� ��뷮 ���
//void print_memory_usage() {
//    PROCESS_MEMORY_COUNTERS memInfo;
//    GetProcessMemoryInfo(GetCurrentProcess(), &memInfo, sizeof(memInfo));
//    std::cout << "Memory usage: " << memInfo.PeakWorkingSetSize / 1024 << " KB\n";
//}
//
//// ���� �Լ�
//int main() {
//    try {
//        auto start_time = std::chrono::high_resolution_clock::now();
//
//        // ������ �ε�
//        std::string reference = load_reference("reference_sequence.bin");
//        auto reads = load_reads("paired_reads_with_errors.bin");
//
//        // Suffix Array ����
//        auto suffix_array = build_suffix_array(reference);
//
//        std::cout << "Reference sequence length: " << reference.size() << '\n';
//        std::cout << "Number of paired reads: " << reads.size() << '\n';
//
//        // ���⼭�� �籸��
//        std::string reconstructed_sequence = reconstruct_sequence(reference, reads, suffix_array);
//
//        // ��Ȯ�� ���
//        float accuracy = calculate_accuracy(reference, reconstructed_sequence);
//
//        auto end_time = std::chrono::high_resolution_clock::now();
//        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
//
//        // ��� ���
//        std::cout << "Reconstructed Sequence (First 100 bases): " << reconstructed_sequence.substr(0, 100) << '\n';
//        std::cout << "Reconstruction Accuracy: " << accuracy << "%\n";
//        std::cout << "Execution time: " << elapsed_time << " ms\n";
//        print_memory_usage();
//
//    }
//    catch (const std::exception& e) {
//        std::cerr << "Error: " << e.what() << '\n';
//        return EXIT_FAILURE;
//    }
//
//    return EXIT_SUCCESS;
//}
