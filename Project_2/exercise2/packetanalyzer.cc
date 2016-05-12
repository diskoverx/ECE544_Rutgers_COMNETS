
// This packet analyzes packets

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include "packetanalyzer.hh"
#include "custom_packet.hh"

CLICK_DECLS

PacketAnalyzer::PacketAnalyzer(){}
PacketAnalyzer::~PacketAnalyzer(){}

int PacketAnalyzer::configure(Vector<String> &conf, ErrorHandler *errh) {

        return 0;
}


void PacketAnalyzer::push(int port, Packet *p) {

        click_chatter("Got a packet of size %d",p->length());

        custom_packet_format *cHdr = (custom_packet_format *)(p->data());

	click_chatter("Packet Type = %d, Payload Length = %d", cHdr->type, cHdr->payload_length);

	if (cHdr->type == 0){
                click_chatter("Packet with type 0. Pushing out of port 0");
		output(0).push(p);
                click_chatter ("---------------------------------");
        }

	else if(cHdr->type == 1){
                 click_chatter("Packet with field 1. Pushing out of port 1. Packet will be dropped.");
                 output(1).push(p);
                 click_chatter ("---------------------------------");
 	}

}

CLICK_ENDDECLS
EXPORT_ELEMENT(PacketAnalyzer)
