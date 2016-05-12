#ifndef CLICK_PACKETPROCESSOR_HH
#define CLICK_PACKETPROCESSOR_HH

#include <click/element.hh>

CLICK_DECLS

class PacketProcessor : public Element {

public:
         PacketProcessor();
	~PacketProcessor();
	
	const char *class_name() const { return "PacketProcessor";}
	const char *port_count() const { return "0/1"; }
	const char *processing() const { return PUSH; }
	
	int configure(Vector<String>&, ErrorHandler*);

	void push(int, Packet *);

        void sendMessage();
	void run_timer(Timer*);
	
private:

        String content;
	uint32_t packetType;
        Timer timer; // timer required for periodic generation of packets
};

CLICK_ENDDECLS

#endif
