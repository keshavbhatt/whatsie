# Changelog

Versions follow [Semantic Versioning](https://semver.org/): a new catalog field or widget option
is a minor version, a change that needs the host app to change its code is a major version.

## [1.1.0] - 2026-11-10

- `MoreAppsView`: the full list as a widget, for a host that shows it in a window of its own,
  with a column count, `setIntro()` and a `status()` / `statusChanged()` line (how many apps,
  or what Refresh did). `MoreAppsDialog` is now this view in a plain dialog; its Refresh button
  sits beside the search field.
- `AppCatalog::setShowAll()`: the host's own way to open the full list from a strip's See all;
  without it See all opens a `MoreAppsDialog` as before.
- `CatalogConfig::oneLineText`: each card's name and description on one line, shortened with an
  ellipsis (the tooltip has both whole), so every card is one fixed height in any font. Off by
  default: the description takes up to two lines, now measured in the font it ends up with.
- `MoreAppsView` never scrolls sideways.
- `AppStrip`: See all is not underlined.
- `AppCard`: Tab reaches a card and Enter or Space opens its page; object names
  `ktechpitAppName`, `ktechpitAppSummary` and `ktechpitAppGet`, and `ktechpitAppStripSeeAll`
  on the strip's link, for host style sheets.

## [1.0.1] - 2026-11-04

- `AppStrip`: side-by-side cards share the width equally, so a long description no longer
  squeezes the next card, with 10 px between cards.
- `AppStrip`: the heading has the object name `ktechpitAppStripTitle`, for host style sheets.

## [1.0.0] - 2026-11-04

First release.

- `AppCatalog`: the list of Ktechpit apps from `https://ktechpit.com/USS/public/api/apps.php`
  (schema 1), with a bundled copy for offline and first starts, refreshed in the background at
  most once a week and on request (`refresh()`, `refreshFinished`). Leaves out the host app and
  apps for other platforms.
- `AppStrip`: a heading, cards drawn at random each time (trending apps more often, installed
  apps skipped) and a "See all" link.
- `MoreAppsDialog`: every app in the site's order, installed ones marked, with search and a
  Refresh button.
- `AppCard`: icon, name, one line and a Get button that opens the software centre for the way
  the host app was installed (`snap://`, `appstream://`), else the product page with
  `?ref=<source>-<place>`.
- A setting-friendly switch (`setEnabled`) that hides every card.
