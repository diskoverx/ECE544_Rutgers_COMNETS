
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

        custom_packet_format *cHdr = (custom_packet_format *)(p->data());
	click_chatter("Packet Received: Type = %d, Seq. Number = %d ", cHdr->type, cHdr->sequence_number);
  
	if (cHdr->type == 0){

                // forwarding received packet through port 0 
		output(0).push(p);

                int tailroom = 0;
                int packetsize = sizeof(custom_packet_format);
                int headroom = sizeof(click_ether);

                // creating acknowledgement packet
                click_chatter("Sending ack for packet...");
                WritablePacket *ack_packet = Packet::make(headroom,0,packetsize,tailroom);
                if (ack_packet == 0 )return click_chatter( "cannot make packet!");

                custom_packet_format *ack_header = (custom_packet_format *)ack_packet->data();
                ack_header->type = 1;
                ack_header->payload_length = 0;
                ack_header->sequence_number = cHdr->sequence_number;

                // sending ack packet through port 1
                output(1).push(ack_packet);
        }

	else {

                // unwanted packet. no need of acknowledgements
                p->kill();
 	}

        click_chatter ("---------------------------------");
}

CLICK_ENDDECLS
EXPORT_ELEMENT(PacketAnalyzer)
