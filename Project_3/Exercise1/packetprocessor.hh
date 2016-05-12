#ifndef CLICK_PACKETPROCESSOR_HH
#define CLICK_PACKETPROCESSOR_HH

#include <click/element.hh>
#include <click/packet.hh>

CLICK_DECLS

class PacketProcessor : public Element {

public:
    
        PacketProcessor();
	~PacketProcessor();
	
	const char *class_name() const { return "PacketProcessor";}
	const char *port_count() const { return "-/-"; }
	const char *processing() const { return AGNOSTIC; }
	
	int configure(Vector<String>&, ErrorHandler*);

	void push(int, Packet *);

        void sendMessage(); // Main message sender
        void retryMessage(); // Retry sender

	void run_timer(Timer*);
	
private:

 	String content;
    	uint32_t packetType;
    	unsigned int seqnum;

        bool ack_received; // to keep track of acknowledgements
        bool timeout_timer_status; // to keep track of the acknowledgement timer status

        Packet * _waiting_packet; // buffer to hold packet till acknowledged
 
   	Timer timer; // timer for periodic packet generation
        Timer ack_timeout_timer; // acknowledgement timeout timer
};

CLICK_ENDDECLS

#endif
