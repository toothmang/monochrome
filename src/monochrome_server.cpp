#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <winsock2.h>
    typedef int socklen_t;
    #define close closesocket
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/ioctl.h>
    #include <sys/socket.h>
#endif

#include <sys/types.h>
#include <emscripten.h>

#include "glm/glm.hpp"

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

// Fwd declarations, so they can be used as callbacks in other funcs. Really, just _close.
void async_socket_connection(int fd, void* userData);
void async_socket_message(int fd, void* userData);
void async_socket_close(int fd, void* userData);

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

// These should all be wrapped in a context...

Game *game = NULL;

std::vector<int> connections;
std::vector<playerID_t> connection_IDs;
playerID_t next_pID = 1;
timestamp_t simTime = 0;
float time_remainder = 0.;

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
            do_msg_enqueue(client, msg, async_socket_close);
    }

    printf("\n");
    do_msg_dispatch();

    return;
}

void async_socket_connection(int fd, void* userData) {
    client.fd = accept(server.fd, NULL, NULL);
    connections.push_back(fd);
    connection_IDs.push_back(next_pID);

    do_msg_enqueue(fd, serialize_intromessage(next_pID, simTime), async_socket_close);
    next_pID += 1;
}

void broadcast(std::string message)
{
    for (auto client : connections)
        do_msg_enqueue(client, message, async_socket_close);
}

void deserialize(std::string message)
{
    playerID_t pID;
    sequence_t seq;
    colorID_t cID;
    timestamp_t ts;
    glm::vec2 position, velocity;
    glm::vec3 color;

    int playerIndex;

    if (message[0] == 0x1) {
        std::tie(pID, position, velocity, cID, ts) = deserialize_player(message);
        auto lookup = game->player_lookup.find(pID);
        if (lookup == game->player_lookup.end()) {
            playerIndex = game->players.size();
            //game->addPlayer(pID, true, cID);
            game->players.push_back(Player(game, position, pID, true, cID, 30., 15.));
        } else {
            playerIndex = lookup->second;
        }
        auto &player = game->players[playerIndex];

        player.pos = position;
        player.vel = velocity;
        player.timestamp = ts;
        player.colorId = cID;

        broadcast(message);
    }
    if (message[0] == 0x2) {
        std::tie(pID, seq, position, velocity, cID, ts) = deserialize_projectile(message);
        //TODO: DO
    }
    if (message[0] == 0x3) {
        std::tie(cID, color) = deserialize_color(message);

        while (game->colors.size() <= cID) game->colors.push_back( glm::vec4(0.) );

        game->colors[cID] = glm::vec4(color, 1.);

        broadcast(message);
    }
    // Message 0x4 == dropmessage  is never sent from the client.
    // Message 0x5 == intromessage is never sent from the client.
    // Message 0x6 == event   is never sent from the client.

}

void async_socket_message(int fd, void* userData) {
    do_msg_read(fd, deserialize);
}

void async_socket_close(int fd, void* userData) {
    int index = 0;
    while (index < connections.size())
        if (connections[index] == fd)
            break;

    if (index >= connections.size()) return;

    //connections.erase(fd);

    purge_buffers(fd);
    for (auto client : connections)
        do_msg_enqueue(client, serialize_dropmsg(connection_IDs[index]), async_socket_close);

    connections.erase( connections.begin() + index );
    connection_IDs.erase( connection_IDs.begin() + index);
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

    emscripten_set_socket_connection_callback(NULL, async_socket_connection);
    emscripten_set_socket_message_callback(NULL, async_socket_message);
    emscripten_set_socket_close_callback(NULL, async_socket_close);

    emscripten_set_main_loop(main_loop, 1, 0);

    return EXIT_SUCCESS;
}
