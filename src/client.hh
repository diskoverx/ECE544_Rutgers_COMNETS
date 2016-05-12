
/* Client.hh - Client implementation. ARQ Strategy - Stop and wait, hop by hop */
/* Prepared by Nithin Raju Chandy - nithin.chandy@rutgers.edu  */

#ifndef CLICK_CLIENT_HH
#define CLICK_CLIENT_HH

#include <click/packet.hh>
#include <click/element.hh>
#include <queue>
#include <vector>
#include <map>

CLICK_DECLS

class Client : public Element {

public:
        Client();
	~Client();
	
	const char *class_name() const { return "Client";}
        const char *port_count() const { return "-/-"; }
	const char *processing() const { return PUSH; }
	
	int configure(Vector<String>&, ErrorHandler*);
        int initialize(ErrorHandler*);

	void push(int, Packet *);
	void run_timer(Timer*);
        void send_hello();
        void send_data();
        void check_queue_status();	

private:

        uint8_t current_seqnum; // most recent seq num 
        uint8_t data_period; // how frequent data has to be sent
        uint8_t hello_period; // how frequent hello messages will be sent
        uint8_t retransmissions; // retransmission count
        uint8_t time_out; // timeout value
 
        /* Remember all parsing element should be integers */

        int data_send_enable;
        int delay; // delay
        int myaddress; // my address
        int kvalue; // k value

        bool RTT_timer_status; // to check whether timeout timer is active or not

        int dst_01_address; // destination 1
        int dst_02_address; // destination 2
        int dst_03_address; // destination 3

        String multicast_payload; // payload

        Timer hello_timer; // timer required for periodic generation of packets
        Timer data_packet_timer; // timer that sends multicast data
        Timer RTT_timer; // timeout timer   
     
        std::queue <WritablePacket *> packet_queue; // queue to hold packets - to implement reliability

};

CLICK_ENDDECLS

#endif


