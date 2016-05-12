
/* Forwarder Element - This element basically kills HELLO and ACK updates meant for this router.
   It forwards DATA acks and DATA to the next element */

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <click/packet.hh>
#include "forwarder.hh"
#include "custom_packets.hh"

#define HELLO_TYPE 1
#define ROUTING_UPDATE_TYPE 2
#define ACKNOWLEDGMENT_TYPE 3
#define MULTICAST_DATA_TYPE 4
#define _anno 8

CLICK_DECLS

Forwarder::Forwarder() {
}

Forwarder::~Forwarder(){
	
}

int Forwarder::initialize(ErrorHandler *errh){
    return 0;
}


int Forwarder::configure(Vector<String> &conf, ErrorHandler *errh) {

        if (cp_va_kparse(conf, this, errh,
            "TP_ELEMENT", cpkP+cpkM, cpElement, &topologyElement,
            "ROUTER_ELEMENT", cpkP+cpkM, cpElement, &routerElement,
            "SWITCH_ELEMENT", cpkP+cpkM, cpElement, &switchElement,
            "MY_ADDRESS", cpkM, cpInteger, &myaddress,
            cpEnd) < 0) {

                  return -1;
        }

        return 0;
}


void Forwarder::push(int port, Packet *packet) {

	assert(packet);
	struct common_header *header = (struct common_header *)packet->data();

        // Router can receive acks from it's neighbours, just ignore those. 
        // But need to forward data acks to routing element 

        if(header->type == ACKNOWLEDGMENT_TYPE){
            

                         for (int i=0; i<10; i++) {

                             if(switchElement -> router_queue[i].size()>0) {
               				click_chatter("Queue port %d size is %d", i, switchElement -> router_queue[i].size());
               				struct ack_packet *ack_header = (struct ack_packet *)packet->data();
               				Packet* qfront = switchElement -> router_queue[i].front(); 
               				struct common_header *qfront_header = (struct common_header*) qfront->data();
               				if (ack_header->header.seq_num == qfront_header->seq_num) {

                    				click_chatter("Data popped from queue port %d",i);
                    				switchElement -> router_queue[i].pop();
               			}
                         }
                    }

		struct ack_packet *ack_header = (struct ack_packet *)packet->data();
               click_chatter("~~~~~ Forwarder: Received an ack at forwarder port %d~~~~ ", port);
               if (ack_header->dst_address == uint16_t(myaddress)) {
                   
                   click_chatter ("Forwarder: This ack if for me (from a nearby router). ignoring... ");
                   packet -> kill();


                   // implement logic to push from router queue based on sequence number and port number 
               }

        }

        else if (header->type == MULTICAST_DATA_TYPE){

                click_chatter(" ~~~~ Forwarder : Received a DATA at forwarder ~~~~ "); 

                // First thing to do is to send an acknowledgement

                int headroom = sizeof(click_ether);
                WritablePacket *ack = Packet::make(headroom,0,sizeof(struct ack_packet), 0);
                memset(ack->data(),0,ack->length());

                struct ack_packet *format = (struct ack_packet*) ack->data();

                        format->header.type = ACKNOWLEDGMENT_TYPE;
                        format->header.seq_num = header->seq_num; //use same sequence number from the received packet
                        format->header.src_address = uint16_t(myaddress);
                        format->dst_address = header->src_address;

                click_chatter("DATA SOURCE %d", header->src_address);
                int destination_port = topologyElement->get_output_port(int(header->src_address));

                if (destination_port == -1){

                    click_chatter( "FORWARDER : Not sure where to send acknowledgement\n");
                    ack->kill();

               } else {
                    click_chatter("sending ack with seqnum %d to address %d", header->seq_num, header->src_address);
                    ack->set_anno_u8(_anno, destination_port);
                    output(0). push(ack);
               }


               // process the packet

                struct data_packet *mpacket = (struct data_packet *)packet->data();

                

                click_chatter("---------MULTICAST PACKET DETAILS-------");
		click_chatter("Source address is %u", mpacket -> header.src_address);
     		click_chatter("Seq number is %u",  mpacket -> header.seq_num);
                click_chatter("K value is %u", mpacket ->k_value);
                click_chatter("Destination 1 is %u", mpacket -> dst_01_address);
                click_chatter("Destination 2 is %u", mpacket -> dst_02_address);
                click_chatter("Destination 3 is %u", mpacket -> dst_03_address);
                click_chatter("Payload length is %u", mpacket -> payload_length);
                click_chatter("-----------------------------------------");

                // change packet source address to this device's address
                mpacket -> header.src_address = uint16_t(myaddress);
               
                int k = (int) mpacket ->k_value;
                int dst1 = (int) mpacket -> dst_01_address;
                int dst2 = (int) mpacket -> dst_02_address;
                int dst3 = (int) mpacket -> dst_03_address;

                if (k == 1) {

                    if (dst2 == 0 && dst3 == 0){

                     	click_chatter("K = 1 already updated. just forward");

                        int nexthop = routerElement -> get_next_hop(dst1);
                        int next_hop_port = topologyElement -> get_output_port(nexthop);

                        click_chatter("forwarding to dst1 %d through nexthop %d at port %d", dst1, nexthop, next_hop_port);

                        if (nexthop == -1 || next_hop_port == -1) {

                            click_chatter ("dst1 nexthop or port missing. kill packet.");
                            packet -> kill();
 
                        } else {

                        	packet->set_anno_u8(_anno, next_hop_port);
                    		output(0). push(packet);
                        }

                    } else {

                        click_chatter("K = 1 not updated. going to update");
                      
                        int best_one = routerElement -> find_best(dst1, dst2, dst3);
                        int nexthop = routerElement -> get_next_hop(best_one);
                        int next_hop_port = topologyElement -> get_output_port(nexthop);
                        
                        mpacket -> dst_01_address = uint16_t(best_one);
                        mpacket -> dst_02_address = 0;
                        mpacket -> dst_03_address = 0;
                       
                        click_chatter("Unicast to %d through nexthop %d ", best_one, nexthop);     

                        if (nexthop == -1 || next_hop_port == -1) {

                           	click_chatter ("dst1 nexthop or port missing. kill packet.");
                            	packet -> kill();

                        } else {

                                packet->set_anno_u8(_anno, next_hop_port);
                                output(0). push(packet);
                        } 

                    }  


                } else if (k ==2) {

                    if (dst2 == 0 || dst1 ==0){
                       
                        click_chatter("Something wrong.. k=2 but dst1 or dst2 is zero");

                    }

                    if (dst3 == 0){
          		
 			click_chatter("K = 2 but already updated");
                        
                        int common_hop = routerElement -> get_common_hop(dst1,dst2); // yet to implement
                        click_chatter(" K = 2, common hop = %d", common_hop);
       
                        if (common_hop == -1) // means there's no common path, we have to split packet.
                        {

                                int p1_nexthop = routerElement -> get_next_hop(dst1);
                                int p2_nexthop = routerElement -> get_next_hop(dst2);

                                // find nex hops separately
                                int p1_next_hop_port = topologyElement -> get_output_port(p1_nexthop);
                                int p2_next_hop_port = topologyElement -> get_output_port(p2_nexthop);
                               
                                
                                Packet *q = packet -> clone();
				WritablePacket *clone = packet->uniqueify();
                                WritablePacket *clone2 = q->uniqueify();
 
                 		        click_chatter("packet cloned (1). so total 2 copies will leave this router");

                                	// create 2 separate copies
                                	struct data_packet *mpacket1 = (struct data_packet *)clone->data();
                                        mpacket1 -> k_value = 1;
                                	mpacket1 -> dst_01_address = uint16_t(dst1);
                        		mpacket1 -> dst_02_address = 0;
                        		mpacket1 -> dst_03_address = 0;
                                	clone->set_anno_u8(_anno, p1_next_hop_port);
                                        click_chatter("sending first copy to %d through next hop %d", dst1, p1_next_hop_port);
                                	output(0).push(clone);   
                               
                                	struct data_packet *mpacket2 = (struct data_packet *)clone2->data();
                                        mpacket2 -> k_value = 1;
                                	mpacket2 -> dst_01_address = uint16_t(dst2);
                                	mpacket2 -> dst_02_address = 0;
                                	mpacket2 -> dst_03_address = 0;
                                	clone2->set_anno_u8(_anno, p2_next_hop_port);
                                        click_chatter("sending second copy to %d through next hop %d", dst2, p2_next_hop_port);
                                	output(0).push(clone2);

                        } else {
                         
                        	int next_hop_port = topologyElement -> get_output_port(common_hop);
                                packet->set_anno_u8(_anno, next_hop_port);
                                click_chatter("olaaaaa");
                                output(0).push(packet);
                                //packet -> kill();
                        }

                  } else {

                             click_chatter("K = 2 but value found in dst3. Going to select best 2 out of 3");
                  
                             // find best 2 out of 3 and make dst3 = 0.
                             
                             std::pair<int,int> best2 = routerElement -> get_best_two (dst1, dst2, dst3);

                                      mpacket -> dst_01_address = best2.first;
                                      mpacket -> dst_02_address = best2.second;
                                      mpacket -> dst_03_address = 0;

                             click_chatter("K = 2 update completed. selected best %d and %d out of 3", best2.first, best2.second);
                                      
                             int common_hop = routerElement -> get_common_hop(best2.first,best2.second);

                             click_chatter("Common hop for %d and %d is %d", best2.first, best2.second, common_hop); // so far so good

	                     if (common_hop == -1) // means there's no common path, we have to split packet.
                             {   
                             	   	// find nex hops separately
                                	int p1_nexthop = routerElement -> get_next_hop(dst1);
                                	int p2_nexthop = routerElement -> get_next_hop(dst2);

                                	int p1_next_hop_port = topologyElement -> get_output_port(p1_nexthop);
                                	int p2_next_hop_port = topologyElement -> get_output_port(p2_nexthop);
    
                                                Packet *q = packet -> clone();
                                                WritablePacket *clone = packet->uniqueify();
                                                WritablePacket *clone2 = q->uniqueify();
                                        	click_chatter("packet cloned. so total 2 copies will leave this router");

                                        	// create 2 separate copies
                                               
                                        	struct data_packet *mpacket1 = (struct data_packet *)clone->data();
                                                mpacket1 -> k_value = 1;
                                        	mpacket1 -> dst_01_address = uint16_t(dst1);
                                        	mpacket1 -> dst_02_address = 0;
                                        	mpacket1 -> dst_03_address = 0;
                                        	clone->set_anno_u8(_anno, p1_next_hop_port);
                                                click_chatter("sending first copy to %d through next hop %d", dst1, p1_next_hop_port);
                                        	output(0). push(clone);   

                                        	struct data_packet *mpacket2 = (struct data_packet *)clone2->data();
                                                mpacket2 -> k_value = 1;
                                        	mpacket2 -> dst_01_address = uint16_t(dst2);
                                        	mpacket2 -> dst_02_address = 0;
                                        	mpacket2 -> dst_03_address = 0;
                                        	clone2->set_anno_u8(_anno, p2_next_hop_port);
                                                click_chatter("sending second copy to %d through next hop %d", dst2, p2_next_hop_port);
                                        	output(0).push(clone2);

                       	     } else {
                                         click_chatter("no need to clone. common hop is there... ");
                               		 int next_hop_port = topologyElement -> get_output_port(common_hop);
                                	 packet->set_anno_u8(_anno, next_hop_port);
                                         click_chatter("sending to common hop %d through port %d", common_hop, next_hop_port);
                                	 output(0).push(packet);

                  	     }   


                  }


                } else if (k == 3) {

                        if(dst1==0 || dst2==0 || dst3==0) {

				click_chatter("Error: K=3 none of the destinations can be zero");
                        }

                        // check whether all three have a common hop, no cloning required
                        int common_hop_3 = routerElement -> get_three_common_hop(dst1,dst2,dst3); 
                        click_chatter(" K = 3, common hop = %d", common_hop_3);

                        if (common_hop_3 == -1) {

                                 // no common hop for all 3, check first 2
                                 int common_hop_first_2 = routerElement -> get_common_hop(dst1,dst2);

                                 if (common_hop_first_2 == -1) {
                                     
                                              // find common hop for last two
                                              int common_hop_last_2 = routerElement -> get_common_hop(dst2,dst3);                                          

                                              if (common_hop_last_2 == -1) {

                                                           // find common hop for first and last
                                                           int common_hop_first_last = routerElement -> get_common_hop(dst1,dst3);
                                                           
                                                           if (common_hop_first_last == -1) {

                                                                        // no common hop in any order, create 3 copies, unicast to 3 destinations
									click_chatter(" K=3, no common hop found in any order. creating 3 copies ");

                                                                        int p1_nexthop = routerElement -> get_next_hop(dst1);
                                                                        int p2_nexthop = routerElement -> get_next_hop(dst2);
                                                                        int p3_nexthop = routerElement -> get_next_hop(dst3);

                                                                        int p1_next_hop_port = topologyElement -> get_output_port(p1_nexthop);
                                                                        int p2_next_hop_port = topologyElement -> get_output_port(p2_nexthop);
                                                                        int p3_next_hop_port = topologyElement -> get_output_port(p3_nexthop);

                                                                        Packet *q = packet -> clone();
                                                                        Packet *w = packet -> clone();

                                                                        WritablePacket *clone = packet->uniqueify();
                                                                        WritablePacket *clone2 = q->uniqueify();
                                                                        WritablePacket *clone3 = w->uniqueify();
                                                                        click_chatter("packet cloned twice. so total 3 copies will leave this router");


                                                                        // create 3 separate copies

                                                                        struct data_packet *mpacket1 = (struct data_packet *)clone->data();
                                                                        mpacket1 -> k_value = 1;
                                                                        mpacket1 -> dst_01_address = uint16_t(dst1);
                                                                        mpacket1 -> dst_02_address = 0;
                                                                        mpacket1 -> dst_03_address = 0;
                                                                        clone->set_anno_u8(_anno, p1_next_hop_port);
                                                                        click_chatter("sending first copy to %d through common next hop %d", dst1, p1_next_hop_port);

                                                                        if (p1_next_hop_port == -1)
                                                                        	clone->kill();
                                                                        else
                                                                                output(0). push(clone);           

                                                                        struct data_packet *mpacket2 = (struct data_packet *)clone2->data();
                                                                        mpacket2 -> k_value = 1;
                                                                        mpacket2 -> dst_01_address = uint16_t(dst2);
                                                                        mpacket2 -> dst_02_address = 0;
                                                                        mpacket2 -> dst_03_address = 0;
                                                                        clone2->set_anno_u8(_anno, p2_next_hop_port);
                                                                        click_chatter("sending second copy to %d through next hop %d", dst2, p2_next_hop_port);

                                                                        if (p2_next_hop_port == -1) 
                                                                                clone2->kill();
                                                                        else
                                                                                output(0). push(clone2); 

                                                                        struct data_packet *mpacket3 = (struct data_packet *)clone3->data();
                                                                        mpacket3 -> k_value = 1;
                                                                        mpacket3 -> dst_01_address = uint16_t(dst3);
                                                                        mpacket3 -> dst_02_address = 0;
                                                                        mpacket3 -> dst_03_address = 0;
                                                                        clone3->set_anno_u8(_anno, p3_next_hop_port);
                                                                        click_chatter("sending second copy to %d through next hop %d", dst3, p3_next_hop_port);

                                                                        if (p3_next_hop_port == -1) 
                                                                                clone3->kill();
                                                                        else
                                                                                output(0). push(clone3); 

                                                                        packet->kill();
                                                                        q -> kill();
                                                                        w -> kill();

                                                           }

                                                           else {
		                                                      click_chatter(" K=3, common hop found for first and last.. creating 2 copies ");
                		                                      // create 2 copies, one with k=2 other one with k=1

                                		                        int p1_nexthop = routerElement -> get_common_hop(dst1,dst3);
                                                		        int p2_nexthop = routerElement -> get_next_hop(dst2);

                                                      			int p1_next_hop_port = topologyElement -> get_output_port(p1_nexthop);
                                                        		int p2_next_hop_port = topologyElement -> get_output_port(p2_nexthop);

                                                        		Packet *q = packet -> clone();
                                                        		WritablePacket *clone = packet->uniqueify();
                                                        		WritablePacket *clone2 = q->uniqueify();
                                                        		click_chatter("packet cloned. so total 2 copies will leave this router");

                                                        		// create 2 separate copies

                                                        		struct data_packet *mpacket1 = (struct data_packet *)clone->data();
                                                        		mpacket1 -> k_value = 2;
                                                        		mpacket1 -> dst_01_address = uint16_t(dst1);
                                                        		mpacket1 -> dst_02_address = uint16_t(dst3);
                                                        		mpacket1 -> dst_03_address = 0;
                                                        		clone->set_anno_u8(_anno, p1_next_hop_port);
                                                        		click_chatter("sending first copy to %d, %d through common next hop %d", dst1,dst3, p1_next_hop_port);
                                                        		output(0). push(clone);

                                                        		struct data_packet *mpacket2 = (struct data_packet *)clone2->data();
                                                        		mpacket2 -> k_value = 1;
                                                        		mpacket2 -> dst_01_address = uint16_t(dst2);
                                                      		 	mpacket2 -> dst_02_address = 0;
                                                        		mpacket2 -> dst_03_address = 0;
                                                        		clone2->set_anno_u8(_anno, p2_next_hop_port);
                                                        		click_chatter("sending second copy to %d through next hop %d", dst2, p2_next_hop_port);
                                                        		output(0).push(clone2);

                                                           }

                                              }

                                              else {

			                              click_chatter(" K=3, common hop found for last two.. creating 2 copies ");
                                   		      // create 2 copies, one with k=2 other one with k=1

		                                        int p1_nexthop = routerElement -> get_common_hop(dst2,dst3);
                		                        int p2_nexthop = routerElement -> get_next_hop(dst1);

                                		        int p1_next_hop_port = topologyElement -> get_output_port(p1_nexthop);
                                        		int p2_next_hop_port = topologyElement -> get_output_port(p2_nexthop);

                                                	Packet *q = packet -> clone();
                                                	WritablePacket *clone = packet->uniqueify();
                                                	WritablePacket *clone2 = q->uniqueify();
                                                	click_chatter("packet cloned. so total 2 copies will leave this router");

                                                	// create 2 separate copies

                                                	struct data_packet *mpacket1 = (struct data_packet *)clone->data();
                                                	mpacket1 -> k_value = 2;
                                                	mpacket1 -> dst_01_address = uint16_t(dst2);
                                                	mpacket1 -> dst_02_address = uint16_t(dst3);
                                                	mpacket1 -> dst_03_address = 0;
                                                	clone->set_anno_u8(_anno, p1_next_hop_port);
                                                	click_chatter("sending first copy to %d,%d through next hop %d", dst2, dst3, p1_next_hop_port);
                                                	output(0). push(clone);

                                                	struct data_packet *mpacket2 = (struct data_packet *)clone2->data();
                                                	mpacket2 -> k_value = 1;
                                                	mpacket2 -> dst_01_address = uint16_t(dst1);
                                                	mpacket2 -> dst_02_address = 0;
                                                	mpacket2 -> dst_03_address = 0;
                                                	clone2->set_anno_u8(_anno, p2_next_hop_port);
                                                	click_chatter("sending second copy to %d through next hop %d", dst1, p2_next_hop_port);
                                                	output(0).push(clone2);

                                              }


                                 } else {

                                        click_chatter(" K=3, common hop found for first two.. creating 2 copies ");

                                        // create 2 copies, one with k=2 other one with k=1
                                        int p1_nexthop = routerElement -> get_common_hop(dst1,dst2);
                                        int p2_nexthop = routerElement -> get_next_hop(dst3);

                                        int p1_next_hop_port = topologyElement -> get_output_port(p1_nexthop);
                                        int p2_next_hop_port = topologyElement -> get_output_port(p2_nexthop);

                                                Packet *q = packet -> clone();
                                                WritablePacket *clone = packet->uniqueify();
                                                WritablePacket *clone2 = q->uniqueify();
                                                click_chatter("packet cloned. so total 2 copies will leave this router");

                                                // create 2 separate copies

                                                struct data_packet *mpacket1 = (struct data_packet *)clone->data();
                                                mpacket1 -> k_value = 2;
                                                mpacket1 -> dst_01_address = uint16_t(dst1);
                                                mpacket1 -> dst_02_address = uint16_t(dst2);
                                                mpacket1 -> dst_03_address = 0;
                                                clone->set_anno_u8(_anno, p1_next_hop_port);
                                                click_chatter("sending first copy to %d, %d through next hop %d", dst1, dst2, p1_next_hop_port);
                                                output(0). push(clone);

                                                struct data_packet *mpacket2 = (struct data_packet *)clone2->data();
                                                mpacket2 -> k_value = 1;
                                                mpacket2 -> dst_01_address = uint16_t(dst3);
                                                mpacket2 -> dst_02_address = 0;
                                                mpacket2 -> dst_03_address = 0;
                                                clone2->set_anno_u8(_anno, p2_next_hop_port);
                                                click_chatter("sending second copy to %d through next hop %d", dst3, p2_next_hop_port);
                                                output(0).push(clone2);

                                 }
                               


                        } else {
                                  click_chatter(" K=3, no need to clone. common hop is there for all 3... ");
                                  int next_hop_port = topologyElement -> get_output_port(common_hop_3);
                                  packet->set_anno_u8(_anno, next_hop_port);
                                  click_chatter("sending to common hop %d through port %d", common_hop_3, next_hop_port);
                                  output(0).push(packet);           

                        } 


                } else {
                    
                     click_chatter("Error: Invalid k value received at forwarder");

                }
        }

        else {

            click_chatter("Forwarder : Wrong packet type received.");
            packet->kill();
        }

}

CLICK_ENDDECLS
EXPORT_ELEMENT(Forwarder)


