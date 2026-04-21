import asyncio
import sys
from hm10_bleak import HM10BleakClient

async def receive_task(hm10):
    """Background task that constantly checks the buffer and prints."""
    try:
        while True:
            incoming = hm10.listen()
            if incoming:
                print(f"\r[HM10]: {incoming}")
                print("You: ", end="", flush=True)
            # Short sleep to allow other tasks (like send_task) to run
            await asyncio.sleep(0.1)
    except asyncio.CancelledError:
        # This is expected when we shut down
        pass

async def send_task(hm10):
    """Task that waits for user input and sends it."""
    loop = asyncio.get_running_loop()
    print("--- Chat Started. Type 'exit' to quit or press Ctrl+D/Ctrl+C. ---")
    
    while True:
        print("You: ", end="", flush=True)
        # sys.stdin.readline blocks this thread until Enter is pressed
        user_msg = await loop.run_in_executor(None, sys.stdin.readline)
        
        # Handle EOF (Ctrl+D on Linux/Mac, Ctrl+Z on Windows)
        if not user_msg:
            print("\nEOF detected. Exiting...")
            return
        
        user_msg = user_msg.strip()

        if user_msg.lower() in ['exit', 'quit']:
            print("Exiting...")
            return  # Returning here triggers the cleanup logic in main()
        
        if user_msg:
            await hm10.send(user_msg + "\n")

async def main():
    hm10 = HM10BleakClient(target_name="HM10_Mega")
    
    if not await hm10.connect():
        return

    # Create the tasks
    rt = asyncio.create_task(receive_task(hm10))
    st = asyncio.create_task(send_task(hm10))

    try:
        # Wait for the SEND task to finish (happens when you type 'exit')
        await st 
    finally:
        # When send_task is done, cancel the receiver and disconnect
        rt.cancel()
        await asyncio.gather(rt, return_exceptions=True)
        await hm10.disconnect()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        # Handle Ctrl+C gracefully
        print("\nKeyboard interrupt detected. Exiting...")
        pass