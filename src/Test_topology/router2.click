
// ROUTER 2 - 505

require(library /home/comnetsii/elements/lossyrouterport.click);

rp1 :: LossyRouterPort(DEV veth4, IN_MAC d2:ca:7f:ad:6f:9a, OUT_MAC ba:23:0d:8c:1d:c5, LOSS 0.9, DELAY 0.2);
rp2 :: LossyRouterPort(DEV veth7, IN_MAC 42:e4:71:50:19:22, OUT_MAC 1e:fa:dd:4d:24:be, LOSS 0.9, DELAY 0.2);

topology :: Topology(MY_ADDRESS 505);
router :: BasicRouter(TP_ELEMENT topology, MY_ADDRESS 505);
switch::Pswitch();
forwarder :: Forwarder(TP_ELEMENT topology, ROUTER_ELEMENT router, SWITCH_ELEMENT switch, MY_ADDRESS 505);

// PACKET CLASSIFIERS 

pc1 :: PacketClassifier();
pc2 :: PacketClassifier();

// FIRST RP INPUT

rp1 -> pc1;

pc1[0] -> [0]topology; // hello
pc1[1] -> [0]router; // update
pc1[2] -> [0]forwarder; // ack
pc1[3] -> [0]forwarder; // data


// SECOND RP INPUT

rp2 -> pc2;

pc2[0] -> [1]topology; // hello
pc2[1] -> [1]router; // update
pc2[2] -> [1]forwarder; // ack
pc2[3] -> [1]forwarder; // data

forwarder[0] -> switch;
router[0] -> switch;
topology[0] -> switch;

// SWITCH CONFIGURATION

switch[0] -> rp1;
switch[1] -> rp2;






