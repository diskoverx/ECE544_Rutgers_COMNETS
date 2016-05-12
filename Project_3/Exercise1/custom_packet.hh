#ifndef CLICK_PACKETHEADER_HH
#define CLICK_PACKETHEADER_HH

typedef struct {
    
    uint8_t type; // packet type, 0 for DATA and 1 for ACK
    uint32_t sequence_number; // sequence number to keep track of acknowledgements
    uint32_t payload_length; // Length of the payload
    in_addr destination; // Destination address
    in_addr source; // Sender address
    
} custom_packet_format;


#endif
