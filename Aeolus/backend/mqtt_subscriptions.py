import asyncio
import json
from mqtt_client import fast_mqtt, mqtt_start


@fast_mqtt.subscribe("aeolus/pressure/high")
async def high_pressure_callback(client, topic, payload, qos, properties):
    raw_string = payload.decode("utf-8")
    data = json.loads(raw_string)
    pressure = data.get("pressure")
    print(f"[Received on {topic}] Pressure is: {pressure}")
