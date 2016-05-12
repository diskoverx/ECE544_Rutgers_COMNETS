#ifndef CLICK_ROUTINGTABLE_HH
#define CLICK_ROUTINGTABLE_HH

#include <click/element.hh>
#include <clicknet/udp.h>
#include <clicknet/ether.h>
#include <click/ipaddress.hh>
#include <click/vector.hh>
#include <string>
CLICK_DECLS


struct FT_Entry{

        int destination;
	int nexthop;
	
};


// vector of forward table entries
typedef Vector<FT_Entry> ForwardingTable;


class RoutingTable : public Element {
	
	public:
		RoutingTable();
		~RoutingTable();
		
		const char *class_name() const {return "RoutingTable";}
		const char *port_count() const {return "6/3";}
		const char *processing() const {return AGNOSTIC;}
		
		
		int configure(Vector<String>&, ErrorHandler*);
		
		void push(int, Packet *);

		void Update_Table(int, int );

                void send_Hello_Ack(int);

		int  FT_Search(int);
	
	private:
	
	ForwardingTable forwardingtable;
	
};


CLICK_ENDDECLS

#endif
