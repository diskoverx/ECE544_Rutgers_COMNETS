
require(library /home/comnetsii/elements/lossyrouterport.click);

define($loss 0.2);
define($delay 0.2);

rp :: LossyRouterPort(DEV $dev, IN_MAC $in_mac, OUT_MAC $out_mac, LOSS $loss, DELAY $delay);
pg :: PacketProcessor();

pg -> Print(Sending, MAXLENGTH -1, CONTENTS ASCII) -> rp ;

rp -> pg;


