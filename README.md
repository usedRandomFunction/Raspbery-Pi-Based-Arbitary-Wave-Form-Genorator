# A quick disscalamer

This project does mostly work. However i dont think its a good idea to recreate it.<br>
Just buy a real one. It will be faster and cheaper.<br>
This project was a good learning expience, but that is becouse i had to make it from scrach.<br>
I work on this thing every once in a while.


# Synopsis

This project can contain (up to) 4 R2R DACs. They can be 2,8,16 bit. Or switchable between the above.<br>
The DACs use two 74HC595's running slightly above their rated clock speed as IO expanders.<br>
The goal is to make a Arbitary-Wave-Form-Genorator (AWG) using a pi3b and some 74HC595's.<br>
It has an LCD (HDMI) and keypad. The device runs around 10 mega samples per second but<br>
I dont have the gear to give a precice timing.<br>
<br>
What started as "just use a pi pico" has turned into "use a pi3b as a mcu by programing it as bare metal"<br>
This truned into "virtual memory would be nice", "Appilcation loading would be nice",<br>
"Kernal use mode seporation would be nice". So its close ish to a simple operating system as well.<br>

## This repo - Software

Contains

- An ardunio script for a pi pico w to be used to controll the serial remotly [./SerialOverTCP]
- Example SD card contents (Exluding boat code, applications and specfic configs for DAC card) [./Disk]
- Incomplete documentation [./doc]
- The "os" kernal" [./kernal]
- User mode apps and libarys [./user]
- A python script to incerface with the AWG [./uart_interface.py]


## Hardware

TODO - upload


## Case parts

TODO - upload
