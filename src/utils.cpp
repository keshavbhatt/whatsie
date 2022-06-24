#include "utils.h"
#include "def.h"
#include <time.h>

utils::utils(QObject *parent) : QObject(parent) { setParent(parent); }

utils::~utils() { this->deleteLater(); }

// calculate dir size
quint64 utils::dir_size(const QString &directory) {
  quint64 sizex = 0;
  QFileInfo str_info(directory);
  if (str_info.isDir()) {
    QDir dir(directory);
    QFileInfoList list =
        dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::Hidden |
                          QDir::NoSymLinks | QDir::NoDotAndDotDot);
    for (int i = 0; i < list.size(); ++i) {
      QFileInfo fileInfo = list.at(i);
      if (fileInfo.isDir()) {
        sizex += dir_size(fileInfo.absoluteFilePath());
      } else {
        sizex += fileInfo.size();
      }
    }
  }
  return sizex;
}

// get the size of cache folder in human readble format
QString utils::refreshCacheSize(const QString cache_dir) {
  qint64 cache_size = dir_size(cache_dir);
  QString cache_unit;
  if (cache_size > 1024 * 1024 * 1024) {
    cache_size = cache_size / (1024 * 1024 * 1024);
    cache_unit = " GB";
  }
  if (cache_size > 1024 * 1024) {
    cache_size = cache_size / (1024 * 1024);
    cache_unit = " MB";
  } else if (cache_size > 1024) {
    cache_size = cache_size / (1024);
    cache_unit = " kB";
  } else {
    cache_unit = " B";
  }
  return QString::number(cache_size) + cache_unit;
}

bool utils::delete_cache(const QString cache_dir) {
  bool deleted = QDir(cache_dir).removeRecursively();
  QDir(cache_dir).mkpath(cache_dir);
  return deleted;
}

// returns string with first letter capitalized
QString utils::toCamelCase(const QString &s) {
  QStringList parts = s.split(' ', Qt::SkipEmptyParts);
  for (int i = 0; i < parts.size(); ++i)
    parts[i].replace(0, 1, parts[i][0].toUpper());
  return parts.join(" ");
}

QString utils::generateRandomId(int length) {

  QString str = QUuid::createUuid().toString();
  str.remove(QRegularExpression("{|}|-"));
  if (str.length() < length) {
    while (str.length() != length) {
      int required_char = length - str.length();
      str = str + str.append(genRand(required_char));
    }
  }
  if (str.length() > length) {
    while (str.length() != length) {
      str = str.remove(str.length() - 1, 1);
    }
  }
  return str;
}

QString utils::genRand(int length) {
  QDateTime cd = QDateTime::currentDateTime();
  const QString possibleCharacters(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" +
      QString::number(cd.currentMSecsSinceEpoch())
          .remove(QRegExp("[^a-zA-Z\\d\\s]")));

  const int randomStringLength = length;
  QString randomString;
  int rand = QRandomGenerator::global()->generate();
  for (int i = 0; i < randomStringLength; ++i) {
    int index = rand % possibleCharacters.length();
    QChar nextChar = possibleCharacters.at(index);
    randomString.append(nextChar);
  }
  randomString = randomString.trimmed().simplified().remove(" ");
  return randomString;
}

QString utils::convertSectoDay(qint64 secs) {
  int day = secs / (24 * 3600);

  secs = secs % (24 * 3600);
  int hour = secs / 3600;

  secs %= 3600;
  int minutes = secs / 60;

  secs %= 60;
  int seconds = secs;

  QString days = QString::number(day) + " " + "days " + QString::number(hour) +
                 " " + "hours " + QString::number(minutes) + " " + "minutes " +
                 QString::number(seconds) + " " + "seconds ";
  return days;
}

// static on demand path maker
QString utils::returnPath(QString pathname,QString standardLocation = QStandardPaths::writableLocation(
            QStandardPaths::DataLocation)) {
    QChar sepe = QDir::separator();
    QDir d(standardLocation + sepe + pathname);
    d.mkpath(standardLocation + sepe + pathname);
  return standardLocation + sepe + pathname + sepe;
}

QString utils::EncodeXML(const QString &encodeMe) {

  QString temp;
  int length = encodeMe.size();
  for (int index = 0; index < length; index++) {
    QChar character(encodeMe.at(index));

    switch (character.unicode()) {
    case '&':
      temp += "&amp;";
      break;

    case '\'':
      temp += "&apos;";
      break;

    case '"':
      temp += "&quot;";
      break;

    case '<':
      temp += "&lt;";
      break;

    case '>':
      temp += "&gt;";
      break;

    default:
      temp += character;
      break;
    }
  }

  return temp;
}

QString utils::DecodeXML(const QString &decodeMe) {

  QString temp(decodeMe);

  temp.replace("&amp;", "&");
  temp.replace("&apos;", "'");
  temp.replace("&quot;", "\"");
  temp.replace("&lt;", "<");
  temp.replace("&gt;", ">");

  return temp;
}

QString utils::htmlToPlainText(QString str) {
  QString out;
  QTextDocument text;
  text.setHtml(str);
  out = text.toPlainText();
  text.deleteLater();
  return out.replace("\\\"", "'")
      .replace("&amp;", "&")
      .replace("&gt;", ">")
      .replace("&lt;", "<")
      .replace("&#39;", "'");
}

QString utils::appDebugInfo() {

  QStringList debugInfo;
  debugInfo << "<h3>" + QApplication::applicationName() + "</h3>"
            << "<ul>"
            << "<li><b>" + QObject::tr("Version") + ":</b>             " +
                   QString(VERSIONSTR) + "</li>"
            << "<li><b>" + QObject::tr("Source Branch") + ":</b>             " +
                   QString(GIT_BRANCH) + "</li>"
            << "<li><b>" + QObject::tr("Commit Hash") + ":</b>             " +
                   QString(GIT_HASH) + "</li>"
            << "<li><b>" + QObject::tr("Build Datetime") + ":</b>          " +
                   QString::fromLatin1(BUILD_TIMESTAMP) + "</li>"
            << "<li><b>" + QObject::tr("Qt Runtime Version") + ":</b>   " +
                   QString(qVersion()) + "</li>"
            << "<li><b>" + QObject::tr("Qt Compiled Version") + ":</b> " +
                   QString(QT_VERSION_STR) + "</li>"
            << "<li><b>" + QObject::tr("System") + ":</b>              " +
                   QSysInfo::prettyProductName() + "</li>"
            << "<li><b>" + QObject::tr("Architecture") + ":</b>        " +
                   QSysInfo::currentCpuArchitecture() + "</li>";
  debugInfo << "</ul>";
  return debugInfo.join("\n");
}

void utils::DisplayExceptionErrorDialog(const QString &error_info) {
  QMessageBox message_box(QApplication::activeWindow());
  message_box.setAttribute(Qt::WA_DeleteOnClose, true);
  message_box.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
  message_box.setModal(true);
  message_box.setIcon(QMessageBox::Critical);
  message_box.setWindowTitle(QApplication::applicationName() +
                             QObject::tr("Exception"));
  // spaces are added to the end because otherwise the dialog is too small
  message_box.setText(QApplication::applicationName() +
                      QObject::tr(" has encountered a problem."));
  message_box.setInformativeText(
      QApplication::applicationName() +
      QObject::tr(
          " may need to Restart. Please report the error to developer."));
  message_box.setStandardButtons(QMessageBox::Close);
  QStringList detailed_text;
  detailed_text << "Error info: " + error_info
                << "\nApp version: " + QString(VERSIONSTR)
                << "\nSource Branch: " + QString(GIT_BRANCH)
                << "\nCommit Hash: " + QString(GIT_HASH)
                << "\nBuild Datetime: " + QString(BUILD_TIMESTAMP)
                << "\nQt Runtime Version: " + QString(qVersion())
                << "\nQt Compiled Version: " + QString(QT_VERSION_STR)
                << "\nSystem: " + QSysInfo::prettyProductName()
                << "\nArchitecture: " + QSysInfo::currentCpuArchitecture();
  message_box.setDetailedText(detailed_text.join("\n"));
  message_box.exec();
}

// Returns the same number, but rounded to one decimal place
float utils::RoundToOneDecimal(float number) {
  return QString::number(number, 'f', 1).toFloat();
}

// Returns a value for the environment variable name passed;
// if the env var isn't set, it returns an empty string
QString utils::GetEnvironmentVar(const QString &variable_name) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
  // The only time this might fall down is on Linux when an
  // environment variable holds bytedata. Don't use this
  // utility function for retrieval if that's the case.
  return qEnvironmentVariable(variable_name.toUtf8().constData(), "").trimmed();
#else
  // This will typically only be used on older Qts on Linux
  return QProcessEnvironment::systemEnvironment()
      .value(variable_name, "")
      .trimmed();
#endif
}

void utils::desktopOpenUrl(const QString str) {
  QProcess *xdg_open = new QProcess(0);
  xdg_open->start("xdg-open", QStringList() << str);
  if (xdg_open->waitForStarted(1000) == false) {
    // try using QdesktopServices
    bool opened = QDesktopServices::openUrl(QUrl(str));
    if (opened == false) {
      qWarning() << "failed to open url" << str;
    }
  }
  connect(xdg_open,
          static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
              &QProcess::finished),
          [xdg_open](int exitCode, QProcess::ExitStatus exitStatus) {
            Q_UNUSED(exitCode);
            Q_UNUSED(exitStatus);
            xdg_open->close();
            xdg_open->deleteLater();
          });
}
