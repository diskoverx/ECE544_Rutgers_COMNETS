#ifndef CLICK_BASIC_ROUTER_HH
#define CLICK_BASIC_ROUTER_HH

#include<algorithm>
#include<set>
#include<map>
#include <click/element.hh>
#include <click/timer.hh>
#include <vector>
#include "topology.hh"

CLICK_DECLS

class BasicRouter: public Element {

    public:

        BasicRouter();
        ~BasicRouter();

        const char *class_name() const { return "BasicRouter";}
        const char *port_count() const { return "-/-";}
        const char *processing() const { return PUSH; }

        void push(int port, Packet *packet);
        int initialize(ErrorHandler*);
	int configure(Vector<String>&, ErrorHandler*);
        void run_timer(Timer*); 

        typedef std::map<int,int> ptable;
	typedef std::map<int,int>::iterator ptable_iterator;

	typedef std::set<int> nexthops;
	typedef std::map<int, nexthops> innerMap;
	typedef std::map<int, innerMap> mainMap;

	typedef std::set<int>::const_iterator nexthop_iterator;
	typedef std::map<int, nexthops>::const_iterator innermap_iterator;
	typedef std::map<int, innerMap>::const_iterator outermap_iterator;

        void initialize_neighbour_table();
        void print_entire_routing_table();
        void send_updates();

        int find_best(int,int,int);
        std::pair<int,int> get_best_two (int, int, int);

        int get_next_hop(int); 
        int get_common_hop(int,int);
        int get_three_common_hop(int, int, int);

        int two_sets_intersect(std::set<int>, std::set<int>);
        int three_sets_intersect(std::set<int>, std::set<int>, std::set<int>);
        


    private:

        Timer test_timer;
        Timer update_timer;

        int myaddress;
        
        Topology *topologyElement;

	std::vector<int> dst_1_nxt_hops;
	std::vector<int> dst_2_nxt_hops;
	std::vector<int> dst_3_nxt_hops;

        mainMap RoutingTable;

};

CLICK_ENDDECLS
#endif
