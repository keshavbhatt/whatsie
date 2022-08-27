## 4.8.0 (2022-08-27)

#### 🎁 Feature

* add toggle theme desktop action (f8c9b339)

#### 📄 Documentation

* **changelog:** update cl & ver after release (572d6948)

#### 🚧 Chores

* unlock action button (771625da)
* **build:** use Qt5.15.4 for build (54f97210)
* update readme (0d3bd466)
* udapte new settings window screenshot (6f3f18c6)
* remove unused xml module (2d71f12c)
* version bump in pro (211699e3)
* **ci:** disable auto release and version file (6f134db2)


## 4.7.2 (2022-07-22)

#### 📄 Documentation

* **changelog:** update cl & ver after release (e0d15c2e)


## 4.7.1 (2022-07-04)

#### 🐞 Bug Fixes

* focus on password edit when echo (cee2dc85)
* **web:** bypass lock check while loading quirk (6c6275c3)

#### 📄 Documentation

* **changelog:** update cl & ver after release (974933d0)

#### 🚧 Chores

* Merge branch 'dev' (9f566869)
* **ui:** update ui color (8a74ccbc)


## 4.7.0 (2022-07-03)

#### 🎁 Feature

* **ci:** add release workflow (83cd6383)
* **i18n:** add Italian localization (#55) (ced5547d)
* enable support for traybar entries on GNOME dash (#53) (66d20d3e)
* some new features (21113900)
* unlock animation plus some cleanup (0a182a9e)
* implement IPC & other improvements (81faa022)
* add open downloads directory button in download widget (419ffb29)
* app auto locking (d06a4abb)
* v4.0 (#35) (474b9212)
* start application minimized. closes #19 (c5bf7a98)

#### 🐞 Bug Fixes

* obey fullview settings on first launch & initial window size (b2f0fe49)
* properly hide custom notification on multi monitor setups (20057675)
* use availableGeometry to map position of notification (538d7d5d)
* add missing icon, enabling install_icon target generation (clos… (#45) (48b9028f)
* show notifications on correct screen (ff99a5f7)
* logout flow during changepassword (92382d7b)
* properly load setting for autoapplock checkbox (522eb75a)
* save geometry in quit event (4a968554)
* raise window from hidden state when clicked on notification (0620e43e)
* debug in debug mode (147487f2)
* notification popup click behavior (e800208f)
* **snap:** supress warnings (f2b06da6)
* improve logout flow, on change password (ed5f760b)
* change lock screen password beahvior (fa4012a5)
* theme switching (7cd4b219)
* improve download file behavior (#32) (8f071469)

#### 🎨 Styles

* code refactor (21940ee6)

#### 🚧 Chores

* use pre-commit (f82dcc68)
* update todo (2aa08e03)
* setQuitOnLastWindowClosed false (c751be26)
* set a minimum of 4 digits for the lock code (#56) (79b2b791)
* notification connect before show (f8455de7)
* update app description (c6fd2e8d)
* use appinstall artifacts from dist (247ed75f)
* distribution related files (88c46fad)
* **CI:** use latest version of install-qt-action (60b6c225)
* **CI:** build with github action (ac31abdb)
* define fallback values for macros (14f190c0)
* **qmake:** avoid error message when .git folder is missing (close #49) (#52) (91d0cf11)
* add full view support closes #46 (b96a28db)
* version 4.4 (26f5659b)
* install dicts using qmake (90210de2)
* add git sponser link (122828f4)
* improve settings window show behavior (d9909011)
* improve window geo restore (3a08d5d5)
* nitification popup tweak; code cleanup (5c2764f7)
* update readme (a4c73b0f)
* version 4.3 (3dae93a1)
* use Ctrl+W to hide window to tray (dba5a9bc)
* filter contextmenu items (6f4750c8)
* restore window directly when another instance is launched (39117158)
* use new chat trigger method to invoke new chats (1d950cd8)
* update changelog (59abd9d9)
* version 4.2 (1f4816a2)
* remove runguard (8c0df6d3)
* window show behavior (7d302466)
* update default UA (dfb5b9ca)
* stop timer instantly if rated already (cc43d4c7)
* bump version 4.1 (a1af1bde)
* minor improvements (ea4056dc)
* clean UA & disable js debug in app stdout (8cfbcf4b)
* set default zoom factor for maximized windows to 1.0 (046e2e13)
* inform app is minimized via notification (19734a99)
* unify passowrd echomode in lock widget (5be4cae9)
* test qpt gtk3 (020ac6da)
* add Desktop entry GenericName (e4bbdd15)
* move desktop file to src (4f0558a9)
* use desktop-launch from content snap (dcc39239)

#### 📦 Build

* **snap:** use SNAPCRAFT_ARCH_TRIPLET (8962c8bb)
* migrate to qt 5.15 (9867a6b6)

#### chaore

* **CI:** use Qt 5.15.2 (846d1218)

#### cleanup

* removed snap_launcher (e658c464)


## 4.6.5 (2022-07-03)

#### 🚧 Chores

* **ci:** update release wf (#59) (f40ac9c9)


## 4.6.3 (2022-07-03)

#### 📄 Documentation

* **changelog:** update changelog after release (7699d885)

#### 🚧 Chores

* **ci:** fix update file name (c0158c0d)


## 4.6.2 (2022-07-03)

#### 📄 Documentation

* **changelog:** update changelog after release (c87524db)

#### 🚧 Chores

* **ci:** update version on release (d715c8eb)


## 4.6.1 (2022-07-03)

#### 🚧 Chores

* **CI:** commit changelog on release (75b0cffe)


## Change log:

### 4.3
- feat: IPC; restore window directly when another instance is launched
- feat: allow context menu on editable, selected and copyble data types
- fix: properly load setting for autoapplock checkbox
- fix: logout flow during changepassword
- fix: the minimize behavior; replace Ctrl+H with Ctrl+W to hide window to tray

### 4.2
- fix: raise window from hidden state when clicked on notification
- updated new UA
- fix: window geometry persistence behavior
- feat: open download directory straight from the download manager
- fix: consistent window show behavior
- feat: implement IPC
   - lets run only one instance of application
   - lets pass arguments from secondary instances to main instance
   - open new chat without reloading page
   - restore application with command line argument to secondary instance:
          example: `whatsie whatsapp://whatsie`
          will restore the primary instance of whatsie process

### 4.0
- fix(SystemTray) tray icon uses png rather than SVG
- feat(SystemTray) added settings to lets users change the system tray icon click behavior(minimize/maximize on right-click)
- feat(Download) added setting that lets the user set default download directory, avoid asking while saving files
- fix(Notification) clicking popup now correctly restores the app window
- feat(Lock) added setting to let users change the current set password for the lock screen
- feat(Lock) added setting to enable disable auto app locking, with defined duration
- feat(Lock) current set password is now hidden by default and can be revealed for 5 seconds by pressing the view button
- feat(Style/Theme) added ability to change widget style on the fly, added default light palette (prevent breaking of light theme on KDE EVs)
- fix(Theme) dark theme update
- feat(WebApp) added setting to set zoom factor when the window is maximized and fullscreen (gives user ability to set different zoom factor for Normal, Maximized(Fullscreen WindowStates)
- fix(Setting) settings UI is more organized
- fix(WebApp) enable JavaScript execCommand("paste")
- feat(WebApp) tested for new WhatsApp Web that lets users use Whatsie without requiring the phone connected to the internet
- fix(Lock) unify passowrd echomode in lock widget


