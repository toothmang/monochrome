
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

#include <string>
#include <map>

#define WS_PORT 8987

typedef uint8_t headertype;
const size_t headerlen = sizeof(headertype);

// TODO: CIRCULAR buffers to avoid the data slide on send

const size_t READ_BUFFER_LEN = 1024 - sizeof(int);
struct readbuffer {
    char data[READ_BUFFER_LEN];
    int used;
};

const size_t WRITE_BUFFER_LEN = 4096 - sizeof(int);
struct writebuffer {
    char data[WRITE_BUFFER_LEN];
    int used;
};

std::map<int, readbuffer> readbuffers;
std::map<int, writebuffer> writebuffers;

typedef void (*message_parse_callback)(std::string message);

int do_msg_read(int sockfd, message_parse_callback callback)
{
    int res;

    if (! readbuffers.count(sockfd))
        readbuffers[sockfd] = readbuffer();

    readbuffer &buf = readbuffers[sockfd];
    int available = READ_BUFFER_LEN - buf.used - 1;

    res = recv(sockfd, &buf.data[buf.used], available, 0);

    if (res == -1) {
        assert (errno == EAGAIN);
        return res;
    }

    // printf("\tRead %d bytes: ", res);
    // for (int j = 0; j < res; j++)
    //     printf("%02X ", buf.data[buf.used+j]);
    // printf("\n");

    buf.used += res;

    while (buf.used > headerlen && buf.used >= (headertype)buf.data[0])
    {
        int message_len = headerlen + (headertype)buf.data[0];
        callback( std::string(buf.data+headerlen, buf.data+message_len) );

        for (int i = 0; i < buf.used - message_len; i++)
            buf.data[i] = buf.data[i+message_len];

        buf.used -= message_len;

        printf("\tParsed message with length %d\n", message_len-headerlen);
    }
    return res;
}

void do_msg_enqueue(int sockfd, std::string msg)
{
    if (! writebuffers.count(sockfd))
        writebuffers[sockfd] = writebuffer();

    writebuffer &buf = writebuffers[sockfd];
    int available = WRITE_BUFFER_LEN - buf.used - 1;

    if (available < msg.length() + headerlen)
    {
        //TODO: force disconnect client
    }

    *((headertype*) &(buf.data[buf.used])) = msg.length();
    buf.used += headerlen;
    for (int i = 0; i < msg.length(); i++) {
        buf.data[buf.used] = msg[i];
        buf.used += 1;
    }
}

void do_msg_dispatch()
{
    int res;

    for (auto iter = writebuffers.begin(); iter != writebuffers.end(); ++iter)
    {
        writebuffer &buf = iter->second;
        res = send(iter->first, buf.data, buf.used, 0);

        if (res <= 0) continue;

        for (int i = 0; i < buf.used - res; ++i){
            buf.data[i] = buf.data[i+res];
        }
        buf.used -= res;
    }
}

int do_msg_write(int sockfd, std::string msg)
{
    printf("MSG WRITE\n");

    static char send_buffer[READ_BUFFER_LEN];
    int res;

    printf("\tRequest to send [%s]\n", msg.c_str());

    send_buffer[0] = (headertype)msg.length();
    for (int i = 0; i < msg.length(); i++)
        send_buffer[i+headerlen] = msg[i];

    res = send(sockfd, send_buffer, msg.length() + headerlen, 0);
    //printf("\tWrote %d bytes\n", res);

    printf("Wrote %d bytes: ", res);
    for (int i = 0; i < res; i++) printf("%02X ", send_buffer[i]);
    printf("\n");

    if (res == -1) {
        assert(errno == EAGAIN);
    }

    assert(res == msg.length() + headerlen);
    return res;
}


// #### Serialization and Deserialization Functions ################################################

typedef int playerID_t;
typedef int sequence_t;
typedef uint8_t colorID_t;
typedef uint16_t timestamp_t;

std::string serialize_player(playerID_t pID, glm::vec2 position, glm::vec2 velocity, colorID_t color, timestamp_t timestamp)
{
    char buffer[24];

    buffer[0] = 0x1;
    memcpy(&color,      &buffer[1],  1);
    memcpy(&timestamp,  &buffer[2],  2);
    memcpy(&pID,        &buffer[4],  4);
    memcpy(&position.x, &buffer[8],  4);
    memcpy(&position.y, &buffer[12], 4);
    memcpy(&velocity.x, &buffer[16], 4);
    memcpy(&velocity.y, &buffer[20], 4);


    return std::string(buffer, buffer+24);
}
std::string serialize_projectile(playerID_t pID, sequence_t seqID, glm::vec2 position, glm::vec2 velocity, colorID_t color, timestamp_t launch_time)
{
    char buffer[28];

    buffer[0] = 0x2;
    memcpy(&color,      &buffer[1],  1);
    memcpy(&launch_time,&buffer[2],  2);
    memcpy(&pID,        &buffer[4],  4);
    memcpy(&seqID,      &buffer[8],  4);
    memcpy(&position.x, &buffer[12], 4);
    memcpy(&position.y, &buffer[16], 4);
    memcpy(&velocity.x, &buffer[20], 4);
    memcpy(&velocity.y, &buffer[24], 4);

    return std::string(buffer, buffer+28);
}
std::string serialize_newcolor(colorID_t cID, glm::vec3 color)
{
    char buffer[16];

    buffer[0] = 0x3;
    memcpy(&cID,     &buffer[1],  1);
    memcpy(&color.r, &buffer[4],  4);
    memcpy(&color.g, &buffer[8],  4);
    memcpy(&color.b, &buffer[12], 4);

    return std::string(buffer, buffer+16);
}

std::tuple<playerID_t, glm::vec2, glm::vec2, colorID_t, timestamp_t> deserialize_player(std::string message)
{
    assert(message[0] == 0x1);
    assert(message.length() == 24);

    colorID_t color       =   *(colorID_t*) &message[1];
    timestamp_t timestamp = *(timestamp_t*) &message[2];
    playerID_t pID        =  *(playerID_t*) &message[4];
    glm::vec2 position    = glm::vec2( *(float*) &message[8],  *(float*) &message[12] );
    glm::vec2 velocity    = glm::vec2( *(float*) &message[16], *(float*) &message[20] );

    return std::tuple<playerID_t, glm::vec2, glm::vec2, colorID_t, timestamp_t>(pID, position, velocity, color, timestamp);
}

std::tuple<playerID_t, int, glm::vec2, glm::vec2, colorID_t, timestamp_t> deserialize_projectile(std::string message)
{
    assert(message[0] == 0x2);
    assert(message.length() == 28);

    colorID_t color       =   *(colorID_t*) &message[1];
    timestamp_t timestamp = *(timestamp_t*) &message[2];
    playerID_t pID        =  *(playerID_t*) &message[4];
    sequence_t sID        =  *(playerID_t*) &message[8];
    glm::vec2 position    = glm::vec2( *(float*) &message[12], *(float*) &message[16] );
    glm::vec2 velocity    = glm::vec2( *(float*) &message[20], *(float*) &message[24] );

    return std::tuple<playerID_t, int, glm::vec2, glm::vec2, colorID_t, timestamp_t>(pID, sID, position, velocity, color, timestamp);
}

std::pair<colorID_t, glm::vec3> deserialize_color(std::string message)
{
    assert(message[0] == 0x3);
    assert(message.length() == 16);

    colorID_t cID   = *(colorID_t*) &message[1];
    glm::vec3 color = glm::vec3( *(float*) &message[4], *(float*) &message[8], *(float*) &message[12] );

    return std::pair<colorID_t, glm::vec3>(cID, color);
}
