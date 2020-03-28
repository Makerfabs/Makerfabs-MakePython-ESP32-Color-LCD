import sys
import time
import machine
import st7789py as st7789
import uos
import random
import network
import urequests
import ujson
import romans
from machine import Pin
import dht   
          
fonts = [romans]
bl = machine.Pin(5, machine.Pin.OUT)
bl.value(1) 
spi = machine.SPI(1, baudrate=80000000, polarity=1, phase=0, sck=machine.Pin(14), mosi=machine.Pin(13), miso=machine.Pin(12))#HSPI

def pick_item(sequence):
    div = 0x3fffffff // len(sequence)
    return sequence[random.getrandbits(30) // div]
    
#Connection Network
wlan=0
def connectWiFi(ssid, passwd):
  global wlan
  wlan=network.WLAN(network.STA_IF)
  wlan.active(True)
  wlan.disconnect()
  wlan.connect(ssid, passwd)
  while(wlan.ifconfig()[0]=='0.0.0.0'):
    time.sleep(1)
  return True
  
#Get real time date  
def get_date():
  URL="http://quan.suning.com/getSysTime.do"
  res=urequests.get(URL).text
  j=ujson.loads(res)
  list=j['sysTime2'].split()[0]  
  #list1=j['sysTime2'].split()[1] 
  return list
 
display = st7789.ST7789(
  spi, 240, 240,
  reset=machine.Pin(21, machine.Pin.OUT),
  dc=machine.Pin(22, machine.Pin.OUT),
  cs=machine.Pin(15, machine.Pin.OUT),
  backlight=machine.Pin(5, machine.Pin.OUT),
  )
  
#Measure the temperature and humidity function  
sensor = dht.DHT11(Pin(18))
def read_sensor():
  global temp, temp_percentage, hum
  temp = temp_percentage = hum = 0
  try:
    sensor.measure()
    temp = sensor.temperature()  #get temperature
    hum = sensor.humidity()   #get humidity
    if (isinstance(temp, float) and isinstance(hum, float)) or (isinstance(temp, int) and isinstance(hum, int)):
      msg = ('{0:3.1f} , {1:3.1f}'.format(temp, hum))
      temp_percentage = (temp+6)/(40+6)*(100)
      hum = round(hum, 2)
      return(msg)
      
  except OSError as e:
    return('Failed to read')
    
display.init()
connectWiFi("Makerfabs", "20160704") #Change the network name and password

again = True
try:
    while again: 
      #Set a random color
      color = st7789.color565(
          random.getrandbits(8),
          random.getrandbits(8),
          random.getrandbits(8))
      #display
      display.text(pick_item(fonts), "Makerfabs", 32, 0, color)
      display.text(pick_item(fonts), str(get_date()), 64, 0, color)
      display.text(pick_item(fonts), "temp / hum", 96, 0, color)
      display.text(pick_item(fonts), str(read_sensor()), 128, 0, color)
      time.sleep(1)
      display.fill(st7789.BLACK)  #clear screen
      
except KeyboardInterrupt:
          pass



