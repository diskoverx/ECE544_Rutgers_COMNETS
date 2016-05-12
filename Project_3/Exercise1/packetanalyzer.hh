#ifndef CLICK_PACKETANALYZER_HH
#define CLICK_PACKETANALYZER_HH

#include <click/element.hh>

CLICK_DECLS

class PacketAnalyzer : public Element {

public:
         PacketAnalyzer();
	~PacketAnalyzer();
	
	const char *class_name() const { return "PacketAnalyzer";}
	const char *port_count() const { return "1/2"; }
	const char *processing() const { return PUSH; }
	
	int configure(Vector<String>&, ErrorHandler*);

	void push(int, Packet *);


};

CLICK_ENDDECLS

#endif
