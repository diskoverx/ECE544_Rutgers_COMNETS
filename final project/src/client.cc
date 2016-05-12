
/* Client.cc - Client implementation. ARQ Strategy - Stop and wait, hop by hop */
/* Prepared by Nithin Raju Chandy - nithin.chandy@rutgers.edu  */

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <inttypes.h>
#include "client.hh"
#include <click/packet_anno.hh>
#include "custom_packets.hh"

#define HELLO_TYPE 1
#define ROUTING_UPDATE_TYPE 2
#define ACKNOWLEDGMENT_TYPE 3
#define MULTICAST_DATA_TYPE 4


CLICK_DECLS

Client::Client():data_packet_timer(this), hello_timer(this), RTT_timer(this){

        current_seqnum = 0;
        data_period = 10;
        hello_period = 5;
        delay = 0;
        retransmissions = 0;
        time_out = 1;
        kvalue = 0;
        dst_01_address = 0;
        dst_02_address = 0;
        dst_03_address = 0;
        RTT_timer_status = false;
        myaddress = 0;
        data_send_enable = 1;
}


Client::~Client(){}


int Client::initialize(ErrorHandler *errh) {

        data_packet_timer.initialize(this);
	hello_timer.initialize(this);
        RTT_timer.initialize (this);

	if(delay>0) {
           data_packet_timer.schedule_after_sec(delay);
	}

	hello_timer.schedule_after_sec(1);
        return 0;
}


int Client::configure(Vector<String> &conf, ErrorHandler *errh) {
    
   	if (cp_va_kparse(conf, this, errh, "PAYLOAD", cpkP+cpkM, cpString, &multicast_payload,
            "K_VALUE", cpkM, cpInteger, &kvalue, "MY_ADDRESS", cpkM, cpInteger, &myaddress,
	    "DST1", cpkM, cpInteger, &dst_01_address,
	    "DST2", cpkM, cpInteger, &dst_02_address,
            "DST3", cpkM, cpInteger, &dst_03_address,
            "DELAY", cpkM, cpInteger, &delay, 
	    "DATA_SEND_ENABLE", cpkM, cpInteger, &data_send_enable,
             cpEnd) < 0) {

           	  return -1;
        }

        /* if this is the sending node, following conditions must be satisfied */

        if (data_send_enable == 1) {

       		 if (kvalue == 0 || dst_01_address == 0 || dst_02_address == 0 || dst_03_address == 0){
            		click_chatter("K must be greater than zero (can be 1,2,3) & DST1, DST2, DST3 addresses cannot be zero");
            		return -1;
                 }
        }

        return 0;
}


/* Timer configurations */
void Client::run_timer(Timer* timer){
       
       if(timer == &data_packet_timer){

              if (data_send_enable == 1) {
		 	send_data();
              		data_packet_timer.schedule_after_sec(data_period);
              }

	} else if (timer == &hello_timer) {
 
              send_hello(); 
              hello_timer.schedule_after_sec(hello_period);

        } else if (timer == &RTT_timer) {

              click_chatter("Current Queue Size: %d ", packet_queue.size());
             
              Packet *_waiting_packet = packet_queue.front()->clone();
              struct common_header *qfront_header = (struct common_header*) _waiting_packet->data();
              click_chatter("Resending packet with seqnum = %d, retry number = %d", qfront_header->seq_num, retransmissions++);

              output(0).push(_waiting_packet);
              RTT_timer.reschedule_after_sec(time_out);   
        } 
}


/* Function to send DATA messages */
void Client::send_data(){

            int headroom = sizeof(click_ether);
            WritablePacket *packet = Packet::make(headroom,0,sizeof(struct data_packet)+multicast_payload.length(), 0);
            memset(packet->data(),0,packet->length());

            struct data_packet *format = (struct data_packet*) packet->data();

                format->header.type = MULTICAST_DATA_TYPE;
                format->header.seq_num = current_seqnum;
                format->header.src_address = uint16_t(myaddress);
                format->k_value = kvalue;
                format->dst_01_address = uint16_t(dst_01_address);
                format->dst_02_address = uint16_t(dst_02_address);
                format->dst_03_address = uint16_t(dst_03_address);
                format->payload_length = multicast_payload.length();

            char *data = (char*)(packet->data()+sizeof(struct data_packet));
            memcpy(data, multicast_payload.c_str(), multicast_payload.length());

            packet_queue.push(packet);
            click_chatter("Queue Update : DATA (sequence no = %u) queued.", current_seqnum);
            current_seqnum++;
            check_queue_status();
}


/* Function to send HELLO messages */
void Client::send_hello() {

            int headroom = sizeof(click_ether);
            WritablePacket *packet = Packet::make(headroom,0,sizeof(struct hello_packet), 0);
            memset(packet->data(),0,packet->length());
            struct hello_packet *format = (struct hello_packet*) packet->data();

                format->header.type = HELLO_TYPE;
                format->header.src_address = uint16_t(myaddress);
                format->header.seq_num = current_seqnum;

            packet_queue.push(packet);
            click_chatter("Queue Update : HELLO (sequence no = %u) queued.", current_seqnum);
            current_seqnum++;
            check_queue_status();
}


/* Function to check current status of Queue */
void Client::check_queue_status() 
{
	if (packet_queue.size()>0 && RTT_timer_status == false) {

             Packet *_waiting_packet = packet_queue.front()->clone();
             output(0).push(_waiting_packet);
             RTT_timer.schedule_after_sec(time_out);
             RTT_timer_status = true;
        }

}



/* PUSH implementation */
void Client::push(int port, Packet *packet) {

	assert(packet);

        // classifier sends DATA to port 0
	if(port == 0) { 

	    struct data_packet *header = (struct data_packet *)packet->data();

            click_chatter("\n\noooooooooooooooooooooooooooooooooooooooooooooooo");
            click_chatter("Received MULTICAST DATA packet with seqnum = %u from %u", header->header.seq_num, header->header.src_address);
            click_chatter("oooooooooooooooooooooooooooooooooooooooooooooooo\n\n");
        
	    // now we need to send an ack for this data with same seqnum            
            int headroom = sizeof(click_ether);
	    WritablePacket *ack = Packet::make(headroom,0,sizeof(struct ack_packet), 0);
	    memset(ack->data(),0,ack->length());

            click_chatter("Sending ACK for the received data packet with seqnum = %u", header->header.seq_num);
            struct ack_packet *format = (struct ack_packet*) ack->data();

                format->header.type = ACKNOWLEDGMENT_TYPE;
		format->header.seq_num = header->header.seq_num; // use same sequence number from the received packet
		format->header.src_address = uint16_t(myaddress);
		format->dst_address = header->header.src_address;
	
            packet->kill(); // kill received data packet as it is no longer needed
            output(0).push(ack);

	} else if (port == 1) { // classifier sends ACKs (hello/data) to port 1 

		struct ack_packet *header = (struct ack_packet *)packet->data();
		click_chatter("Received some ACK (HELLO or DATA) with seqnum =  %d from %d", header->header.seq_num, header->header.src_address);
              
                // make sure the ack seqnum is same as the outstanding seqnum in the queue
                WritablePacket *qfront = packet_queue.front();
                struct common_header *qfront_header = (struct common_header*) qfront->data();

		if(header->header.seq_num == qfront_header->seq_num) {

	                RTT_timer.unschedule(); 
                        packet_queue.pop();
                        RTT_timer_status = false;
			retransmissions=0;
                        packet->kill(); // extra addition

		} else {

                        // received wrong sequence number
			packet->kill();
		}

	} else {

                click_chatter("client received unknown packet");
		packet->kill(); // unknown packet
	}

        // Need to consider the case of hello and update packets. instead of discarding, need to send acks back
        // once the queue is implemented. Remember to edit the click config file as well
}


CLICK_ENDDECLS
EXPORT_ELEMENT(Client)



