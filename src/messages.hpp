#pragma once

#include "glm/glm.hpp"
#include "glm/vec2.hpp"

#include <string>
#include <map>
#include <tuple>

#define WS_PORT 8987

typedef int playerID_t;
typedef int sequence_t;
typedef uint8_t colorID_t;
typedef uint16_t timestamp_t;

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

typedef void (*message_parse_callback)(std::string message);
typedef void (*disconnect_callback)(int sockfd, void* unused);

int connect_to_server(std::string server, unsigned short port);

int do_msg_read(int sockfd, message_parse_callback callback);
void do_msg_enqueue(int sockfd, std::string msg, disconnect_callback disconn);
void do_msg_dispatch();
int do_msg_write(int sockfd, std::string msg);

void purge_buffers(int sockfd);

// #### Serialization and Deserialization Functions ################################################

std::string serialize_player(playerID_t pID, glm::vec2 position, glm::vec2 velocity, colorID_t color, timestamp_t timestamp);
std::string serialize_projectile(playerID_t pID, sequence_t seqID, glm::vec2 position, glm::vec2 velocity, colorID_t color, timestamp_t launch_time);
std::string serialize_newcolor(colorID_t cID, glm::vec3 color);
std::string serialize_dropmsg(playerID_t pID);
std::string serialize_intromessage(playerID_t pID, timestamp_t simTime);
std::string serialize_event(playerID_t colorer, playerID_t coloree, sequence_t proj_seq);

std::tuple<playerID_t, glm::vec2, glm::vec2, colorID_t, timestamp_t> deserialize_player(std::string message);
std::tuple<playerID_t, sequence_t, glm::vec2, glm::vec2, colorID_t, timestamp_t> deserialize_projectile(std::string message);
std::pair<colorID_t, glm::vec3> deserialize_color(std::string message);
playerID_t deserialize_dropmsg(std::string message);
std::pair<playerID_t, timestamp_t> deserialize_intromessage(std::string message);
std::tuple<playerID_t, playerID_t, sequence_t> deserialize_event(std::string message);
