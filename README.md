# happy_screen

A small project to turn an [inkplate 6 or 10](https://inkplate.io/) into a digital photo frame. 

## Features

- Image changes once per day
- Load a random image from the SD card
- Synchronize time by using WIFI
- Very low power usage

## Requirements

- Files must be stored in root directory of the SD card
- Files must have a maximum size of 800x600 pixels for inkplate 6 or 1200x825 pixels for inkplate 10 
- Files must be named image000.jpg, image001.jpg, image002.jpg,...
- Specify settings in `config.h`. See comment in `happy_screen.ino`.
