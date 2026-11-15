import asyncio
from bleak import BleakClient

DEVICE = "A0:F2:62:73:AE:E2"

RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"


def notification_handler(sender, data):
    #print(f"\nESP32: {data.decode(errors='replace')}", end="\n> ", flush=True)
    print(f"{data.decode(errors='replace')}", end="", flush=True)


async def main():
    async with BleakClient(DEVICE) as client:
        print(f"Connected: {client.is_connected}")

        await client.start_notify(TX_UUID, notification_handler)

        print("BLE UART ready. Type 'exit' to disconnect.")

        while True:
            message = await asyncio.to_thread(input, "> ")

            if message.lower() == "exit":
                break

            if message:
                await client.write_gatt_char(
                    RX_UUID,
                    message.encode(),
                    response=False
                )


asyncio.run(main())
