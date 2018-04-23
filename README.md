# XdkHeadTrack
Using the Bosch [XDK](http://xdk.io) Orientation-Sensor, a PC software and [OpenTrack](https://github.com/opentrack/opentrack) to stream head tracking data from the sensor board to the final application.

![App Screenshot](https://raw.githubusercontent.com/FreeGeronimo/xdkheadtrack/e8a70925ab15098b361935bb97fb0e85a72c8a07/assets/screenshot1.png)

## Builds
|Build|Status|
| :-: | :--: |
| *master* | [![Build status](https://ci.appveyor.com/api/projects/status/0s3i5e2m659m7kgj/branch/master?svg=true)](https://ci.appveyor.com/project/FreeGeronimo/xdkheadtrack/branch/master) |
| *current* | [![Build status](https://ci.appveyor.com/api/projects/status/0s3i5e2m659m7kgj?svg=true)](https://ci.appveyor.com/project/FreeGeronimo/xdkheadtrack) |

## Requirements
To use this project you'll need:
* An [XDK Cross Domain Development Kit](https://xdk.bosch-connectivity.com/home)
* A OpenTrack installation (or the portable version) running in UDP-receive mode

To build this project you'll need:
* The [XDK-Workbench](https://xdk.bosch-connectivity.com/software-downloads) to build the embedded firmware
* [Visual Studio](https://www.visualstudio.com) to build the client software

## Getting Started
1. Start by downloading the repository files and building them.
	* For the embedded software you'll need the [XDK-Workbench](https://xdk.bosch-connectivity.com/software-downloads). Import the project files into the Workbench and run the debug or release build target. Afterwards you can flash the firmware onto your XDK hardware via the "_Flash_" button in the XDK Device - View.
	* For the client software simply install the [Visual Studio](https://www.visualstudio.com) version of your liking and open the Solution-file. Run the Debug or Release build targets and execute the software.
2. Power on the XDK and let it boot for a second. The red LED should light up constantly when ready. To get the best results it's recommended to calibrate the orientation sensor algorithm by first letting the device sit still on a flat surface for a few seconds (calibrating the gyroscope) and then perform a figure of eight (calibrating the magnetometer).
3. If not already done, connect the XDK via USB to your host-PC and mount the device on your head (using a headset and some Velcro&trade; tape should work).
4. Start-up the PC client software, identify your XDK in the COM-port list and hit the connect-switch. You may check out the Windows Device Manager to find the exact COM-port your XDK is available under. After connecting the serial-port the software should start receiving orientation data and the 3D head model should start moving respective to the XDKs' orientation.
5. Start your OpenTrack software, select "_UDP over network_" as input and hit the start tracking button.
6. In the PC software enter the IP of the host OpenTrack is running on (usually _localhost_/_127.0.0.1_) and hit the connect-switch. You should now see the OpenTrack preview react to the XDKs' movement.
7. Use _BUTTON1_ on the XDK to calibrate the sensor initially (so that the axis are correct). Afterwards and sometimes during use it may be necessary to compensate the sensor drift by re-calibrate, however this small drift can be compensated by using the OpenTrack center feature (bind the key in OpenTrack first).