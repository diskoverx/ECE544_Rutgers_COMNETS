
// Submitted by Nithin Raju Chandy (RUID 158007346)
// Sends hello as a rated source and print it to the terminal and then discard

// By default, data will printed in HEX mode
// RatedSource(DATA "hello", RATE 1, LIMIT 1, STOP true) -> Print(DATA) -> Discard();

// To print in ASCII format
   RatedSource(DATA "hello", RATE 1, LIMIT 1, STOP true) -> Print(DATA, MAXLENGTH -1, CONTENTS ASCII) -> Discard();

