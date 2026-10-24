import asyncio
import json
from mqtt_client import fast_mqtt, mqtt_start, mqtt_publish
from mqtt_subscriptions import HIGH_PRESSURE_TOPIC
from loguru import logger
from serial_reader import read_port

class Colours:
# Source - https://stackoverflow.com/a/39452138
# Posted by qubodup, modified by community. See post 'Timeline' for change history
# Retrieved 2026-10-20, License - CC BY-SA 4.0
    CEND      = '\33[0m'
    CBOLD     = '\33[1m'
    CITALIC   = '\33[3m'
    CURL      = '\33[4m'
    CBLINK    = '\33[5m'
    CBLINK2   = '\33[6m'
    CSELECTED = '\33[7m'

    CBLACK  = '\33[30m'
    CRED    = '\33[31m'
    CGREEN  = '\33[32m'
    CYELLOW = '\33[33m'
    CBLUE   = '\33[34m'
    CVIOLET = '\33[35m'
    CBEIGE  = '\33[36m'
    CWHITE  = '\33[37m'

    CBLACKBG  = '\33[40m'
    CREDBG    = '\33[41m'
    CGREENBG  = '\33[42m'
    CYELLOWBG = '\33[43m'
    CBLUEBG   = '\33[44m'
    CVIOLETBG = '\33[45m'
    CBEIGEBG  = '\33[46m'
    CWHITEBG  = '\33[47m'

    CGREY    = '\33[90m'
    CRED2    = '\33[91m'
    CGREEN2  = '\33[92m'
    CYELLOW2 = '\33[93m'
    CBLUE2   = '\33[94m'
    CVIOLET2 = '\33[95m'
    CBEIGE2  = '\33[96m'
    CWHITE2  = '\33[97m'

    CGREYBG    = '\33[100m'
    CREDBG2    = '\33[101m'
    CGREENBG2  = '\33[102m'
    CYELLOWBG2 = '\33[103m'
    CBLUEBG2   = '\33[104m'
    CVIOLETBG2 = '\33[105m'
    CBEIGEBG2  = '\33[106m'
    CWHITEBG2  = '\33[107m'


RETRY_DELAY_SECONDS = 2.0
LOOP_SYNC_DELAY_SECONDS = 0.2


async def startup() -> None:
    """Initialize the MQTT client connection with automatic retries."""
    while True:
        try:
            await mqtt_start()
            break
        except Exception as err:
            print(f"\033[93m[WARNING] Failed to start MQTT client ({err}), retrying in {RETRY_DELAY_SECONDS}s...\033[0m")
            await asyncio.sleep(RETRY_DELAY_SECONDS)

    print("\033[92m[INFO] Successfully started MQTT client\033[0m")


async def publish_pressure(topic: str, value: float) -> None:
    """Publish a validated float pressure reading to the specified MQTT topic."""
    payload = json.dumps({"pressure": value})
    fast_mqtt.publish(topic, payload)
    logger.info(f"[PUBLISHER] Published new pressure: {value}")
    print(f"{Colours.CRED}[PUBLISHER] Published new pressure: {value} {Colours.CEND}")
    # print(f"\033[92m[PUBLISHER] Published new pressure: {value}\033[0m")


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
        print("\n\033[94m[INFO] Keyboard interrupt detected, shutting down...\033[0m")
    finally:
        # Putting shutdown in a 'finally' block guarantees the client disconnects cleanly
        await fast_mqtt.mqtt_shutdown()
        print("\033[92m[INFO] MQTT client disconnected cleanly.\033[0m")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n\033[94m[INFO] Program exited.\033[0m")
