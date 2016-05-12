#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include "packetprocessor.hh"
#include "custom_packet.hh"

CLICK_DECLS


/* Constructor */
PacketProcessor::PacketProcessor():timer(this),ack_timeout_timer(this){

	this->packetType = 0;
	this->seqnum = 0;
        this->ack_received = false;
        this->timeout_timer_status = false;
}


/* Destructor */
PacketProcessor::~PacketProcessor(){}



/* PacketProcessor: parsing */
int PacketProcessor::configure(Vector<String> &conf, ErrorHandler *errh) {

    	timer.initialize(this); // initializing timer (mandatory)
	timer.schedule_after_sec(1); // call run_timer after 3 seconds 
    	return 0;
}



/* runs when timer is called */
void PacketProcessor::run_timer(Timer *_timer){
       
        // packet generation timer 
        if (_timer == &timer) {

		Timestamp now = Timestamp::now();
    		click_chatter ("--------------------------------");
    		click_chatter ("%s : %{timestamp}: timer fired!\n",
		declaration().c_str(), &now);
    		this->sendMessage();
    		click_chatter ("--------------------------------");
        }

        // packet acknowledgement timer
        else if (_timer == &ack_timeout_timer){

                if (this->ack_received) {
                	_waiting_packet -> kill(); // kills the packet (clear the buffer) if ack received.
                	return;
                }
 
                click_chatter("******** ACK TIMEOUT *********");
        	click_chatter("Timeout : Ack not received. Resending the packet..");
        	this -> retryMessage();
                click_chatter("******************************");

                // if packet is not received till timeout 
                ack_timeout_timer.reschedule_after_sec(5);
        }

}

/* runs when acknowledgment is not received before the timeout */
void PacketProcessor::retryMessage(){

     Packet *p = _waiting_packet->clone();
     output(0).push(p);
     
}



void PacketProcessor::sendMessage() {

        // reset both flags
        this -> ack_received = false;
        this -> timeout_timer_status = false;

        content = "COMMUNICATION NETWORKS";
	click_chatter("Creating packet with payload: %s, packet type: %d, sequence number: %d", content.c_str(), packetType, seqnum);

	int tailroom = 0;
	int packetsize = sizeof(custom_packet_format)+content.length();
	int headroom = sizeof(click_ether);

	WritablePacket *packet = Packet::make(headroom,0,packetsize,tailroom);
	if (packet == 0 )return click_chatter( "cannot make packet!");
	
    	custom_packet_format *custom_header = (custom_packet_format *)packet->data();
    	custom_header->type = packetType;
	custom_header->payload_length = content.length();
    	custom_header->sequence_number = seqnum;

    	memcpy((char *)(custom_header+1), content.data(), content.length());

        _waiting_packet = packet->clone();
	output(0).push(packet);

        click_chatter("Packet sent.. Waiting for acknowledgment");

        // start acknowledgement timer for this packet
        ack_timeout_timer.initialize(this);
       	ack_timeout_timer.schedule_after_sec(5);      

}



void PacketProcessor::push(int port, Packet *p) {

        assert(port == 0);

        custom_packet_format *cHdr = (custom_packet_format *)(p->data());

        if (cHdr->type == 1  && cHdr->sequence_number == seqnum){

                // if the correct acknowledgement is received
                click_chatter("Received acknowledgment for packet with sequence Number = %d", cHdr->sequence_number);
                seqnum++; // increase sequence number for the next packet
                this->ack_received = true; // set flag
                ack_timeout_timer.clear(); // clear timeout timer
                timer.reschedule_after_sec(1); // reschedule packet generation timer
        }

       else {
             
             p -> kill(); // if wrong acknowledgement  
             click_chatter ("wrong ack. resending packet in 5 secs ...");
             //ack_timeout_timer.reschedule_after_sec(5); 
       }

       return;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(PacketProcessor)



