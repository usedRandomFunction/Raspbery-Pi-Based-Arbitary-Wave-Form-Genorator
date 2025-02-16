#ifndef LIBGUI_DRAWING_FUNCTIONS_H
#define LIBGUI_DRAWING_FUNCTIONS_H

#include "common/display.h"

#include <stdint.h>

// Draws a rect from (x0, y0) to (x1, y1), but only filling the edges, inward for boarder_size px
// @param x0 The X coordinate of the top left corner
// @param y0 The Y coordinate of the top left corner
// @param x1 The X coordinate of the bottom right corner
// @param y1 The Y coordinate of the bottom right corner
// @param boarder_size How many pixels the outline should go in
// @param The color (including alpha) to write
// @param buffer The ID of the target frame buffer, if -1 is given the active frame buffer will be used
void draw_outline(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t boarder_size, display_color color, int bufffer);

#endif