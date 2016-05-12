
// ROUTER 5 - 565

require(library /home/comnetsii/elements/lossyrouterport.click);

rp1 :: LossyRouterPort(DEV veth14, IN_MAC 86:88:48:d2:20:b3, OUT_MAC 1a:60:f3:df:e7:02, LOSS 0.9, DELAY 0.2);
rp2 :: LossyRouterPort(DEV veth15, IN_MAC 06:94:41:b9:c0:df, OUT_MAC 5a:43:23:83:49:02, LOSS 0.9, DELAY 0.2);

topology :: Topology(MY_ADDRESS 565);
router :: BasicRouter(TP_ELEMENT topology, MY_ADDRESS 565);
switch::Pswitch();
forwarder :: Forwarder(TP_ELEMENT topology, ROUTER_ELEMENT router,SWITCH_ELEMENT switch, MY_ADDRESS 565);


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


// ROUTER ELEMENT OUTPUT

forwarder[0] -> switch;
router[0] -> switch;
topology[0] -> switch;


// SWITCH CONFIGURATION

switch[0] -> rp1;
switch[1] -> rp2;





