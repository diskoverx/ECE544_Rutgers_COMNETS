
require(library /home/comnetsii/elements/routerport.click);

rp :: RouterPort(DEV $dev, IN_MAC $in_mac, OUT_MAC $out_mac);

Idle -> rp;

rp ->  Print(Received, MAXLENGTH -1, CONTENTS ASCII) -> Discard();

