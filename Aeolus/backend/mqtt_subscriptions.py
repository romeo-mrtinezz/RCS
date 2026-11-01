import json
from mqtt_client import fast_mqtt
from loguru import logger

# Define topic constants in one place to avoid typos
HIGH_PRESSURE_TOPIC = "aeolus/pressure/high"

@fast_mqtt.subscribe(HIGH_PRESSURE_TOPIC)
async def high_pressure_callback(client, topic: str, payload: bytes, qos: int, properties) -> None:
    """Callback triggered when a message is published to the high pressure topic."""
    try:
        raw_string = payload.decode("utf-8")
        data = json.loads(raw_string)
        pressure = data.get("pressure")

        if pressure is None:    
            logger.warning(f"Payload on '{topic}' missing 'pressure' key: {data}")
            return
        logger.info(f"[SUBSCRIBER | {topic}] Pressure received: {pressure}")

    except (UnicodeDecodeError, json.JSONDecodeError) as err:
        logger.error(f"Failed to decode JSON payload on '{topic}': {err}")
