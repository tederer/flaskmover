# flaskMover

This repository contains the code for an ESP32 microcontroller to periodically move a platform containing in vitro cultures.

## Preconditions

To build this project a development environment is necessary to compile the code. You can build a [docker image](https://github.com/tederer/esp32dev) containing the environment.

## Compiling the code

1. Build the docker images mentioned in the previous section.
2. Clone this repository.
3. Execute `startDevEnvInDocker.sh`, which starts the build environment and maps the project into it.
4. Execute `idf.py build` to build the project.

When compilation finished, the build-folder will contain:

| Artifact Type | Description |
| ----------- | ----------- |
| Arguments for the flasher |  build/flash_args |
| Application | build/flaskMover.bin |
| Bootloader | build/bootloader/bootloader.bin |
| Partion table | build/partition_table/partition-table.bin |

## Installing the artifacts on the ESP32

The process of installing the artifacts on the microcontroller is called flashing and you need to connect the ESP32 via a USB cable to your computer. There are multiply ways to flash the device. 

If you have access to the USB interface in the docker container, then you can use `idf.py flash`. Another way is to copy the artifacts to a Windows computer and use the [Flash Download Tool](https://docs.espressif.com/projects/esp-test-tools/en/latest/esp32/production_stage/tools/flash_download_tool.html). In this tool you need to list all three artifacts (partion table, bootloader and application) and their start address. You can find the addresses and other settings required by the tool in `build/flash_args`.
