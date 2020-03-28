import time
import machine
import st7789py as st7789
import uos
import random

import romans,italicc   #Import the font

fonts = [romans,italicc]  #Add fonts to the list

def pick_item(sequence):
    div = 0x3fffffff // len(sequence)
    return sequence[random.getrandbits(30) // div]

bl = machine.Pin(5, machine.Pin.OUT)
bl.value(1)

spi = machine.SPI(1, baudrate=80000000, polarity=1, phase=0, sck=machine.Pin(14), mosi=machine.Pin(13), miso=machine.Pin(12))#HSPI

display = st7789.ST7789(
        spi, 240, 240,
        reset=machine.Pin(21, machine.Pin.OUT),
        dc=machine.Pin(22, machine.Pin.OUT),
        cs=machine.Pin(15, machine.Pin.OUT),
        backlight=machine.Pin(5, machine.Pin.OUT),
)
display.init() #initialize
display.fill(st7789.BLACK)

row = 0
again = True
try:
    while again:
      #Set RGB565 random color
      color = st7789.color565(
          random.getrandbits(8),
          random.getrandbits(8),
          random.getrandbits(8))
          
      row += 32
      #Displays random color words
      display.text(pick_item(fonts), "Makepython!!!", row, 0, color)
      
      if row > 192:
        display.fill(st7789.BLACK) 
        row = 0
      
except KeyboardInterrupt:
          pass





