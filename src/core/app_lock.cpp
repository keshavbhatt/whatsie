#include "core/app_lock.h"

#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>

#include <algorithm>
#include <array>

namespace whatsie::core {

namespace {
constexpr int kSaltBytes = 16;
constexpr int kKeyBytes = 32;

bool constantTimeEquals(const QByteArray& a, const QByteArray& b)
{
    if (a.size() != b.size()) {
        return false; // length of a fixed-size derived key is not secret
    }
    quint8 diff = 0;
    for (int i = 0; i < a.size(); ++i) {
        diff |= static_cast<quint8>(a.at(i)) ^ static_cast<quint8>(b.at(i));
    }
    return diff == 0;
}
} // namespace

QByteArray derivePasscodeHash(const QString& passcode, const QByteArray& salt, int iterations)
{
    // PBKDF2-HMAC-SHA256 with a single output block (dkLen == SHA-256 size),
    // built on QtCore's HMAC so `core` needs no QtNetwork (QPasswordDigestor
    // lives there). RFC 2898: T1 = U1 ^ U2 ^ ... ^ Uc.
    const QByteArray key = passcode.toUtf8();
    QByteArray block = salt;
    const std::array<char, 4> indexBe{0, 0, 0, 1}; // INT_32_BE(1)
    block.append(indexBe.data(), indexBe.size());

    QByteArray u = QMessageAuthenticationCode::hash(block, key, QCryptographicHash::Sha256);
    QByteArray result = u;
    for (int i = 1; i < iterations; ++i) {
        u = QMessageAuthenticationCode::hash(u, key, QCryptographicHash::Sha256);
        for (int j = 0; j < result.size(); ++j) {
            result[j] = static_cast<char>(result.at(j) ^ u.at(j));
        }
    }
    return result.left(kKeyBytes);
}

PasscodeRecord makePasscode(const QString& passcode, int iterations)
{
    QByteArray salt(kSaltBytes, Qt::Uninitialized);
    QRandomGenerator::system()->fillRange(reinterpret_cast<quint32*>(salt.data()),
                                          kSaltBytes / int(sizeof(quint32)));
    PasscodeRecord record;
    record.salt = salt;
    record.iterations = iterations;
    record.hash = derivePasscodeHash(passcode, salt, iterations);
    return record;
}

bool verifyPasscode(const QString& passcode, const PasscodeRecord& record)
{
    if (!record.isValid() || passcode.isEmpty()) {
        return false;
    }
    return constantTimeEquals(derivePasscodeHash(passcode, record.salt, record.iterations), record.hash);
}

std::chrono::milliseconds lockoutDuration(int consecutiveFailures)
{
    using namespace std::chrono;
    if (consecutiveFailures < 4) {
        return 0ms;
    }
    static constexpr seconds kSteps[] = {5s, 15s, 30s, 60s, 120s, 300s};
    const int index = std::min<int>(consecutiveFailures - 4, std::size(kSteps) - 1);
    return duration_cast<milliseconds>(kSteps[index]);
}

} // namespace whatsie::core
