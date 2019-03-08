# Scanner Controller

This project is part of my bachelor’s thesis on developing an interface for a Large Area High Resolution Hall Scanner. The complete system consists of a LAUNCHXL-F28377S MCU that controls the interferometry positioning system of a scanning probe and analyses positioning data in real time. Furthermore, the controller is connected via USB to a custom-built GUI running on a host PC.

# Scope

Due to increasing demands the project soon exceeded the scope of a bachelor’s thesis. Hence the documentation is split in two parts. The first part is covered in "Development of a Serial Peripheral Interface for a Large Area High Resolution Hall Scanner" and covers the following parts:

* Core systems
* Direct Memory Access (DMA)
* Serial Peripheral Interface (SPI)
* Hardware drivers
* Programmable function generator

The Second Part which has yet to be released as part of my master’s program covers the following parts:

* USB Interface
* digital filtering
* Graphical User Interface (different repository)

# Requirements

In order to compile and upload the program the following software is required:

* Code Composer Studio v6.2 (IDE)
* TI v15.12.4 LTS Compiler
* control SUITE v3.4.5 (support library)

For more information please refer the documentation (thesis/Bachelorarbeit_Hiti_Mario.pdf).





