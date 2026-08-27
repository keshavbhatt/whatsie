#include "core/downloads/download_model.h"
#include "core/downloads/file_naming.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;
using namespace whatsie::core;

class TestDownloadModel : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void addsNewestFirstAndAssignsIds()
    {
        DownloadModel model;
        QSignalSpy changed(&model, &DownloadModel::changed);
        DownloadEntry a;
        a.fileName = u"a.jpg"_s;
        a.directory = u"/tmp"_s;
        const quint64 idA = model.add(a);
        DownloadEntry b;
        b.fileName = u"b.pdf"_s;
        b.directory = u"/tmp"_s;
        const quint64 idB = model.add(b);
        QVERIFY(idB > idA);
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.data(model.index(0), DownloadModel::FileNameRole).toString(), u"b.pdf"_s);
        QCOMPARE(model.data(model.index(0), DownloadModel::FilePathRole).toString(), u"/tmp/b.pdf"_s);
        QCOMPARE(model.activeCount(), 2);
        QCOMPARE(changed.count(), 2);
        QVERIFY(model.entry(idA)->startedAt.isValid());
    }

    void progressAndFinish()
    {
        DownloadModel model;
        DownloadEntry e;
        e.fileName = u"x"_s;
        e.directory = u"/tmp"_s;
        const quint64 id = model.add(e);
        QSignalSpy data(&model, &QAbstractItemModel::dataChanged);
        QSignalSpy finished(&model, &DownloadModel::entryFinished);

        model.updateProgress(id, 500, 1000, 250.0);
        QCOMPARE(data.count(), 1);
        QCOMPARE(model.entry(id)->receivedBytes, 500);
        QCOMPARE(model.entry(id)->bytesPerSecond, 250.0);

        model.finish(id, DownloadState::Completed);
        QCOMPARE(finished.count(), 1);
        QCOMPARE(model.entry(id)->state, DownloadState::Completed);
        QVERIFY(model.entry(id)->finishedAt.isValid());
        QCOMPARE(model.entry(id)->bytesPerSecond, 0.0);
        QCOMPARE(model.activeCount(), 0);

        model.finish(id, DownloadState::Failed, u"late"_s); // already finished: ignored
        QCOMPARE(model.entry(id)->state, DownloadState::Completed);
        QCOMPARE(finished.count(), 1);

        model.updateProgress(999, 1, 1, 0); // unknown id: no crash
        model.finish(999, DownloadState::Cancelled);
    }

    void completedWithUnknownTotalUsesReceived()
    {
        DownloadModel model;
        DownloadEntry e;
        e.fileName = u"x"_s;
        e.totalBytes = -1;
        const quint64 id = model.add(e);
        model.updateProgress(id, 4242, -1, 0);
        model.finish(id, DownloadState::Completed);
        QCOMPARE(model.entry(id)->totalBytes, 4242);
    }

    void removeAndClearFinished()
    {
        DownloadModel model;
        DownloadEntry e;
        e.fileName = u"x"_s;
        const quint64 active = model.add(e);
        const quint64 done = model.add(e);
        const quint64 gone = model.add(e);
        model.finish(done, DownloadState::Completed);
        model.finish(gone, DownloadState::Cancelled);
        model.remove(gone);
        QCOMPARE(model.rowCount(), 2);
        model.clearFinished();
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.entries().first().id, active);
        model.remove(12345); // unknown
        QCOMPARE(model.rowCount(), 1);
    }

    void historyIsBoundedKeepingActive()
    {
        DownloadModel model;
        DownloadEntry e;
        e.fileName = u"x"_s;
        const quint64 active = model.add(e);
        for (int i = 0; i < DownloadModel::kMaxEntries + 20; ++i) {
            const quint64 id = model.add(e);
            model.finish(id, DownloadState::Completed);
        }
        QCOMPARE(model.rowCount(), DownloadModel::kMaxEntries);
        QVERIFY(model.entry(active).has_value());
    }

    void persistsAndMarksInterrupted()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(u"downloads.json"_s);
        quint64 doneId = 0;
        quint64 activeId = 0;
        {
            DownloadModel model;
            DownloadEntry e;
            e.fileName = u"photo.jpg"_s;
            e.directory = u"/tmp/dl"_s;
            e.mimeType = u"image/jpeg"_s;
            e.totalBytes = 10;
            doneId = model.add(e);
            model.updateProgress(doneId, 10, 10, 0);
            model.finish(doneId, DownloadState::Completed);
            e.fileName = u"video.mp4"_s;
            activeId = model.add(e);
            model.updateProgress(activeId, 3, 10, 99.0);
            QVERIFY(model.save(path));
        }
        DownloadModel reloaded;
        QVERIFY(reloaded.load(path));
        QCOMPARE(reloaded.rowCount(), 2);
        const auto done = reloaded.entry(doneId);
        QVERIFY(done.has_value());
        QCOMPARE(done->state, DownloadState::Completed);
        QCOMPARE(done->mimeType, u"image/jpeg"_s);
        QCOMPARE(done->filePath(), u"/tmp/dl/photo.jpg"_s);
        const auto active = reloaded.entry(activeId);
        QCOMPARE(active->state, DownloadState::Failed);
        QCOMPARE(active->error, u"Interrupted"_s);
        QCOMPARE(active->bytesPerSecond, 0.0);
        // New ids continue after the loaded ones.
        DownloadEntry e;
        e.fileName = u"n"_s;
        QVERIFY(reloaded.add(e) > activeId);
    }

    void loadToleratesMissingAndGarbage()
    {
        QTemporaryDir dir;
        DownloadModel model;
        QVERIFY(model.load(dir.filePath(u"nope.json"_s)));
        const QString bad = dir.filePath(u"bad.json"_s);
        {
            QFile f(bad);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("{ not json");
        }
        QVERIFY(!model.load(bad));
        QCOMPARE(model.rowCount(), 0);
    }

    void uniqueNames()
    {
        QSet<QString> taken{u"/d/photo.jpg"_s, u"/d/photo (1).jpg"_s, u"/d/README"_s};
        auto exists = [&](const QString& p) { return taken.contains(p); };
        QCOMPARE(uniqueFileName(u"/d"_s, u"photo.jpg"_s, exists), u"photo (2).jpg"_s);
        QCOMPARE(uniqueFileName(u"/d"_s, u"other.jpg"_s, exists), u"other.jpg"_s);
        QCOMPARE(uniqueFileName(u"/d"_s, u"README"_s, exists), u"README (1)"_s);
        QCOMPARE(uniqueFileName(u"/d"_s, u"a.tar.gz"_s,
                                [](const QString& p) { return p.endsWith(u"a.tar.gz"_s); }),
                 u"a.tar (1).gz"_s);
    }

    void sanitizesNames()
    {
        QCOMPARE(sanitizeFileName(u"../../etc/passwd"_s), u"_.._etc_passwd"_s);
        QCOMPARE(sanitizeFileName(u"  a:b\\c  "_s), u"a_b_c"_s);
        QCOMPARE(sanitizeFileName(u".hidden"_s), u"hidden"_s);
        QCOMPARE(sanitizeFileName(QString()), u"download"_s);
        QCOMPARE(sanitizeFileName(u"ok name.pdf"_s), u"ok name.pdf"_s);
    }

    void humanSizes()
    {
        QCOMPARE(humanSize(-1), QString());
        QCOMPARE(humanSize(0), u"0 B"_s);
        QCOMPARE(humanSize(1023), u"1023 B"_s);
        QCOMPARE(humanSize(1024), u"1 KB"_s);
        QCOMPARE(humanSize(1536), u"2 KB"_s);
        QCOMPARE(humanSize(1024 * 1024), u"1.0 MB"_s);
        QCOMPARE(humanSize(qint64{3} * 1024 * 1024 * 1024 + 512 * 1024 * 1024), u"3.5 GB"_s);
    }
};

QTEST_GUILESS_MAIN(TestDownloadModel)
#include "tst_download_model.moc"
