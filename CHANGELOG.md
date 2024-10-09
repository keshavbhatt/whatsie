## 4.16.0 (2024-10-09)

#### 🎁 Feature

* Add secure compilation flags (4720ffeb)

#### 🐞 Bug Fixes

* change zoom factor when app starts minimized in systray (be47a73d)
* set QTWEBENGINE_DICTIONARIES_PATH to fix spell checker (#199) (40da519b)

## 4.15.1 (2024-08-01)

#### 🚧 Chores

* revert to build action Qt to version 5.15.2 (bbccc551)
* bump version 4.15.1 (d8746929)
* update dist icon (d0304ad1)
* new icon (f54e608c)
* update notification icons (2718a85d)
* add new keywords to appdata (4e803bfa)
* update appstream metadata for 4.15.0 release (a535ee1b)
* do not mark statusCode unused in desktopOpenUrl util method (51e28237)
* udapte CHANGELOG (5940b6ec)


## 4.15.0 (2024-05-25)

#### 🎁 Feature

* efficient use of element mutation observers (e553c00c)
* add line widget at bottom of cutom notification widget (0d19713d)
* use xdg-open as fallback to open local files (30921582)

#### 🐞 Bug Fixes

* restore window state video fullscreen toggle (e0483dda)
* prioritize xdg-open for handling openUrl request (36cd46b7)
* set correct lock state when app starts minimized (b58d9422)

#### 🚧 Chores

* CI Update Qt version to 5.15.4 for build (e030486b)
* update UA to latest Chrome (cc2852a7)
* snap use portal if available (8cef2047)
* version 4.15 (08fba4cf)

## 4.14.2 (2023-12-01)

#### 🐞 Bug Fixes

* incorrect full width modification (#150) (3d20ebe2)

#### 🚧 Chores

* bump version 4.14.2 (c478a7d6)
* Use qmake-provided _DATE_ (#146) (fec5644c)



## 4.14.1 (2023-05-20)

#### 🐞 Bug Fixes

* fix unread message parsing (906ca7eb)

#### 🚧 Chores

* update workflow and changelog (41225b19)



## 4.14.0 (2023-05-17)

#### 🎁 Feature

* minor fixes + code cleanup (5f10a0f9)

#### 📄 Documentation

* **changelog:** update cl & ver after release (b4b5dc33)

#### 🚧 Chores

* bump version to 4.14 (c235b8ae)


## 4.13.0 (2023-03-22)

#### 🎁 Feature

* add open file option in download item (7b64cf51)

#### 🐞 Bug Fixes

* prevent overwrite if file exists (dd687dc9)

#### 📄 Documentation

* **changelog:** update cl & ver after release (fa5add01)

#### 🚧 Chores

* bump version 4.13 (3484bccc)
* update js to improve perf (1b030ca7)
* code cleanup (c2bd1a32)
* update utils (8f983031)


## 4.12.1 (2023-01-27)

#### 🐞 Bug Fixes

* icon on Plasma Wayland (#100) (7fc4ce38)

#### 📄 Documentation

* **changelog:** update cl & ver after release (4417eced)

#### 🚧 Chores

* code cleanup (085205eb)
* cleanup (011db449)
* escape key to close child windows (56c55a94)


## 4.12.0 (2023-01-26)

#### 🎁 Feature

* close permission dialog with esc key (ee519bcc)
* close with esc button (2119c3d1)

#### 🐞 Bug Fixes

* prevent zoom with ctrl+mouse (0eb7ea05)

#### 📄 Documentation

* **changelog:** update cl & ver after release (b3e2a2be)

#### 🚧 Chores

* bump version 4.12 (12bce6d2)
* cleanup (c78394d1)


## 4.11.0 (2023-01-26)

#### 🎁 Feature

* bump version 4.11 (63440d96)

#### 🐞 Bug Fixes

* applock state fix (32b0e5ca)

#### 📄 Documentation

* **changelog:** update cl & ver after release (74216cfd)

#### 🚧 Chores

* remove debugging from MoreApps widget (103a3686)
* cleanup + inhancements (f31197a4)
* cleanup + addition (21ca36bd)


## 4.10.3 (2022-12-18)

#### 📄 Documentation

* **changelog:** update cl & ver after release (d7f1faee)

#### 🚧 Chores

* add moreapps widget in lock screen (074b0f98)


## 4.10.2 (2022-09-20)

#### 📄 Documentation

* **changelog:** update cl & ver after release (3a71de33)

#### 🚧 Chores

* make description under 1000 chars (1c5bfc42)


## 4.10.1 (2022-09-17)

#### 📄 Documentation

* **changelog:** update cl & ver after release (56c06a92)

#### 🚧 Chores

* update appdata, to make flatpak-builder happy (c8b1b838)


## 4.10.0 (2022-09-17)

#### 🎁 Feature

* systemtray notification counter (530c24bf)
* add toggle theme desktop action (f8c9b339)
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

* duplicate action in desktop file (2837c87e)
* auto lock while scrolling (baa52666)
* **build:** fix build due to missing icon (5464060d)
* focus on password edit when echo (cee2dc85)
* **web:** bypass lock check while loading quirk (6c6275c3)
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

#### 📄 Documentation

* **changelog:** update cl & ver after release (1f2bb6fc)
* **changelog:** update cl & ver after release (5cdba515)
* **changelog:** update cl & ver after release (9472e9e6)
* **changelog:** update cl & ver after release (bd7386a1)
* **changelog:** update cl & ver after release (554ceff4)
* **changelog:** update cl & ver after release (572d6948)
* **changelog:** update cl & ver after release (e0d15c2e)
* **changelog:** update cl & ver after release (974933d0)

#### 🎨 Styles

* code refactor (21940ee6)

#### 🚧 Chores

* update appdata (273a9138)
* **snap:** use svg icon for snap store (bcec3eef)
* skip dictionaries conversion if build type is flatpak build (f16086c2)
* Version bump 4.9 (af39ff62)
* **snap:** use svg icon for desktop entry (51dcb0d4)
* **ci:** release dry-run action (7ef46aaf)
* delete filedialog after exec (5e50519c)
* add notification icons (997ae821)
* **webengine:** disable support for Pepper plugins (325d841e)
* some enhancements (ff575c45)
* update UA to 104 (d03e9fc6)
* bump version 4.8 (7fde1e4c)
* unlock action button (771625da)
* **build:** use Qt5.15.4 for build (54f97210)
* update readme (0d3bd466)
* udapte new settings window screenshot (6f3f18c6)
* remove unused xml module (2d71f12c)
* version bump in pro (211699e3)
* **ci:** disable auto release and version file (6f134db2)
* Merge branch 'dev' (9f566869)
* **ui:** update ui color (8a74ccbc)
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


## 4.9.1 (2022-09-13)

#### 📄 Documentation

* **changelog:** update cl & ver after release (5cdba515)

#### 🚧 Chores

* skip dictionaries conversion if build type is flatpak build (f16086c2)


## 4.9.0 (2022-09-03)

#### 🎁 Feature

* systemtray notification counter (530c24bf)

#### 📄 Documentation

* **changelog:** update cl & ver after release (9472e9e6)

#### 🚧 Chores

* Version bump 4.9 (af39ff62)
* **snap:** use svg icon for desktop entry (51dcb0d4)
* **ci:** release dry-run action (7ef46aaf)
* delete filedialog after exec (5e50519c)
* add notification icons (997ae821)
* **webengine:** disable support for Pepper plugins (325d841e)
* some enhancements (ff575c45)
* update UA to 104 (d03e9fc6)


## 4.8.2 (2022-08-27)

#### 🐞 Bug Fixes

* duplicate action in desktop file (2837c87e)
* auto lock while scrolling (baa52666)

#### 📄 Documentation

* **changelog:** update cl & ver after release (bd7386a1)


## 4.8.1 (2022-08-27)

#### 🐞 Bug Fixes

* **build:** fix build due to missing icon (5464060d)

#### 📄 Documentation

* **changelog:** update cl & ver after release (554ceff4)

#### 🚧 Chores

* bump version 4.8 (7fde1e4c)


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


