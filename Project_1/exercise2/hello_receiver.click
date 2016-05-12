
// Submitted by Nithin Raju Chandy (RUID 158007346)

// veth1 is the sender
// veth2 is the receiver

// Print the packets received from the sender plus echoes it back to the receiver.
FromDevice(veth1) -> Print(Received, MAXLENGTH -1) -> Queue -> ToDevice(veth1) -> Discard;


