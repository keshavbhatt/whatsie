#include "utils.h"
#include "def.h"
#include <time.h>

Utils::Utils(QObject *parent) : QObject(parent) { setParent(parent); }

Utils::~Utils() { this->deleteLater(); }

// calculate dir size
quint64 Utils::dir_size(const QString &directory) {
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
QString Utils::refreshCacheSize(const QString cache_dir) {
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

bool Utils::delete_cache(const QString cache_dir) {
  bool deleted = QDir(cache_dir).removeRecursively();
  QDir(cache_dir).mkpath(cache_dir);
  return deleted;
}

// returns string with first letter capitalized
QString Utils::toCamelCase(const QString &s) {
  QStringList parts = s.split(' ', Qt::SkipEmptyParts);
  for (int i = 0; i < parts.size(); ++i)
    parts[i].replace(0, 1, parts[i][0].toUpper());
  return parts.join(" ");
}

QString Utils::generateRandomId(int length) {
  return genRand(length, false, true, false);
}

QString Utils::genRand(int length, bool useUpper, bool useLower,
                       bool useDigits) {
  QString possibleCharacters;
  if (useUpper) {
    possibleCharacters += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  }
  if (useLower) {
    possibleCharacters += "abcdefghijklmnopqrstuvwxyz";
  }
  if (useDigits) {
    possibleCharacters += "0123456789";
  }

  if (possibleCharacters.isEmpty()) {
    return genRand(length, true, true, true);
  }

  QByteArray randomBytes;
  randomBytes.resize(length);
  for (int i = 0; i < length; ++i) {
    randomBytes[i] =
        possibleCharacters[QRandomGenerator::securelySeeded().generate() %
                           possibleCharacters.length()]
            .toLatin1();
  }
  return QString::fromLatin1(randomBytes);
}

QString Utils::convertSectoDay(qint64 secs) {
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

/**
 * @brief Returns the full path for a given file or directory name within a
 * specified location.
 *
 * The method constructs the full path by concatenating the specified `pathname`
 * with the provided `standardLocation`. If `standardLocation` is not provided,
 * the default value is obtained from
 * `QStandardPaths::writableLocation(QStandardPaths::DataLocation)`.
 *
 * @param pathname The name of the file or directory within the specified
 * location.
 * @param standardLocation (optional) The base directory to prepend to
 * `pathname`. Default value:
 * QStandardPaths::writableLocation(QStandardPaths::DataLocation).
 *
 * @return The full path, including the `pathname` and the `standardLocation`,
 * separated by the appropriate directory separator. If the specified location
 * does not exist, the method creates it using QDir::mkpath. The returned path
 * ends with a directory separator.
 */
QString
Utils::returnPath(QString pathname,
                  QString standardLocation = QStandardPaths::writableLocation(
                      QStandardPaths::DataLocation)) {
  QChar sepe = QDir::separator();
  QDir d(standardLocation + sepe + pathname);
  d.mkpath(standardLocation + sepe + pathname);
  return standardLocation + sepe + pathname + sepe;
}

/**
 * @brief Encodes XML special characters in a given string and returns the
 * encoded version.
 *
 * The method iterates over each character in the input `encodeMe` string and
 * replaces XML special characters
 * ('&', '\'', '"', '<', and '>') with their corresponding XML entities
 * ('&amp;', '&apos;', '&quot;', '&lt;', and '&gt;'). The encoded string is then
 * returned.
 *
 * @param encodeMe The string to be encoded.
 * @return The encoded version of the input string, where XML special characters
 * are replaced with their corresponding entities.
 */
QString Utils::encodeXML(const QString &encodeMe) {
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

/**
 * @brief Decodes XML entities in a given string and returns the decoded
 * version.
 *
 * The method creates a copy of the input `decodeMe` string and replaces XML
 * entities
 * ('&amp;', '&apos;', '&quot;', '&lt;', and '&gt;') with their corresponding
 * characters. The decoded string is then returned.
 *
 * @param decodeMe The string to be decoded.
 * @return The decoded version of the input string, where XML entities are
 * replaced with their corresponding characters.
 */
QString Utils::decodeXML(const QString &decodeMe) {
  QString temp(decodeMe);

  temp.replace("&amp;", "&");
  temp.replace("&apos;", "'");
  temp.replace("&quot;", "\"");
  temp.replace("&lt;", "<");
  temp.replace("&gt;", ">");

  return temp;
}

/**
 * @brief Retrieves the installation type based on the environment variables and
 * heuristics.
 *
 * The method retrieves the value of the "INSTALL_TYPE" environment variable
 * using QProcessEnvironment. If the "INSTALL_TYPE" variable is empty, the
 * method performs heuristics to determine the installation type. It checks if
 * the "SNAP" environment variable is set, in which case the installation type
 * is considered as "snap". If the "SNAP" variable is not set, it checks if the
 * "FLATPAK_ID" environment variable is not empty, in which case the
 * installation type is considered as "flatpak".
 *
 * @return The installation type as a QString. It can be "snap", "flatpak", or
 * an empty string if the installation type could not be determined.
 */
QString Utils::getInstallType() {
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

  // Retrieve the value of "INSTALL_TYPE" environment variable
  QString installType = env.value("INSTALL_TYPE");

  // Try guessing if it is empty
  if (installType.isEmpty()) {
    if (env.contains("SNAP")) {
      installType = "snap";
    } else if (env.value("FLATPAK_ID").trimmed().isEmpty() == false) {
      installType = "flatpak";
    }
  }

  return installType.trimmed();
}

/**
 * @brief Retrieves debug information about the application.
 *
 * The method generates a formatted string containing various debug information
 * about the application. It includes information such as the application name,
 * version, source branch, commit hash, build datetime, Qt runtime version, Qt
 * compiled version, system information, and installation mode.
 *
 * @return A QString containing the debug information about the application.
 */
QString Utils::appDebugInfo() {
  QString installType = Utils::getInstallType();
  installType =
      "<li><b>" + QObject::tr("Install mode") + ":</b>        " +
      QString(installType.trimmed().isEmpty() ? "Unknown" : installType) +
      "</li>";

  QString applicationName = QApplication::applicationName();
  QString version = QString(VERSIONSTR);
  QString sourceBranch = QString(GIT_BRANCH);
  QString commitHash = QString(GIT_HASH);
  QString buildDatetime = QString::fromLatin1(BUILD_TIMESTAMP);
  QString qtRuntimeVersion = QString(qVersion());
  QString qtCompiledVersion = QString(QT_VERSION_STR);
  QString systemInfo = QSysInfo::prettyProductName();
  QString architecture = QSysInfo::currentCpuArchitecture();
  QString installMode = Utils::getInstallType();

  QStringList debugInfo;
  debugInfo << "<h3>" + applicationName + "</h3>"
            << "<ul>"
            << "<li><b>" + QObject::tr("Version") + ":</b>             " +
                   version + "</li>"
            << "<li><b>" + QObject::tr("Source Branch") + ":</b>             " +
                   sourceBranch + "</li>"
            << "<li><b>" + QObject::tr("Commit Hash") + ":</b>             " +
                   commitHash + "</li>"
            << "<li><b>" + QObject::tr("Build Datetime") + ":</b>          " +
                   buildDatetime + "</li>"
            << "<li><b>" + QObject::tr("Qt Runtime Version") + ":</b>   " +
                   qtRuntimeVersion + "</li>"
            << "<li><b>" + QObject::tr("Qt Compiled Version") + ":</b> " +
                   qtCompiledVersion + "</li>"
            << "<li><b>" + QObject::tr("System") + ":</b>              " +
                   systemInfo + "</li>"
            << "<li><b>" + QObject::tr("Architecture") + ":</b>        " +
                   architecture + "</li>"
            << "<li><b>" + QObject::tr("Install mode") + ":</b>        " +
                   QString(installMode.trimmed().isEmpty() ? "Unknown"
                                                           : installMode) +
                   "</li>"
            << "</ul>";

  return debugInfo.join("\n");
}

void Utils::DisplayExceptionErrorDialog(const QString &error_info) {
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
                << "\nVersion: " + QString(VERSIONSTR)
                << "\nSource Branch: " + QString(GIT_BRANCH)
                << "\nCommit Hash: " + QString(GIT_HASH)
                << "\nBuild Datetime: " + QString(BUILD_TIMESTAMP)
                << "\nQt Runtime Version: " + QString(qVersion())
                << "\nQt Compiled Version: " + QString(QT_VERSION_STR)
                << "\nSystem: " + QSysInfo::prettyProductName()
                << "\nArchitecture: " + QSysInfo::currentCpuArchitecture()
                << "\nInstall type: " + Utils::getInstallType();
  message_box.setDetailedText(detailed_text.join("\n"));
  message_box.exec();
}

/**
 * Returns the same number, but rounded to one decimal place
 *
 * @brief Utils::RoundToOneDecimal
 * @param number
 * @return
 */
float Utils::RoundToOneDecimal(float number) {
  return QString::number(number, 'f', 1).toFloat();
}

/**
 * Returns a value for the environment variable name passed;
 * if the env var isn't set, it returns an empty string
 * @brief Utils::GetEnvironmentVar
 * @param variable_name
 * @return
 */
QString Utils::GetEnvironmentVar(const QString &variable_name) {
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

/**
 * @brief Opens the specified URL in the default desktop application.
 *
 * The method attempts to open the URL using the `xdg-open` command-line tool.
 * If the `xdg-open` process fails to start, it falls back to
 * using `QDesktopServices` to open the URL. If both methods fail, a warning
 * message is printed to the console.
 *
 * @param str The URL to be opened.
 */
void Utils::desktopOpenUrl(const QString &filePathStr) {
  qDebug() << "Trying opening file using xdg-open" << filePathStr;

  QProcess *xdg_open = new QProcess;
  QObject::connect(xdg_open,
                   static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                       &QProcess::finished),
                   [=](int exitCode, QProcess::ExitStatus exitStatus) {
                     Q_UNUSED(exitCode);
                     xdg_open->close();
                     xdg_open->deleteLater();
                     if (exitStatus != QProcess::NormalExit) {
                       qWarning() << "Failed to open URL using xdg-open";

                       QUrl url = QUrl::fromLocalFile(filePathStr);
                       qDebug() << "Opening file using desktop-services"
                                << url.toString();
                       if (!QDesktopServices::openUrl(url)) {
                         qWarning()
                             << "Failed to open URL using desktop-services";
                       }
                     }
                   });

  xdg_open->start("xdg-open", {filePathStr});
}

bool Utils::isPhoneNumber(const QString &phoneNumber) {
  const QString phone = "^\\+(((\\d{2}))\\s?)?((\\d{2})|(\\((\\d{2})\\))\\s?)?("
                        "\\d{3,15})(\\-(\\d{3,15}))?$";
  static QRegularExpression reg(phone);
  return reg.match(phoneNumber).hasMatch();
}

QString Utils::detectDesktopEnvironment() {
    QString desktopEnv = qgetenv("XDG_CURRENT_DESKTOP");

    if (!desktopEnv.isEmpty()) {
        return desktopEnv;
    }

    QString wm = qgetenv("WINDOWMANAGER");
    QString session = qgetenv("DESKTOP_SESSION");

    if (!wm.isEmpty()) {
        return wm;
    }

    if (!session.isEmpty()) {
        return session;
    }

    return "Unknown Desktop Environment";
}
