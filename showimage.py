import serial
import subprocess
import os
import time


try:
    ser = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
    print("Successfully connected to Arduino serial port /dev/ttyACM0") 
except serial.SerialException: 
    print("can not connect Arduino，please check USB connection！")
    ser = None 

image_paths = {
    "1": "/home/yingqi/Desktop/state1.jpg",  
    "2": "/home/yingqi/Desktop/state2.png",  
    "3": "/home/yingqi/Desktop/state3.png"   
}

def show_image(image_path):
    subprocess.run(["pkill", "-f", "feh"])  
    time.sleep(0.1)  
    subprocess.Popen(["feh", "--fullscreen", "--hide-pointer", image_path]) 


while True:
    if ser and ser.in_waiting > 0: 
        command = ser.readline().decode("utf-8").strip()
        if command.startswith("STATE"):
            state = command.split(" ")[1]
            if state in image_paths:
                print(f" change to state {state}，show image：{image_paths[state]}")
                show_image(image_paths[state])

