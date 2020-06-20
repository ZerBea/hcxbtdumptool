hcxbtdumptool
==============

Small tool to capture packets from Bluetooth devices.

Initial commit - no Bluetooth functions - only help and version menue and Bluetooth device information available.

This is a playground to get some knowledge about coding Bluetooth.

Everything is high experimental.


Brief description
--------------

Stand-alone binaries - designed to run on Raspberry Pi's with installed Arch Linux.
It may work on other Linux systems (notebooks, desktops) and distributions, too.


Detailed description
--------------

| Tool           | Description                                                                                            |
| -------------- | ------------------------------------------------------------------------------------------------------ |
| hcxbtdumptool  | Tool to dump Bluetooth packets                                                                         |


Get source
--------------
```
git clone https://github.com/ZerBea/hcxbtdumptool.git
cd hcxbtdumptool
```


Compile
--------------
```
make
make install (as super user)
```


Requirements
--------------

* Operatingsystem: Arch Linux (strict), Kernel >= 4.19 (strict). It may work on other Linux systems (notebooks, desktops) and distributions, too (no support for other distributions, no support for other operating systems)

* bluez installed

* Raspberry Pi A, B, A+, B+, Zero (WH). (Recommended: Zero (WH) or A+, because of a very low power consumption), but notebooks and desktops may work, too.

* GPIO hardware mod recommended (push button and LED).


Adapters
--------------

| VENDOR MODEL         | ID                                                                                            |
| -------------------- | --------------------------------------------------------------------------------------------- |
| CSR 4.0              | ID 0a12:0001 Cambridge Silicon Radio, Ltd Bluetooth Dongle (HCI mode)                         |
