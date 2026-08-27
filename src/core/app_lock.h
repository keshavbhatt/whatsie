#pragma once

#include <QByteArray>
#include <QString>

#include <chrono>

// App lock crypto and throttling (FEATURES P1, ADR-015). Pure; unit-tested.
namespace whatsie::core {

/// A stored passcode: PBKDF2-HMAC-SHA256 hash, its random salt, and the
/// iteration count used — so verification re-derives with the same parameters.
struct PasscodeRecord
{
    QByteArray salt;
    int iterations = 0;
    QByteArray hash;

    [[nodiscard]] bool isValid() const { return iterations > 0 && !salt.isEmpty() && !hash.isEmpty(); }
};

/// Default PBKDF2 iteration count for new passcodes.
inline constexpr int kDefaultLockIterations = 210000;

/// Derives a passcode record with a fresh random salt.
[[nodiscard]] PasscodeRecord makePasscode(const QString& passcode, int iterations = kDefaultLockIterations);

/// Derives with a caller-supplied salt (deterministic; for verification/tests).
[[nodiscard]] QByteArray derivePasscodeHash(const QString& passcode, const QByteArray& salt, int iterations);

/// Constant-time verification against a stored record.
[[nodiscard]] bool verifyPasscode(const QString& passcode, const PasscodeRecord& record);

/// Escalating lock-out after N consecutive wrong entries (attempt throttling).
/// 0 for the first few, then growing delays capped at five minutes.
[[nodiscard]] std::chrono::milliseconds lockoutDuration(int consecutiveFailures);

} // namespace whatsie::core
