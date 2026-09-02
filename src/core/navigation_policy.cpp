#include "core/navigation_policy.h"

#include <QUrlQuery>

using namespace Qt::StringLiterals;

namespace whatsie::core {

namespace {

const QString kWebHost = u"web.whatsapp.com"_s;

bool isHttp(const QUrl& url)
{
    const QString scheme = url.scheme().toLower();
    return scheme == u"http"_s || scheme == u"https"_s;
}

std::optional<NewChatRequest> fromQuery(const QString& phone, const QUrlQuery& query)
{
    const QString digits = normalizePhone(phone);
    if (digits.isEmpty()) {
        return std::nullopt;
    }
    return NewChatRequest{.phone = digits, .text = query.queryItemValue(u"text"_s, QUrl::FullyDecoded)};
}

} // namespace

QString normalizePhone(const QString& raw)
{
    QString digits;
    digits.reserve(raw.size());
    for (const QChar c : raw) {
        if (c.isDigit()) {
            digits.append(c);
        }
    }
    return digits;
}

bool isWhatsAppWebUrl(const QUrl& url)
{
    return isHttp(url) && url.host().compare(kWebHost, Qt::CaseInsensitive) == 0;
}

bool shouldOpenExternally(const QUrl& url)
{
    return isHttp(url) && !isWhatsAppWebUrl(url) && !isPdfIntegrationUrl(url);
}

bool isPdfIntegrationUrl(const QUrl& url)
{
    if (!isHttp(url)) {
        return false;
    }
    const QString host = url.host().toLower();
    static const QString kAdobe = u"adobe.com"_s;
    static const QString kAdobeLogin = u"adobelogin.com"_s;
    const auto matches = [&host](const QString& domain) {
        return host == domain || host.endsWith(u'.' + domain);
    };
    return matches(kAdobe) || matches(kAdobeLogin);
}

std::optional<NewChatRequest> parseChatLink(const QUrl& url)
{
    if (!url.isValid()) {
        return std::nullopt;
    }
    const QString scheme = url.scheme().toLower();
    const QString host = url.host().toLower();
    const QUrlQuery query(url);

    // whatsapp://send?phone=..&text=..  (also whatsapp://send/?phone=..)
    if (scheme == u"whatsapp"_s) {
        if (host != u"send"_s && !url.path().startsWith(u"/send"_s) && !url.path().startsWith(u"send"_s)) {
            return std::nullopt;
        }
        return fromQuery(query.queryItemValue(u"phone"_s, QUrl::FullyDecoded), query);
    }

    if (!isHttp(url)) {
        return std::nullopt;
    }

    // https://wa.me/<phone>?text=..
    if (host == u"wa.me"_s || host == u"www.wa.me"_s) {
        return fromQuery(url.path(), query);
    }

    // https://api.whatsapp.com/send?phone=..  /  https://web.whatsapp.com/send?phone=..
    if ((host == u"api.whatsapp.com"_s || host == kWebHost) && url.path().startsWith(u"/send"_s)) {
        return fromQuery(query.queryItemValue(u"phone"_s, QUrl::FullyDecoded), query);
    }

    return std::nullopt;
}

std::optional<NewChatRequest> parseChatLink(const QString& urlOrPhone)
{
    const QString trimmed = urlOrPhone.trimmed();
    if (trimmed.isEmpty()) {
        return std::nullopt;
    }
    if (trimmed.contains(u"://"_s) || trimmed.startsWith(u"wa.me"_s, Qt::CaseInsensitive)) {
        const QUrl url = QUrl::fromUserInput(trimmed);
        return parseChatLink(url);
    }
    const QString digits = normalizePhone(trimmed);
    if (digits.isEmpty()) {
        return std::nullopt;
    }
    return NewChatRequest{.phone = digits, .text = {}};
}

QUrl newChatUrl(const NewChatRequest& request)
{
    QUrl url(u"https://web.whatsapp.com/send"_s);
    QUrlQuery query;
    query.addQueryItem(u"phone"_s, normalizePhone(request.phone));
    if (!request.text.isEmpty()) {
        query.addQueryItem(u"text"_s, QString::fromUtf8(QUrl::toPercentEncoding(request.text)));
    }
    url.setQuery(query);
    return url;
}

} // namespace whatsie::core
