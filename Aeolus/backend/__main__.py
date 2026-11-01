import asyncio
import json
from mqtt_client import fast_mqtt, mqtt_start, mqtt_publish
from mqtt_subscriptions import HIGH_PRESSURE_TOPIC
from loguru import logger
from serial_reader import read_port

RETRY_DELAY_SECONDS = 2.0
LOOP_SYNC_DELAY_SECONDS = 0.2


async def startup() -> None:
    """Initialize the MQTT client connection with automatic retries."""
    while True:
        try:
            await mqtt_start()
            break
        except Exception as err:
            logger.error(f"[WARNING] Failed to start MQTT client ({err}), retrying in {RETRY_DELAY_SECONDS}s...")
            await asyncio.sleep(RETRY_DELAY_SECONDS)

    logger.success("Succesfully started Aeolus backend as client")

async def publish_pressure(topic: str, value: float) -> None:
    """Publish a validated float pressure reading to the specified MQTT topic."""
    payload = json.dumps({"pressure": value})
    fast_mqtt.publish(topic, payload)
    logger.info(f"[PUBLISHER] Published new pressure: {value}")

async def main() -> None:
    """Main execution loop for reading user input and publishing telemetry."""
    await startup()

    try:
        while True:
            try:
                async for line in read_port():
                    for topic, payload in line.items():
                        # if (topic == "aeolus/pitch"):
                        #     print(f"{payload} published to {topic}\n")
                        await mqtt_publish(topic, payload)
                

            except ValueError:
                logger.error("Failed to publish message")
                continue

    except (KeyboardInterrupt, asyncio.CancelledError):
        logger.info("Keyboard interrupt detected",)
    finally:
        # Putting shutdown in a 'finally' block guarantees the client disconnects cleanly
        await fast_mqtt.mqtt_shutdown()
        logger.warning("MQTT client disconnected cleanly")

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.warning("Terminating program...")
