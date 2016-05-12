#ifndef CLICK_PACKETHEADER_HH
#define CLICK_PACKETHEADER_HH

typedef struct {
	uint8_t type; // 8 bit = 1 byte
	uint32_t payload_length; // 32 bit = 4 bytes
} custom_packet_format;


#endif
