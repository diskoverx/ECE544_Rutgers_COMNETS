
require(library /home/comnetsii/elements/routerport.click);

rp :: RouterPort(DEV $dev, IN_MAC $in_mac, OUT_MAC $out_mac);

PacketProcessor(PACKET_TYPE 0, PAYLOAD "COMMUNICATION_NETWORKS") -> Print(Sending, MAXLENGTH -1, CONTENTS ASCII) -> rp ;

rp->Idle;

