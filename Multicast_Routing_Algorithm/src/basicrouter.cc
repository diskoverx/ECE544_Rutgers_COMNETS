
/* BasicRouter element sends routing table updates to neighbouring routers. Maintains neighbour table. */
/* Updates its own neighbour table when it receives routing updates. */ 
/* Prepared by Nithin Raju Chandy - nithin.chandy@rutgers.edu  */   


#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <click/packet.hh>
#include "basicrouter.hh"
#include "custom_packets.hh"

#define HELLO_TYPE 1
#define ROUTING_UPDATE_TYPE 2
#define ACKNOWLEDGMENT_TYPE 3
#define MULTICAST_DATA_TYPE 4
#define _anno 8

CLICK_DECLS


/* constructor */
BasicRouter::BasicRouter():test_timer(this), update_timer(this){
   
}


/* destructor */
BasicRouter::~BasicRouter(){
	
}


/* Initializer */
int BasicRouter::initialize(ErrorHandler *errh){
    
    test_timer.initialize(this);
    update_timer.initialize(this);
    test_timer.schedule_after_sec(2);
    update_timer.schedule_after_sec(5);
    return 0;
}

/* parser */
int BasicRouter::configure(Vector<String> &conf, ErrorHandler *errh) {

        if (cp_va_kparse(conf, this, errh,"TP_ELEMENT", cpkP+cpkM, cpElement, &topologyElement,
            "MY_ADDRESS", cpkM, cpInteger, &myaddress,
            cpEnd) < 0) {

                  return -1;
        }
      
        return 0;
}


/* Timer configurations */
void BasicRouter::run_timer(Timer* timer){

        if (timer == &test_timer) {
         	initialize_neighbour_table();
         	test_timer.schedule_after_sec(3);
        }
        
        else if (timer == &update_timer) {
                send_updates();
                update_timer.schedule_after_sec(5);
        }
}


/* initialize routing table */
void BasicRouter::initialize_neighbour_table(){

        // add content from topology element's port table
        std::map<int,int>&  port_table = topologyElement->get_port();

        for (ptable_iterator iter = port_table.begin(); iter != port_table.end(); iter++)
        {
                click_chatter("DESTINATION: %d | PORT: %d", iter->first, iter->second);
                RoutingTable[iter->first][1].insert(iter->first);
                // close neighbours have a cost of 1
        }

        // add my own address as well with cost 0
        RoutingTable[myaddress][0].insert(0);
        print_entire_routing_table();
}



/* print routing table */
void BasicRouter::print_entire_routing_table(){
    
    
    /*           Code to print the entire routing table
     ----------------------------------------------------------------
     |  destination  |  cost |  next_hop 1, next_hop 2, next_hop 3  |
     ----------------------------------------------------------------  */
    

    click_chatter("\n\n ****** Routing Table begins here  ***** \n\n");
    for ( outermap_iterator ot = RoutingTable.begin() ; ot != RoutingTable.end(); ot++ ) {
        
        click_chatter( "Destination : %d", (*ot).first) ;
        
        for(innermap_iterator inner_it=(*ot).second.begin(); inner_it != (*ot).second.end(); inner_it++){
            
            click_chatter(" -- Total Hop cost : %d", (*inner_it).first);
            
            click_chatter( " -- Next hops : ");

            for(nexthop_iterator set_it=(*inner_it).second.begin(); set_it != (*inner_it).second.end(); set_it++){
                
                click_chatter("            %d", *set_it);
            }
            
        }  click_chatter("\n");
    }

    click_chatter("****** Routing Table ends here ****** \n\n");
}


/* find the best one out of 3 given destinations. Used when k=1 */
int BasicRouter::find_best(int dst1, int dst2, int dst3){

       int dst1cost, dst2cost, dst3cost, best;

       if ( RoutingTable.find(dst1) != RoutingTable.end() ) {

             dst1cost = RoutingTable[dst1].begin()->first;
             click_chatter("dst1 = %d dst1cost = %d", dst1, dst1cost);

       } else
             dst1cost = 999; // if not present means at infinite distance 

       if ( RoutingTable.find(dst2) != RoutingTable.end() ) {

             dst2cost = RoutingTable[dst2].begin()->first;
             click_chatter("dst2 = %d dst2cost = %d", dst2, dst2cost);

       } else 
            dst2cost = 999; // if not present means at infinite distance

       if ( RoutingTable.find(dst3) != RoutingTable.end() ) {

             dst3cost = RoutingTable[dst3].begin()->first;
             click_chatter("dst3 = %d dst3cost = %d", dst3, dst3cost);

       } else 

            dst3cost = 999; // if not present means at infinite distance 

       best = dst1;

       if (dst2cost < dst1cost && dst2cost < dst3cost)
                best = dst2;

       else if (dst3cost < dst1cost && dst3cost < dst2cost)
                best = dst3;

       else if (dst1cost < dst2cost && dst1cost < dst3cost)
                best = dst1;

       if (RoutingTable[best].begin() != RoutingTable[best].end()) {
       	
		click_chatter("Best out of all 3 is %d and cost is %d", best, RoutingTable[best].begin()->first);
       		return best;
       }

       else 
                return -1;
}




/* Get next hop given a destination */
int BasicRouter::get_next_hop(int best_one){

        if ( RoutingTable.find(best_one) != RoutingTable.end() ){

	        std::set<int> temp_set = RoutingTable[best_one].begin()->second;

                // Returns the first entry in the next hop set
                if (!temp_set.empty()) {

            click_chatter("First value in next hop set : %d", *temp_set.begin());
                	return *temp_set.begin();
                }

       }

       return -1;
}



/* compare function for sorting map entries by value */
template <typename T1, typename T2>
struct less_second {
    typedef std::pair<T1, T2> type;
    bool operator ()(type const& a, type const& b) const {
        return a.second < b.second;
    }
};



/* Returns best two destinations out of three */
std::pair<int,int> BasicRouter :: get_best_two (int dst1, int dst2, int dst3){

         std::vector<std::pair<int,int> > dst_cost_pair;


	 if ( RoutingTable.find(dst1) != RoutingTable.end() ) {
            
              dst_cost_pair.push_back(std::make_pair(dst1,RoutingTable[dst1].begin()->first));

         } else 
              click_chatter ("+++ oops..... destination %d missing in the routing table\n", dst1);

         if ( RoutingTable.find(dst2) != RoutingTable.end() ) {

              dst_cost_pair.push_back(std::make_pair(dst2,RoutingTable[dst2].begin()->first));

         } else 
              click_chatter ("+++ oops..... destination %d missing in the routing table\n", dst2);

         if ( RoutingTable.find(dst3) != RoutingTable.end() ) {

              dst_cost_pair.push_back(std::make_pair(dst3,RoutingTable[dst3].begin()->first));

         } else 
             click_chatter ("+++ oops..... destination %d missing in the routing table\n", dst3);
	
         sort(dst_cost_pair.begin(), dst_cost_pair.end(), less_second<int, int>());

         std::vector<std::pair<int,int> >::const_iterator pos = dst_cost_pair.begin();
     
         int required1, required2;

         if (pos != dst_cost_pair.end()) {
    		required1 = pos->first;
    		pos++;

                if (pos != dst_cost_pair.end()) {
	    		required2 = pos -> first;
                }

                return std::make_pair(required1, required2);
         } 
         else {
                click_chatter("cannot find best 2 out of 3. destinations missing!");
                return std::make_pair(0,0);
         }
}



/* 2 set intersect function */
int BasicRouter :: two_sets_intersect(std::set<int> s1, std::set<int> s2){

	std::set<int> two_intersect;
        set_intersection(s1.begin(),s1.end(),s2.begin(),s2.end(),std::inserter(two_intersect,two_intersect.begin()));
  
        std::set<int>::iterator it;
        it = two_intersect.begin();
    
    	if(two_intersect.begin()!=two_intersect.end()){
        	return *two_intersect.begin();
   	 }
    
    	else
        	return -1;
}


/* get common hop of any 2 destinations */
int BasicRouter::get_common_hop(int dst1, int dst2) {

     std::set<int> dst1_hops;
     std::set<int> dst2_hops;


    if (  (RoutingTable[dst1].begin() != RoutingTable[dst1].end()) ){
        if ( RoutingTable[dst1][RoutingTable[dst1].begin()->first].size() > 0 ){
            dst1_hops = RoutingTable[dst1].begin()->second;
        }
    
    }
    
    
    if (  (RoutingTable[dst2].begin() != RoutingTable[dst2].end()) ){
        if ( RoutingTable[dst2][RoutingTable[dst2].begin()->first].size() > 0 ){
            dst2_hops = RoutingTable[dst2].begin()->second;
        }
        
    }

     return two_sets_intersect(dst1_hops, dst2_hops);

}


/* 3-set intersect function */
int BasicRouter::three_sets_intersect(std::set<int> s1, std::set<int> s2, std::set<int> s3){
    
    	std::set<int> two_intersect;
    	std::set<int> three_intersect;
    
    	std::set_intersection(s1.begin(),s1.end(),s2.begin(),s2.end(),std::inserter(two_intersect,two_intersect.begin()));
    	std::set_intersection(s3.begin(),s3.end(),two_intersect.begin(),two_intersect.end(),std::inserter(three_intersect,three_intersect.begin()));
    
    	std::set<int>::iterator it;
    
    	it = three_intersect.begin();
    
    	if(three_intersect.begin()!=three_intersect.end()){
        	return *three_intersect.begin();
    	}
    
    	else
        
        return -1;
}



/* Get common hop of 3 destinations if any */

int BasicRouter::get_three_common_hop(int dst1, int dst2, int dst3) {

     std::set<int> dst1_hops;
     std::set<int> dst2_hops;
     std::set<int> dst3_hops;


    if (  (RoutingTable[dst1].begin() != RoutingTable[dst1].end()) ){
        if ( RoutingTable[dst1][RoutingTable[dst1].begin()->first].size() > 0 ){
            dst1_hops = RoutingTable[dst1].begin()->second;
        }
    }


    if (  (RoutingTable[dst2].begin() != RoutingTable[dst2].end()) ){
        if ( RoutingTable[dst2][RoutingTable[dst2].begin()->first].size() > 0 ){
            dst2_hops = RoutingTable[dst2].begin()->second;
        }
    }


    if (  (RoutingTable[dst3].begin() != RoutingTable[dst3].end()) ){
        if ( RoutingTable[dst3][RoutingTable[dst3].begin()->first].size() > 0 ){
            dst3_hops = RoutingTable[dst3].begin()->second;
        }
    }

     return three_sets_intersect(dst1_hops, dst2_hops, dst3_hops);

}



/* Function that sends router update messages */
void BasicRouter::send_updates(){

    	// prepares update packet
    	// topology is the central server that generates sequence number 

    	int packet_seq_num = topologyElement->get_latest_seq_num();

    	int tailroom = 0;
    	int packetsize = sizeof(struct routing_update_packet) ;
    	int headroom = sizeof(click_ether);

        WritablePacket *packet = Packet::make(headroom,0,packetsize,tailroom);
        if (packet == 0) return click_chatter( "cannot make packet!");

        struct routing_update_packet *rpkt = (routing_update_packet *)packet->data();

        rpkt->header.type = ROUTING_UPDATE_TYPE;
        rpkt->header.seq_num = packet_seq_num;
        rpkt->header.src_address = myaddress;

        rpkt->routing_packet_length = sizeof(routing_update_packet);
        rpkt->route_entries = RoutingTable.size();

        int index = 0;

        for ( outermap_iterator ot = RoutingTable.begin() ; ot != RoutingTable.end(); ot++ ) {

       		rpkt->adv[index].destination_address = (*ot).first ;
                
                if (  (RoutingTable[(*ot).first].begin() != RoutingTable[(*ot).first].end()) ) {
                	rpkt->adv[index].metric = RoutingTable[(*ot).first].begin()->first;
                }

                index++;
         }

        click_chatter("Sending router update with seqnum = %d to switch", packet_seq_num);

  	if (Packet *clone = packet->clone()){
                        clone->set_anno_u8(_anno, 255);
                        output(0).push(clone);
                }

}



/* Push configuration for incoming packets */
void BasicRouter::push(int port, Packet *packet) {

	assert(packet);
        struct common_header *header = (struct common_header *)packet->data();

        if (header->type == ROUTING_UPDATE_TYPE) { // update routing table 

                click_chatter("Received an update packet with seq num %d from %d", header->seq_num, header->src_address); 

        	struct routing_update_packet *rpkt = (routing_update_packet *)packet->data();
        	int number_of_entries = int(rpkt->route_entries);

         	for(int i=0; i < number_of_entries; i++) {

        	   click_chatter("\t%d at cost %d\n",
                   rpkt->adv[i].destination_address,
                   rpkt->adv[i].metric);

      		}

                for (int i=0; i<number_of_entries; i++) {
                     
                        // increase cost of every entry in the update packet by 1 before inserting to the routing table
                        int updated_cost = int(++(rpkt->adv[i].metric));
                        int destination = int(rpkt->adv[i].destination_address);
                        int src = int(header->src_address);

                        // update core logic starts here
 			if ( RoutingTable.find(destination) == RoutingTable.end() ) {

                    		// if entry doesn't exist in the routing table map, simply add 
                                click_chatter("Router update: Entry %d not present, adding to routing table", destination);
                    		RoutingTable[destination][updated_cost].insert(src);

			} else {
       
                    		// if key present, check the inner cost value 
                                click_chatter("Router update: Entry %d present, checking whether inner map is empty", destination);

                        	if (  (RoutingTable[destination].begin() != RoutingTable[destination].end()) ){
        
	     				   if ( (RoutingTable[destination].begin()->first) > updated_cost ){

                                                 click_chatter("Router update: greater cost present already, clearing...");
                                                 RoutingTable[destination].clear();
                                                 RoutingTable[destination][updated_cost].insert(src);                                               	

        		       	   	   } else if ( (RoutingTable[destination].begin()->first) <  updated_cost ){

                                                 click_chatter("Router update : got better value already, ignoring...");

                                  	   } else if ( (RoutingTable[destination].begin()->first) ==  updated_cost ) {

                                                	 // if equal value, then check size first, we need no more than 3
                                                
							 if ( RoutingTable[destination][(RoutingTable[destination].begin()->first)].size() < 3 ){
                                                        	 RoutingTable[destination][updated_cost].insert(src);
                                                                 click_chatter("Router update : Less than 3 next hops..Adding ...");
                                                         }

                                                 	else 
                                                        	 click_chatter("Router update : Already 3 next hops.. ignoring...");

                                           }
   				 }
                                 else {

                                            click_chatter("Inner map empty");
                                            RoutingTable[destination][updated_cost].insert(src);
                                 }
			}
                }


                // send ack for routing update packet
                int headroom = sizeof(click_ether);
                WritablePacket *ack = Packet::make(headroom,0,sizeof(struct ack_packet), 0);
                memset(ack->data(),0,ack->length());

                struct ack_packet *format = (struct ack_packet*) ack->data();

                        format->header.type = ACKNOWLEDGMENT_TYPE;
                        format->header.seq_num = header->seq_num; //use same sequence number from the received packet
                        format->header.src_address = uint16_t(myaddress);
                        format->dst_address = header->src_address;

                packet->kill(); // kill received data packet as it is no longer needed

                int destination_port = topologyElement->get_output_port(int(header->src_address));

                if (destination_port == -1){

                    click_chatter( "Basic Router : Key not present in the ports table. killing the packet.\n");
                    packet->kill();

               } else {

                    ack->set_anno_u8(_anno, destination_port);
                    output(0). push(ack);
               }

        } else {

              click_chatter( "Basic Router : Received unknown packet.\n");
              packet -> kill();

        }
}



CLICK_ENDDECLS
EXPORT_ELEMENT(BasicRouter)


