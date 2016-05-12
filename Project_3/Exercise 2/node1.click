
require(library /home/comnetsii/elements/routerport.click);


rp :: RouterPort(DEV veth1, IN_MAC 56:a0:77:bd:8d:35, OUT_MAC ce:30:e3:04:fa:82);


// DATA_SEND_ENABLE = 1, this node will send data packets and hello packets
// DATA_SEND_ENABLE = 0, this node will not send data packets but will receive packets and will send hello packets


pg :: PacketProcessor(DATA_SEND_ENABLE 1, MY_ADDRESS 101, DESTINATION_ADDRESS 103);


pg -> Print(Sending, MAXLENGTH -1, CONTENTS ASCII) -> rp ;

rp -> Print(Received, MAXLENGTH -1, CONTENTS ASCII) -> pg;


