#ifndef BOARD_DECTETION_H
#define BOARD_DECTETION_H



// finds the board type automaticly
// @return 1 for pi 1 / 0, 2 for pi 2, 3 for pi 3, 4 for pi 4, and -1 for unkown
int get_board_type();

// converts the board type integer to a string
// @return The board type string
const char* get_board_name(int boardType);



#endif