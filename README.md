# KtechpitApps

"More apps from Ktechpit" for Ktechpit's Qt desktop apps: cards that suggest the other apps,
for an About dialog, a What's new sheet, a welcome page or a menu entry. Qt 6.5+, Widgets,
C++17. MIT licence.

<img src="docs/strip.png" alt="The strip in an About dialog" width="460">

## What it shows

The catalog comes from `https://ktechpit.com/USS/public/api/apps.php` (the USSPublic site:
`api/apps.php`, built from `includes/products.php` and `includes/app_catalog.php`).
A copy is bundled in the resources, so the cards work offline and on the first start. About 20
seconds after start, and at most once every 7 days, the module downloads the current list and
keeps it in `~/.cache/ktechpit-apps/`, with the icons. It sends nothing about the user: a GET
of the list and of the icons, with the user agent `KtechpitApps/1 (<host id>)`.

Which apps:

- never the host app, never an app for another platform, never one installed here as a snap
  or a Flatpak;
- what counts as installed depends on the host's sandbox: a native or AppImage app sees every
  snap and Flatpak; a snap sees other snaps and per-user Flatpaks but not system-wide ones; a
  Flatpak sees none (Flathub does not grant a view of other apps). A missed install only shows
  **Get**, and the software centre then says it is installed;
- a strip draws its cards at random each time it is built, so over a few visits users see most
  of the catalog without opening the full list. Apps with a trending rank on the site come up
  more often: #1 six times as often as a non-trending app, #5 twice as often. A catalog refresh never swaps the cards of a
  strip already on screen;
- the full list (`MoreAppsDialog`) shows every app in the site's order, installed ones too, marked
  *Installed*. Its **Refresh** button fetches the list right away and says whether it worked;
  if not, the saved list stays.

**Get** opens the store page in the desktop's software centre for the way the host app was
installed: `snap://<name>` for a snap, `appstream://<flatpak id>` for a Flatpak or a distro
package, the product page for an AppImage or on Windows. Clicking the card opens the product
page. Web links carry `?ref=<source>-<place>` (for example `ref=red-about`), so the site's logs
show which app and which spot a visit came from.

## Add it to an app

Pull it in as a subtree (once):

```sh
git subtree add --prefix src/modules/KtechpitApps git@github.com:keshavbhatt/ktechpit-apps.git main --squash
```

and later updates with `git subtree pull` and the same arguments. To stay on a release
instead of `main`, use its tag (`v1.0.0`) in place of `main`; [CHANGELOG.md](CHANGELOG.md)
says what changed. Then:

```cmake
add_subdirectory(src/modules/KtechpitApps)
target_link_libraries(myapp PRIVATE ktechpit_apps)
```

```cpp
#include "AppCatalog.h"
#include "AppStrip.h"
#include "MoreAppsDialog.h"

// once, after QApplication
ktechpit::CatalogConfig config;
config.hostAppId = u"whatsie"_s;            // the app's slug on ktechpit.com
config.package = ktechpit::Package::Flatpak; // how this copy was installed
config.source = u"whatsie"_s;               // ?ref=whatsie-<place>
config.oneLineText = true;                 // optional: one line per name and description, with "…"
ktechpit::AppCatalog::init(config);
ktechpit::AppCatalog::instance()->setEnabled(settings.showOtherApps()); // the user's choice

// in the About dialog: a heading, 3 cards and a "See all" link
layout->addWidget(new ktechpit::AppStrip(u"about"_s, 3));

// a menu entry
ktechpit::MoreAppsDialog(u"menu"_s, this).exec();
```

To show the full list in a window of the app's own (a sheet, a page) instead of the plain
dialog, put a `MoreAppsView` in it and send See all there:

```cpp
ktechpit::AppCatalog::instance()->setShowAll([](const QString& place, QWidget* from) {
    MyMoreAppsWindow(place, from).exec(); // holds a ktechpit::MoreAppsView(place, 3)
});
```

The widgets draw with the host's palette and fonts. Object names for style sheets:
`ktechpitAppCard`, `ktechpitAppName`, `ktechpitAppSummary`, `ktechpitAppGet`,
`ktechpitAppStrip`, `ktechpitAppStripTitle` (its heading), `ktechpitAppStripSeeAll`,
`ktechpitMoreAppsView`, `ktechpitMoreAppsIntro`, `ktechpitMoreAppsRefresh`,
`ktechpitMoreAppsDialog`.

Give users a way to turn it off (a "Show other Ktechpit apps" setting that calls
`setEnabled`); every strip then hides.

## Rules for using it

- Only in places the user opens: About, What's new, welcome, a menu entry. Never a pop-up,
  never over the app's main work.
- At most one strip per screen.
- A setting turns it off.

## Develop

```sh
cmake -S . -B build -DKTECHPIT_APPS_BUILD_TESTS=ON -DKTECHPIT_APPS_BUILD_DEMO=ON
cmake --build build && ctest --test-dir build
build/ktechpit_apps_demo whatsie            # an About-like window as Whatsie
build/ktechpit_apps_demo red grab.png       # saves grab.png and grab-all.png, then quits
```

`scripts/update-catalog.sh` refreshes the bundled catalog, icons and `.qrc` from the live
endpoint; run it before tagging a release of the module.

## Adding an app to the catalog

On the site: the product goes in `includes/products.php` as before. `includes/app_catalog.php`
can give it a short name for the cards, a Flathub id when its product entry has no Flathub
link, or `'promote' => false` to keep it out of the apps (KimiDoc, a server product). Installed
apps pick it up within a week; run `scripts/update-catalog.sh` here so new installs have it too.
