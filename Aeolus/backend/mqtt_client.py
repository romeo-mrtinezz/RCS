
from contextlib import asynccontextmanager
from typing import Any

from fastapi import FastAPI
from gmqtt import Client as MQTTClient

from fastapi_mqtt import FastMQTT, MQTTConfig
import json
from loguru import logger

HOST = "localhost"
PORT = 1883
mqtt_config = MQTTConfig(
    host = HOST,
    port = PORT
)

fast_mqtt = FastMQTT(config=mqtt_config)

## I want to publish a value to the topic aeolus/pressure/high



async def mqtt_start():
    print("Attempting to start mqtt client")
    await fast_mqtt.mqtt_startup()

async def mqtt_publish(topic, payload):
    payload = json.dumps(payload)
    fast_mqtt.publish(topic, payload)
    logger.info("Published message to mqtt network")
# @fast_mqtt.on_connect()
# def connect(client: MQTTClient, flags: int, rc: int, properties: Any):
#     print("Connected: ", client, flags, rc, properties)
#     client.subscribe("pressure/high")  # subscribing to mqtt topic
#     client.subscribe("pressure/low")

# Publishing
# async def func():
#     fast_mqtt.publish("/pressure/low", "Hello from Fastapi")
#     return {"result": True, "message": "Published"}


# Subscribing
# @fast_mqtt.on_connect()
# def connect(client, flags, rc, properties):
#     client.subscribe("/pressure/low") # subscribing to mqtt topic
#     print("Connected: ", client, flags, rc, properties)

