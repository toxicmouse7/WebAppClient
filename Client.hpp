//
// Created by Алексей Гладков on 20.01.2022.
//

#ifndef WEBAPPCLIENT_CLIENT_HPP
#define WEBAPPCLIENT_CLIENT_HPP

typedef unsigned long long ulong;

#include <string>

class Client
{
private:
    int sock_d;

    void send_all(const std::string& message) const;

    long send_chunk(char* buf, size_t length) const;

    [[nodiscard]] std::string receive_all() const;

    std::string receive_chunk(ulong to_receive) const;

    static std::string get_binary_file(const std::string& path);

public:
    Client();

    void connect_to(const std::string& ip_address, int port) const;

    [[noreturn]] void run();
};


#endif //WEBAPPCLIENT_CLIENT_HPP
