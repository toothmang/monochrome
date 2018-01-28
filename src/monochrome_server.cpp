// #include <assert.h>
// #include <errno.h>
// #include <fcntl.h>
// #include <signal.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// #ifdef _WIN32
//     #include <winsock2.h>
//     typedef int socklen_t;
//     #define close closesocket
//     #pragma comment(lib, "Ws2_32.lib")
// #else
//     #include <unistd.h>
//     #include <arpa/inet.h>
//     #include <netinet/in.h>
//     #include <sys/ioctl.h>
//     #include <sys/socket.h>
// #endif

#include <sys/types.h>
#include <emscripten.h>

#include "glm/glm.hpp"
#include "messages.hpp"

#include "game.h"

#include <vector>

typedef enum {
    MSG_READ,
    MSG_WRITE
} msg_state_t;

typedef struct {
    int fd;
} server_t;

typedef struct {
    int fd;
    struct sockaddr_in addr;
    std::string msg;
    msg_state_t state;
    int read;
    int wrote;
} client_t;

server_t server;
client_t client;

void cleanup(int ignored) {
    if (client.fd) {
        close(client.fd);
        client.fd = 0;
    }
    if (server.fd) {
        close(server.fd);
        server.fd = 0;
    }
}

void cleanup2() {
    cleanup(0);
}

std::vector<int> connections;

void main_loop() {
    printf("LOOP  ");

    static char msg_buffer[32];
    for (int i = 0; i < 5; ++i)
    {
        int val = rand() % 2048;
        // printf("V:%d\n", val);
        sprintf(msg_buffer, "%X", val);
        printf("%s  ", msg_buffer);
        std::string msg(msg_buffer);

        for (auto client : connections)
        {
            //do_msg_write(client, msg);
            do_msg_enqueue(client, msg);
            // printf(">");
        }
    }

    printf("\n");
    do_msg_dispatch();

    return;
}

void message_loop() {
    int res;
    fd_set fdr;
    fd_set fdw;

    // see if there are any connections to accept or read / write from
    FD_ZERO(&fdr);
    FD_ZERO(&fdw);
    //NWX// FD_SET(server.fd, &fdr);
    FD_SET(server.fd, &fdw);

    if (client.fd) FD_SET(client.fd, &fdr);
    if (client.fd) FD_SET(client.fd, &fdw);

    res = select(64, &fdr, &fdw, NULL, NULL);
    if (res == -1) {
        perror("select failed");
        exit(EXIT_SUCCESS);
    }

//NWX//
    // for TCP sockets, we may need to accept a connection
//     if (FD_ISSET(server.fd, &fdr)) {
// #if TEST_ACCEPT_ADDR
//         // Do an accept with non-NULL addr and addlen parameters. This tests a fix to a bug in the implementation of
//         // accept which had a parameter "addrp" but used "addr" internally if addrp was set - giving a ReferenceError.
//         struct sockaddr_in addr = {0};
//         addr.sin_family = AF_INET;
//         socklen_t addrlen = sizeof(addr);
//         client.fd = accept(server.fd, (struct sockaddr *) &addr, &addrlen);
// #else
//         client.fd = accept(server.fd, NULL, NULL);
// #endif
//     }

    int fd = client.fd;
    if (client.state == MSG_READ) {

        if (!FD_ISSET(fd, &fdr)) {
            return;
        }

        socklen_t addrlen = sizeof(client.addr);
        //res = do_msg_read(fd, &client.msg, client.read, 0, (struct sockaddr *)&client.addr, &addrlen);
        res = do_msg_read(fd, [](std::string message){printf("Callback: [%s]\n", message.c_str()); client.msg = message;});
        printf("Received message [%s]\n", client.msg.c_str());
        if (res == -1) {
            return;
        } else if (res == 0) {
            // client disconnected
            memset(&client, 0, sizeof(client_t));
            return;
        }

        client.read += res;

        // once we've read the entire message, echo it back
        if (client.read >= client.msg.length()) {
            client.read = 0;
            client.state = MSG_WRITE;
        }
    }

    if (client.state == MSG_WRITE) {
        if (!FD_ISSET(fd, &fdw)) {
            return;
        }

        //res = do_msg_write(fd, &client.msg, client.wrote, 0, (struct sockaddr *)&client.addr, sizeof(client.addr));
        //res = do_msg_write(fd, client.msg);
        if (res == -1) {
            return;
        } else if (res == 0) {
            // client disconnected
            memset(&client, 0, sizeof(client_t));
            return;
        }

        client.wrote += res;

        if (client.wrote >= client.msg.length()) {
            client.wrote = 0;
            client.state = MSG_READ;

#if CLOSE_CLIENT_AFTER_ECHO
            close(client.fd);
            memset(&client, 0, sizeof(client_t));
#endif
        }
    }
}

// The callbacks for the async network events have a different signature than from
// emscripten_set_main_loop (they get passed the fd of the socket triggering the event).
// In this test application we want to try and keep as much in common as the timed loop
// version but in a real application the fd can be used instead of needing to select().
void async_main_loop(int fd, void* userData) {
    printf("%s callback\n", userData);
    message_loop();
}

void async_socket_connection(int fd, void* userData) {
    printf("Woop woop!\n");
    client.fd = accept(server.fd, NULL, NULL);
    connections.push_back(fd);
}

void async_socket_message(int fd, void* userData) {

}

void async_socket_close(int fd, void* userData) {

}

int main() {

#ifdef _WIN32
    WSADATA wsaData = {};
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result) {
        printf("WSAStartup failed!\n");
        exit(1);
    }
#endif

    struct sockaddr_in addr;
    int res;

    atexit(cleanup2);
    signal(SIGTERM, cleanup);

    memset(&server, 0, sizeof(server_t));
    memset(&client, 0, sizeof(client_t));

    // create the socket and set to non-blocking
    server.fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server.fd == -1) {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }
#ifdef _WIN32
    unsigned long nonblocking = 1;
    ioctlsocket(server.fd, FIONBIO, &nonblocking);
#else
    fcntl(server.fd, F_SETFL, O_NONBLOCK);
#endif

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(WS_PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }

    res = bind(server.fd, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    res = listen(server.fd, 50);
    if (res == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // The first parameter being passed is actually an arbitrary userData pointer
    // for simplicity this test just passes a basic char*
    printf("EMSCRIPTEN ASYNC MODE\n");
    emscripten_set_socket_connection_callback((void*)"connection", async_socket_connection);
    emscripten_set_socket_message_callback((void*)"message", async_main_loop);
    emscripten_set_socket_close_callback((void*)"close", async_main_loop);

    printf("EMSCRIPTEN SYNCHRONOUS MODE\n");
    emscripten_set_main_loop(main_loop, 1, 0);

    return EXIT_SUCCESS;
}
