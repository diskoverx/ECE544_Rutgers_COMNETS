
// ROUTER 5 - 595

require(library /home/comnetsii/elements/lossyrouterport.click);

rp1 :: LossyRouterPort(DEV veth16, IN_MAC 5a:43:23:83:49:02, OUT_MAC 06:94:41:b9:c0:df, LOSS 0.9, DELAY 0.2);
rp2 :: LossyRouterPort(DEV veth17, IN_MAC 02:24:40:38:f3:07, OUT_MAC 22:7e:d0:53:e6:18, LOSS 0.9, DELAY 0.2);
rp3 :: LossyRouterPort(DEV veth19, IN_MAC da:95:7c:49:de:23, OUT_MAC d6:f5:8f:75:96:84, LOSS 0.9, DELAY 0.2);

topology :: Topology(MY_ADDRESS 595);
router :: BasicRouter(TP_ELEMENT topology, MY_ADDRESS 595);
switch::Pswitch();
forwarder :: Forwarder(TP_ELEMENT topology, ROUTER_ELEMENT router,SWITCH_ELEMENT switch, MY_ADDRESS 595);


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

pc2[0] -> [1]topology; // hello
pc2[1] -> [1]router; // update
pc2[2] -> [1]forwarder; // ack
pc2[3] -> [1]forwarder; // data


//THIRD RP INPUT

rp3 -> pc3;

pc3[0] -> [2]topology; // hello
pc3[1] -> [2]router; // update
pc3[2] -> [2]forwarder; // ack
pc3[3] -> [2]forwarder; // data




// ROUTER ELEMENT OUTPUT

forwarder[0] -> switch;
router[0] -> switch;
topology[0] -> switch;


// SWITCH CONFIGURATION

switch[0] -> rp1;
switch[1] -> rp2;
switch[2] -> rp3;




