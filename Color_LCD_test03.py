#code for micropython 1.10 on esp32
#test ok on st7789 TFTLCD
#test ok on st7789 1.3inch TFTLCD using Makepython ESP32_Charlin
import random

import machine
import st7789py as st7789
import time
from machine import SPI,Pin

def test_main(filename):
  back=machine.Pin(5, machine.Pin.OUT)
  back.value(1)
  spi = SPI(1, baudrate=40000000, polarity=1, phase=0, sck=Pin(14), mosi=Pin(13), miso=Pin(12))#HSPI
  display = st7789.ST7789(
        spi, 240, 240,
        reset=machine.Pin(21, machine.Pin.OUT),
        dc=machine.Pin(22, machine.Pin.OUT),
        cs=machine.Pin(15, machine.Pin.OUT),
        backlight=machine.Pin(5, machine.Pin.OUT),
  )
  display.init()
  
  display.fill(
            st7789.color565(
                random.getrandbits(8),
                random.getrandbits(8),
                random.getrandbits(8),
            ),
  )
  # Pause 2 seconds.
  time.sleep(2)
  
  f=open(filename, 'rb')  #Read binary files
  if f.read(2) == b'BM':  #header
    dummy = f.read(8) #file size(4), creator bytes(4)
    offset = int.from_bytes(f.read(4), 'little')
    hdrsize = int.from_bytes(f.read(4), 'little')
    width = int.from_bytes(f.read(4), 'little')
    height = int.from_bytes(f.read(4), 'little')
    if int.from_bytes(f.read(2), 'little') == 1: #planes must be 1
        depth = int.from_bytes(f.read(2), 'little')
        if depth == 24 and int.from_bytes(f.read(4), 'little') == 0:#compress method == uncompressed
            print("Image size:", width, "x", height)
            rowsize = (width * 3 + 3) & ~3
            if height < 0:
                height = -height
                flip = False
            else:
                flip = True
            w, h = width, height
            if w > 240: w = 240
            if h > 240: h = 240
            #tft._setwindowloc((0,0),(w - 1,h - 1))
            display.set_window(0,0,(w - 1),(h - 1))
            for row in range(h):
                if flip:
                    pos = offset + (height - 1 - row) * rowsize
                else:
                    pos = offset + row * rowsize
                if f.tell() != pos:
                    dummy = f.seek(pos)
                for col in range(w):
                    bgr = f.read(3)

                    aColor=st7789.color565(bgr[2], bgr[1], bgr[0])
                    colorData = bytearray(2)
                    colorData[0] = aColor >> 8
                    colorData[1] = aColor
                    display.write(None, colorData)

try:
    test_main('logo01_240X240.bmp') #Display pictures
    time.sleep(5)
except KeyboardInterrupt:
          pass








