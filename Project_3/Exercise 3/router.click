require(library /home/comnetsii/elements/lossyrouterport.click);

// veth2 to veth1 connection
rp1 :: LossyRouterPort(DEV veth2, IN_MAC ce:30:e3:04:fa:82, OUT_MAC 56:a0:77:bd:8d:35, LOSS 0.2, DELAY 0);

// veth3 to veth4 connection
rp2 :: LossyRouterPort(DEV veth3, IN_MAC be:71:c6:a5:a4:2e, OUT_MAC ee:c7:96:b5:f2:07, LOSS 0.2, DELAY 0);

//veth5 to veth6 connection
rp3 :: LossyRouterPort(DEV veth5, IN_MAC ae:2d:cd:b2:22:fb, OUT_MAC 1a:38:d5:ee:e9:ae, LOSS 0.2, DELAY 0);

mc :: PacketClassifier; // classifier element with 3 inputs and 6 outputs
rt :: RoutingTable; // routing table element with 6 inputs and 3 outputs


// INCOMING PACKET --> ROUTERPORT INPUT PORT  --> CLASSIFIER --> ROUTING TABLE --> ROUTERPORT OUTPUT PORT 



// ROUTER PORT OUTPUTS --> CLASSIFIER INPUT PORTS

rp1 -> Print(rp1_Received: , CONTENTS ASCII, MAXLENGTH -1 ) -> [0]mc;   // whatever you receive on rp1, send it to classifier input port 0
rp2 -> Print(rp2_Received: , CONTENTS ASCII, MAXLENGTH -1) -> [1]mc;   // whatever you receive on rp2, send it to classifier input port 1
rp3 -> Print(rp3_Received: , CONTENTS ASCII, MAXLENGTH -1) -> [2]mc;   // whatever you receive on rp3, send it to classifier input port 2



// CLASSIFIER OUTPUT PORTS --> FORWARDING TABLE ELEMENT INPUTS

// mc's output ports 0,1 carries routerport rp1's data.
mc[0] -> [0]rt;  // mc[0] for sending rp1's data/ack
mc[1] -> [1]rt;  // mc[1] for rp1's hello messages

// mc's output ports 2,3 carries routerport rp2's data.
mc[2] -> [2]rt;  // mc[2] for rp2's sending data/ack
mc[3] -> [3]rt;  // mc[3] for rp2's hello messages

// mc's output ports 4,5 carries routerports  rp3's data.
mc[4] -> [4]rt;  // mc[4] for rp3's sending data/ack
mc[5] -> [5]rt;  // mc[5] for rp3's hello messages



// FORWARDING TABLE OUTPUT PORTS --> ROUTER PORT INPUTS

rt[0] -> Print(Sending_to_rp1: ) -> rp1;
rt[1] -> Print(Sending_to_rp2: ) -> rp2;
rt[2] -> Print(Sending_to_rp3: ) -> rp3;







