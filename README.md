# Robot Football Club Old Tackle Sensor Firmware

This repository contains code for the tackle sensors used by the College
Robot football league prior to 2024.

## Compilation

To compile you need to download and install the XC8 compiler (known working version 2.36)
and the the MPLABX IDE (known working version 6.00),
if you don't have them installed already.

You can install the XC8 compiler from here: <https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers/xc8>

You can install the MPLABX ide from here: <https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide#tabs>

You can eaither clone this repository, or download the zip of
this repository from GitHub to obtain the code.
Add a ".X" on the end of the name of the folder that `main.c` is in (the root of
this repoistory). This allows it to be imported into MPLABX as a project.
Next, open MPLABX and select "File"->"Open Project..." and select the folder you
just renamed (the root of this repository). To compile the code in MPLAB select
the "Projects" pane, right clik on the name for this project and select the
"Set as Main Project" option. Now you can click on the hammer icon in the 
top bar to build the project.

## Programing

### Hardware Required

* An old Tackle Sensor
* PicKit 3 or PicKit 4 (ICD debbugers will also work but are more expensive)
* 4 Male to Female Jumpper Wires
* 1 Male to Male Jumpper Wire
* Power source for the Old Tackle Sensor

### Powering the Tackle Sensors

The Tackle Sensor Board will need to be powered to program it. As each team
has modified and powered these boards in different ways, there no one good
way that I could put here that would work for most people. My advice would be
to leave the tackle sensor installed in the robots while your reprogram them.
Use the robots power systme to provide the voltage for reprograming.

### Conecting Debugger

#### Rev 2

For the Rev 2 board, connect the tackle sensor to the PicKit 3 as shown in the
pictures below. Make sure to connect the power wire from the debugger
(brown jumpper in picture) to one of the vias on top of the voltage regulator
so the PicKit can detect the board voltage.

![Overhead view of PicKit3 connected to Tackle Sensor](photos/rev2_debbugger_overhead.jpeg)

![Closeup Side view of PicKit3 connected to Tackle Sensor](photos/rev2_debugger_closeup_side.jpeg)

TODO: Add detailed debugger connection information for each of the board revs.

### Flashing the Board

Once the board is powered and the debugger is connected to the board and computer,
you can press the "Make and Program Device Main Project" button in the top bar
of MPLAB X IDE. If the LEDs light up the new idle color and turn red when you shake
the board, then programming was successful.

## Color Modes

This firmware has two possible idle colors (white and green) that you can change
between. To change the idle color, you connect the UART port on the tackel sensor
to a USB port on a computer. You send a message over this port to change the idle
color (exact message to send TBD, right now it's sending a 'G' for green and a
'W' for white).

You can eaither use a COM to UART program (like
[RealTerm](https://sourceforge.net/projects/realterm/)) to send the message yourself,
or use this webapp (TBD on the exact recomend method, we might want to add this
in to the configuration program for the new Tackle Sensors or keep this seperate).

### Rev 2 UART Connection

On the Rev 2 Boards, the UART is connected to the USB Type B port on the board.
To connect this to a computer you can use a Type B to Type A cable or a Type B to
Type C cable.

TODO: Add this info for other Revisions of the Board.

## Current State of Repository

Currently this firmware is a working demo for the old tackle sensors. All the
hardware works and has code written for it. Currently it is set to register
a tackle at 1G of acceleration with just the built in filter of the accelerometer.
This behavior will need to be tunned and adjusted to match the desiered tackle
behavior of the new tackle sensors. This code also needs general polish
and organization.
