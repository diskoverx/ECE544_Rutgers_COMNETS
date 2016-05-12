#ifndef CLICK_TOPOLOGY_HH
#define CLICK_TOPOLOGY_HH

#include <map>
#include <click/hashmap.hh>
#include <click/element.hh>
#include <click/timer.hh>
#include <vector>

CLICK_DECLS

class Topology: public Element {

    public:

        Topology();
        ~Topology();

        typedef std::map<int, int> portmap;
        typedef std::map<int,int>::iterator ports_table_iterator;

        const char *class_name() const { return "Topology";}
        const char *port_count() const { return "-/-";}
        const char *processing() const { return PUSH; }

        void push(int port, Packet *packet);
        int initialize(ErrorHandler*);
	int configure(Vector<String>&, ErrorHandler*);

        void print_ports_table();
        int get_output_port(int);
        void send_hello();
        void run_timer(Timer*); 

        portmap& get_port();
        int get_latest_seq_num();

    private:

        int myaddress;
        int current_seqnum;
        int hello_period;
  
        portmap ports_table;

        Timer hello_timer;
        int seq;
        int _anno;
};

CLICK_ENDDECLS
#endif
