
/* Topology element sends hello to neighbouring routers. Maintains a ports table.
   Forwards data,ack,update to corresponding destinations */

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <click/packet.hh>
#include <click/glue.hh>
#include <click/packet_anno.hh>
#include "topology.hh"
#include "custom_packets.hh"

#define HELLO_TYPE 1
#define ROUTING_UPDATE_TYPE 2
#define ACKNOWLEDGMENT_TYPE 3
#define MULTICAST_DATA_TYPE 4


CLICK_DECLS


/* Constructor */
Topology::Topology():hello_timer(this) {
   
    	current_seqnum = 0;
    	hello_period = 5;
    	_anno = 8;
}


/* destructor */
Topology::~Topology(){
	
}


/* initializer */
int Topology::initialize(ErrorHandler *errh){
    
    	hello_timer.initialize(this);
    	hello_timer.schedule_after_sec(1);
    	return 0;
}


/* returns latest sequence number in use */
int Topology::get_latest_seq_num(){

	return current_seqnum++;

}


/* Function to send HELLO messages */
void Topology::send_hello() {

     	// Broadcast hello to all ports.
     	int headroom = sizeof(click_ether);
     	WritablePacket *packet = Packet::make(headroom,0,sizeof(struct hello_packet), 0);
     	memset(packet->data(),0,packet->length());

     	struct hello_packet *format = (struct hello_packet*) packet->data();

            format->header.type = HELLO_TYPE;
            format->header.src_address = uint16_t(myaddress);
            format->header.seq_num = current_seqnum++;

	if (Packet *clone = packet->clone()){

                clone->set_anno_u8(_anno, 255);
                output(0).push(clone);
        }

            
        typedef std::map<int,int>::const_iterator MapIterator;
        for (MapIterator iter = ports_table.begin(); iter != ports_table.end(); iter++)
        {
              if (Packet *clone = packet->clone()){

                	click_chatter("sending routers periodic hello from to device %d at port %d", iter->first, iter->second); 
                	clone->set_anno_u8(8, iter->second);
                        output(0).push(clone); 
                        // instead put in to that ports queue

                }
         }

         packet->kill();
}


/* Timer configurations */
void Topology::run_timer(Timer* timer){

         send_hello();
         hello_timer.schedule_after_sec(hello_period);

}


/* Returns a copy of the current port map */
std::map<int,int>& Topology::get_port(){

  return ports_table;

}


/* parser */
int Topology::configure(Vector<String> &conf, ErrorHandler *errh) {

        if (cp_va_kparse(conf, this, errh,
            "MY_ADDRESS", cpkM, cpInteger, &myaddress,
            cpEnd) < 0) {

                  return -1;
        }
        return 0;
}



/* Print ports table */
void Topology::print_ports_table(){

	typedef std::map<int,int>::const_iterator MapIterator;
        click_chatter("-----Ports Table of router %d-------", myaddress);

	for (MapIterator iter = ports_table.begin(); iter != ports_table.end(); iter++)
	{
    		click_chatter("DESTINATION: %d | PORT: %d", iter->first, iter->second);
	}

        click_chatter("-----------------------");
 
}


/* Returns output port for a given destination */
int Topology::get_output_port(int destination){

               ports_table_iterator key = ports_table.find(destination);

               if(key != ports_table.end()) {

                      click_chatter("Next port of the pending data is %d", key->second);
                      return key->second;
               }
               else
                      return -1;

}


/* Push configuration for incoming packets */
void Topology::push(int port, Packet *packet) {

	assert(packet);
	struct common_header *header = (struct common_header *)packet->data();

         // if ack, just cheque queue only. and then pop, else ignore.

         if (header->type == HELLO_TYPE) {  // incoming hello will be used to update the ports table

                // adding entry to the ports table
		click_chatter("Received Hello from %u on port %d", header->src_address, port);
                ports_table[header->src_address] = port;
                this->print_ports_table();

                // send ACK back to the received port with same sequence.
                int headroom = sizeof(click_ether);
                WritablePacket *ack = Packet::make(headroom,0,sizeof(struct ack_packet), 0);
                memset(ack->data(),0,ack->length());

                struct ack_packet *format = (struct ack_packet*) ack->data();

                	format->header.type = ACKNOWLEDGMENT_TYPE;
                	format->header.seq_num = header->seq_num; //use same sequence number from the received packet
                	format->header.src_address = uint16_t(myaddress);
                	format->dst_address = header->src_address;

                packet->kill(); // kill received data packet as it is no longer needed

		int destination_port = get_output_port(int(header->src_address));

                if (destination_port == -1){

                    click_chatter( "Key not present in the ports table. killing the packet.\n");
                    packet->kill();
               
               } else {

                    ack->set_anno_u8(_anno, destination_port);
                    output(0). push(ack);
               }

      	}

        else {

		click_chatter("Wrong packet type");
		packet->kill();
	}
}


CLICK_ENDDECLS
EXPORT_ELEMENT(Topology)


