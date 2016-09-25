[![Build Status](https://travis-ci.org/kodi-pvr/pvr.vbox.svg?branch=master)](https://travis-ci.org/kodi-pvr/pvr.vbox)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/5120/badge.svg)](https://scan.coverity.com/projects/5120)

# VBox Home TV Gateway PVR Client

This repository provides a [Kodi] (http://kodi.tv) PVR addon for 
interfacing with the VBox Communications XTi TV Gateway devices. This 
README serves as a quick overview of the functionality and architecture 
of the addon, to make it easier for others to possible contribute.

## Build instructions

### Linux

1. `git clone https://github.com/xbmc/xbmc.git`
2. `git clone https://github.com/kodi-pvr/pvr.vbox.git`
3. `cd pvr.vbox && mkdir build && cd build`
4. `cmake -DADDONS_TO_BUILD=pvr.vbox -DADDON_SRC_PREFIX=../.. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=../../xbmc/addons -DPACKAGE_ZIP=1 ../../xbmc/project/cmake/addons`
5. `make`

#### Building for Kodi built with CMake

If you've built Kodi with CMake you'll want to change the 
`CMAKE_INSTALL_PREFIX` to `../../xbmc/kodi-build/addons` instead 
(assuming `kodi-build` is your build directory).

### Settings

This list contains some explanation for the non-obvious settings:
* `Connection`: There are two tabs in the settings dialog with identical settings, which means you can configure your addon to contact the VBox TV Gateway using both its internal and external address/port. This is useful for e.g. a laptop which is not permanently inside your internal network. When the addon starts it first attempts to make a connection using the internal settings. If that fails, it will try the external settings instead. The addon restarts itself if the connection is lost so it will automatically switch back without having to restart Kodi.
  * `HTTP and UPnP port`: You'll only need to change these from their respective defaults if you wish to use a VBox TV Gateway over the Internet (i.e. the external connection settings tab) and you're using asymmetric port forwarding (e.g. port 8080 -> 80 and 12345 -> 55555). The HTTP port is used to communicate with the device while the UPnP port is used when streaming media.
* `Prefer external EPG over OTA`: If a specific channel has guide data both from the VBox itself and from the external XMLTV file, this setting controls which guide data is used. Note that the external data is always used if the VBox doesn't provide any guide data for a specific channel.
* `Timeshift buffer path`: The path where the timeshift buffer files should be stored when timeshifting is enabled. Make sure you have a reasonable amount of disk space available since the buffer will grow indefinitely until you stop watching or switch channels!

### Using external XMLTV guide data

The addon supports specifying the path to an XMLTV file which will be used for guide data for channels that the VBox doesn't provide any EPG data for. You can also make the addon prefer the external guide data by enabling the `Prefer external EPG over OTA` checkbox in the settings (see above for possible caveats). For this feature to work, the addon needs a way to map channel names from the VBox with channel names in the XMLTV file.

When the addon starts, it looks for and loads channel name mappings from a file named `channel_mappings.xml` in the addon's data folder (`userdata\addon_data\pvr.vbox`). If this file doesn't exist it is created. The file has the following structure:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<xmltvmap>
    <mapping vbox-name="Yle TV1" xmltv-name="Yle TV1"/>
    <mapping vbox-name="Yle TV2" xmltv-name="Yle TV2"/>
    <mapping vbox-name="MTV3 HD" xmltv-name=""/>
    <mapping vbox-name="Nelonen HD" xmltv-name=""/>
    ...
</xmltvmap>
```

When creating the default map, only channels which names match perfectly will be mapped automatically. The rest of the channel mappings are empty, like MTV3 HD and Nelonen HD in the example above. To map these to the correct channels, simply modify the file and restart Kodi.

### Build instructions

The main instructions for how to build PVR addons using the new build system can be found [here](http://forum.kodi.tv/showthread.php?tid=219166). Here's a short guide since those instructions only work out-of-the-box for addons that are already officially part of Kodi. Note that the build system was recently reworked and the steps to set up a development environment will surely get easier over time.

I'm assuming here that you'll check out all source code into `C:\Projects`, you'll have to adjust that if you use another path.

#### Windows

1. Check out the Kodi source from Github (`C:\Projects\xbmc`) and make sure you have Microsoft Visual Studio 2013 installed.
2. Check out this repository (`C:\Projects\pvr.vbox`)
3. This step has to be done once only. Go to `C:\Projects\xbmc\project\cmake\addons\addons`. You'll see there's a folder for each binary addon. In order to build your locally checked out copy of the addon instead of the version that is bundled with Kodi you'll need to open the file `pvr.vbox/pvr.vbox.txt` and change its contents to the following: `pvr.vbox file://C:/Projects/pvr.vbox`.
4. Open a command prompt (or Powershell) and browse to `C:\Projects\xbmc\tools\windows`. From here, run `prepare-binary-addons-dev.bat clean` and then `prepare-binary-addons-dev.bat pvr.vbox`.
5. Go to `C:\Projects\xbmc\project\cmake\addons\build` and open and build the `kodi-addons.sln` solution.
6. The addon DLL is built and located in `C:\Projects\xbmc\addons`. If you run Kodi now from inside Visual Studio the addon will appear automatically under "System addons". If you don't want to bother compiling Kodi from source, install it as you normally would and copy the `pvr.vbox` into `%APPDATA%\Kodi\addons`.
7. Run Kodi, configure and enable the addon, then enable Live TV.

### Architecture

This addon has been built from the ground up using C++11. The main functionality is contained within the `vbox` namespace, the only file outside that is `client.cpp` which bridges the gap between the Kodi API and the API used by the main library class, `vbox::VBox`. The idea is to keep the addon code as decoupled from Kodi as possible.

The addon communicates with a VBox TV Gateway using the TV gateway's HTTP API. Since the structure of the responses vary a little bit, a factory is used to construct meaningful objects to represent the various responses. All response-related code is located under the `vbox::response` namespace.

The addon requires XMLTV parsing since that's the format the gateway provides EPG data over. The classes and utilities for handling this are kept separate under the `xmltv` namespace so that the code can potentially be reused.

The `vbox::VBox` class which `client.cpp` interfaces with is designed so that an exception of base type `VBoxException` (which is an extension of `std::runtime_error`) is thrown whenever ever a request fails. A request can fail for various reasons:

* the request failed to execute, i.e. the backend was unavailable
* the XML parsing failed, i.e. the response was invalid
* the request succeeded but the response represented an error
 
Similar to the XMLTV code, the code for the timeshift buffer is fairly generic and lives in a separate `timeshift` namespace. Currently there is a base class for all buffers and two implementations, a `FilesystemBuffer` which buffers the data to a file on disc, and a `DummyBuffer` which just relays the read operations to the underlying input handle. This is required since Kodi uses a different code paths depending on whether clients handle input streams on their own or not, and we need this particular code path for other features like signal status handling to work.

### Versioning

The addon follows semantic verisoning. Each release is tagged with its respective version number. Since each release of Kodi requires a separate branch for the addon, the major version changes whenever the Kodi version changes. This means that versions `1.x.x` are for Kodi v15, versions `2.x.x` are for Kodi v16 and so on.

### Useful links

* [Kodi's PVR user support] (http://forum.kodi.tv/forumdisplay.php?fid=167)
* [Kodi's PVR development support] (http://forum.kodi.tv/forumdisplay.php?fid=136)
* [Kodi's about VBox TV Gateway] (http://kodi.wiki/view/VBox_Home_TV_Gateway)
* [Coverity Scan for pvr.vbox] (https://scan.coverity.com/projects/4605?tab=overview)

###  License

The code is licensed under the GNU GPL version 2 license.

