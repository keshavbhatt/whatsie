#include "moreapps.h"
#include "ui_moreapps.h"

#include <QDesktopServices>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPushButton>
#include <QRandomGenerator>
#include <algorithm>
#include <cstdlib>
#include <ctime>

MoreApps::MoreApps(QWidget *parent, QNetworkAccessManager *nam,
                   const QString &publisherName, const QUrl &remoteFilterUrl,
                   bool uIdebugMode, bool remoteIconPreCaching)
    : QWidget(parent), ui(new Ui::MoreApps),
      mFields("publisher,summary,title,media"), mPublisherName(publisherName),
      mRemoteFilterUrl(remoteFilterUrl), mUiDebugMode(uIdebugMode),
      mRemoteIconPreCaching(remoteIconPreCaching) {

  init(nam);
}

void MoreApps::initNetworkManager(QNetworkAccessManager *nam) {

  ui->setupUi(this);

  // set to use external nam
  mNetworkManager = nam;

  QString diskCachePath =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

  // use internal nam
  if (mNetworkManager == nullptr) {
    mNetworkManager = new QNetworkAccessManager(this);
    mNetworkManager->setObjectName("internal_nm");

    QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(diskCachePath);
    mNetworkManager->setCache(diskCache);
  } else {
    qDebug() << QT_STRINGIFY(MoreApps) << "using external network manager";
  }

  if (mNetworkManager->cache() == nullptr) {
    QString nmType = (mNetworkManager->objectName() == "internal_nm")
                         ? "internal"
                         : "external";
    qWarning() << "no cache set on" << nmType << "network manager"
               << Q_FUNC_INFO;
  }
}

void MoreApps::init(QNetworkAccessManager *nam) {

  initNetworkManager(nam);

  getAppsMeta();
}

void MoreApps::getAppsMeta() {

  qDebug() << "getting apps meta...";

  QUrlQuery query;
  query.addQueryItem("q", mPublisherName);
  query.addQueryItem("fields", mFields);

  // TODO: check if snapcraft api provinding list by publisher name
  QUrl reqUrl = QUrl("https://api.snapcraft.io/v2/snaps/find");
  reqUrl.setQuery(query);

  QNetworkRequest request(reqUrl);
  request.setRawHeader("Snap-Device-Series", "16");
  request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                       QNetworkRequest::PreferCache);

  QNetworkReply *reply = mNetworkManager->get(request);
  ui->loadingLabel->show();
  connect(reply, &QNetworkReply::finished, this, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
      mAppMeta = reply->readAll();
    } else {
      qDebug() << "Error getting app meta from store";
      this->hide();
    }
    reply->deleteLater();
    applyFilter();
    ui->loadingLabel->hide();
  });
}

QList<AppItem> MoreApps::prepareAppsToShow(const QByteArray &bytes) {

  QList<AppItem> appList;

  QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes);
  if (jsonResponse.isEmpty()) {
    return appList;
  }

  QJsonArray jsonArray = jsonResponse.object().value("results").toArray();
  foreach (const QJsonValue &val, jsonArray) {
    QJsonObject object = val.toObject();

    // publisher
    QString publisher = object.value("snap")
                            .toObject()
                            .value("publisher")
                            .toObject()
                            .value("username")
                            .toString();
    publisher = publisher.isEmpty() ? "-" : publisher;

    // name
    QString name = object.value("name").toString();

    // title
    QString title = object.value("snap").toObject().value("title").toString();
    title = title.isEmpty() ? "-" : title;

    // icon
    QJsonArray mediaArr =
        object.value("snap").toObject().value("media").toArray();
    QString iconUrl;
    foreach (const QJsonValue &mediaItem, mediaArr) {
      if (mediaItem.toObject().value("type") == "icon")
        iconUrl = mediaItem.toObject().value("url").toString();
    }

    // summary
    QString summary =
        object.value("snap").toObject().value("summary").toString();
    summary = summary.isEmpty() ? "-" : summary;

    // link
    QString storeUrl = "https://snapcraft.io/" + name;

    // if filterList is set show only filtered items
    if (mFilterList.isEmpty() == false && (publisher == mPublisherName) &&
        (!name.isEmpty() || !storeUrl.isEmpty() || !iconUrl.isEmpty())) {
      if (mFilterList.contains(name)) {
        AppItem app(name, title, summary, QUrl(iconUrl), QUrl(storeUrl));
        appList.append(app);
      }
    } else { // else show all items returned
      AppItem app(name, title, summary, QUrl(iconUrl), QUrl(storeUrl));
      appList.append(app);
    }
  }

  return appList;
}

void MoreApps::applyFilter() {

  if (mRemoteFilterUrl.isEmpty() == false) {

    applyRemoteFilter(mRemoteFilterUrl);

  } else {

    mAppList = prepareAppsToShow(mAppMeta);

    showApps();
  }
}

void MoreApps::applyRemoteFilter(const QUrl &remoteFilterUrl) {

  qDebug() << "getting remote filter...";

  QNetworkRequest request(remoteFilterUrl);
  request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                       QNetworkRequest::PreferCache);

  QNetworkReply *reply = mNetworkManager->get(QNetworkRequest(request));
  ui->loadingLabel->show();

  connect(reply, &QNetworkReply::finished, this, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
      auto replyBytes = reply->readAll();
      foreach (QString line, replyBytes.split('\n')) {
        // ignore commented line
        if (line.startsWith("#")) {
          continue;
        }
        addToAppFilterList(line.trimmed());
      }
    } else {
      qDebug() << "Error getting filter list";
    }

    ui->loadingLabel->hide();

    reply->deleteLater();

    mAppList = prepareAppsToShow(mAppMeta);

    showApps();
  });
}

void MoreApps::setRemoteIcon(const QUrl &iconUrl, QLabel *lb) {

  QNetworkRequest request(iconUrl);
  request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                       QNetworkRequest::PreferCache);

  QNetworkReply *reply = mNetworkManager->get(QNetworkRequest(request));
  connect(reply, &QNetworkReply::finished, this, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
      if (lb != nullptr) {
        auto replyBytes = reply->readAll();
        QPixmap pixmap;
        pixmap.loadFromData(replyBytes);
        qDebug() << "after load" << lb->size();
        lb->setPixmap(pixmap.scaled(lb->size(), Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));
      }
    } else {
      qDebug() << "Error getting icon" << iconUrl.toString();
      if (lb != nullptr) {
        QByteArray data = QByteArray::fromBase64(mSnapIconBin.toLatin1());
        QPixmap pixmap;
        pixmap.loadFromData(data, "PNG");
        lb->setPixmap(QPixmap(pixmap.scaled(lb->size(), Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation)));
      }
    }
    reply->deleteLater();
  });
}

void MoreApps::showApps() {

  qDebug() << "showing apps...";

  std::srand(unsigned(std::time(0)));
  // rand generator
  auto rand = [](auto i) { return std::rand() % i; };
  // shuffle appList before adding
  std::random_shuffle(mAppList.begin(), mAppList.end(), rand);

  auto fallbackIconUrl =
      QUrl("https://dashboard.snapcraft.io/site_media/appmedia/"
           "2019/09/snapd.png");

  if (mRemoteIconPreCaching) {
    // cache fallback icon
    setRemoteIcon(fallbackIconUrl, nullptr);
    foreach (auto a, mAppList) {
      auto iconUrl = a.getIconUrl();
      qDebug() << "pre-caching icon for" << a.getName();
      setRemoteIcon(iconUrl, nullptr);
    }
  }

  // calculate icon height width
  double pheight = this->height();
  double ratio = pheight / pheight;
  double height = this->height() / 1.8;
  double width = ratio * height;

  // add appItem to appItemWidget
  for (int i = 0; i < getAppsToShowCount(); ++i) {

    auto appItem = mAppList.at(i);

    QLabel *iconLabel = new QLabel();
    iconLabel->setFixedSize(width, height);
    if (appItem.getIconUrl().isEmpty() == false) {
      setRemoteIcon(appItem.getIconUrl(), iconLabel);
    } else {
      qDebug() << "icon empty for " << appItem.getName();
      setRemoteIcon(fallbackIconUrl, iconLabel);
    }

    QPushButton *pb = new QPushButton();
    pb->setText(appItem.getTitle());
    connect(pb, &QPushButton::clicked, this,
            [=]() { QDesktopServices::openUrl(appItem.getStoreUrl()); });

    QVBoxLayout *vl = new QVBoxLayout();
    vl->addWidget(iconLabel);
    vl->addWidget(pb);
    vl->setAlignment(iconLabel, Qt::AlignCenter);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(6);

    QWidget *appItemWidget = new QWidget();
    if (mUiDebugMode)
      appItemWidget->setStyleSheet("border: 1px solid red;");
    appItemWidget->setToolTip(appItem.getSummary());
    appItemWidget->setLayout(vl);
    appItemWidget->setMaximumWidth(
        getMaxWidth(iconLabel->sizeHint().width(), pb->sizeHint().width()) +
        (2 * 3));
    appItemWidget->setMinimumWidth(
        getMaxWidth(iconLabel->sizeHint().width(), pb->sizeHint().width()));
    appItemWidget->setMinimumHeight(iconLabel->sizeHint().height() +
                                    pb->sizeHint().height() + (2 * 3));
    ui->horizontalLayout->setSpacing(9);
    ui->horizontalLayout->addWidget(appItemWidget);
  }
}

int MoreApps::getMaxWidth(int x, int y) {
  int z = x;
  x > y ? z = x : z = y;
  return z;
}

int MoreApps::getAppsToShowCount() {

  int min = 0;
  int max = mAppList.count();
  int proposed = 3;

  if (max > min && max >= proposed) {
    return proposed;
  } else {
    return max;
  }
}

void MoreApps::addToAppFilterList(const QString &appName) {

  // filter out if appname string contains running application name substring
  // or appname is empty or already added to filter list
  if (mFilterList.contains(appName) == false &&
      appName.trimmed().isEmpty() == false &&
      appName.contains(QApplication::applicationName(), Qt::CaseInsensitive) ==
          false)
    mFilterList.append(appName);
}

MoreApps::~MoreApps() { delete ui; }

QString AppItem::getName() const { return name; }

void AppItem::setName(const QString &newName) { name = newName; }

QString AppItem::getSummary() const {
  return QString(summary).replace(" & ", " and ");
}

void AppItem::setSummary(const QString &newSummary) { summary = newSummary; }

QUrl AppItem::getIconUrl() const { return iconUrl; }

void AppItem::setIconUrl(const QUrl &newIconUrl) { iconUrl = newIconUrl; }

QUrl AppItem::getStoreUrl() const { return storeUrl; }

void AppItem::setStoreUrl(const QUrl &newLink) { storeUrl = newLink; }

QString AppItem::getTitle() const {
  return QString(title).replace(" & ", " and ");
}

void AppItem::setTitle(const QString &newTitle) { title = newTitle; }

AppItem::AppItem() {}

AppItem::AppItem(const QString &name, const QString &title,
                 const QString &summary, const QUrl &iconUrl,
                 const QUrl &storeUrl)
    : name(name), title(title), summary(summary), iconUrl(iconUrl),
      storeUrl(storeUrl) {}
