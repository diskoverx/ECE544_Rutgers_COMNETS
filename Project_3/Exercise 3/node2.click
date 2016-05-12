
require(library /home/comnetsii/elements/lossyrouterport.click);

rp :: LossyRouterPort(DEV veth4, IN_MAC ee:c7:96:b5:f2:07, OUT_MAC be:71:c6:a5:a4:2e, LOSS 0.2, DELAY 0);

// DATA_SEND_ENABLE = 1, this node will send data packets and hello packets
// DATA_SEND_ENABLE = 0, this node will not send data packets but will receive packets and will send hello packets

pg :: PacketProcessor(DATA_SEND_ENABLE 0, MY_ADDRESS 102, DESTINATION_ADDRESS 101);


pg -> Print(Sending, MAXLENGTH -1, CONTENTS ASCII) -> rp ;

rp -> Print(Received, MAXLENGTH -1, CONTENTS ASCII) -> pg;


