#ifndef CLICK_PACKETCLASSIFIER_HH
#define CLICK_PACKETCLASSIFIER_HH

#include <click/element.hh>

CLICK_DECLS

class PacketClassifier : public Element {

public:
         PacketClassifier();
	~PacketClassifier();
	
	const char *class_name() const { return "PacketClassifier";}
	const char *port_count() const { return "3/6"; }
	const char *processing() const { return PUSH; }
	
	int configure(Vector<String>&, ErrorHandler*);

	void push(int, Packet *);

};

CLICK_ENDDECLS

#endif
