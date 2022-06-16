# Change log:

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

## 4.0
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


