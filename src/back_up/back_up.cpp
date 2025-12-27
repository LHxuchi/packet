//
// Created by hyh on 2025/12/26.
//

#include "../../include/back_up/back_up.h"
#include "../../include/packet/packet.h"
#include "../../include/file_system/get_entries.h"
#include <fstream>
#include "../../include/compression_method/huffman.h"
#include "../../include/compression_method/lz77.h"
#include "../../include/encryption_method/encryption.h"


namespace
{
    data_packet::local_file_header::compression_method c_method(const std::string& method)
    {
        if (method == "LZ77")
        {
            return data_packet::local_file_header::compression_method::LZ77;
        }
        else if (method == "HUFFMAN")
        {
            return data_packet::local_file_header::compression_method::HUFFMAN;
        }
        else if (method == "NONE")
        {
            return data_packet::local_file_header::compression_method::None;
        }
        else
        {
            throw std::invalid_argument("compression method was not recognised.");
        }
    }

    data_packet::local_file_header::encryption_method e_method(const std::string& method)
    {
        if (method == "AES_256_CBC")
        {
            return data_packet::local_file_header::encryption_method::AES_256_CBC;
        }
        else if (method == "NONE")
        {
            return data_packet::local_file_header::encryption_method::None;
        }
        else
        {
            throw std::invalid_argument("encryption method was not recognised.");
        }
    }

    std::string to_string(data_packet::local_file_header::compression_method method)
    {
        switch (method)
        {
            case data_packet::local_file_header::compression_method::LZ77:
                return "LZ77";
            case data_packet::local_file_header::compression_method::HUFFMAN:
                return "HUFFMAN";
            case data_packet::local_file_header::compression_method::None:
                return "NONE";
        }

        return "UNKNOWN";
    }

    std::string to_string(data_packet::local_file_header::encryption_method method)
    {
        switch (method)
        {
            case data_packet::local_file_header::encryption_method::AES_256_CBC:
            return "AES 256 CBC";
            case data_packet::local_file_header::encryption_method::None:
            return "NONE";
            case data_packet::local_file_header::encryption_method::my_method:
            return "UNKNOWN";
        }

        return "UNKNOWN";
    }
}


std::string data_packet::back_up(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    const std::string& compression_method,
    const std::string& encryption_method,
    const std::string& password,
    const std::string& not_including_files)
{
    try
    {
        if (!std::filesystem::exists(source))
        {
            throw std::invalid_argument("source directory does not exist.");
        }
        auto pkt = make_packet(source);
        std::set<std::string> filtered_files;
        auto begin = not_including_files.begin(),end = not_including_files.begin();
        while (end != not_including_files.end())
        {
            if (*end == '\n')
            {
                filtered_files.insert({begin, end});
                begin = end + 1;
            }
            ++end;
        }

        for (auto it = pkt.packets().begin(); it != pkt.packets().end();)
        {
            if (filtered_files.find(it->info().get_file_name()) != filtered_files.end())
            {
                it = pkt.packets().erase(it);
                continue;
            }
            ++it;
        }

        pkt.refresh_file_number();

        for (auto& local_pkt:pkt.packets())
        {
            local_pkt.set_compression_method(c_method(compression_method));
            local_pkt.set_encryption_method(e_method(encryption_method));

            auto data = local_pkt.get_data().get();
            auto size = local_pkt.info().get_file_size();

            std::pair<std::unique_ptr<byte[]>, size_t> stream = {nullptr,size};

            switch (local_pkt.info().get_compression_method())
            {
                case local_file_header::compression_method::LZ77:
                    stream = lz77_compress(data,data+size);
                    break;
                case local_file_header::compression_method::HUFFMAN:
                    stream = Huffman_compress(data,data+size);
                    break;
                case local_file_header::compression_method::None:
                    break;
            }

            if (stream.first != nullptr)
            {
                local_pkt.set_data(std::move(stream.first));
                local_pkt.set_file_size(stream.second);
            }

            data = local_pkt.get_data().get();
            size = local_pkt.info().get_file_size();
            stream = {nullptr,size};

            switch (local_pkt.info().get_encryption_method())
            {
                case local_file_header::encryption_method::AES_256_CBC:
                    stream = encrypt(data,data+size,password);
                    break;
                case local_file_header::encryption_method::None:
                case local_file_header::encryption_method::my_method:
                break;
            }

            if (stream.first != nullptr)
            {
                local_pkt.set_data(std::move(stream.first));
                local_pkt.set_file_size(stream.second);
            }

            local_pkt.refresh_crc_32();
            local_pkt.refresh_creation_time();
            local_pkt.refresh_checksum();
        }

        pkt.refresh_original_file_size();
        pkt.refresh_file_size();
        pkt.refresh_creation_time();
        pkt.refresh_crc_32();
        pkt.refresh_checksum();

        std::ofstream out(destination, std::ios::binary);
        if (!out.is_open())
        {
            throw std::runtime_error("Could not open output file " + destination.string());
        }
        out << pkt;
        return {"OK"};
    }
    catch(const std::exception& e)
    {
        return e.what();
    }
}


std::string data_packet::info(const std::filesystem::path& path)
{
    try
    {
        std::ifstream input(path, std::ios::binary);
        if (!input.is_open())
        {
            throw std::runtime_error("Could not open output file " + path.string());
        }

        packet pkt;
        input >> pkt;

        std::string result;

        result.append(std::format("version:{}\n",pkt.info().get_version()));
        result.append(std::format("file size:{}\n",pkt.info().get_file_size()));
        result.append(std::format("original file size:{}\n",pkt.info().get_original_file_size()));
        result.append(std::format("creation time:{}\n",pkt.info().get_creation_time()));
        result.append(std::format("file number:{}\n",pkt.info().get_file_number()));
        result.append(std::format("compression method:{}\n",to_string(pkt.packets().front().info().get_compression_method())));
        result.append(std::format("encryption method:{}\n",to_string(pkt.packets().front().info().get_encryption_method())));
        result.append("all file names:\n");

        for (const auto& local_pkt:pkt.packets())
        {
            result.append(local_pkt.info().get_file_name() + "\n");
        }

        return result;
    }
    catch (const std::exception& e)
    {
        return e.what();
    }

}


std::string data_packet::restore_backup(const std::filesystem::path& source, const std::filesystem::path& destination,
                                        const std::string& password)
{
    try
    {
        std::ifstream input(source, std::ios::binary);
        if (!input.is_open())
        {
            throw std::runtime_error("Could not open output file " + source.string());
        }

        packet pkt;
        input >> pkt;

        for (auto& local_pkt:pkt.packets())
        {
            auto data = local_pkt.get_data().get();
            auto size = local_pkt.info().get_file_size();

            std::pair<std::unique_ptr<byte[]>, size_t> stream = {nullptr,size};

            switch (local_pkt.info().get_encryption_method())
            {
            case local_file_header::encryption_method::AES_256_CBC:
                {
                    stream = decrypt(data,data+size,password);
                    if (stream.first == nullptr)
                    {
                        throw std::runtime_error("Fail to decrypt the file " + destination.string() + ". Wrong password");
                    }
                    break;
                }
            default:
                break;
            }

            if (stream.first != nullptr)
            {
                local_pkt.set_data(std::move(stream.first));
                local_pkt.set_file_size(stream.second);
            }

            data = local_pkt.get_data().get();
            size = local_pkt.info().get_file_size();

            stream = {nullptr,size};

            switch (local_pkt.info().get_compression_method())
            {
            case local_file_header::compression_method::LZ77:
                stream = lz77_decompress(data,data+size);
                break;
            case local_file_header::compression_method::HUFFMAN:
                stream = Huffman_decompress(data,data+size);
                break;
            case local_file_header::compression_method::None:
                break;
            }

            if (stream.first != nullptr)
            {
                local_pkt.set_data(std::move(stream.first));
                local_pkt.set_file_size(stream.second);
            }

        }

        unpack_packet(destination,pkt);
        return "OK";
    }
    catch (const std::exception& e)
    {
        return e.what();
    }
}
