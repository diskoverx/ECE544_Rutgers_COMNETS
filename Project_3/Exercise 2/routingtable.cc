#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include "custom_packets.hh"
#include "routingtable.hh"


CLICK_DECLS

/* constructor */
RoutingTable::RoutingTable()
{
	forwardingtable.clear();
};

/* destructor */
RoutingTable::~RoutingTable(){};


int RoutingTable::configure(Vector<String> &conf, ErrorHandler *errh){
	return 0;
}

void RoutingTable::send_Hello_Ack(int routerport){

        // creating acknowledgement packet

        String content = "";
        int tailroom = 0;
        int packetsize = sizeof(custom_packet_format)+content.length();
        int headroom = sizeof(click_ether);

        WritablePacket *hello_ack = Packet::make(headroom,0,packetsize,tailroom);
        if (hello_ack == 0 )return click_chatter( "cannot make packet!");

        custom_packet_format *hello_ack_header = (custom_packet_format *)hello_ack->data();
        hello_ack_header->type = 2;
        hello_ack_header->payload_length = 0;

        // sending ack packet through port 1

        output(routerport).push(hello_ack);

}



void RoutingTable::push(int port, Packet *p){
	
	if(port == 1 || port == 3 || port == 5){   // it's a HELLO packet

                
                hello_packet_format *hp = (hello_packet_format *)(p->data());
		click_chatter(" Routing : forwarding table update..");
	

                if (port == 1) {

          		Update_Table(hp->source, 0);
                        send_Hello_Ack(0);

                        // send hello_ack to rp1 (rt[0])
                }

                else if (port == 3) {

                	Update_Table(hp->source, 1);
                        send_Hello_Ack(1);

                        // send hello_ack to rp2 (rt[1])
                }

                else if (port == 5) {

                	Update_Table(hp->source, 2);
                        send_Hello_Ack(2);

                        // send hello_ack to rp3  (rt[2])
                }
		
	} else if(port == 0 || port == 2 || port == 4){  // It's a data/ack packet
		
                // click_chatter("DATA came from classifier port %d",port);
                custom_packet_format *cp = (custom_packet_format *)(p->data());
		int temp_nexthop = FT_Search(cp->destination);
                
                if (temp_nexthop == -1){

                        // entry missing in the routing table, simply kill the packet
               	 	p -> kill();

                }

                click_chatter("Routing: forwarding DATA/ACK to port %d", temp_nexthop);
		output(temp_nexthop).push(p);
		
	} else {

		click_chatter("Routing: unknown packet killed");
		p->kill();
	}
}



void RoutingTable::Update_Table(int sender, int routerport){
	
	
	FT_Entry ftentry;
	int found_flag = 0;

	for(ForwardingTable::iterator i=forwardingtable.begin(); i!=forwardingtable.end(); i++){

		if (sender == i->destination){

			click_chatter("Routing Table : no update required");
			found_flag = 1;
		}
	}

	if(found_flag==0){

		click_chatter("Routing Table : entry inserted.");
		ftentry.destination = sender;
		ftentry.nexthop = routerport;
		forwardingtable.push_back(ftentry);
	}
	
}


int RoutingTable::FT_Search(int destination){
	
	int found_flag = 0;
        int result;

	for(ForwardingTable::iterator i=forwardingtable.begin(); i!=forwardingtable.end(); i++){
		if (destination == i->destination){
			return i->nexthop;
		}
						
	}
	
        click_chatter("Routing::LookupTable: Entry missing. Kill the packet.");
	return -1;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(RoutingTable)
