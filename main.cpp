#include <iostream>

#include "Client.hpp"

int main()
{
    Client client;
    client.connect_to("127.0.0.1", 13372);
    client.run();
    return 0;
}
