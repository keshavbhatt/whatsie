#include "core/log_sink.h"

#include <QFile>
#include <QLoggingCategory>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;
using whatsie::core::LogSink;

Q_LOGGING_CATEGORY(lcTest, "whatsie.test")

class TestLogSink : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        // QTest installs its own handler; ours must chain to it.
        LogSink::install();
        LogSink::install(); // idempotent
    }

    void ringBufferKeepsFormattedLines()
    {
        qCInfo(lcTest) << "hello ring";
        const QStringList lines = LogSink::recentLines();
        QVERIFY(!lines.isEmpty());
        const QString last = lines.last();
        QVERIFY(last.contains(u"[whatsie.test]"_s));
        QVERIFY(last.contains(u"info: hello ring"_s));
    }

    void writesToFileAndRotates()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(u"sub/whatsie.log"_s);

        LogSink::setLogFile(path, 200);
        QCOMPARE(LogSink::logFilePath(), path);
        qCWarning(lcTest) << "to file";
        QVERIFY(QFile::exists(path));
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            QVERIFY(QString::fromUtf8(f.readAll()).contains(u"warning: to file"_s));
        }

        // Push past 200 bytes → rotation to .1
        for (int i = 0; i < 10; ++i) {
            qCInfo(lcTest) << "padding line number" << i << "with some extra words";
        }
        QVERIFY(QFile::exists(path + u".1"_s));

        LogSink::setLogFile(QString());
        QVERIFY(LogSink::logFilePath().isEmpty());
        qCInfo(lcTest) << "after close"; // must not crash
    }

    void unwritablePathIsIgnored()
    {
        LogSink::setLogFile(u"/proc/definitely/not/writable/whatsie.log"_s);
        QVERIFY(LogSink::logFilePath().isEmpty());
        qCInfo(lcTest) << "still fine";
    }

    void ringIsCapped()
    {
        // info, not debug: the test environment disables whatsie.*.debug.
        for (int i = 0; i < 1200; ++i) {
            qCInfo(lcTest) << "line" << i;
        }
        QCOMPARE(LogSink::recentLines().size(), 1000);
    }
};

QTEST_GUILESS_MAIN(TestLogSink)
#include "tst_log_sink.moc"
