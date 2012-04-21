#ifndef PACKET_H_
#define PACKET_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>


struct NetPacket {
	enum {
		PACKET_MSG = 0,
		PACKET_BROADCAST_PEERID = 1,
		PACKET_ASSIGN_PEERID = 2,
		PACKET_BROADCAST_PEERLISTSIZE = 3,
		PACKET_HELLO = 4,
		PACKET_ACTIONS_DONE = 5
	};
	enum {
		HEADER_LEN = sizeof(int)*3, MAX_PACKET_SIZE = 512
	};

	NetPacket(int packet_origin = 0);
	size_t length() const;
    char *body();
    bool decode_header();
    void encode_header();

	void add_int(long val);
	void add_double(double val);
	long get_int();
	double get_double();

	template<typename T>
	void add(const T& t) {
		memcpy(body() + body_length, &t, sizeof(T));
		body_length += sizeof(T);
	}
	template<typename T>
	void get(T& t) {
		body_length -= sizeof(T);
		memcpy(&t, body() + body_length, sizeof(T));
	}

	char data[HEADER_LEN + MAX_PACKET_SIZE];
	size_t body_length;
	int packet_type;
	int packet_origin;
};

#endif /* PACKET_H_ */