#include "gui/drawing_functions.h"

void draw_outline(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t boarder_size, display_color color, int bufffer)
{
    if (!boarder_size)
        return;

    boarder_size--;

    display_fill_rect(x0, y0, x1, y0 + boarder_size, color, bufffer);   // Top
    display_fill_rect(x0, y1 - boarder_size, x1, y1, color, bufffer);   // Bottom

    display_fill_rect(x0, y0, x0 + boarder_size, y1, color, bufffer);   // Left
    display_fill_rect(x1 - boarder_size, y0, x1, y1, color, bufffer);   // right
}