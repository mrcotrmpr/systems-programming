#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <thread>
#include "networking.hpp"
#include "throw.hpp"

void handle_client(int clientSocket, std::string word) {
    try {
        std::vector<char> buf(1024);
        int n{ 0 };

        // read and print the data received from the client
        while ((n = recv(clientSocket, buf.data(), buf.size(), 0)) > 0) {
            std::cerr << "server: received from client: ";
            for (int i = 0; i < n; ++i) {
                std::cerr << buf[i];
            }
            break;
        }

        throw_if_min1(send(clientSocket, word.c_str(), word.size(), 0));

        throw_if_min1(closesocket(clientSocket));
        std::cerr << "server: closed connection from thread: " << std::this_thread::get_id() << '\n';
    }
    catch (const std::exception& ex) {
        std::cerr << "server: " << ex.what() << '\n';
    }
}

int main() {
    // initialize words
    std::ifstream file("data/nl.words.txt");
    std::vector<std::string> words;
    std::string word;
    while (file >> word) {
        words.push_back(word);
    }

    // initialize shared random engine
    auto rng = std::make_shared<std::mt19937>(std::random_device{}());
    std::uniform_int_distribution<size_t> distribution(0, words.size() - 1);

    try {
        // create stream socket
        int server{ 0 };
        throw_if_min1(server = socket(AF_INET, SOCK_STREAM, 0));

        // fill in address struct for server address
        struct sockaddr_in sa;
        std::memset(&sa, 0, sizeof(struct sockaddr_in));
        sa.sin_family = AF_INET;
        sa.sin_port = htons(4242);
        throw_if_min1(inet_pton(AF_INET, "localhost", &sa.sin_addr.s_addr));

        // bind server to this address
        throw_if_min1(bind(server, reinterpret_cast<struct sockaddr*>(&sa),
            sizeof(sa)));

        // make server socket passive
        throw_if_min1(listen(server, SOMAXCONN));

        while (true) {
            // setup client address struct so we can show it
            int client{ 0 };
            struct sockaddr_storage client_storage;
            socklen_t len{ sizeof(struct sockaddr_storage) };
            struct sockaddr* client_endpoint =
                reinterpret_cast<struct sockaddr*>(&client_storage);

            // wait for a client to connect
            std::cerr << "server: waiting for client\n";
            throw_if_min1(client = accept(server, client_endpoint, &len));

            // translate client address to human readable form
            const struct sockaddr_in* addr =
                reinterpret_cast<const struct sockaddr_in*>(client_endpoint);
            std::vector<char> cbuf(INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &addr->sin_addr, cbuf.data(), cbuf.size());

            // show client address
            std::cerr << "server: got connection from " << cbuf.data() << ":"
                << ntohs(addr->sin_port) << std::endl;

            // start a new thread to handle this client
            std::string word = words[distribution(*rng)];
            std::thread client_thread(handle_client, client, word);
            client_thread.detach();
        }
        throw_if_min1(closesocket(server));

    }
    catch (const std::exception& ex) {
        std::cerr << "server: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
