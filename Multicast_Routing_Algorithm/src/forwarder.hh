#ifndef CLICK_FORWARDER_HH
#define CLICK_FORWARDER_HH

#include <click/hashmap.hh>
#include <click/element.hh>
#include <click/timer.hh>
#include "topology.hh"
#include "basicrouter.hh"
#include "pswitch.hh"
#include <vector>

CLICK_DECLS

class Forwarder: public Element {

    public:

        Forwarder();
        ~Forwarder();
        const char *class_name() const { return "Forwarder";}
        const char *port_count() const { return "-/-";}
        const char *processing() const { return PUSH; }

        void push(int port, Packet *packet);
        int initialize(ErrorHandler*);
	int configure(Vector<String>&, ErrorHandler*);

    private:

        int myaddress;
        int seq;
        Topology* topologyElement;
        BasicRouter* routerElement;
        Pswitch* switchElement;


};

CLICK_ENDDECLS
#endif
