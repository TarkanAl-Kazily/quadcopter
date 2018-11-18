# Building Better Bots
Developing and Building an STM32 based quadcopter.

## Development Toolchain Installation

### Windows
#### 1) Install STM32CubeMX
This software allows configurable generation STM32 projects with the option of generating a Makefile for creating a loadable binary from the project.
#### 2) Install STM Flasher Demo
Simple GUI software for flashing binaries to the memory on the STM Board.
#### 3) Install GNU Make for Windows
Build software for C/C++
#### 4) Install ARM GNU Toolchain for Windows
C/C++ compiler/assembler/etc for ARM ISA
#### Familiarize yourself with Windows powershell (recommended) or the command prompt
This will be used to build the CubeMX projects
#### Add necessary paths to environment path variable
Circumvents annoying windows/unix(esque) file path differences

### MacOS
#### Ask Tarkan

### Linux
#### Use your package manager

## Black Magic Probe
1. Get the Black Magic Probe source code from GitHub
2. Modify the target/cortexm.c file to remove unneeded targets
3. Compile and install the dfu using the serial adapter
4. Install the firmware using USB

## Pin connections
SWD Pins:  
__TARGET__ __DEBUGGER__  
GND        GND  
SWDIO      PB14  
SWCLK      PA5  
3.3V       3.3V (OPTIONAL)  

Serial Port Pins:  
__TARGET__ __DEBUGGER__  
RX         PA2  
TX         PA3  

## Using GDB
1. Run gdb
2. `target extended-remote *serial port*`
3. `monitor swdp_scan`
4. `attach 1`
5. `file build/*program.elf*`
6. `load`
7. `run`

These steps can be automated - see gdbinit\_example for more information.

### Links
[STM32CubeMX (All OS)](https://www.st.com/en/development-tools/stm32cubemx.html#getsoftware-scroll)

[STM Flasher Demo (Windows)](https://www.st.com/en/development-tools/flasher-stm32.html#getsoftware-scroll)

[GNU Make for Windows](http://gnuwin32.sourceforge.net/packages/make.htm)

[GNU ARM Embedded Toolchain (All OS)](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)

[BMP Blue Pill Setup (Has some bugs!)](https://medium.com/@paramaggarwal/converting-an-stm32f103-board-to-a-black-magic-probe-c013cf2cc38c)

### Glossary
#### STM Blue Pill
![Blue Pill](https://jeelabs.org/img/2016/DSC_5474.jpg)

### ESC - Electronic Speed Controller

[Hobbypower 30A brushless ESC](https://artofcircuits.com/product/hobbypower-30a-brushless-esc)
(has links to information relevant to our ESC's)

