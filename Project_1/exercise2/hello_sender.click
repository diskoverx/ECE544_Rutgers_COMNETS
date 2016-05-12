
// Submitted by Nithin Raju Chandy (RUID 158007346)

// Sends 10 "hello" messages, 1 per second to specified destination mac address
// Receiver will send the packets back and that will be printed as well

// Sending the packets from veth1 to veth2
   RatedSource(DATA "hello", RATE 1, LIMIT 10) -> IPEncap(200, $src_ip, $dst_ip)
						-> EtherEncap(0x0800, $src_mac, $dst_mac)
						-> Print(SENDING, MAXLENGTH -1)
						-> ToDevice($dest_dev)

// Print the echoed packets from veth2
   FromDevice($dest_dev) -> Print(RECEIVED, MAXLENGTH -1, CONTENTS ASCII) -> Discard;

