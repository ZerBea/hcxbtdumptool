hcxbtdumptool
==============

Small tool to capture packets from Bluetooth devices.


Brief description
--------------

Stand-alone binaries - designed to run on Raspberry Pi's with installed Arch Linux.
It may work on other Linux systems (notebooks, desktops) and distributions, too.


Detailed description
--------------

| Tool           | Description                                                                                            |
| -------------- | ------------------------------------------------------------------------------------------------------ |
| hcxbtdumptool  | Tool to run several tests to determine if Bluetooth device vulnerable                                  |


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

* Operatingsystem: Arch Linux (strict), Kernel >= 4.19 (strict). It may work on other Linux systems (notebooks, desktops) and distributions, too (no support for other distributions, no support for other operating systems). Don't use Kernel 4.4 (rt2x00 driver regression)

* bluez installed

* Raspberry Pi A, B, A+, B+, Zero (WH). (Recommended: Zero (WH) or A+, because of a very low power consumption), but notebooks and desktops may work, too.

* GPIO hardware mod recommended (push button and LED).


Adapters
--------------

| VENDOR MODEL         | ID                                                                                            |
| -------------------- | --------------------------------------------------------------------------------------------- |
| CSR 4.0              | ID 0a12:0001 Cambridge Silicon Radio, Ltd Bluetooth Dongle (HCI mode)                         |
