
import asyncio
from mqtt_client import mqtt_start, fast_mqtt
from fastapi_mqtt import FastMQTT, MQTTConfig
import json
import mqtt_subscriptions


async def startup():
    # start mqtt client
    while True:
        try:
            await mqtt_start()
            break
        except Exception:
            print("Failed to start mqtt client")
            await asyncio.sleep(3)

    print("Succesfully started mqtt client")

async def publish_pressure(topic, value):
    fast_mqtt.publish(topic,json.dumps({"pressure":value}))
    print("new pressure published")

async def main():
    try:
        await startup()
        while True:
            value = await asyncio.to_thread(input, "pressure: ")
            await publish_pressure("aeolus/pressure/high",float(value))
            await asyncio.sleep(0.2)

    except (KeyboardInterrupt, asyncio.CancelledError):
        print("keyboard interrupt detected, shutting down")
        await fast_mqtt.mqtt_shutdown()


asyncio.run(main=main())