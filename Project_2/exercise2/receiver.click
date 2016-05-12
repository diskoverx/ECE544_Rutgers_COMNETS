
require(library /home/comnetsii/elements/routerport.click);

rp :: RouterPort(DEV $dev, IN_MAC $in_mac, OUT_MAC $out_mac);

pa :: PacketAnalyzer;

// Passing received packet in to packet analyzer
rp[0] ->  Print(Received, MAXLENGTH -1, CONTENTS ASCII) -> pa;

// If it comes out of port 0, push it back to the router.
pa[0] -> [0]rp ;

// if it comes out of port 1, discard the packet.
pa[1] -> Discard();



