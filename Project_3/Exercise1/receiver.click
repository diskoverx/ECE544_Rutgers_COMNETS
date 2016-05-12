
require(library /home/comnetsii/elements/routerport.click);

rp :: RouterPort(DEV $dev, IN_MAC $in_mac, OUT_MAC $out_mac);
pa :: PacketAnalyzer();

rp ->  Print(Received, MAXLENGTH -1, CONTENTS ASCII) -> pa;

pa[0] -> Discard();

pa[1] -> rp;


