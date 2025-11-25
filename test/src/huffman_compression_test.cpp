#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <cstring>
#include <algorithm>
#include <utility>
#include "../../include/compression_method/huffman.h"

using namespace data_packet;

// 辅助函数：比较两个字节序列是否相等
template<typename Iter1, typename Iter2>
bool compare_bytes(Iter1 begin1, Iter1 end1, Iter2 begin2, Iter2 end2) {
    return std::equal(begin1, end1, begin2, end2);
}

TEST_CASE("Huffman Tree Construction", "[huffman_tree]") {
    Huffman huff;

    SECTION("Single character frequency") {
        huff.times['A'] = 5;
        auto tree = huff.Create_Huffman_Tree();
        REQUIRE(tree != nullptr);
        REQUIRE(tree->word == -1);
        REQUIRE(tree->weight == 5);
        REQUIRE(tree->lson != nullptr);
        REQUIRE(tree->rson == nullptr);
        REQUIRE(tree->lson->word=='A');
        REQUIRE(tree->lson->weight==5);

    }

    SECTION("Multiple characters") {
        huff.times['a'] = 10;
        huff.times['b'] = 20;
        auto tree = huff.Create_Huffman_Tree();
        REQUIRE(tree != nullptr);
        REQUIRE(tree->word == -1);  // 根节点不是叶子
        REQUIRE(tree->weight == 30);
        
        // 验证左右子树权重关系（左小右大）
        REQUIRE(tree->lson->weight <= tree->rson->weight);
    }
}

TEST_CASE("Huffman Coding Generation", "[huffman_code]") {
    Huffman huff;

    SECTION("Basic coding generation") {
        huff.times['x'] = 1;
        huff.times['y'] = 1;
        auto tree = huff.Create_Huffman_Tree();
        huff.encoding_dfs(tree.get(), "");
        
        // 两个字符应该生成长度为1的不同编码
        REQUIRE(huff.Huffman_coding['x'].size() == 1);
        REQUIRE(huff.Huffman_coding['y'].size() == 1);
        REQUIRE(huff.Huffman_coding['x'] != huff.Huffman_coding['y']);
    }

    SECTION("Uneven frequencies") {
        huff.times['a'] = 8;  // 高频字符应该有短编码
        huff.times['b'] = 3;
        huff.times['c'] = 1;
        huff.times['d'] = 1;
        huff.times['e'] = 1;
        huff.times['f'] = 1;
        huff.times['g'] = 1;
        huff.times['h'] = 1;
        
        auto tree = huff.Create_Huffman_Tree();
        huff.encoding_dfs(tree.get(), "");
        REQUIRE(huff.Huffman_coding['a'].size() <= huff.Huffman_coding['b'].size());
    }
}

TEST_CASE("Huffman Compression and Decompression", "[huffman_io]") {
    SECTION("Empty input") {
        std::vector<byte> empty;
        //std::pair<std::unique_ptr<data_packet::byte[]>, size_t> a=Huffman_compress(empty.begin(), empty.end()); 
        auto [compressed, comp_size] = Huffman_compress(empty.begin(), empty.end());
        auto [decompressed, decom_size] = Huffman_decompress(compressed.get(), comp_size);
        REQUIRE(decom_size == 0);
    }

    SECTION("Single character repeated") {
        const std::string input(1000, 'Z');  // 重复字符压缩率应极高
        auto [compressed, comp_size] = Huffman_compress(input.begin(), input.end());
        auto [decompressed, decom_size] = Huffman_decompress(compressed.get(), comp_size);
        
        REQUIRE(decom_size == input.size());
        REQUIRE(compare_bytes(decompressed.get(), decompressed.get() + decom_size,
                            input.begin(), input.end()));
    }

    SECTION("Random mixed characters") {
        const std::string input = "Hello, Huffman Compression! This is a test string with various characters...";
        auto [compressed, comp_size] = Huffman_compress(input.begin(), input.end());
        auto [decompressed, decom_size] = Huffman_decompress(compressed.get(), comp_size);
        
        REQUIRE(decom_size == input.size());
        REQUIRE(compare_bytes(decompressed.get(), decompressed.get() + decom_size,
                            input.begin(), input.end()));
    }

    SECTION("All possible bytes") {
        std::vector<byte> input(256);
        for (int i = 0; i < 256; ++i) {
            input[i] = static_cast<byte>(i);  // 包含所有可能的字节值
        }
        
        auto [compressed, comp_size] = Huffman_compress(input.begin(), input.end());
        auto [decompressed, decom_size] = Huffman_decompress(compressed.get(), comp_size);
        
        REQUIRE(decom_size == input.size());
        REQUIRE(compare_bytes(decompressed.get(), decompressed.get() + decom_size,
                            input.begin(), input.end()));
    }

    SECTION("Edge case: 1 byte input") {
        byte input = 0xFF;
        auto [compressed, comp_size] = Huffman_compress(&input, &input + 1);
        auto [decompressed, decom_size] = Huffman_decompress(compressed.get(), comp_size);
        
        REQUIRE(decom_size == 1);
        REQUIRE(*decompressed.get() == input);
    }
}

TEST_CASE("Huffman Compression Ratio", "[huffman_ratio]") {
    SECTION("High redundancy data") {
        const std::string input(1024, 'A');  // 高冗余数据应压缩
        auto [compressed, comp_size] = Huffman_compress(input.begin(), input.end());
        // 原始大小1024字节，压缩后应显著减小（头信息2049字节是固定开销）
        REQUIRE(comp_size < 1024 + 2049);  // 实际会小很多，这里宽松判断
    }

    SECTION("Low redundancy data") {
        std::vector<byte> input(1024);
        for (int i = 0; i < 1024; ++i) {
            input[i] = static_cast<byte>(i % 256);  // 低冗余数据可能膨胀
        }
        auto [compressed, comp_size] = Huffman_compress(input.begin(), input.end());
        // 包含所有字节类型时，头信息2049字节+数据可能膨胀
        REQUIRE(comp_size > 1024);
    }
}