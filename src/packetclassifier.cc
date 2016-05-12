#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <click/packet.hh>

#include "packetclassifier.hh" 
#include "custom_packets.hh"

#define HELLO_TYPE 1
#define ROUTING_UPDATE_TYPE 2
#define ACKNOWLEDGMENT_TYPE 3
#define MULTICAST_DATA_TYPE 4


CLICK_DECLS 

PacketClassifier::PacketClassifier() {
}

PacketClassifier::~PacketClassifier(){
	
}

int PacketClassifier::initialize(ErrorHandler *errh){
    return 0;
}

void PacketClassifier::push(int port, Packet *packet) {

	assert(packet);
	struct common_header *header = (struct common_header *)packet->data();

	if(header->type == HELLO_TYPE) {

                // click_chatter("It's a hello packet.");
		output(0).push(packet);

	} else if(header->type == ROUTING_UPDATE_TYPE) {

                // click_chatter("It's an update packet.");
		output(1).push(packet);

	} else if(header->type == ACKNOWLEDGMENT_TYPE) {

                // click_chatter("It's an ack packet.");
		output(2).push(packet);

	} else if(header->type == MULTICAST_DATA_TYPE) {

                // click_chatter("It's a multicast data packet.");
                output(3).push(packet);

        } else {

		// click_chatter("Wrong packet type");
		packet->kill();
	}

}


CLICK_ENDDECLS 
EXPORT_ELEMENT(PacketClassifier)

