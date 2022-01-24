//
// Created by Алексей Гладков on 20.01.2022.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Client.hpp"

Client::Client()
{
    this->sock_d = socket(AF_INET, SOCK_STREAM, 0);
}

/**
 * Connects to server. Must be called before run
 * @param ip_address
 * @param port
 * @throws std::runtime_error
 */
void Client::connect_to(const std::string& ip_address, int port) const
{
    sockaddr_in address_info{};
    address_info.sin_family = AF_INET;
    address_info.sin_port = htons(port);
    inet_aton(ip_address.c_str(), reinterpret_cast<in_addr*>(&(address_info.sin_addr.s_addr)));

    auto connect_result = connect(this->sock_d, reinterpret_cast<const sockaddr*>(&address_info),
                                  sizeof(address_info));

    if (connect_result == -1)
        throw std::runtime_error(strerror(errno));
}

/**
 * Runs main app loop
 */
void Client::run()
{
    std::string request;

    auto answer = receive_all();
    std::cout << answer;

    std::getline(std::cin, request);
    send_all(request);

    while (true)
    {
        std::getline(std::cin, request);

        auto start = std::chrono::high_resolution_clock::now();
        if (request.find("send") == 0)
        {
            try
            {
                request = "send " + get_binary_file(request.substr(5));

                std::cout << "Sending..." << std::endl;
            }
            catch (std::runtime_error& error)
            {
                std::cout << error.what() << std::endl;
                continue;
            }
        }

        send_all(request);

        answer = receive_all();
        std::cout << std::string(answer.begin(), answer.end()) << std::endl;
        auto stop = std::chrono::high_resolution_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " s." << std::endl;
    }
}

/**
 * Sends message to server. Message is split into arrays of 4096 bytes and sent chunk by chunk
 * @param message - message to be sent
 */
void Client::send_all(const std::string& message) const
{
    char data[4096] = {0};
    size_t to_send = message.size() + sizeof(ulong);

    std::cout << "Chunks in message:" << message.size() / 4096 + 1 << std::endl;

    reinterpret_cast<ulong*>(data)[ 0 ] = static_cast<ulong>(to_send);


    memcpy(reinterpret_cast<char*>(data + sizeof(ulong)),
           message.data(),
           std::min(sizeof(data) - sizeof(ulong), to_send));

    to_send -= send_chunk(data, std::min(to_send, sizeof(data)));

    int count = 1;
    while (to_send > 0)
    {
        ulong length = (ulong)std::min(to_send, sizeof(data));

        memcpy(reinterpret_cast<char*>(data),
               (message.data() + sizeof(data) * count - sizeof(ulong)),
               length);
        ++count;

        to_send -= send_chunk(data, length);
    }
}

/**
 * Sends an array of bytes to server
 * @param buf - bytes array
 * @param length - a number of bytes to be sent
 * @return a number of actually sent bytes
 */
long Client::send_chunk(char* buf, size_t length) const
{
    ssize_t total = 0;
    ssize_t sent = 0;
    while (total < length)
    {
        sent = send(this->sock_d, buf + total, length - total, 0);
        if (sent == -1)
        {
            throw std::runtime_error(strerror(errno));
        }
        total += sent;
    }

    return total;
}

/**
 * Receives message from server
 * @return received message
 */
std::string Client::receive_all() const
{
    std::string result;
    char data[4096] = {0};

    auto size_chunk = receive_chunk(sizeof(ulong));
    for (int i = 0; i < sizeof(ulong); ++i)
        data[ i ] = size_chunk[ i ];

    if (reinterpret_cast<ulong*>(data)[ 0 ] > sizeof(data) - sizeof(ulong))
    {
        auto to_receive = reinterpret_cast<ulong*>(data)[ 0 ] - sizeof(ulong);

        while (to_receive > 0)
        {
            auto chunk = std::string();
            if (to_receive < sizeof(data) - sizeof(ulong))
                chunk = receive_chunk(to_receive);
            else
                chunk = receive_chunk(sizeof(data));

            to_receive -= chunk.size();
            result.insert(result.end(), chunk.begin(), chunk.end());
        }
    }
    else
    {
        auto chunk = receive_chunk(reinterpret_cast<ulong*>(data)[ 0 ] - sizeof(ulong));
        result.insert(result.end(), chunk.begin(), chunk.end());
    }

    return result;
}

/**
 * Receives bytes array
 * @param to_receive - a number of bytes to receive
 * @return received array converted to string
 */
std::string Client::receive_chunk(ulong to_receive) const
{
    std::string result;
    char buf[4096] = {0};
    long total = 0;

    while (total != to_receive)
    {
        auto received = recv(this->sock_d, buf + total, to_receive - total, 0);
        if (received == -1)
            throw std::runtime_error(strerror(errno));
        total += received;
    }

    for (int i = 0; i < to_receive; ++i)
    {
        result.push_back(buf[i]);
    }

    return result;
}

/**
 * Loads file into string
 * @param path - path to file
 * @return string with file data
 */
std::string Client::get_binary_file(const std::string& path)
{
    std::ifstream file_reader(path, std::ios::binary);
    system(("md5 " + path).c_str());

    if (!file_reader.is_open())
        throw std::runtime_error("File not found!");

    std::stringstream s;
    s << file_reader.rdbuf();

    return s.str();

}
