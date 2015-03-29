# VBox Home TV Gateway PVR Client

This repository provides a [Kodi] (http://kodi.tv) PVR addon for interfacing with the VBox Communications XTi TV Gateway devices. This README serves as a quick overview of the functionality and architecture of the addon, to make it easier for others to possible contribute.

### Settings

This list contains some explanation for the non-obvious settings:
* `External IP`: The external address of the VBox device when used over the internet. Without this setting, all URLs the device responds with will use its internal IP address.
* `External XMLTV path`: The addon can augment the EPG data it receives from the VBox device with external data in XMLTV format. This option is for specifying the path to the XML file that holds the guide data. It is your responsibility to make sure this file is updated on a regular basis.
* `Prefer external EPG over OTA`: If a specific channel has guide data both from the VBox itself and from the external XMLTV file, this setting controls which guide data is used. Note that the external data is always used if the VBox doesn't provide any guide data for a specific channel.
* `Timeshift buffer path`: The path where the timeshift buffer files should be stored when timeshifting is enabled. Make sure you have a reasonable amount of disk space available since the buffer will grow indefinitely until you stop watching or switch channels!

### Build instructions

The main instructions for how to build PVR addons using the new build system can be found [here](http://forum.kodi.tv/showthread.php?tid=219166). Here's a short guide since those instructions only work out-of-the-box for addons that are already officially part of Kodi. Note that the build system was recently reworked and the steps to set up a development environment will surely get easier over time.

I'm assuming here that you'll check out all source code into `C:\Projects`, you'll have to adjust that if you use another path.

#### Windows

1. Check out the Kodi source from Github (`C:\Projects\xbmc`) and make sure you have Microsoft Visual Studio 2013 installed.
2. Check out this repository (`C:\Projects\pvr.vbox`)
3. This step has to be done once only. Go to `C:\Projects\xbmc\project\cmake\addons\addons`. You'll see there's a folder for each binary addon. You'll need to copy one of the existing PVR addons and rename the folder to `pvr.vbox`. Inside the folder, rename the `pvr.*.txt` file to `pvr.vbox.txt` and change its contents to the following: `pvr.vbox file://C:/Projects/pvr.vbox`
4. Open a command prompt (or Powershell) and browse to `C:\Projects\xbmc\tools\windows`. From here, run `prepare-binary-addons-dev.bat clean` and then `prepare-binary-addons-dev.bat pvr.vbox`.
5. Go to `C:\Projects\xbmc\project\cmake\addons\build` and open and build the `kodi-addons.sln` solution.
6. Open the Kodi solution (`C:\Projects\xbmc\project\VS2010Express\XBMC for Windows.sln`), right-click the solution and choose to add an existing project. Change the file filter to include all files, then add `C:\Projects\xbmc\project\cmake\addons\build\pvr.vbox-prefix\src\pvr.vbox-build\pvr.vbox.sln`.
7. Build only the `libxmltv` and `pvr.vbox` projects (unless you want to build Kodi yourself, in which case you'll need to do a lot more work).
8. The addon DLL is built and located in `C:\Projects\xbmc\project\cmake\addons\build\pvr.vbox-prefix\src\pvr.vbox-build\Debug`. For now you'll have to additionally copy the contents of `C:\Projects\pvr.vbox\pvr.vbox` to this directory. Now you have a complete addon and you can either copy or symlink this directory into `%APPDATA%\Kodi\addons` (make sure it's named `pvr.vbox`, not `Debug`).
9. Run Kodi, configure and enable the addon, then enable Live TV.

### Architecture

This addon has been built from the ground up using C++11. The main functionality is contained within the `vbox` namespace, the only file outside that is `client.cpp` which bridges the gap between the Kodi API and the API used by the main library class, `vbox::VBox`. The idea is to keep the addon code as decoupled from Kodi as possible.

The addon communicates with a VBox TV Gateway using the TV gateway's HTTP API. Since the structure of the responses vary a little bit, a factory is used to construct meaningful objects to represent the various responses. All response-related code is located under the `vbox::response` namespace.

The addon requires XMLTV parsing since that's the format the gateway provides EPG data over. The classes and utilities for handling this are shipped as a separate library (libxmltv) and available through the `xmltv` namespace.

The `vbox::VBox` class which `client.cpp` interfaces with is designed so that an exception of base type `VBoxException` (which is an extension of `std::runtime_error`) is thrown whenever ever a request fails. A request can fail for various reasons:

* the request failed to execute, i.e. the backend was unavailable
* the XML parsing failed, i.e. the response was invalid
* the request succeeded but the response represented an error
 
The code for the timeshift buffer is fairly generic and at some point it will probably move out of the `vbox` namespace, since it doesn't depend on it in any way. Currently there is a base class for all buffers and two implementations, a `FilesystemBuffer` which buffers the data to a file on disc, and a `DummyBuffer` which just relays the read operations to the underlying input handle. This is required since Kodi uses a different code paths depending on whether clients handle input streams on their own or not, and we need this particular code path.

### Useful links

* [Kodi's PVR user support] (http://forum.kodi.tv/forumdisplay.php?fid=167)
* [Kodi's PVR development support] (http://forum.kodi.tv/forumdisplay.php?fid=136)
* [Kodi's about VBox TV Gateway] (http://kodi.wiki/view/VBox_Home_TV_Gateway)

###  License

The code is licensed under the GNU GPL version 2 license.

