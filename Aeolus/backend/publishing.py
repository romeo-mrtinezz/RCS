
from fastapi import FastAPI
from gmqtt import Client as MQTTClient

from fastapi_mqtt import FastMQTT, MQTTConfig
import time
mqtt_config = MQTTConfig()
fast_mqtt = FastMQTT(config=mqtt_config)

# read serial port, extract low side and high side pressure

# focus on just publishing for now
dummy_pressure = 2 # bar
# publish to mqtt broker
fast_mqtt.publish("pressure/low","{pressure: dummy_pressure}")

