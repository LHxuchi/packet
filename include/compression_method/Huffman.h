#ifndef COMPRESSION_METHOD_HUFFMAN
#define COMPRESSION_METHOD_HUFFMAN
#include <utility>
#include <memory>
#include "../utils/byte_conversion.h"
#include <iterator>
#include <vector>
#include <string>
#include <queue>
#include <cstring>
#include <bitset>
namespace data_packet
{

    // struct Huffman_header
    // {
    //     /* data */
    //     byte padding_length = 0;//补位的长度
    //     qword Word_with_times[256] = {0};//字母对应ASCII码+出现次数（词频表）
    //     std::unique_ptr<byte[]> Compressed_data;//压缩后数据
    // };
    
    class Huffman
    {
    public:
        struct Huffman_tn//节点
        {
            qword weight = 0;
            int32_t word = -1;//ASCII标识
            std::unique_ptr<Huffman_tn> lson, rson;

            Huffman_tn() = default;

            Huffman_tn(qword w) :weight(w)
            {
                word = -1;
            }

            Huffman_tn(qword w, int32_t word) :weight(w), word(word)
            {

            }

            Huffman_tn(const Huffman_tn& other) = delete;

            Huffman_tn(Huffman_tn&& other) noexcept :
                weight(std::move(other.weight)),
                word(std::move(other.word)),
                lson(std::move(other.lson)),
                rson(std::move(other.rson)) {
            }
            /* data */
        };
    private:
        class cmp
        {
        public:
            bool operator()(const std::unique_ptr<Huffman_tn>& a, const std::unique_ptr<Huffman_tn>& b)
            {
                return a->weight > b->weight;
            };
        };

    public:
        /**
         * @brief 建树
         * @param t 字符及出现次数
         * @return 树的头指针
         */
        Huffman()
        {
            memset(times, 0, sizeof(times));
            //memset(Huffman_coding, 0, sizeof(Huffman_coding));
        }
        std::unique_ptr<Huffman_tn> Create_Huffman_Tree();
        void encoding_dfs(Huffman_tn* node, std::string prefix);

        qword times[256];//词频表
        std::string Huffman_coding[256];//字母对应的Huffman编码
    };

    /**
  * @brief 接受前向输出迭代器指定压缩或加密数据范围，区间为[begin,end)
  * @param begin 闭区间起始位置
  * @param end 开区间结束位置
  * @return 已压缩的数据及其长度
  */
    template<typename Iter>
    std::pair<std::unique_ptr<byte[]>, size_t> Huffman_compress(const Iter& begin, const Iter& end);

    /**
      * @brief 接受前向输出迭代器指定压缩或加密数据范围，区间为[begin,begin+size)
      * @param begin 闭区间起始位置
      * @param size 区间大小
      * @return 已压缩的数据及其长度
      */
    template<typename Iter>
    std::pair<std::unique_ptr<byte[]>, size_t> Huffman_compress(const Iter& begin, size_t buffer_size);

    template<typename Iter>
    std::pair<std::unique_ptr<byte[]>, size_t> Huffman_decompress(const Iter& begin, const Iter& end);

    template<typename Iter>
    std::pair<std::unique_ptr<byte[]>, size_t> Huffman_decompress(const Iter& begin, size_t buffer_size);



    template<typename Iter>
    std::pair<std::unique_ptr<data_packet::byte[]>, size_t>data_packet::Huffman_compress(const Iter& begin, const Iter& end)
    {
        return Huffman_compress(begin, size_t(end - begin));
    }
    template<typename Iter>
    std::pair<std::unique_ptr<data_packet::byte[]>, size_t> data_packet::Huffman_compress(const Iter& begin, size_t buffer_size)
    {
        static_assert(sizeof(typename std::iterator_traits<Iter>::value_type) == sizeof(uint8_t), "Element type must be 1 byte");
        data_packet::Huffman huffman;
        auto it = begin;
        for (size_t i = 0; i < buffer_size; i++)
        {
            huffman.times[(uint8_t)*it++]++;
        }

        //建Huffman树
        std::unique_ptr<data_packet::Huffman::Huffman_tn> huffman_tree_head = huffman.Create_Huffman_Tree();

        //遍历获取字符序列
        huffman.encoding_dfs(huffman_tree_head.get(), std::string());

        //释放Huffman树空间
        huffman_tree_head.release();

        //用字符串暂存编码后的01序列
        std::string encoding_stream;

        it = begin;
        for (size_t i=0;i<buffer_size;i++)
        {
            encoding_stream += huffman.Huffman_coding[(uint8_t)*it++];
        }

        //计算补位长度
        byte padding_length = (encoding_stream.size() % 8 == 0 ? 0 : 8 - encoding_stream.size() % 8);

        //计算长度
        const size_t compressed_data_length = (8 + 64 * 256 + encoding_stream.size() + padding_length) / 8;//2049个byte存头文件，string/8+[1]

        std::unique_ptr<byte[]> compressed_data = std::make_unique<byte[]>(compressed_data_length);

        //写入文件头及压缩数据
        compressed_data[0] = padding_length;

        //定长词频表
        for (uint32_t i = 1; i <= 256 * 8; i += 8)
        {
            compressed_data[i] |= (byte)(huffman.times[(i - 1) >> 3] >> 56 & 0xff);
            compressed_data[i + 1] |= (byte)(huffman.times[(i - 1) >> 3] >> 48 & 0xff);
            compressed_data[i + 2] |= (byte)(huffman.times[(i - 1) >> 3] >> 40 & 0xff);
            compressed_data[i + 3] |= (byte)(huffman.times[(i - 1) >> 3] >> 32 & 0xff);
            compressed_data[i + 4] |= (byte)(huffman.times[(i - 1) >> 3] >> 24 & 0xff);
            compressed_data[i + 5] |= (byte)(huffman.times[(i - 1) >> 3] >> 16 & 0xff);
            compressed_data[i + 6] |= (byte)(huffman.times[(i - 1) >> 3] >> 8 & 0xff);
            compressed_data[i + 7] |= (byte)(huffman.times[(i - 1) >> 3] & 0xff);
        }

        size_t i = 0, j = 256 * 8 + 1;
        //压缩数据
        if(encoding_stream.size()>=8)
        for (; i < encoding_stream.size() + (padding_length==0?8:padding_length) - 8; i += 8, j++)
        {
            std::bitset<8> tmp(encoding_stream.substr(i, 8));
            compressed_data[j] = (byte)tmp.to_ullong();
        }
        //补位
        if (padding_length != 0)
        {
            std::bitset<8> tmp(encoding_stream.substr(i, 8 - padding_length));
            compressed_data[j] = (byte)((tmp << padding_length).to_ullong());
        }

        return std::make_pair(std::move(compressed_data), compressed_data_length);
    }

    template <typename Iter>
    std::pair<std::unique_ptr<data_packet::byte[]>, size_t> data_packet::Huffman_decompress(const Iter& begin, const Iter& end)
    {
        return Huffman_decompress(begin, size_t(end - begin));
    }

    template <typename Iter>
    std::pair<std::unique_ptr<data_packet::byte[]>, size_t> data_packet::Huffman_decompress(const Iter& begin, size_t buffer_size)
    {
        static_assert(sizeof(typename std::iterator_traits<Iter>::value_type) == sizeof(uint8_t), "Element type must be 1 byte");
        Huffman decoding_huffman;
        auto it = begin;
        byte padding_length = *it++;
        for (size_t i = 0; i < 256; i++)
        {
            for (size_t j = 0; j < 8; j++)
            {
                decoding_huffman.times[i] <<= 8;
                decoding_huffman.times[i] |= (uint8_t)*it++;
            }
        }
        
        std::string encoded_stream;

        if(buffer_size>=2050)
        {
            for (size_t i = 0; i < buffer_size - 2049-1; i++)
            {
                std::bitset<8> tmp = *it++;
                encoded_stream += tmp.to_string();
            }
            {
                std::bitset<8> tmp = *it;
                encoded_stream += tmp.to_string().substr(0,8-padding_length);
            }
        }

        std::unique_ptr<Huffman::Huffman_tn> huffman_tree_head = decoding_huffman.Create_Huffman_Tree();
        std::string decoding_stream;
        Huffman::Huffman_tn* huffman_tree_node = huffman_tree_head.get();//遍历树指针

        for (size_t i = 0; i < encoded_stream.size(); i++)
        {
            if (encoded_stream[i] == '0')
            {
                huffman_tree_node = huffman_tree_node->lson.get();
            }
            else
            {
                huffman_tree_node = huffman_tree_node->rson.get();
            }
            if (huffman_tree_node->word != -1)
            {
                decoding_stream += (byte)(huffman_tree_node->word);
                huffman_tree_node = huffman_tree_head.get();
            }
        }
        huffman_tree_node = nullptr;
        huffman_tree_head.release();
        std::unique_ptr<byte[]> decompressed_data = std::make_unique<byte[]>(decoding_stream.size());
        for (size_t i = 0; i < decoding_stream.size(); i++)
        {
            decompressed_data[i] = (byte)decoding_stream[i];
        }
        return std::pair<std::unique_ptr<byte[]>, size_t>(std::move(decompressed_data), decoding_stream.size());
    }

}
#endif