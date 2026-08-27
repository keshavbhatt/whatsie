#pragma once

#include <QString>
#include <QStringList>

class QLocale;

// Spell-check helpers (FEATURES L1). Pure; unit-tested.
namespace whatsie::core {

/// Maps a QLocale to a Chromium `.bdic` base name: "en_US" → "en-US",
/// "de" → "de". Falls back to "en-US" for the C/empty locale.
[[nodiscard]] QString dictionaryNameForLocale(const QLocale& locale);

/// The first existing "qtwebengine_dictionaries" directory among the candidate
/// roots, or an empty string. QtWebEngine reads `.bdic` files from there.
[[nodiscard]] QString findDictionariesPath(const QStringList& candidateRoots);

} // namespace whatsie::core
