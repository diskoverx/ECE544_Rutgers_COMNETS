#ifndef CLICK_CUSTOM_PACKETS_HH
#define CLICK_CUSTOM_PACKETS_HH

/* common header */
struct common_header{
	uint8_t type; 
        uint16_t src_address;
        uint8_t seq_num;
};

/* routing entry */
struct route_advertisement {
  	uint16_t destination_address; // The address of the destination node
  	uint8_t metric;              // The cost of the path to the destination
};

/* routing update packets */
struct routing_update_packet {
  	struct common_header header; 
        uint16_t routing_packet_length;
 	struct route_advertisement adv[356]; // Maximum 356 (100 nodes, 256 hosts)
        uint16_t route_entries;
};

/* hello packets */
struct hello_packet {
	struct common_header header;
};

/* acknowledgement packets */
struct ack_packet {
	struct common_header header;
        uint16_t dst_address;
};

/* data packet format */
struct data_packet {
        struct common_header header;
	uint8_t k_value;
	uint16_t dst_01_address;
	uint16_t dst_02_address;
	uint16_t dst_03_address;
	uint16_t payload_length;
}; 

#endif
