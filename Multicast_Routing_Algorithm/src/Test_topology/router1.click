
// ROUTER 1 - 500

require(library /home/comnetsii/elements/lossyrouterport.click);

rp1 :: LossyRouterPort(DEV veth2, IN_MAC 16:20:64:7f:e6:55, OUT_MAC 82:e7:a0:ce:7c:6d, LOSS 0.9, DELAY 0.2);
rp2 :: LossyRouterPort(DEV veth3, IN_MAC ba:23:0d:8c:1d:c5, OUT_MAC d2:ca:7f:ad:6f:9a, LOSS 0.9, DELAY 0.2);
rp3 :: LossyRouterPort(DEV veth5, IN_MAC 4a:4c:cc:98:68:5a, OUT_MAC 2a:8a:2d:06:7f:e8, LOSS 0.9, DELAY 0.2);

topology :: Topology(MY_ADDRESS 500);
router :: BasicRouter(TP_ELEMENT topology, MY_ADDRESS 500);
switch::Pswitch();
forwarder :: Forwarder(TP_ELEMENT topology, ROUTER_ELEMENT router,SWITCH_ELEMENT switch, MY_ADDRESS 500);

// PACKET CLASSIFIERS 

pc1 :: PacketClassifier();
pc2 :: PacketClassifier();
pc3 :: PacketClassifier();

// FIRST RP INPUT

rp1 -> pc1;

pc1[0] -> [0]topology; // hello
pc1[1] -> [0]router; // update
pc1[2] -> [0]forwarder; // ack
pc1[3] -> [0]forwarder; // data


// SECOND RP INPUT 

rp2 -> pc2;

pc2[0]->[1]topology; // hello
pc2[1]-> [1]router; // update
pc2[2]-> [1]forwarder; //ack
pc2[3]-> [1]forwarder;// data


// THIRD RP INPUT

rp3 -> pc3;

pc3[0]->[2]topology; // hello
pc3[1]-> [2]router; // update
pc3[2]-> [2]forwarder; //ack
pc3[3]-> [2]forwarder;// data


// ELEMENT OUTPUTS

forwarder[0] -> switch;
router[0] -> switch;
topology[0] -> switch;


// SWITCH CONFIGURATION

switch[0] -> rp1;
switch[1] -> rp2;
switch[2] -> rp3;

