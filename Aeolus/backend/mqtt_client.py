
from contextlib import asynccontextmanager
from typing import Any

from fastapi import FastAPI
from gmqtt import Client as MQTTClient

from fastapi_mqtt import FastMQTT, MQTT_Config


mqtt_config = MQTT_Config(
    host = "mqtt.mosquito.org",
    port = 1883
)

fast_mqtt = FastMQTT(config=mqtt_config)

# Subscriptions

