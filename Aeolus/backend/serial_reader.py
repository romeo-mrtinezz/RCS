import serial
import asyncio
from mqtt_client import mqtt_publish

"""
Designed not to know anything about mqtt, just to read serial port
This way, minimal changes need to be made when the data being streamed changes
"""

COM_PORT = "COM5"
BAUD_RATE = 115200 # arbitrary as mcu appears as USB CDC VCP, data moving at usb speed

TOPICS = [ # same as Full_data struct in global.h
    ("aeolus/timestamp", "a"),
    ("aeolus/rate_x", "a"),
    ("aeolus/rate_y", "a"),
    ("aeolus/rate_z", "a"),
    ("aeolus/acc_x", "a"),
    ("aeolus/acc_y", "a"),
    ("aeolus/acc_z", "a"),
    ("aeolus/pitch_accel", "a"),
    ("aeolus/yaw_accel", "a"), # add pitch/yaw gyro too if needed
    ("aeolus/pitch", "a"),
    ("aeolus/yaw", "a"),
    
    ("aeolus/pitch_error", "a"),
    ("aeolus/yaw_error", "a"),
    ("aeolus/pitch_duty", "a"), 
    ("aeolus/yaw_duty", "a"), 
    ("aeolus/pressure/high", "pressure"),
    ("aeolus/pressure/low", "pressure")
]

async def read_port():
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    while True:
        raw_data = await asyncio.to_thread(ser.readline)
        line = raw_data.decode().strip()
        if not line:
            continue
        fields = line.split(",") # fields is a list of strings ["102", "1", "2"], gotta convert to float
        values = []
        for f in fields:
            values.append(float(f))
        yield {
            topic: {key: value}
            for (topic, key), value in zip(TOPICS, values) # each yield "{aeolus/pressure/high": {"pressure": 101.3}}
        }     

    # Read a line then return its contents?
    # Every line format is the same, value separated by a comma.
    
    ser.close