from curses import wrapper
import curses.ascii
import textwrap
import argparse
import curses
import socket
import serial
import select
import time
import json
import os

dont_save_edit_command_history = False
dont_update_output_window = False
uart_output_log_window = None
uart_input_log_window = None
keypad_details_window = None
console_scroll_position_y = 0
console_scroll_position_x = 0
console_text_input = ""
console_error_text = ""
config_file_path = ""
console_window = None
terminal_height = 0
terminal_width = 0
connection = None
arguments = None
running = True
stdscr = None
config = None

magic_word_uart_ready_recive_offset = 0
magic_word_uart_ready_recived = False
magic_word_uart_ready = "UARTRDY\n"

full_name = "AWG uart interface V 0.8.2"

uart_output_log = [""]
uart_output_log_scroll_y = 0
uart_output_log_scroll_x = 0

uart_input_hiddle_bytes_counter = -1 # Used while sending files as to not just delete the entrie log
uart_input_max_lines = 0
uart_input_log = []

class awg_connection:
    def __init__(self, conn_type, address, port=None, baudrate=None):
        if conn_type == "tcp":
            self.conenction = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.conenction.connect((address, port))
            self.conenction.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 65536)  # Set buffer size to 64 KB
            self.conenction.setblocking(False)
            self.target_string = f"{address}:{port}"
            self.connection_type = 0
        elif conn_type == "serial":
            self.target_string = f"{address} @{baudrate}"
            self.conenction = serial.Serial(address, baudrate)
            self.connection_type = 1
        elif conn_type == 'stt':    # Serial over Tcp Tacked
            self.conenction = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.conenction.connect((address, 4242))
            self.conenction.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 65536)  # Set buffer size to 64 KB
            self.conenction.setblocking(False)
            self.tracking_conenction = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.tracking_conenction.connect((address, 4243))
            self.tracking_conenction.setblocking(False)
            self.target_string = f"{address} (STT)"
            self.connection_type = 2
        elif conn_type ==  "test":
            self.target_string = "Testing mode, no connection"
            self.connection_type = 3
        else:
            raise ValueError("Connection type must be 'tcp', 'serial', 'stt' or 'test'")

    def write(self, data):
        if self.connection_type == 1:
            self.conenction.write(data)
        elif self.connection_type == 0:
            self.conenction.sendall(data)
        elif self.connection_type == 2:
            self.stt_wait_for_tx_threshold_clear()
            self.conenction.sendall(data)
            time.sleep(0.05)            # Wait for 50 ms to wait for any status responce
            self.stt_wait_for_tx_threshold_clear()

    def read(self, size=1024):
        if self.connection_type == 1:
            return self.conenction.read(size).decode("utf-8", "replace")
        elif self.connection_type == 0 or 2:
            try:
                return self.conenction.recv(size).decode("utf-8", "replace")
            except BlockingIOError:
                return None

    def available(self):
        if self.connection_type == 1:
            return self.conenction.in_waiting != 0
        elif self.connection_type == 0 or self.connection_type == 2:
            return bool(select.select([self.conenction], [], [], 0.01))

    def close(self):
        self.conenction.close()

        if self.connection_type == 2:
            self.tracking_conenction.close()

    def stt_wait_for_tx_threshold_clear(self):
        if self.connection_type != 2:
            return

        status_value = b'\00'

        try:
            status_value = self.tracking_conenction.recv(1)
        except BlockingIOError:
            return

        while status_value != b'\00':
            try:
                status_value = self.tracking_conenction.recv(1)
            except BlockingIOError:
                pass

def horizontaly_center_text(window: curses.window, y: int, text, attribute = curses.A_NORMAL):
    _, width = window.getmaxyx()

    if len(text) > (width - 4):
        window.addnstr(y, 2, text, width - 4)
        return

    x = (width - len(text)) // 2

    window.addstr(y, x, text, attribute)

def create_windows():
    global uart_output_log_window, uart_input_log_window, keypad_details_window
    global terminal_height, terminal_width, console_window

    terminal_height, terminal_width = stdscr.getmaxyx()

    stdscr.clear()
    horizontaly_center_text(stdscr, 0, full_name)
    stdscr.refresh()


    keypad_window_height = 19
    info_windows_width = 34

    output_log_window_width = terminal_width - info_windows_width
    output_log_window_height = terminal_height - 2
    input_log_window_height = terminal_height - keypad_window_height - 2


    uart_output_log_window = curses.newwin(output_log_window_height, output_log_window_width, 1, 0)
    draw_uart_output_window()
    
    uart_input_log_window = curses.newwin(input_log_window_height, info_windows_width, 1, output_log_window_width)
    resize_uart_input_window(input_log_window_height)
    draw_uart_input_window()

    keypad_details_window = curses.newwin(keypad_window_height, info_windows_width, input_log_window_height + 1, output_log_window_width)
    draw_keypad_window()

    console_window = curses.newwin(1, terminal_width, terminal_height - 1, 0)
    draw_console_window()
    
def uart_output_window_handle_input(input):
    global uart_output_log_scroll_y, uart_output_log_scroll_x

    if input == curses.KEY_UP:
        uart_output_log_scroll_y = max(uart_output_log_scroll_y - 1, 0)
        draw_uart_output_window()
    elif input == curses.KEY_DOWN:
        uart_output_log_scroll_y = min(uart_output_log_scroll_y + 1, max(len(uart_output_log) - 5, 0))
        uart_output_log_scroll_y = min(uart_output_log_scroll_y + 1, max(len(uart_output_log) - 5, 0))
        draw_uart_output_window()
    elif input == curses.KEY_LEFT:
        uart_output_log_scroll_x = max(uart_output_log_scroll_x - 1, 0)
        draw_uart_output_window()
    elif input == curses.KEY_RIGHT:
        uart_output_log_scroll_x = uart_output_log_scroll_x + 1
        draw_uart_output_window()

def draw_uart_output_window():
    cursor_y, cursor_x = stdscr.getyx() # Dont move the cursor
    uart_output_log_window.clear()
    uart_output_log_window.border("|", "|", "=", "=", "+", "+", "+", "+")
    horizontaly_center_text(uart_output_log_window, 0, " UART Output (From AWG) ")

    lines, width = uart_output_log_window.getmaxyx()
    lines = lines - 2
    width = width - 4

    start_line = uart_output_log_scroll_y
    number_of_lines_to_show = min(len(uart_output_log) - start_line, lines)
    end_line = start_line + number_of_lines_to_show
    
    for i in range(start_line, end_line, 1):
        screen_possition_y = 1 + (i - start_line)
        line = uart_output_log[i]

        if len(line) < uart_output_log_scroll_x:    # Skip line if its scrolled of the side
            continue

        line = line[uart_output_log_scroll_x:]      # Remove parts of the string scrolled of the screen
        
        uart_output_log_window.addnstr(screen_possition_y, 2, line, width)



    uart_output_log_window.refresh()
    stdscr.move(cursor_y, cursor_x)

def resize_uart_input_window(new_height):
    global uart_input_max_lines

    uart_input_max_lines =  new_height - 5

    remove_last_uart_log_entry_if_required()

def remove_last_uart_log_entry_if_required():
    global uart_input_log

    while len(uart_input_log) > uart_input_max_lines:
        entry = "Value"

        while entry != "" and len(uart_input_log) > 0: # "" is used to indicate blank lines
            entry = uart_input_log.pop(0)   # so we remove all of a entry by waiting untill a blank line
            print(entry)

def split_byte_array(byte_array, chunk_size):
    return [byte_array[i:i + chunk_size] for i in range(0, len(byte_array), chunk_size)]

def uart_send_wrapper(data):
    global uart_input_hiddle_bytes_counter
    global uart_input_log

    connection.write(data)

    if uart_input_hiddle_bytes_counter == -1:
        # we split into ten bytes each as ten fit on a line
        split_data = split_byte_array(data, 10)

        new_lines = []

        for section in split_data:
            line = section.hex(' ').upper()
            new_lines.append(line)

        uart_input_log += reversed(new_lines)
        uart_input_log.append("")
        remove_last_uart_log_entry_if_required()
        

        draw_uart_input_window()
    else:
        uart_input_hiddle_bytes_counter = uart_input_hiddle_bytes_counter + len(data)

def uart_stop_tracking_input(): # Used while sending files as to not just delete the entrie log
    global uart_input_hiddle_bytes_counter

    uart_input_hiddle_bytes_counter = 0

def uart_restart_tracking_input():
    global uart_input_hiddle_bytes_counter
    global uart_input_log

    if uart_input_hiddle_bytes_counter <= 0:
        uart_input_hiddle_bytes_counter = -1
        return

    line = ""
    
    if uart_input_hiddle_bytes_counter > 999 * 1024:
        line = f"    < {(uart_input_hiddle_bytes_counter // 1024):05f} KiB unshown >    "
    elif uart_input_hiddle_bytes_counter > 99999 * 1024:
        line = "    < 99999+ KiB unshown >   "
    else:
        number_string = f"{(uart_input_hiddle_bytes_counter / 1024):.1f}"

        while len(number_string) < 5:
            number_string = f" {number_string}"

        line = f"    < {number_string} KiB unshown >    "

    uart_input_log += [line]
    uart_input_log.append("")

    remove_last_uart_log_entry_if_required()
        
    draw_uart_input_window()
    
    uart_input_hiddle_bytes_counter = -1

def draw_uart_input_window():
    cursor_y, cursor_x = stdscr.getyx() # Dont move the cursor
    uart_input_log_window.clear()

    uart_input_log_window.border("|", "|", "=", "=", "+", "+", "+", "+")
    horizontaly_center_text(uart_input_log_window, 0, " UART Input (To AWG) ")
    horizontaly_center_text(uart_input_log_window, 1, "00 01 02 03 04 05 06 07 08 09")

    number_of_entrys = len(uart_input_log)

    for i, line in enumerate(uart_input_log):
        uart_input_log_window.addstr(1 + number_of_entrys - i, 2, line)

    uart_input_log_window.refresh()
    stdscr.move(cursor_y, cursor_x)

def draw_keypad_window():
    cursor_y, cursor_x = stdscr.getyx() # Dont move the cursor
    keypad_details_window.attron(curses.A_BOLD)
    keypad_details_window.border("|", "|", "=", "=", "+", "+", "+", "+")
    horizontaly_center_text(keypad_details_window, 0, " Keypad Emmulation ")
    keypad_details_window.attroff(curses.A_BOLD)
    
    horizontaly_center_text(keypad_details_window, 2,  "+-----+-----+-----+----------+")
    horizontaly_center_text(keypad_details_window, 3,  "|  7  |  8  |  9  |    A     |")
    horizontaly_center_text(keypad_details_window, 4,  "+-----+-----+-----+----------+")
    horizontaly_center_text(keypad_details_window, 5,  "|  4  |  5  |  6  |    B     |")
    horizontaly_center_text(keypad_details_window, 6,  "+-----+-----+-----+----------+")
    horizontaly_center_text(keypad_details_window, 7,  "|  1  |  2  |  3  |    C     |")
    horizontaly_center_text(keypad_details_window, 8,  "+-----+-----+-----+----------+")
    horizontaly_center_text(keypad_details_window, 9,  "|  .  |  0  |  ±  |    D     |")
    horizontaly_center_text(keypad_details_window, 10, "+-----+-----+-----+----------+")
    horizontaly_center_text(keypad_details_window, 11, "| DEL | CLR | ENT | PRG_EXIT |")
    horizontaly_center_text(keypad_details_window, 12, "+-----+-----+-----+----------+")
    horizontaly_center_text(keypad_details_window, 13, "| CH1 | CH2 | CH3 |   CH4    |")
    horizontaly_center_text(keypad_details_window, 14, "+-----+-----+-----+----------+")
    horizontaly_center_text(keypad_details_window, 15, "| BUF | BUF | BUF |   BUF    |")
    horizontaly_center_text(keypad_details_window, 16, "+-----+-----+-----+----------+")

    keypad_details_window.refresh()
    stdscr.move(cursor_y, cursor_x)

def redraw_subwindows():
    draw_uart_output_window()
    draw_uart_input_window()
    draw_keypad_window()

def create_new_popup_window(height, width, height_offest = 0, width_offset = 0) -> curses.window:
    # basicly they all follow the same rule for where they go so this function exists
    y = (terminal_height - height) // 4
    x = (terminal_width - width) // 2

    return curses.newwin(height, width, y + height_offest, x + width_offset)

def create_centered_window(height, width, parent: curses.window, height_offest = 0, width_offset = 0) -> curses.window:
    parent_start_y, parent_start_x = parent.getbegyx()
    parent_height, parent_width = parent.getmaxyx()
    y = (parent_height - height) // 2
    x = (parent_width - width) // 2
    
    return parent.subwin(height, width, y + height_offest + parent_start_y, x + width_offset + parent_start_x)

def connect_to(conn_type, args):
    global connection

    info_window = create_new_popup_window(10, 40)
    info_window.border("|", "|", "=", "=", "+", "+", "+", "+")
    horizontaly_center_text(info_window, 0, " Connecting ")
    horizontaly_center_text(info_window, 2, conn_type)

    if len(args) > 0:
        horizontaly_center_text(info_window, 3, args[0])
    
    number_of_arguments = 0

    if conn_type == "tcp":
        number_of_arguments = 2
    elif conn_type == "serial":
        number_of_arguments = 2
    elif conn_type == "stt":
        number_of_arguments = 1
    elif conn_type == "test":
        number_of_arguments = 0
    else:
        raise ValueError("Connection type must be 'tcp', 'serial', 'stt', or 'test'")

    if len(args) != number_of_arguments:
        raise ValueError(f"{number_of_arguments} arguments expected for {conn_type}, got {len(args)}.")

    if conn_type == "tcp":
        horizontaly_center_text(info_window, 4, args[1])
        info_window.refresh()
        connection = awg_connection(conn_type, args[0], port=int(args[1]))
    elif conn_type == "serial":
        horizontaly_center_text(info_window, 4, args[1])
        info_window.refresh()
        connection = awg_connection(conn_type, args[0], baudrate=args[1])
    elif conn_type == "stt":
        info_window.refresh()
        connection = awg_connection(conn_type, args[0])
    elif conn_type == "test":
        connection = awg_connection(conn_type, None)

    draw_console_window()
    redraw_subwindows()

def parse_cfg_file(filepath):
    lines = []
    file = {}

    with open(filepath, 'r') as fd:
        lines = fd.readlines()

    for line_index in range(len(lines)):
        line = lines[line_index]
        comment_index = line.find('#')

        if comment_index != -1:
            line = line[:comment_index] # remove comments

        if len(line) == 0 or line == "\n":
            continue
        
        first_break = min(max(line.find(' '), 0), max(line.find('='), 0))

        name = line[0:first_break]
        value = line[first_break:]  # This still has leading spaces and equals

        value = value.lstrip()

        while value[0] == '=':
            value = value[1:].lstrip()

        if value[0] != '"':
            file[name] = value.replace('\n', '')
            continue

        # Now handle qoutes

        line = value[1:] # Work around since this is how the rest of the code will work
        value = ""
        done = False


        while line_index < len(lines) and done == False:
            i = 0

            while i < len(line):
                if line[i] == '"' and (line[i - 1] != '\\' and line[i - 2] != '\\'):
                    done = True
                    break
                i = i + 1
            
            value += line[:i]
            
        file[name] = value

    return file

def wait_for_magic_word():
    global magic_word_uart_ready_recived

    while magic_word_uart_ready_recived == False:

        if connection.available():
            handle_uart_input()
        

    magic_word_uart_ready_recived = False
        
def uart_input_handle_magic_words(last_recived_char):
    global magic_word_uart_ready_recive_offset
    global magic_word_uart_ready_recived

    if magic_word_uart_ready_recived == True:   # If data is sent after a ready message then it is invalid
        magic_word_uart_ready_recived = False

    if magic_word_uart_ready[magic_word_uart_ready_recive_offset] == last_recived_char:
        magic_word_uart_ready_recive_offset = magic_word_uart_ready_recive_offset + 1

        if magic_word_uart_ready_recive_offset == len(magic_word_uart_ready):
            magic_word_uart_ready_recive_offset = 0
            magic_word_uart_ready_recived = True
    else:
        magic_word_uart_ready_recive_offset = 0
        magic_word_uart_ready_recived = False
    
def handle_uart_input():
    data = connection.read()

    if data == None or data == "":
        return
    
    for char in data:
        uart_input_handle_magic_words(char)
    
    lines = data.replace('\r', '\n\\r').replace('\b','\n\\b').split('\n')

    uart_output_log[len(uart_output_log) - 1] = uart_output_log[len(uart_output_log) - 1] + lines[0]

    for i in range(1, len(lines), 1):
        uart_output_log.append(lines[i])

    if not dont_update_output_window:
        draw_uart_output_window()
    
def draw_console_window():
    console_window.clear()

    if console_error_text != "":
        console_window.addnstr(0, 1, console_error_text, terminal_width - 2, curses.color_pair(1))
        console_window.refresh()

        return


    if console_text_input != "":
        curses.curs_set(1)
        offset = 0

        if console_scroll_position_x > terminal_width - 20:
            offset = (console_scroll_position_x - terminal_width + 20) // 20 * 20 + 1

        console_window.addnstr(0, 1, console_text_input[offset:], terminal_width - 2)
        console_window.move(0, 1 + console_scroll_position_x - offset)

        console_window.refresh()
        return
    
    console_window.addstr(0, 1, "^H", curses.A_REVERSE)
    console_window.addstr(0, 3, " for help")

    if connection != None:
        target_indicator = "connected to: " + connection.target_string

        if len(target_indicator) < (terminal_width - 13):
            console_window.addstr(0, terminal_width - len(target_indicator) - 1, target_indicator)

    curses.curs_set(0)
    console_window.refresh()

help_window_pages = ["""
This program is used to see the uart
outputs and controll keypad emmulat-
-ion. The console is used to input 
commands and keys.

By defult, it sends keys 0-9 for 0-9
A-D for A-D, '.' for '.' '+' or '-'
for '±'. The rest of the keys are
sent with names, the 4 'BUF's are
given the number of the 'CH' key ab-
-ove. They are comma seporated and,
sent when enter is pressed.

When ':' is prefixed it signals a
command, these run on the interface
not the AWG.

Currently thre are 11 commands:

- help (:help / :h) shows this page

- quit (:quit / :q) cloess the app

- clear (:clear) clears the screen
"""[1:], """
- send key (:key) used to send a 
  keypress, with any state. Give 
  the state followed by the buttons.
  i.e :key forced_on a, b, c, d
  the buttons are the same as normal
  and the states are: off, on, 
  force_off, and forced_on.

- kernal reload (:kernal_reload / 
  :kreload) used to reload the 
  kernal. First hold PRG_EXIT while
  power cycling the system wait for
  uart mesage about reloading. Run
  this command with the file path
  to the image file as the agument.
  (relitve paths are allowed)

- run user app (:run_user_app /
  :rua). Used to upload a user app,
  run with out writing to the 
  disk. 

  1. Run uartrun on the AWG
  2. Call :rua ${PATH TO DIR}
"""[1:], """
- save logs (:save \ :save_logs)
  used to save the output log to a
  file. Filepath is first argument

Note for the next four commands:
  File paths on the AWG are limmited
  to 255 characters, by uartupld.

- file upload (:fupld \ 
  :file_upload) Used with uartupld,
  to upload a file. Arguments:
  1. File path on client (this sys)
  2. Destionation path on AWG,

- file move (:fmove \ :file_move).
  Used with uartupld to move a file.
  Aguments:
  1. Old path (On AWG)
  2. New path (On AWG)

- file remove (:fremove \ 
  file_remove), Used with uartupld
  to Delete a file Aguments:
  1. File path (On AWG)
"""[1:], """
- list directory (:ls \ :list_dir \\
  :list_directory), Used with 
  uartupld to list a directory. Args:
  1. Dirrectory path
"""[1:]]

def draw_help_page(page_number, window):
    window.clear()

    window.border("|", "|", "=", "=", "+", "+", "+", "+")
    horizontaly_center_text(window, 0, " Help ")
    horizontaly_center_text(window, 2, full_name)

    text = help_window_pages[page_number]

    for i, line in enumerate(text.splitlines()):
        window.addstr(4 + i, 2, line)

    nubmer_string = f"({page_number + 1} / {len(help_window_pages)})"

    window.addstr(28, 38 - len(nubmer_string), nubmer_string)

    window.refresh()

def show_help_window():
    help_window = create_new_popup_window(30, 40)
    console_window.clear()
    console_window.addstr(0, 1, "q", curses.A_REVERSE)
    console_window.addstr(0, 2, " to close")
    console_window.refresh()
    draw_help_page(0, help_window)
    stdscr.nodelay(0)
    

    page_number = 0

    while True:
        input = stdscr.getch()
        
        if input == ord('q'):
            break
        elif input == curses.KEY_DOWN or input == curses.KEY_RIGHT:
            page_number = min(page_number + 1, len(help_window_pages) - 1)
            draw_help_page(page_number, help_window)
        elif input == curses.KEY_UP or input == curses.KEY_LEFT:
            page_number = max(page_number - 1, 0)
            draw_help_page(page_number, help_window)

    stdscr.nodelay(1)

    redraw_subwindows()
    draw_console_window()

def handle_console_input(key):
    global console_scroll_position_y
    global console_scroll_position_x
    global console_text_input
    global console_error_text

    allowed_chars = set("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz 0123456789,.-:+_/")

    if 0 <= key <= 255 and chr(key) in allowed_chars:
        console_text_input = console_text_input[:console_scroll_position_x] + chr(key) + console_text_input[console_scroll_position_x:]
        console_scroll_position_x = console_scroll_position_x + 1

        console_error_text = ""
    else:
        if key == curses.KEY_BACKSPACE and console_scroll_position_x > 0:
            console_scroll_position_x = console_scroll_position_x - 1
            console_text_input = console_text_input[:console_scroll_position_x] + console_text_input[console_scroll_position_x + 1:]
        elif key == curses.ascii.ESC and stdscr.getch() == -1: # Escape key (this is a weird one)
            console_scroll_position_y = 0
            console_scroll_position_x = 0
            console_text_input = ""
            console_error_text = ""
            draw_console_window()
            return
        elif key == curses.KEY_ENTER or key == ord('\n'):
            if len(console_text_input) != 0:
                if console_text_input[0] == ':':
                    handle_console_command()
                else:
                    console_error_text = send_key_presses_from_string(console_text_input)
            console_scroll_position_y = 0
            console_scroll_position_x = 0
            console_text_input = ""
        else:
            handle_console_scroling(key)
            return

    draw_console_window()

def console_set_text_to_history(possition):
    global console_scroll_position_x
    global console_text_input

    if possition < 0:
        return

    new_text = config["command_history"][possition]
    console_scroll_position_x = len(new_text) + 1
    console_text_input = ":" + new_text


    draw_console_window()

def handle_console_scroling(input):
    global console_scroll_position_y
    global console_scroll_position_x
    global console_text_input

    if console_text_input == "":
        return

    if input == curses.KEY_LEFT:
        console_scroll_position_x = max(console_scroll_position_x - 1, 0)
        draw_console_window()
    elif input == curses.KEY_RIGHT:
        console_scroll_position_x = min(console_scroll_position_x + 1, len(console_text_input))
        draw_console_window()
    elif input == curses.KEY_UP:
        console_scroll_position_y = min(console_scroll_position_y + 1, len(config["command_history"]))
        console_set_text_to_history(console_scroll_position_y - 1)
    elif input == curses.KEY_DOWN:
        new_y = max(console_scroll_position_y - 1, 0)
        new_y = max(console_scroll_position_y - 1, 0)

        if new_y == console_scroll_position_y:
            return
        
        if new_y > 0:
            console_set_text_to_history(console_scroll_position_y - 1)
        else:
            console_scroll_position_y = 0
            console_scroll_position_x = 1
            console_text_input = ":"
            draw_console_window()

        console_scroll_position_y = new_y

def command_clear():
    global uart_output_log_scroll_y, uart_output_log_scroll_x
    global uart_output_log, uart_input_log

    uart_output_log_scroll_y = 0
    uart_output_log_scroll_x = 0
    uart_output_log = [""]
    uart_input_log = []

    redraw_subwindows()

def command_key(arguments):
    state_stirng = arguments[0]
    state = b'\xFF'

    if state_stirng == "off":
        state = b'\x00'
    elif state_stirng == "on":
        state = b'\x01'
    elif state_stirng == "forced_off":
        state = b'\x02'
    elif state_stirng == "forced_on":
        state = b'\x03'
    else:
        raise ValueError(f"Unkown state: {state_stirng}")
    
    send_key_press_uart_message_from_string(','.join(arguments).replace(",,","r"), state)

def handle_chunk_boundarys(file, data, bytes_sent, chunk_size, wait_is_required):
    if chunk_size == 0:
        return data

    chunk_offset = (bytes_sent % chunk_size) + len(data)

    if chunk_offset < chunk_size:               # We havn't reached the chunk boundary yet
        return data
    
    if wait_is_required != None:
        wait_is_required[0] = True

    chunk_offset = chunk_offset % chunk_size    # Now its just of bit after the chunk ends

    if chunk_offset == 0:                       # In the event that we reached a boundary, but haven't exceeded it
        return data                             # we dont after the data we send, just wait before sending more

    if file != None:
        file.seek(-chunk_offset, 1)

    return data[:-chunk_offset]
    
def upload_file(filename, size, progress_bar: curses.window = None, progress_bar_size = 0, chunk_size = 0):
    uart_stop_tracking_input()
    bytes_sent_per_ch = progress_bar_size / size
    wait_is_required = [False]
    current_bar_size = 0
    bytes_sent = 0

    with open(filename, "rb") as file:
        while bytes_sent < size:
            if wait_is_required[0] == True:
                wait_is_required[0] = False
                wait_for_magic_word()

            data = file.read(min(size - bytes_sent, 1024))
            data = handle_chunk_boundarys(file, data, bytes_sent, chunk_size, wait_is_required)
            uart_send_wrapper(data)
            bytes_sent = bytes_sent + len(data)

            if len(data) == 0:  # End if there is no more file to read
                break

            new_bar_size = min(int(bytes_sent * bytes_sent_per_ch), progress_bar_size)

            if progress_bar != None and current_bar_size < new_bar_size:
                progress_bar.addstr(1, current_bar_size + 1, "█" * (new_bar_size - current_bar_size))
                progress_bar.refresh()
                current_bar_size = new_bar_size

    while bytes_sent < size:   # Pad with zeros if needed
        if wait_is_required[0] == True:
            wait_is_required[0] = False
            wait_for_magic_word()

        data = b'\x00' * (size - bytes_sent)
        data = handle_chunk_boundarys(None, data, bytes_sent, chunk_size, wait_is_required)

        uart_send_wrapper(data)
        bytes_sent = bytes_sent + len(data)


    uart_restart_tracking_input()

    return bytes_sent

def command_kenral_reload(filepath):
    size = os.path.getsize(filepath)

    alignment_size = 4  # The kernal reloader app expects the memory to be a multiple of 4
    size = size + ((alignment_size - size % alignment_size) if size % alignment_size > 0 else 0)

    uart_send_wrapper(size.to_bytes(4, 'big'))

    window_width = terminal_width // 3 * 2
    progress_bar_window = create_new_popup_window(9, window_width)
    progress_bar_window.border("|", "|", "=", "=", "+", "+", "+", "+")
    horizontaly_center_text(progress_bar_window, 0, " Uploading file ")
    progress_bar_window.addnstr(1, 2, f"Uploading {os.path.basename(filepath)} ({(size // 1024)} KiB)", window_width - 4)
    progress_bar_window.refresh()

    progress_bar = create_centered_window(3, window_width - 4, progress_bar_window, 1)
    progress_bar.border()
    progress_bar.refresh()

    time.sleep(0.25)     # Give some time for the AWG to get ready
    
    max_bar_size = window_width - 6
    upload_file(filepath, size, progress_bar, max_bar_size)

    redraw_subwindows()

def uartrun_execute_packet(address: int):
    packet = b'\x01'    # Base header (specific header type)

    packet += address.to_bytes(8, 'little')

    uart_send_wrapper(packet)

def uartrun_section_packet(start: int, size: int, flags: int):
    packet = b'\x02'    # Base header (specific header type)

    packet += start.to_bytes(8, 'little')
    packet += size.to_bytes(8, 'little')
    packet += flags.to_bytes(4, 'little')

    uart_send_wrapper(packet)

def uartrun_memset_packet(start: int, size: int, value: int):
    packet = b'\x11\x03'    # Base header (specific header size, type)

    packet += start.to_bytes(8, 'little')
    packet += size.to_bytes(8, 'little')
    packet += value.to_bytes(1, 'little')

    uart_send_wrapper(packet)

def uartrun_memcpy_packet(start: int, size: int, file: str, progress_bar: curses.window = None, progress_bar_size = 0, parent_window: curses.window = None):
    packet = b'\x04'    # Base header (specific header type)

    packet += start.to_bytes(8, 'little')
    packet += size.to_bytes(8, 'little')

    uart_send_wrapper(packet)

    if parent_window != None:
        parent_window.border("|", "|", "=", "=", "+", "+", "+", "+")
        parent_window.refresh()

    wait_for_magic_word()

    upload_file(file, size, progress_bar, progress_bar_size)

# File path is in relation to the computer running this script
def command_run_user_app(filepath): # TODO support ABI version controll
    global dont_update_output_window
    config_file = {}

    if filepath[-1] != '/' or filepath[-1] != '\\':
            filepath = f"{filepath}/"
    
    for file in os.listdir(filepath):   # Find the config file for the app
        if not file.endswith(".cfg"):
            continue
        
        config_file = parse_cfg_file(filepath + file)
        break

    if config_file["APPLICATION_TYPE"] != "MONOLITHIC": 
        application_type = config_file["APPLICATION_TYPE"]
        raise ValueError(f"Invalid application type: \"{application_type}\"")
    
    image_file_path = f"{filepath}bin/" + config_file["IMAGE_PATH"]  # Get the size of monolithic page
    image_file_size = os.path.getsize(image_file_path)
    
    monolithic_page_start = int(config_file["PROGRAM_ADDRESS"], base=0)
    monolithic_page_size = image_file_size
    entry_address = int(config_file["PROGRAM_ENTRY"], base=0)

    image_file_size = min(image_file_size, monolithic_page_size)    # Handles the case of the page size
                                                                    # being smaller then the image
    if "MONOLITHIC_PAGE_SIZE" in config_file: 
        monolithic_page_size = int(config_file["MONOLITHIC_PAGE_SIZE"], base=0)

    dont_update_output_window = True
    program_memory_writability = False
    if "PROGRAM_MEMORY_WRITABILITY" in config_file:
        program_memory_writability = int(config_file["PROGRAM_MEMORY_WRITABILITY"], base=0) != 0

    window_width = terminal_width // 3 * 2
    progress_bar_window = create_new_popup_window(10, window_width)
    progress_bar_window.border("|", "|", "=", "=", "+", "+", "+", "+")
    horizontaly_center_text(progress_bar_window, 0, " Uploading application ")
    progress_bar_window.addnstr(1, 2, f"Uploading {os.path.basename(filepath)} ({(image_file_size // 1024)} KiB)", window_width - 4)
    progress_bar_window.addnstr(2, 2, "Waiting on ready signal", window_width - 4)
    progress_bar_window.refresh()

    progress_bar = create_centered_window(3, window_width - 4, progress_bar_window, 2)
    progress_bar.border()
    progress_bar.refresh()

    wait_for_magic_word()
                                      
    progress_bar_window.addnstr(2, 2, "Creating monolithic page", window_width - 4)
    progress_bar_window.border("|", "|", "=", "=", "+", "+", "+", "+")
    progress_bar_window.refresh()

    uartrun_section_packet(monolithic_page_start, monolithic_page_size, 1) # 1 is VMEMMAP_WRITABILITY

    if monolithic_page_size > image_file_size:
        progress_bar_window.addnstr(2, 2, "Zeroing extra bytes     ", window_width - 4)
        progress_bar_window.border("|", "|", "=", "=", "+", "+", "+", "+")
        progress_bar_window.refresh()

        start_addresss = monolithic_page_start + image_file_size
        size = monolithic_page_size - image_file_size

        wait_for_magic_word()
        uartrun_memset_packet(start_addresss, size, b'\x00')

    progress_bar_window.addnstr(2, 2, "Uploading image         ", window_width - 4)
    progress_bar_window.border("|", "|", "=", "=", "+", "+", "+", "+")
    progress_bar_window.refresh()

    wait_for_magic_word()

    max_bar_size = window_width - 6
    uartrun_memcpy_packet(monolithic_page_start, image_file_size, image_file_path, progress_bar, max_bar_size, progress_bar_window)

    progress_bar_window.addnstr(2, 2, "Enabling executablity", window_width - 4)
    progress_bar_window.border("|", "|", "=", "=", "+", "+", "+", "+")
    progress_bar_window.refresh()

    wait_for_magic_word()

    uartrun_section_packet(monolithic_page_start, monolithic_page_size, 
                           4 | (1 if program_memory_writability else 0)) # 1 is VMEMMAP_WRITABILITY, 4 is VMEMMAP_EXECUTABLE

    wait_for_magic_word()

    redraw_subwindows()

    uartrun_execute_packet(entry_address)
    dont_update_output_window = False

def command_save_logs(filepath):
    with open(filepath, 'w') as file:
        file.truncate(0)
        file.write('\n'.join(uart_output_log))

def command_file_upload(source_file, destionation_path):
    global dont_update_output_window

    if len(destionation_path) > 255:
        raise Exception("AWG File path must be < 255 characters")

    path_bytes = destionation_path.encode("utf-8") + b'\x00'
    size = os.path.getsize(source_file)
    dont_update_output_window = True
    
    window_width = terminal_width // 3 * 2
    progress_bar_window = create_new_popup_window(10, window_width)
    progress_bar_window.border("|", "|", "=", "=", "+", "+", "+", "+")
    horizontaly_center_text(progress_bar_window, 0, " Uploading application ")
    progress_bar_window.addnstr(1, 2, f"Uploading {os.path.basename(source_file)} ({(size // 1024)} KiB)", window_width - 4)
    progress_bar_window.addnstr(2, 2, "Waiting on ready signal", window_width - 4)
    progress_bar_window.refresh()

    progress_bar = create_centered_window(3, window_width - 4, progress_bar_window, 2)
    progress_bar.border()
    progress_bar.refresh()

    wait_for_magic_word()

    header = b'\x01' + size.to_bytes(8, 'little')
    uart_send_wrapper(header)

    progress_bar_window.addnstr(2, 2, "Sending name           ", window_width - 4)
    progress_bar_window.border("|", "|", "=", "=", "+", "+", "+", "+")
    progress_bar_window.refresh()

    wait_for_magic_word()

    uart_stop_tracking_input()
    uart_send_wrapper(path_bytes)
    uart_restart_tracking_input()

    progress_bar_window.addnstr(2, 2, "Uploading file", window_width - 4)
    progress_bar_window.border("|", "|", "=", "=", "+", "+", "+", "+")
    progress_bar_window.refresh()

    wait_for_magic_word()

    max_bar_size = window_width - 6
    upload_file(source_file, size, progress_bar, max_bar_size, 32 * 1024)

    dont_update_output_window = False
    redraw_subwindows()

def command_file_move(old_path, new_path):
    global dont_update_output_window

    dont_update_output_window = True

    if len(old_path) > 255 or len(new_path) > 255:
        raise Exception("AWG File path must be < 255 characters")

    old_path_bytes = old_path.encode("utf-8") + b'\x00'
    new_path_bytes = new_path.encode("utf-8") + b'\x00'
    
    window = create_new_popup_window(10, 40)
    window.border("|", "|", "=", "=", "+", "+", "+", "+")
    horizontaly_center_text(window, 0, f"Renaming {os.path.basename(old_path)} to {os.path.basename(new_path)}")
    window.addstr(2, 2, "Waiting on ready signal")
    window.refresh()

    uart_send_wrapper(b'\x02')  # The header for this one is just the number 2

    window.addstr(2, 2, "Sending old_path       ")
    window.refresh()

    wait_for_magic_word()

    uart_send_wrapper(old_path_bytes)

    window.addstr(2, 2, "Sending new_path       ")
    window.refresh()

    wait_for_magic_word()

    uart_send_wrapper(new_path_bytes)

    dont_update_output_window = False
    redraw_subwindows()

def singal_path_packet(path, packet_id, window_name):
    global dont_update_output_window

    dont_update_output_window = True

    if len(path) > 255:
        raise Exception("AWG File path must be < 255 characters")

    file_path_bytes = path.encode("utf-8") + b'\x00'
    
    window = create_new_popup_window(10, 40)
    window.border("|", "|", "=", "=", "+", "+", "+", "+")
    horizontaly_center_text(window, 0, window_name)
    window.addstr(2, 2, "Waiting on ready signal")
    window.refresh()

    uart_send_wrapper(packet_id)  # The header for this one is just the number 3

    window.addstr(2, 2, "Sending file_path      ")
    window.refresh()

    wait_for_magic_word()

    uart_stop_tracking_input()
    uart_send_wrapper(file_path_bytes)
    uart_restart_tracking_input()

    dont_update_output_window = False
    redraw_subwindows()

def command_file_remove(file_path):
    singal_path_packet(file_path, b'\x03', f"Removing {os.path.basename(file_path)}")
    
def command_list_directory(path):
    if path[-1] != '/' and path[-1] != '\\':
        path = path + "/"

    singal_path_packet(path, b'\x04', f"List directory")

def handle_console_command():
    global dont_update_output_window
    global console_error_text

    console_error_text = console_error_text.lower()
    command_split = console_text_input.split(' ', 1)
    command = command_split[0][1:].lower()
    curses.curs_set(0)
    arguments = [""]
    
    console_window.clear()
    console_window.addstr(0, 1, "^C", curses.A_REVERSE)
    console_window.addstr(0, 3, " to abort command")
    console_window.refresh()

    if len(command_split) > 1:
        arguments = command_split[1].split()

    try:
        if command in ["q", "quit"]:
            exit_wrapper("Command", 0)
        elif command == "clear":
            command_clear()
        elif command == "key":
            command_key(arguments)
        elif command in ["kreload", "kernal_reload"]:
            command_kenral_reload(arguments[0])
        elif command in ["rua", "run_user_app"]:
            command_run_user_app(arguments[0])
        elif command in ["save", "save_logs"]:
            command_save_logs(arguments[0])
        elif command in ["fupld", "file_upload"]:
            command_file_upload(arguments[0], arguments[1])
        elif command in ["fmove", "file_move"]:
            command_file_move(arguments[0], arguments[1])
        elif command in ["fremove", "file_remove"]:
            command_file_remove(arguments[0])
        elif command in ["ls", "list_dir", "list_list_directory"]:
            command_list_directory(arguments[0])
        elif command in ["h", "help"]:
            show_help_window()
        else:
            console_error_text = f"Unkown command \"{command}\""
            return
    except KeyboardInterrupt:   # So you can exit commands with ^C
        uart_restart_tracking_input()
        dont_update_output_window = False
        redraw_subwindows()
    except Exception as e:
        console_error_text = str(e)

    if dont_save_edit_command_history:
        return

    if command in ["q", "quit"]:        # Dont include closing the app in the command history
        return

    command_history = config["command_history"]
    
    # command_history

    if console_text_input[1:] in command_history:                   # If there allready is a occurrence
        index = command_history.index(console_text_input[1:])       # Just move it to the fornt
        command_history.pop(index)                                  # Remove the occurce from histroy

    command_history.insert(0, console_text_input[1:])               # Add the current entry to the history
    
    if len(command_history) > config["command_history_max_size"]:   # remove entrys over size limit
        command_history.pop()

    config["command_history"] = command_history

def send_key_presses_from_string(string):
    error = send_key_press_uart_message_from_string(string, b'\x01')

    return error

def send_key_press_uart_message_from_string(string: str, state_byte):
    keys = string.replace(' ', '').lower().split(',')

    uart_message = bytearray()
    uart_message += len(keys).to_bytes(1, 'little')

    for key in keys:
        button = get_button_code_from_string(key)

        if button == 0:
            return f"Unkown button \"{key}\""
        
        uart_message += button.to_bytes(1, 'little')
        uart_message += state_byte

    uart_send_wrapper(uart_message)
    
    return ""
    
def get_button_code_from_string(button):
    if button == '7':
        return 0x37
    elif button == '8':
        return 0x38
    elif button == '9':
        return 0x39
    elif button == 'a':
        return 0x41
    elif button == '4':
        return 0x34
    elif button == '5':
        return 0x35
    elif button == '6':
        return 0x36
    elif button == 'b':
        return 0x42
    elif button == '1':
        return 0x31
    elif button == '2':
        return 0x32
    elif button == '3':
        return 0x33
    elif button == 'c':
        return 0x43
    elif button == '0':
        return 0x30
    elif button == 'd':
        return 0x44
    elif button == '+' or button == '-':
        return 0x2B
    elif button == ".":
        return 0x2E
    elif button == "del" or button == "delete":
        return 0x7F
    elif button == "clr" or button == "clear":
        return 0x7E
    elif button == "ent" or button == "enter":
        return 0x7D
    elif button == "prg_exit":
        return 0x7C
    elif button == "ch1":
        return 0x78
    elif button == "ch2":
        return 0x79
    elif button == "ch3":
        return 0x7A
    elif button == "ch4":
        return 0x7B
    elif button == "buf1": 
        return 0x74
    elif button == "buf2": 
        return 0x75
    elif button == "buf3": 
        return 0x76
    elif button == "buf4": 
        return 0x77
    
    return 0

def show_excption_window(e: Exception, height_offest = 0, width_offset = 0):
    console_window.clear()
    console_window.addstr(0, 1, "q", curses.A_REVERSE)
    console_window.addstr(0, 2, " to exit")
    console_window.refresh()

    exception_name = type(e).__name__
    error_message = str(e)
    window_width = max(min(max(len(error_message), len(exception_name)) + 4, terminal_width // 3 * 2), 28)
    lines = textwrap.wrap(error_message, window_width - 4)
    window_height = len(lines) + 5

    window = create_new_popup_window(window_height, window_width, height_offest, width_offset)
    window.border("|", "|", "=", "=", "+", "+", "+", "+")
    horizontaly_center_text(window, 0, " An excpetion has occured ")
    horizontaly_center_text(window, 2, f"{exception_name}:")

    for i, line in enumerate(lines):
        window.addstr(3 + i, 2, line)

    window.refresh()



    while stdscr.getch() != ord('q'):
        x = 0                   # We do nothing in a loop since this is a wait loop
        
    exit(-1)

def load_config_or_create_new():
    global config
    
    try:
        if os.path.isfile(config_file_path):
            with open(config_file_path, 'r') as file:
                config = json.load(file)
            return
    
    except Exception as e:
        show_excption_window(e)

    config = {
        "command_history_max_size": 10,
        "command_history": []
    }

def save_config():

    if config == None:      # To hopefully stop it from deleting my config history again
        return

    with open(config_file_path, 'w') as file:
        file.truncate(0)

        json.dump(config, file, indent=4, sort_keys=True)

def exit_wrapper(source: str, code: int):
    curses.endwin()
    print(f"Exited form {source}")
    save_config()
    exit(code)

def main(_stdscr: curses.window):
    global dont_save_edit_command_history
    global console_text_input
    global config_file_path
    global stdscr
    stdscr = _stdscr

    curses.start_color()
    curses.use_default_colors()

    curses.init_pair(1, curses.COLOR_WHITE, curses.COLOR_RED)
    config_file_path = arguments.config_file

    curses.curs_set(0)
    stdscr.nodelay(1)
    create_windows()
    load_config_or_create_new()

    try:
        connection_argument = arguments.connection_type.split(':')
        connect_to(connection_argument[0], connection_argument[1:])
    except Exception as e:
        show_excption_window(e, 4)

    if arguments.exec != "":
        dont_save_edit_command_history = True
        console_text_input = ":" + arguments.exec
        handle_console_command()
        console_text_input = ""
        draw_console_window()
        dont_save_edit_command_history = False


    while running:
        input = stdscr.getch()

        if connection.available():
            handle_uart_input()

        if console_text_input == "":                # if th console is being used we dont try to scroll the output window
            uart_output_window_handle_input(input)

        
        if input == curses.KEY_RESIZE:
            create_windows()
        elif input == 8: # Controll + h, (the function didn't work)
            show_help_window()
        else:
            handle_console_input(input)


    connection.close()

def entry():
    global arguments
    argument_parser = argparse.ArgumentParser()
    argument_parser.add_argument("connection_type", 
        help="Sates the type of connection to be used [tcp/serial/stt/test]. For tcp use tcp:Address:Port, For serial use serial:Address:Baudrate, For stt use stt:Address. test turns off the conneciton",
        metavar="connection type", 
        type=str)
    argument_parser.add_argument("-c", "--config_file",
        help="Used to give the path to the config file.",
        default="./uart_interface_config.json",
        metavar="config_file",
        type=str)
    argument_parser.add_argument("-e", "--exec",
        help="Used to provide a command to auto run when the connection to the AWG is made",
        default="",
        metavar="execute",
        type=str)

    arguments = argument_parser.parse_args()

    os.environ.setdefault('ESCDELAY', '25')

    try:
        wrapper(main)
    except KeyboardInterrupt:
        exit_wrapper("Keyboard interupt", -1)

if __name__ == "__main__":
    entry()
