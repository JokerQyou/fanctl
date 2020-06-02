# fanctl

A simple thermal fan controller for Raspbian.

# Hardware

## Required parts

- 5V DC fan x1, 3mm / 4mm or any other size suitable for your Raspberry Pi.
- 2N2222 NPN transistor x1.
- 680ohm resistor x1.
- (Optional) small piece of PCB. If you don't have one, it is also possible to solder all parts directly to the fan wires..

## The circuit

![the circuit](https://i.imgur.com/jw3qqDq.png)

Or you could reference to [this post](https://howchoo.com/g/ote2mjkzzta/control-raspberry-pi-fan-temperature-python#a-explanation-of-the-circuit) for connecting your parts together.

Choose any GND / VCC (5V) pin to supply power. For the controller pin, use [GPIO 8 (AKA BCM 14)](https://pinout.xyz/pinout/pin8_gpio14).

# Software

## Install dependencies

- First of all update apt cache: `sudo apt update`.
- Install `cmake`, `gcc` and `git`, as you would do for all other C projects: `sudo apt install cmake build-essential gcc git`.
- Install the [MRAA lilbrary](https://github.com/eclipse/mraa) for accessing GPIO:
  - Clone MRAA project: `git clone https://github.com/eclipse/mraa.git`
  - Enter the cloned repository: `cd mraa`
  - Prepare to build: `mkdir build && cd build`
  - Generate makefiles: `cmake ..`
  - Build: `make`
  - Install the built tools: `sudo make install`
  - Refresh linker related stuff so MRAA library can be located by gcc: `sudo ldconfig -v`
  - To verify you have correctly installed the MRAA library, run `mraa-gpio version`, it should print the model of your Raspberry Pi

## Build this project

- Clone this project: `git clone https://github.com/JokerQyou/fanctl.git`
- Enter the cloned repository: `cd fanctl`
- Prepare to build: `mkdir build && cd build`
- Generate makefiles: `cmake ..`
- Build: `make`
- Install: `sudo make install`

## Start service

- Reload systemd units: `sudo systemctl daemon-reload`
- To verify you have correctly installed the service, run `sudo systemctl status fanctl`, it should print some information about this tool
- To start the service: `sudo systemctl start fanctl`

When the chip temperature reaches 60°C, the fan will be started by pulling `FAN_VCC_PIN` to high (causing the transistor to close). When the chip temperature drops to 45°C or below, the fan will be stopped by pulling `FAN_VCC_PIN` to low. When the fan is started or stopped, the corresponding temperature value will be written to journald logs, to follow the log: `sudo journalctl -u fanctl -f`.

## Configure

There is currently no configuration available.

To set the temperature thresholds, modify `THRESHOLD_ON` and `THRESHOLD_OFF` in `fanctl.c`. To use a different pin for controlling the fan, modify `FAN_VCC_PIN` (Notice: MRAA library uses the **physical pin number**). Then rebuild and reinstall again. Service will need to be restarted.

## Why C

Well, I've actually written a Python script and installed that as a service, it kept working for almost 6 months. But I don't think such a simple job is worth using 15MB of memory. The memory footprint dropped under 400KB after transforming to C.
