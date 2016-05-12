#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include "packetprocessor.hh"
#include "custom_packets.hh"

CLICK_DECLS


/* Constructor */
PacketProcessor::PacketProcessor():timer(this), ack_timeout_timer(this), hello_packet_timer(this){

	this->packetType = 0;
	this->seqnum = 0;
        this->ack_received = false;
        this->timeout_timer_status = false;

        // default configuration. will send packet from C1(101) to C3(103).
        this->data_send_enable = 1;
        this->my_address = 101;
        this->dest_address = 103;

}


/* Destructor */
PacketProcessor::~PacketProcessor(){}



/* PacketProcessor: parsing */
int PacketProcessor::configure(Vector<String> &conf, ErrorHandler *errh) {

if (cp_va_kparse(conf, this, errh, "DATA_SEND_ENABLE", cpkM, cpInteger, &data_send_enable, "MY_ADDRESS", cpkM, cpInteger, &my_address, "DESTINATION_ADDRESS", cpkM, cpInteger, &dest_address, cpEnd) < 0) return -1;

        // enable or disable data sending capability of a node. If value is not 1, the node will still receive data packets and exchange hello packets.
        if (data_send_enable == 1) {
    		timer.initialize(this); // initializing timer (mandatory)
		timer.schedule_after_sec(3); // call run_timer after 3 seconds
        }

        hello_packet_timer.initialize(this);
        hello_packet_timer.schedule_after_sec(1);
 
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
 
        	click_chatter(" -- Timeout : Ack not received. Resending the packet with seq number = %d..",this->seqnum);
        	this -> retryMessage();

                // if packet is not received till timeout 
                ack_timeout_timer.reschedule_after_sec(5);
        }

        else if (_timer == &hello_packet_timer){
               this -> sendHello();
        }

}


/* function that sends hello messages */
void PacketProcessor::sendHello(){

        content = "HELLO";
        packetType = 2;

        int tailroom = 0;
        int packetsize = sizeof(hello_packet_format)+content.length();
        int headroom = sizeof(click_ether);

        WritablePacket *hello_packet = Packet::make(headroom,0,packetsize,tailroom);
        if (hello_packet == 0 )return click_chatter( "cannot make packet!");

        hello_packet_format  *hello_packet_header = (hello_packet_format *)hello_packet->data();
        hello_packet_header->type = packetType;
        hello_packet_header->payload_length = content.length();
        hello_packet_header->source= my_address;

        memcpy((char *)(hello_packet_header+1), content.data(), content.length());

        output(0).push(hello_packet);
	
              // click_chatter("sending hello packets");
               hello_packet_timer.reschedule_after_sec(2);
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
    	custom_header->type = 0;
	custom_header->payload_length = content.length();
    	custom_header->sequence_number = seqnum;
        custom_header->source = my_address;
        custom_header->destination = dest_address;

    	memcpy((char *)(custom_header+1), content.data(), content.length());

        _waiting_packet = packet->clone();
	output(0).push(packet);

        click_chatter("Packet sent.. Waiting for acknowledgment");

        // start acknowledgement timer for this packet
        ack_timeout_timer.initialize(this);
       	ack_timeout_timer.schedule_after_sec(5);      

}

void PacketProcessor::sendAckforMessage(int received_packet_sequence_number , int received_packet_source ) {

        // creating acknowledgement packet
        click_chatter("Sending ack for packet...");

        int tailroom = 0;
        int packetsize = sizeof(custom_packet_format)+content.length();
        int headroom = sizeof(click_ether);        

        WritablePacket *ack_packet = Packet::make(headroom,0,packetsize,tailroom);
        if (ack_packet == 0 )return click_chatter( "cannot make packet!");

        custom_packet_format *ack_header = (custom_packet_format *)ack_packet->data();
        ack_header->type = 1;
        ack_header->payload_length = 0;
        ack_header->sequence_number = received_packet_sequence_number;
        ack_header->destination = received_packet_source;

        // sending ack packet through port 1
        output(0).push(ack_packet);

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

       else if (cHdr->type == 2){

               click_chatter("Hello packet ack received"); 
       }

       else if (cHdr->type == 0){

               if (cHdr -> source == 101){

              		click_chatter("*** Received packet from 101 (C1) with sequence Number = %d ***", cHdr->sequence_number);
                        sendAckforMessage(cHdr->sequence_number, cHdr -> source);
      
               } else if (cHdr -> source == 102){
                 	click_chatter("*** Received packet from 102 (C2) with sequence Number = %d ***", cHdr->sequence_number);
                        sendAckforMessage(cHdr->sequence_number, cHdr -> source);

               } else if (cHdr -> source == 103) {
                 	click_chatter("*** Received packet from 103 (C3) with sequence Number = %d ***", cHdr->sequence_number);
                        sendAckforMessage(cHdr->sequence_number, cHdr -> source);
               }

               else 

                  // unknown sender. act as firewall
                  p -> kill();  

       }

       else {

             click_chatter ("wrong ack. resending packet in 5 secs ...");
             p -> kill(); // if wrong acknowledgement  
             //ack_timeout_timer.reschedule_after_sec(5); 
       }

       return;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(PacketProcessor)



