#pragma once

#include <QString>
#include <QUrl>

#include <optional>

// Pure URL rules: what stays in the app, what goes to the browser, and how
// chat deep links are turned into WhatsApp Web URLs (FEATURES S10, S11, M4).
namespace whatsie::core {

struct NewChatRequest
{
    QString phone; ///< digits only, no leading '+'
    QString text;  ///< optional pre-filled message

    bool operator==(const NewChatRequest&) const = default;
};

/// True for https://web.whatsapp.com/... (any path).
[[nodiscard]] bool isWhatsAppWebUrl(const QUrl& url);

/// True when a link click should be handed to the system browser.
[[nodiscard]] bool shouldOpenExternally(const QUrl& url);

/// True for the Adobe Acrobat PDF integration that WhatsApp's "Continue to
/// Acrobat" opens in a popup (acrobat.adobe.com and the adobe.com/adobelogin.com
/// hosts its sign-in flow uses). Such a popup must stay INSIDE the app: WhatsApp
/// transfers the PDF to it over postMessage, which only works while the app owns
/// the child window — handing it to the system browser closes it and the
/// transfer fails ("target window is closed").
[[nodiscard]] bool isPdfIntegrationUrl(const QUrl& url);

/// Accepts whatsapp://send?phone=..&text=.., https://wa.me/<phone>?text=..,
/// https://api.whatsapp.com/send?phone=.., https://web.whatsapp.com/send?phone=..
/// and plain phone numbers ("+49 170 1234567"). Returns nullopt otherwise.
[[nodiscard]] std::optional<NewChatRequest> parseChatLink(const QUrl& url);
[[nodiscard]] std::optional<NewChatRequest> parseChatLink(const QString& urlOrPhone);

/// https://web.whatsapp.com/send?phone=<digits>&text=<encoded>
[[nodiscard]] QUrl newChatUrl(const NewChatRequest& request);

/// Strips everything but digits.
[[nodiscard]] QString normalizePhone(const QString& raw);

} // namespace whatsie::core
