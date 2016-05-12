
#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include "packetclassifier.hh"
#include "custom_packets.hh"

CLICK_DECLS

PacketClassifier::PacketClassifier(){}
PacketClassifier::~PacketClassifier(){}

int PacketClassifier::configure(Vector<String> &conf, ErrorHandler *errh) {

        return 0;
}


void PacketClassifier::push(int port, Packet *p) {

        custom_packet_format *cHdr = (custom_packet_format *)(p->data());

        if (cHdr->type == 0 || cHdr-> type == 1){

                if (port == 0) {
               		click_chatter("PACKET CLASSIFIER:: DATA/ACK FROM PORT 0 (RP1). Pushing to PC o/p 0");
             	   	output(0).push(p);
                }

                else if (port == 1) {
                        click_chatter("PACKET CLASSIFIER:: DATA/ACK FROM PORT 1 (RP2). Pushing to PC o/p 2");
                        output(2).push(p);
                }

                else if (port == 2) {
                        click_chatter("PACKET CLASSIFIER:: DATA/ACK FROM PORT 2 (RP3). Pushing to PC o/p 4");
                        output(4).push(p);
                }

        }
           
        else if (cHdr->type == 2){

                if (port == 0) {
                        click_chatter("PACKET CLASSIFIER:: HELLO FROM PORT 0 (RP1). Pushing to PC o/p 1");
                        output(1).push(p);
                }
                else if (port == 1) {
                        click_chatter("PACKET CLASSIFIER:: HELLO FROM PORT 1 (RP2). Pushing to PC o/p 3");
                        output(3).push(p);
                }

                else if (port == 2) {
                        click_chatter("PACKET CLASSIFIER:: HELLO FROM PORT 2. (RP3). Pushing to PC o/p 5");
                        output(5).push(p);
                }

        }

	else {
                click_chatter("PACKET CLASSIFIER:: unknown packet.. packet killed");
                p->kill();
 	}

}

CLICK_ENDDECLS
EXPORT_ELEMENT(PacketClassifier)
