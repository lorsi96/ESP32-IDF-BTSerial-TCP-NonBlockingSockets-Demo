# Simple ESP32 Demo - Simultaneous BT and WiFi

## Overview
This Demo program showcases the following items:
- How to connect to a TCP Server using wifi and receive/send small data.
- How to allow Serial BT Devices to connect to the ESP32 and send commands.
- How both services, WiFi and Bluetooth, can coexist. 
- How to implement a Table-Based FSM in a small application.
- How to Dockerize and Build an ESP32 Application,

## Descritipon
The Application responds to different external events and changes the Built-In LED behavior in accordance.

### TCP Server
From the TCP server the user can request the ESP32:
- Know if  the LED is blinking and how fast it is doing so.
- Query if it is listening to BT events.
- Enable/Disable capturing BT events.

### Classic Serial Bt
From a Bluetooth device connected to the ESP32 the user can:
- Send a 0 to toggle slow blinking
- Send a 1 to toggle fast blinking
Note that this will not work if the user has disabled capturing BT events
from the server.

### Program State Diagram
- This charts shows which commands produce a change in the LED Blinking State
- Note that, as stated in above sections, other commands exist but are not listed here as they don't produce any state update

![diagram not found](StateDiagram.png "FSM")

## How To Build n' Run (Ubuntu)
To build the application and test it, you'll need two separate terminals:

In the first one you'll start a local TCP Server. 
Modify./components/tcp_client and ./server/server to use your IP address and desired port.

´´´bash
python3 ./server/server.py
´´´

In another terminal, build the app and flash it to the ESP32.

´´´bash
./docker/docker.build.sh # Builds the Docker image
# Connect your ESP32 to your workstation
./docker/docker.run.sh # Starts a Shell in the container
idf.py build flash monitor
´´´
