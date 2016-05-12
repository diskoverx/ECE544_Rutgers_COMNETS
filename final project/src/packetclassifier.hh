
#ifndef CLICK_PACKETCLASSIFIER_HH 
#define CLICK_PACKETCLASSIFIER_HH 

#include <click/element.hh>
#include <click/timer.hh>

CLICK_DECLS

class PacketClassifier : public Element {

    public:

        PacketClassifier();
        ~PacketClassifier();

        const char *class_name() const { return "PacketClassifier";}
        const char *port_count() const { return "1/4";}
        const char *processing() const { return PUSH; }
		
	void push(int port, Packet *packet);
        int initialize(ErrorHandler*);
		
}; 

CLICK_ENDDECLS

#endif 

