// SPDX-FileCopyrightText: 2026 Keshav Bhatt (Ktechpit)
// SPDX-License-Identifier: MIT
#pragma once

#include "AppCatalog.h"

#include <QUrl>

namespace ktechpit::AppLinks {

/// Where "Get" leads for this install type: the snap or Flathub store page in the desktop's
/// software centre (snap://, appstream://), else the product page on ktechpit.com.
[[nodiscard]] QUrl getUrl(const AppInfo& app, Package package, const QString& source,
                          const QString& place);
/// The product page, with ?ref=<source>-<place> when a source is set.
[[nodiscard]] QUrl pageUrl(const AppInfo& app, const QString& source, const QString& place);

/// Opens getUrl(); falls back to the product page when no handler takes the store link.
void openGet(const AppInfo& app, const QString& place);
/// Opens the product page.
void openPage(const AppInfo& app, const QString& place);

/// True when the app is visibly installed here as a snap or a Flatpak. What a sandbox can see
/// (checked 2026-11-04): a native or AppImage host sees every snap and Flatpak; a snap host
/// sees other snaps and per-user Flatpaks, not system-wide Flatpaks; a Flatpak host sees
/// nothing. A miss only means the app shows "Get"; the software centre then says it is
/// installed. Never reports an app that is not installed.
[[nodiscard]] bool isInstalled(const AppInfo& app);

} // namespace ktechpit::AppLinks
