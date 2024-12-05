# WhatSie -  Package for Debian or Deepin 23

Feature rich WhatsApp web client based on Qt WebEngine for Linux Desktop

## Whatsie Key features

- Light and Dark Themes with automatic switching
- Customized Notifications & Native Notifications
- Keyboard Shortcuts
- BuiltIn download manager
- Mute Audio, Disable Notifications
- App Lock feature
- Hardware access permission manager
- Built in Spell Checker (with support for 31 Major languages)
- Other settings that let you control every aspect of WebApp like:
	+ Do not disturb mode
	+ Full view mode, lets you expand the main view to the full width of the window
	+ Ability to switch between Native & Custom notification
	+ Configurable notification popup timeout
	+ Mute all audio from Whatapp
	+ Disabling auto playback of media
	+ Minimize to tray on application start
	+ Toggle to enable single click hide to the system tray
	+ Switching download location
	+ Enable disable app lock on application start
	+ Auto-locking after a certain interval of time
	+ App lock password management
	+ Widget styling
	+ Configurable auto Theme switching based on day night time
	+ Configurable close button action
	+ Global App shortcuts
	+ Permission manager let you toggle camera mic and other hardware level permissions
	+ Configurable page zoom factor, switching based on window state maximized on normal 
	+ Configurable App User Agent
	+ Application Storage management, lets you clean residual cache and persistent data

## Command line options:
Comes with general CLI support, with a bunch of options that let you interact with already running instances of Whatsie.

Run: `whatsie -h` to see all supported options.

```
Usage: whatsie [options]
Feature rich WhatsApp web client based on Qt WebEngine

Options:
  -h, --help           Displays help on commandline options
  -v, --version        Displays version information.
  -b, --build-info     Shows detailed current build infomation
  -w, --show-window    Show main window of running instance of WhatSie
  -s, --open-settings  Opens Settings dialog in a running instance of WhatSie
  -l, --lock-app       Locks a running instance of WhatSie
  -i, --open-about     Opens About dialog in a running instance of WhatSie
  -t, --toggle-theme   Toggle between dark & light theme in a running instance
                       of WhatSie
  -r, --reload-app     Reload the app in a running instance of WhatSie
  -n, --new-chat       Open new chat prompt in a running instance of WhatSie
```

## Build instructions (Linux)
The source code can be built using the regular Qt application development procedure. Whatsie Project makes use of Qt's QMake build system, which simplifies the build process. To build Whatsie locally on your system, follow the steps below.

### Build requirements
 - git (to clone repo)
 - libx11-dev libx11-xcb-dev (required for x11 XKB module support at build time)
 - Qt => 5.9 (5.15 recommended) with the following modules installed with development headers (lib*-dev packages on Ubuntu)
	+ webengine
	+ webenginewidgets
	+ positioning
	
### Build steps
 
 1. **Clone** source code

 	`git clone https://github.com/keshavbhatt/whatsie.git`

 2. Enter into source directory  
	
	`cd whatsie/src`  
	
 3. Run **qmake**
	
	`qmake`
	
 4. Run **make** (with optional **-j** option that specify jobs or commands to run simultaneously while building)
 
	`make -j4`  
	
 5. **Run built whatsie** executable (if build finished with no errors)
 
	`./whatsie`
	
 5. Run **make install** if you want to install (if build finished with no errors)
 
	`sudo make install`



## Install Whatsie on Linux Desktop

### On any snapd supported Linux distributions

 `snap install whatsie`

### On any Arch based Linux distribution
Using Arch User Repository (AUR), [AUR package for Whatsie](https://aur.archlinux.org/packages/whatsie-git) is maintained by [M0Rf30](https://github.com/M0Rf30)

 `yay -S whatsie-git`

## Screenshots (could be old)

![WhatSie for Linux Desktop Light Theme](https://github.com/keshavbhatt/whatsie/blob/main/screenshots/1.jpg?raw=true)
![WhatSie for Linux Desktop Dark Theme](https://github.com/keshavbhatt/whatsie/blob/main/screenshots/2.jpg?raw=true)
![WhatSie for Linux Desktop Setting module](https://github.com/keshavbhatt/whatsie/blob/main/screenshots/4.jpg?raw=true)
![WhatSie for Linux Desktop App Lock screen](https://github.com/keshavbhatt/whatsie/blob/main/screenshots/3.jpg?raw=true)
![WhatSie for Linux Desktop Shortcuts & Permissions](https://github.com/keshavbhatt/whatsie/blob/main/screenshots/5.jpg?raw=true)
