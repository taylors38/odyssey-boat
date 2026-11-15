import asyncio
from bleak import BleakClient

async def main():
    async with BleakClient("A0:F2:62:73:AE:E2") as client:
        for service in client.services:
            print(f"\nService: {service.uuid}")

            for char in service.characteristics:
                print(f"  Characteristic: {char.uuid}")
                print(f"  Properties: {char.properties}")

asyncio.run(main())
