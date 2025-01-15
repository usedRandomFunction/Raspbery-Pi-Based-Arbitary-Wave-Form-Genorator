#ifndef PUTCHAR_H
#define PUTCHAR_H

#ifndef EOF
#define EOF (-1)
#endif

// Resets varibles used by putchar, to printing still works
// properly after the display is blanked
void putchar_on_frame_buffer_blanked();

// @return EOF if failed
int putchar(int ch);

#endif