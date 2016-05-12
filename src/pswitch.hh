#ifndef CLICK_PSWITCH_HH
#define CLICK_PSWITCH_HH

#include <click/element.hh>
#include <click/timer.hh>
#include "pswitch.hh"
#include <queue>

CLICK_DECLS

class Pswitch: public Element {

    public:
        Pswitch();
        ~Pswitch();
        const char *class_name() const { return "Pswitch";}
        const char *port_count() const { return "-/-";}
        const char *processing() const { return PUSH; }

        void push(int port, Packet *packet);
        int initialize(ErrorHandler*);
	int configure(Vector<String>&, ErrorHandler*);

        std::queue <Packet *> router_queue[10]; // router can have up to maximum 10 outputs


};

CLICK_ENDDECLS
#endif
