
// ROUTER 4 - 535

require(library /home/comnetsii/elements/lossyrouterport.click);

rp1 :: LossyRouterPort(DEV veth8, IN_MAC 1e:fa:dd:4d:24:be, OUT_MAC 42:e4:71:50:19:22, LOSS 0.9, DELAY 0.2);
rp2 :: LossyRouterPort(DEV veth10, IN_MAC 26:c9:9a:2b:6e:c5, OUT_MAC 92:87:ae:41:e4:79, LOSS 0.9, DELAY 0.2);
rp3 :: LossyRouterPort(DEV veth11, IN_MAC 3a:34:1e:c2:2d:2e, OUT_MAC da:be:1c:5b:b1:84, LOSS 0.9, DELAY 0.2);

topology :: Topology(MY_ADDRESS 535);
router :: BasicRouter(TP_ELEMENT topology, MY_ADDRESS 535);
switch::Pswitch();
forwarder :: Forwarder(TP_ELEMENT topology, ROUTER_ELEMENT router,SWITCH_ELEMENT switch, MY_ADDRESS 535);


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


// THIRD RP INPUT

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





