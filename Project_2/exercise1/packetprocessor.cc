#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include "packetprocessor.hh"
#include "custom_packet.hh"

CLICK_DECLS

PacketProcessor::PacketProcessor():timer(this){

	this->packetType = 0;
}

PacketProcessor::~PacketProcessor(){}

int PacketProcessor::configure(Vector<String> &conf, ErrorHandler *errh) {

       	if (cp_va_kparse(conf, this, errh, "PACKET_TYPE", cpkM, cpInteger, &packetType, "PAYLOAD", cpkM, cpString, &content, cpEnd) < 0) return -1;

	if (packetType < 0 || packetType >1 ) return errh->error("Error : Packet Type Should be either 1 or 0!");

        timer.initialize(this); // initializing timer (mandatory)
	timer.schedule_after_sec(3); // call run_timer after 3 seconds 
	
        return 0;
}


void PacketProcessor::run_timer(Timer*){
        
        // This function will be called everytime timer fires 

	Timestamp now = Timestamp::now();
        click_chatter ("--------------------------------");
        click_chatter ("%s : %{timestamp}: timer fired!\n",
		declaration().c_str(), &now);

        this->sendMessage(); // calling function to create packets
        click_chatter ("--------------------------------"); 

        timer.reschedule_after_sec(3); // reschedule to fire after after 3 minutes

}


void PacketProcessor::sendMessage() {

	click_chatter("Creating packet with payload: %s and packet type: %d", content.c_str(), packetType);

	int tailroom = 0;
	int packetsize = sizeof(custom_packet_format)+content.length();
	int headroom = sizeof(click_ether);

	WritablePacket *packet = Packet::make(headroom,0,packetsize,tailroom);
	if (packet == 0 )return click_chatter( "cannot make packet!");
	
        // casting packet to custom header

        custom_packet_format *custom_header = (custom_packet_format *)packet->data();
        custom_header->type = packetType;
	custom_header->payload_length = content.length();

        memcpy((char *)(custom_header+1), content.data(), content.length());
	output(0).push(packet);
}


void PacketProcessor::push(int port, Packet *p) {
	click_chatter("ERROR: this should not happen");
	return;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(PacketProcessor)



