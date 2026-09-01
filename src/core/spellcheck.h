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

/// The `.bdic` dictionary names present in `dictionaryDir` (e.g. "en-US"),
/// sorted. Empty when the directory is missing or has none.
[[nodiscard]] QStringList availableDictionaries(const QString& dictionaryDir);

/// Picks the installed dictionary to actually use for `want` (a name from
/// dictionaryNameForLocale, e.g. "en-IN"): the exact one if present, else the
/// bare language ("en"), else the first same-language variant ("en-GB"), else
/// empty. Never crosses languages — a missing language means no spell check,
/// not the wrong dictionary underlining every word.
[[nodiscard]] QString resolveDictionary(const QString& want, const QStringList& available);

} // namespace whatsie::core
