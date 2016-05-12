#ifndef CLICK_PACKETHEADER_HH
#define CLICK_PACKETHEADER_HH

#include <string>

/* DATA/ACK packet */

typedef struct {
    
    uint8_t type; // packet type, 0 for DATA and 1 for ACK
    uint32_t sequence_number; // sequence number to keep track of acknowledgements
    uint32_t payload_length; // Length of the payload
    int  destination; // Destination address
    int  source; // Sender address
    
} custom_packet_format;


/* HELLO PACKET */

typedef struct {
    
    uint8_t type; // packet type, 2 for HELLO
    uint32_t sequence_number; // sequence number to keep track of hello acknowledgements
    uint32_t payload_length; // Length of the payload
    int  destination; // Destination address
    int  source; // Sender address
 
} hello_packet_format;


#endif
