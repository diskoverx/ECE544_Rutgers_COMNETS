
/* Switch element - This element makes the final switch to router ports */

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <click/packet.hh>
#include "pswitch.hh"
#include "custom_packets.hh"
#include <click/packet_anno.hh>

#define HELLO_TYPE 1
#define ROUTING_UPDATE_TYPE 2
#define ACKNOWLEDGMENT_TYPE 3
#define MULTICAST_DATA_TYPE 4

CLICK_DECLS

Pswitch::Pswitch() {
}

Pswitch::~Pswitch(){
	
}

int Pswitch::initialize(ErrorHandler *errh){
    return 0;
}


int Pswitch::configure(Vector<String> &conf, ErrorHandler *errh) {

        return 0;
}


void Pswitch::push(int port, Packet *p) {

	assert(p);

        int output_port = static_cast<int>(p->anno_u8(8));

        if (output_port != 0xFF) { 
       	
                 struct common_header *header = (struct common_header *)p->data();

                 if (header->type == MULTICAST_DATA_TYPE){

                         click_chatter("Data packet with seqnum %d queued at router output port %d",header->seq_num,output_port);
                         router_queue[output_port].push(p);
                     
                         Packet *_waiting_packet = router_queue[output_port].front()->clone();
                         checked_output_push(output_port, _waiting_packet);
                 }
                 else 

   		 	checked_output_push(output_port, p);

        }

        else { // duplicate to all output ports
    		int n = noutputs();
    		for (int i = 0; i < n - 1; i++) 
      			if (Packet *q = p->clone())
				output(i).push(q);
    	
		output(n - 1).push(p);
  	}
        

}

CLICK_ENDDECLS
EXPORT_ELEMENT(Pswitch)

