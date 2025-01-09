.section .text

.macro syscall name, w8_value, svc_value
.global \name
\name:
    mov w8, \w8_value
    svc \svc_value
    ret
.endm

// program management
syscall set_abi_version, 0, #0
syscall exit, 1, #0
syscall vmemmap, 2, #0
syscall switch_to, 3, #0

// basic IO
syscall printf, 0, #1
syscall putchar, 1, #1
syscall uart_putc, 2, #1
syscall uart_getc, 3, #1
syscall uart_poll, 4, #1

// File IO
syscall open, 0, #2
syscall close, 1, #2
syscall get_file_size, 2, #2
syscall read, 3, #2
syscall write, 4, #2
syscall lseek, 5, #2
syscall truncate, 6, #2
syscall ftruncate, 7, #2
syscall remove, 8, #2
syscall fremove, 9, #2
syscall rename, 10, #2
syscall path_exists, 11, #2
syscall diropen, 12, #2
syscall dirread, 13, #2
syscall dirclose, 14, #2

// Display

syscall set_display_pixel, 0, #3
syscall display_fill_rect, 1, #3
syscall copy_to_display, 2, #3
syscall display_draw_string, 3, #3
syscall get_display_width, 4, #3
syscall get_display_height, 5, #3
syscall active_frame_buffer, 6, #3
syscall request_frame_buffers, 7, #3

// Output
syscall dac_output_start, 0, #0x8000
syscall dac_output_end, 1, #0x8000
syscall dac_resolution, 2, #0x8000
syscall dac_channel_buffering, 3, #0x8000
syscall dac_get_sample_rate, 4, #0x8000
syscall dac_channel_supports_config, 5, #0x8000

// Keypad
syscall keypad_polling 0, #0x8001
syscall uart_keypad_emmulation 1, #0x8001
syscall capture_prg_exit 2, #0x8001
syscall get_keypad_state 3, #0x8001
