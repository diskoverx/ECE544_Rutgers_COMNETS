
require(library /home/comnetsii/elements/routerport.click);

rp :: RouterPort(DEV $dev, IN_MAC $in_mac, OUT_MAC $out_mac);
pg :: PacketProcessor();

//pg -> Print(Sending, MAXLENGTH -1, CONTENTS ASCII) -> rp ;

pg -> rp;
rp -> pg;


