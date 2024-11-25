# Lighthouse

[![License: GPL v3](https://img.shields.io/badge/license-GPL%20v3-brightgreen.svg?style=flat-square)](http://www.gnu.org/licenses/gpl-3.0)

Lighthouse is small application that can controls Bluetooth Low Energy smart lights, as well as your computer with the help of Bluetooth Low Energy remotes (or with the mobile companion app).  

> At the moment, it is not intended to be used by anyone who doesn't know what they're doing.  

> Application developed by [Emeric Grange](https://emeric.io/).  


## Supported devices

Remotes and beacons:
* Yeelight S1 button (YLAI003)
* Yeelight Remote Control (YLYK01YL)
* Yeelight Wireless Smart Dimmer (YLKG07YL and YLKG08YL)

The YLKG07YL and YLKG08YL dimmers will NEED a beacon key. You can use the [get_beacon_key.py](https://github.com/custom-components/ble_monitor/blob/master/custom_components/ble_monitor/ble_parser/get_beacon_key.py) python script to extract yours, then set it directly in the app.

Nintendo devices:
* Pokeball Plus (PBP)
* Pokemon Go Plus (PGP)

Lamps and lighbulbs:
* MiPow SmartBulbs

> Lighthouse is NOT associated with Xiaomi, Yeelight, MiPow, Nintendo, or any other device manufacturer.


## Screenshots

![GUI_DESKTOP1](https://i.imgur.com/AApGIrE.png)
![GUI_DESKTOP2](https://i.imgur.com/laODc61.png)
![GUI_DESKTOP3](https://i.imgur.com/MLIASy5.png)
![GUI_DESKTOP4](https://i.imgur.com/y77zhNY.png)
![GUI_DESKTOP5](https://i.imgur.com/u6XLtwO.png)
![GUI_MOBILE1](https://i.imgur.com/WBTuixL.png)


## Documentation

#### Dependencies

You will need a C++17 compiler and Qt 6.7+ with the following 'additional librairies':  
- Qt Connectivity
- Qt Multimedia
- Qt Quick 3D (for desktop builds)

You will also need the `mbedtls` librairie installed in order to handle YLKG07YL and YLKG08YL devices.  
On linux you'll need an `uinput` enabled kernel.  

For macOS and iOS builds, you'll need Xcode (15+) installed.  
For windows builds, you'll need MSVC 2019 (or 2022) installed. Bluetooth won't work with MinGW.  
For Android builds, you'll need the appropriates JDK (17) SDK (23+) and NDK (26b+). You can customize Android build environment using the `assets/android/gradle.properties` file.  

#### Building Lighthouse

```bash
$ git clone https://github.com/emericg/Lighthouse.git
$ cd Lighthouse/
$ cmake -B build/
$ cmake --build build/
```

#### Using Lighthouse

##### Android

The Android operating system requires applications to ask for device location permission in order to scan for nearby Bluetooth Low Energy sensors. Android 10+ will ask for _ACCESS_FINE_LOCATION_, while Android 6+ will only ask for _ACCESS_COARSE_LOCATION_.  
This is a mandatory requirement. The application is neither using nor storing your location. Sorry for the inconvenience.  

You can learn more on Android developer [website](https://developer.android.com/guide/topics/connectivity/bluetooth/permissions#declare-android11-or-lower).  

Some devices also require the GPS to be turned on while scanning for new sensors. This requirement is not documented and there is not much that can be done about it.  

##### iOS

The application will ask you for permission to use Bluetooth. You can learn more on Apple [website](https://support.apple.com/HT210578).

##### macOS

Starting with macOS 11, the application will ask you for permission to use Bluetooth. You can learn more on Apple [developer website](https://developer.apple.com/documentation/bundleresources/information_property_list/nsbluetoothalwaysusagedescription).

#### Third party projects used by Lighthouse

* [Qt6](https://www.qt.io) ([LGPL v3](https://www.gnu.org/licenses/lgpl-3.0.txt))
* [MobileUI](src/thirdparty/MobileUI/README.md) ([MIT](https://opensource.org/licenses/MIT))
* [SingleApplication](https://github.com/itay-grudev/SingleApplication) ([MIT](https://opensource.org/licenses/MIT))
* RC4 code from Christophe Devine ([GPL v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt))
* Graphical resources: [assets/COPYING](assets/COPYING)


## Get involved!

#### Developers

You can browse the code on the GitHub page, submit patches and pull requests! Your help would be greatly appreciated ;-)

#### Users

You can help us find and report bugs, suggest new features, help with translation, documentation and more! Visit the Issues section of the GitHub page to start!


## License

Lighthouse is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.  
Read the [LICENSE](LICENSE.md) file or [consult the license on the FSF website](https://www.gnu.org/licenses/gpl-3.0.txt) directly.

> Emeric Grange <emeric.grange@gmail.com>
