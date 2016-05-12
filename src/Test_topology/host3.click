
// HOST 107

require(library /home/comnetsii/elements/lossyrouterport.click);

rp :: LossyRouterPort(DEV veth20, IN_MAC d6:f5:8f:75:96:84, OUT_MAC da:95:7c:49:de:23, LOSS 0.9, DELAY 0.2);

client::Client( PAYLOAD "STRAWBERRY", K_VALUE 3, 
                MY_ADDRESS 107, 
                DST1 105, DST2 106, DST3 107,
                DELAY 10, DATA_SEND_ENABLE 0);

classifier :: PacketClassifier();

client -> Print(Sending, MAXLENGTH -1, CONTENTS ASCII) -> rp;

rp -> classifier;

classifier[0] -> Discard(); // client doesn't need hello packets
classifier[1] -> Discard(); // client doesn't need update packets

classifier[2] -> [1]client; // ack packets to client's input 1
classifier[3] -> [0]client; // data packet to client's input 0



