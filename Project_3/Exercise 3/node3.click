
require(library /home/comnetsii/elements/lossyrouterport.click);


rp :: LossyRouterPort(DEV veth6, IN_MAC 1a:38:d5:ee:e9:ae, OUT_MAC ae:2d:cd:b2:22:fb, LOSS 0.2, DELAY 0);

// DATA_SEND_ENABLE = 1, this node will send data packets and hello packets
// DATA_SEND_ENABLE = 0, this node will not send data packets but will receive packets and will send hello packets

pg :: PacketProcessor(DATA_SEND_ENABLE 0, MY_ADDRESS 103, DESTINATION_ADDRESS 102);


pg -> Print(Sending, MAXLENGTH -1, CONTENTS ASCII) -> rp ;

rp -> Print(Received, MAXLENGTH -1, CONTENTS ASCII) -> pg;


