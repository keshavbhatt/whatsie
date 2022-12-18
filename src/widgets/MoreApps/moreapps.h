#ifndef MOREAPPS_H
#define MOREAPPS_H

#include <QLabel>
#include <QPair>
#include <QWidget>
#include <QtNetwork>

namespace Ui {
class MoreApps;
}

class AppItem {
public:
  AppItem();
  AppItem(const QString &name, const QString &title, const QString &summary,
          const QUrl &iconUrl, const QUrl &storeUrl);

  QString getName() const;
  void setName(const QString &newName);

  QString getSummary() const;
  void setSummary(const QString &newSummary);

  QUrl getIconUrl() const;
  void setIconUrl(const QUrl &newIconUrl);

  QUrl getStoreUrl() const;
  void setStoreUrl(const QUrl &newLink);

  QString toString() const {
    return QString("name: %1\ntitle: %2\nshortDescription: %3\niconUrl: "
                   "%4\nlink: %5")
        .arg(this->getName(), this->getTitle(), this->getSummary(),
             this->getIconUrl().toString(), this->getStoreUrl().toString());
  }

  QString getTitle() const;
  void setTitle(const QString &newTitle);

private:
  QString name;
  QString title;
  QString summary;
  QUrl iconUrl;
  QUrl storeUrl;
};

class MoreApps : public QWidget {
  Q_OBJECT

public:
  MoreApps(QWidget *parent = nullptr, QNetworkAccessManager *nam = nullptr,
           const QString &publisherName = "",
           const QUrl &remoteFilterUrl = QUrl(""), bool uIdebugMode = false,
           bool remoteIconPreCaching = true);
  ~MoreApps();

private slots:
  void getAppsMeta();

  void applyFilter();

  void addToAppFilterList(const QString &appName);

  QList<AppItem> prepareAppsToShow(const QByteArray &bytes);

  void showApps();

  void setRemoteIcon(const QUrl &iconUrl, QLabel *lb = nullptr);

  int getAppsToShowCount();

  void init(QNetworkAccessManager *nam = nullptr);

  void initNetworkManager(QNetworkAccessManager *nam = nullptr);

  void applyRemoteFilter(const QUrl &remoteFilterUrl);

  int getMaxWidth(int x, int y);
private:
  Ui::MoreApps *ui;
  QString mFields;
  QString mPublisherName;
  QUrl mRemoteFilterUrl;
  bool mUiDebugMode;
  bool mRemoteIconPreCaching;

  QNetworkAccessManager *mNetworkManager = nullptr;
  QList<AppItem> mAppList;
  QStringList mFilterList;
  QByteArray mAppMeta;
  QString mSnapIconBin =
      "iVBORw0KGgoAAAANSUhEUgAAAQAAAAEACAYAAABccqhmAAAABmJLR0QAnQCbAJtGTkUDAAAA"
      "CXBIWXMAAA3XAAAN1wFCKJt4AAAAB3RJTUUH5gwSEQYxKUlO0QAAGhhJREFUeNrtnXucHGWd"
      "r59f9SQhCUKm660ZbupZd+Xo2RUMd1nRs15WQFA5rnKT+"
      "01AJYAY8IIieEHuIIiKCyuuCCThDuLiBUEUFVzPcdeju5496wpKd1VPgACZZLp++"
      "0f1HOdMZiaTTE9PX77P55M/"
      "+CRkOvXW96m3vl31viCEEEIIIYQQQoieoFqtRjoKQvQmlqbpX7v7fDN7EHguhJDrsAjRIwLI"
      "smxLd78S2Ae4A1gFPB5F0XC5XHYdIiG6WAAADQlcAxwGjAD/"
      "BKxw95Vm9m8hhPU6VEJ0qQAmkEAJcOB54GEzWwnc7e6VEEJdh02ILhPAJBIYpQ4MAfe7+"
      "61m9t0oitaUy2X1BUJ0iwA2IoFR1gNPAreb2Qp3/"
      "2m9Xh8eHBxUXyBEpwtgmhKgcYuwDvgnd18ZRdFtwG/"
      "iOF6nwypEBwtgjASuBg6fQgJjZfA88EMzuwW4O8/"
      "zapIkIzrEQnSgAADSNN0SmK4ExvcF3wJWRVH0APCs+"
      "gIhOkwADQksBj4PHLEJEhhlBHgCuMPMbgUey/"
      "N8bZIk6guE6AQBjJHAVcCRmyGBsX3Br4BbzWwV8K/"
      "qC4ToAAEAVKvVxWY2EwmM7wt+BKwA7gKeCiGoLxCiXQXQZAmMkgOrG33BCnd/"
      "oFQqqS8Qoh0F0JDAooYEjmqSBDboC4DbgEfdXX2BEO0kgFmWwCjrgP8N3ErxctK/"
      "hhDUFwjRDgJokQRG+4IXgEcbMrhTfYEQbSCAFkpgbF/wNHB/oy/"
      "4NvBMkiTqC4SYCwEAVCqVRVEUXQkc3QIJjO0L/"
      "tDoC1aoLxBijgQwhxIY3xesAlYCv1ZfIEQLBdAGEhjtC9Y2+oJbgDvd/"
      "Sm9jyBECwTQJhIY3xd8u9EX3G9mz2i9QyFmUQBtJoGJ+oJVZvajer3+wsDAgPoCIQHMxl/"
      "ahhIY2xf8S6MrWAH8Sn2BkAB6SwLj+4JVjdnBk3q+"
      "QEgAvSOB8X3Bd4Bb3f1bFM8XaPFTIQE0SQJXAMe0sQTG9gUVijcUb42i6If1ev0FPV8gJIDe"
      "kcD4vmAVsMrdf5kkybBOGSEB9I4ExvYFj1E8X3CHmT0Zx7H6AiEB9IgExvYFzwDfpVjZ6P48"
      "z59WXyAkgGny1FNPLSyVSpcCJ3SoBCbqC1ZEUfQI8IL2UxQSwMZnAgujKOoGCYztC/"
      "4Pf3y+"
      "4JchBPUFQgLogZnAZH3BCjO7w8yeKJfL2lxVSAA9IoGxfcGaRl9wc57n3yqVSqvjOFZfICSA"
      "UYaGhhbW6/VulcDYvqAK3NPoCx4GnldfIHpeAD0kgVHWN/"
      "qCFe6+0sz+ua+"
      "vb92SJUskA9GbAhgjgUuAE3tAAqN9wTDwj8AtZna7mf1OfYHoSQH0qAQm6gtWuvt9pVJpqFw"
      "uqy8QvSOAHpfAKHUgBe4Gbi2VSg+7u/"
      "oC0RsCkAQm7AtG1zv8Rb1eXzc4OCgZiO4VgCSw8b7A3X8XQlBfILpTAJLARvuCh4Bb3P1eMx"
      "sKIagvEN0lAElgk/"
      "qCle7+kJk9r8VPRdcIYIwELgZOkgSm7Av+L8XGqrcAv8jzfJ0WPxUdLwBJYLP7gpXA7cC/"
      "qy8QHS2AhgS2aNwOSALT7wueA77fmBXcC6gvEJ0pgHESOBHo0xBuUl9Qo/E+grs/"
      "qL5AWCd+6IYERm8HJIEZ9AXuvsrMfp7n+bD6AglAEui9vmAd8L8atwi3qS+"
      "QADpNAp8DTpYEmtYXPEyxP8I9ZpapL5AA2l4CeZ5f6O6nSAJN7wvuBW6lKBGfU18gAUgCvdk"
      "X/"
      "I7ifYQVwD+6+7A2S5EAdDvQu33BzWb29TiOn5zsD1er1YVAorGYMc8mSVKdrb+8awanv79/"
      "ba1W+1Ce5zlwqk68WblYLAB2A5a6ew5cOukfNqsDu1Cs8rR34/8Vm84/"
      "p2n630MIaySAjVAul9fWarXlhQMkgVkkavyalMa267enaXofsAdwCnAAsLibZp4tYGfgb4Ab"
      "Zmsg6TIJDEdRtBy4mmIxTjGHhBCGQwgPufsRZrYHcDnwB4pvHcT0LtKnpWm6pQSw6RK4RhJo"
      "DWmaWpqmL6lWq/"
      "Mn+"
      "v0kSUbiOP5lFEUfNLNdgLOBX2t8psVfuPs7JYDNk8DndZK17Er1BTO7r1qtvrNSqSweGhqyC"
      "cYlj+"
      "P498DFURTtQbFP5A8oCkYxybE1s2XVanVLCWATO4Eois6WBFrGAuANZnZTFEWP1Ov196Vpml"
      "Sr1WiCWwMvl8tPhxC+BrwR2I/iacQ1FN84iHGzAOB/"
      "SACbNxOQBFrLPGCnxv3+"
      "42Z2QbVafXm1Wi1N0RN8x90PB14DXEWxkYp6glmeBUS9cOQkgTkjAnYAzjazH5vZl7Ms2ytN"
      "00l7ghDCL9z9dGAp8FHgXyieTBTwKjM7SAKQBDoNA5YAx7j7d4E70jR9R5Zli2q1mk0ggjyE"
      "8ESe5591993d/XjgEfUE9AHLqtXqYglAEuhUtgD2pVjQ9JE8z0/"
      "Msqw8kQgGBgY8SZKnkyS5wczeCLyVYoWj53u4J9ipmbOAqNeO3hgJXCUJzHlPsDNwtbv/LM/"
      "z89I0fVmWZRP2BHEcrw0hPODu73b3vYEvUCyI2ms9QVNnAVEvnnkNCZwjCbQFJeAljfv9n7j"
      "7tWma7pam6bxJeoL1SZL83N0/AOwKfJxi85Re6gl2NrN3SACSQLf1BGXgeIrXj29P0/"
      "SAWq22cBIR1EMIv3X3T5nZ7sB7gZ/2SE/"
      "QB5xeqVQWSwAzlICZSQLtx0Jgf2BVnucPpml6Qpqm/"
      "WmaTlQYehzHtRDCdWb2OuAg4I5GT9DVs4Aoit4uAcyQOI4lgfbuCXZv3O8/"
      "BnysWq2+"
      "JE3TyXqCF0II97r7u9z9dcCXgCG6szDsA5ZVKpVFEoAk0As9wZ8AnzCzx4Gr0jRdupGe4DEz"
      "O8XMdgU+SbEIarf1BEtnOguQADaUwGWSQFv3BHHjfv9h4OZqtbp/lmVbTDKm9TiO/"
      "y2KovPcfTeKV8Qfo1jlqGu6gFqttkgCaJ4EPioJdIQIFgEHmdlt7v5gmqZH12q1rbMsm+"
      "gFJE+SJAshfLFUKr0OeCdwH7C2G2YBeZ6/TQJongTWSQIdxXyKBUeuy/P8MXc/"
      "p1qtvrhSqUx4bvf39z8fQrgLeAfweuBvgdUd3BPMaBYgAUwtgUslgY7qCf4UON/"
      "MfhpF0RVpmu40WU8QQlgXQvixu5/YuD34FPAfHdoTLM3z/"
      "EAJoPkS+"
      "Jgk0HFEwEDjfv8R4Btpmr4pTdMJ1yRMkqSeJMlvoig6t1EYngb8zw7rCeYBpw8NDS2SACQB8"
      "ceeYDHFO/T3AA+kaXpklmVbTfQ8Qblc9jiOq/V6/"
      "Zp6vb43cAhwfwf1BLvkeX6ABCAJiIl7gtcCX3H3nwBnZVm2bZZlG5z/"
      "g4ODPjg4+FwIYRXwNorFSv4OeKbNe4J57n76ZE9OSgCSgCgKsx2Bz7j7z9z94jRN/"
      "zzLsr4peoJH3P14d98d+BzwBO37AtKumzoLkAA2TwLaPLPze4JBYBnwI3e/"
      "MU3TN0zRE4wkSfJr4MMULyCdQbFBSrtdDOZRfCOwUAKQBMT0eoItG/f79wL/"
      "kGXZIdVqdcuJnicIIeQhhKfc/crGK8mHAd8Bhtvo37RbnudvlQBmVwLnSgJdxwJgH3e/"
      "0cx+"
      "7O7L0jTdZqKeIEkST5JkTQjhVnffH3gz8HXg2TboCTZpFiABSAJiw57glcDFwOPufmGapq9I"
      "07RvktuD4RDCQ2Z2lJntCVzC3G98snue5/"
      "tLAK2RwGWSQNf2BNsCZwKPAtenabrzFOfDSBzHvwSWNzY+"
      "WQ78ao56gnnA6VmWLZQA1AmImfcEWwH7mNlG8xJCyOM4/"
      "r27X2Jmu5vZkcCDc9AT7OHu+0oAuh0QM+ffzezdcRz/bLr/"
      "Q2OhkmfjOL7JzN5CsRDqzbRu45N5wBmTvSkpAcyOBC6RBLoy/"
      "AfHcfzjGZwfwyGE77n7eyg2PrkSqLSgJ9jT3feTAFongY9LAl3Fbxvhf7QZf9noxidmdoaZL"
      "QXOmeWeYKOzAAlAEhAT8x9A08I/7jzJ4zh+0t0vcvc9md0NUvecqguQACQBsSG/"
      "Aw4ZHh5+dDZ/SON5gqdDCF8zszdRbHyyCniuiT3BlLMACWB2JXCRJNB54Xf3g4eHh3+4/"
      "fbbewvPmbUhhAeAQ9z9L4Grad7GJ3u5+1skgBZLADgPuFAS6Jzw53l+"
      "SKvDP5YQwvokSX5OsS7BrsC5zHzjk9HnAraQAFo7mOuA8yWBjrnyH7J+/"
      "fpHdthhB2+DcycPIfw2z/NPu/uuwAnAj2bQE7yG4pFlCWCOJPBZSaBteWI0/"
      "HN15Z+Mxgapq0MI15vZXwFv548bn2zKZ53v7mekabqFBDA3ErhAEmjv8G+"
      "33XZtvTBooyf4JvAuYB+KjU9qmyCCvcfPAiQASaDnwz8yMvKDdg//"
      "+J4ghPC4mZ0KLAU+Mc2eYD5wxth1DyQASaBX+T1w6MjIyA+23XbbjlwSPI7j0Q1Sz8/"
      "zfOwGqeunOwuQACSBngy/mR2a5/"
      "nDnRr+sSRJ4gMDA7UQwnWlUun1FBuf3AO8MNksIMuyBRLA3EvgM5LA3Fz58zz//"
      "sDAQNdtGjpm45ODKDY++TIbbpD6l+7+JglAEui58Lv7YXmefz9JEu/"
      "y82t9COEnURSdbGa7U2x88ttGTzAfOD1N0wUSwBwPkiTQMv7g7ocBD3bjlX8yyuVyPY7j30R"
      "RdK677wK8H/gZsBfwBgmgfSSgTmB2r/yHAg92+5V/ChF4kiRZvV6/Ns/"
      "z1wJHALv06dxoDwmkaXpB4z7tHIpHN0WTrvzA4b0c/"
      "rEMDg46xUNEt6VpepdmAG3UCbi7bgeaH/"
      "7DzOx7Cv+E59yIBNBGJEmyXhJoGhXgiCiKvhfHscI/CRJA+0rg05LAjMJ/"
      "eKlU+na5XFb4JYCOlMCnJIHNC7+ZvadUKn27v79f4ZcAJIEeompm74mi6AGFXwKQBHow/"
      "KVSSeGXALpSAhcwO4tGdlP4/2HJkiUKvwTQfRKg+GbgfElgw/"
      "ADCr8E0N00nhi8UBLYIPxH9PX1KfwSQE9J4JOSABlwtJl9S+GXAHpNAp/"
      "rcQlkwJHufp8e8pEAJIEeDb8e75UAeloC7t5rEsiAoxR+CUDw/74i7BUJjIb/"
      "XoVfAhC9JYEacLTCLwH0PFmWLc2ybMl4CXRxJ1ADjgLuUfglgJ6mWq1u6e5fdPcbsizrn6QT"
      "OK+LJDAEHA3cE0JQ+CWA3mX16tUWRdHBFBtBHOju14+"
      "XQON24KIukcCQmR0TRZHCLwGIkZGRrd39TKCvMW4Huvv1tVqtf4LbgU6XwGozO9bM7iqXy7l"
      "GXwLo9am/"
      "AccAO44buwPzPL9hvARCCOvNrFMlsLpx5b9T4ZcABGBmAxT7xZcmGL8DJpJAHMedKIHV7n6M"
      "u9/"
      "Z39+"
      "v8EsAIsuyCDgZePEUY7gxCXyiAySw2t2PzfP8zhCCwi8BiAYvBk7ayFhtTAIXt7kEVrv7se5"
      "+x+DgoMIvAYjGvX/"
      "J3U8DBqc5lp0ogdXAce5+"
      "x8DAgMIvAYgx7EjxPbhtwngekOf5DZVKpRMk8DRworvfrvBLAGIMaZr2mdmZwJLNGNMDoihq"
      "dwk8A5wArEySROGXAMQ4dgYO3oSr/"
      "7QlAFwMfHQOJfAMcIKZrVThJwGIcWRZNg9YDmw5w7GdUAKNx4YvmyMJjIZ/"
      "RRzHCr8EIMbj7nsDBzZpfA+"
      "IouiGarVaHvsbSZKMjJHAsMIvAYj2uPdfQLFD8BZNHOMDzGyuJfAMcKLCLwGIqflr4K9mYZy"
      "nksDlsyyBZ4AToyi6VeGXAMQk1Gq1RcDZwPxZ+OttjiTw7Gj49Wy/BCCmIM/"
      "zg4A9ZvFHtFoCaygeY1b4JQAxFVmWbQV8iOJ1X7pAAmuA97r7TfqqTwIQU1CtVs3dDwf+"
      "vEU/ckoJAJcDH5mBBNYAJ7v7TXrIRwIQG6cMnMGGr/u2QgJ/N14CIYQRM7tiMyUwOu3/"
      "usIvAYhpXP3N7ATgZXPw4w1460QSiON4cySwBjgF+Lqm/"
      "RKAmE4CzbYD3jeHYzEdCUynE6g3/tzfK/"
      "wSgJgGtVotAk4FtptrDzUkcGOapvF4CUynEwgh5CGEXyj8EoCYJu7+Moo34qwNPo4B+"
      "wEbSGCGnYCQAMQE9/"
      "4ldz8diNvpjgTYtyGB0IROQEgAYhL+G3B4m1z9J5LAVzcigXUaws7HdAhaT5qmfcD1wHva+"
      "Q4FuA84KoSQTvD5+"
      "0MIVY2mBCA2XQCvAR4AFrV7TQF808yOjOM41cjpFkDMPPzzKV74WdQBH9eAfd39q1mWBY2eB"
      "CBmmiiz1wNv6bBZ4r7u/uXVq1fP1whKAGIzybJsC3c/B1jQYR99LfDNkZGREY2iBCA294ba/"
      "QDgtR32sV8AznD36/SQTxfOSHUIWkO1Wl1sZt8Dduuw8J/"
      "p7l9KkqSuUdQMQGwGQ0NDZmYHA69W+IUE0GPU6/"
      "WtgQ8y+4t9KPxCAmizqb9RbO+1Y4d85LUNWSn8EoCYKWY2AJxGaxf7mEn4zwS+GEJQ+"
      "CUAMRMqlUpEsbX3Szok/"
      "GcBX1L4JQDRjIMbRS+mWB4r6oDwL3f3a0MI+"
      "q5fAhAzJU3TCPgAMNjmH3UYONvdr2ksCCokANEEdgSOob2ftRhuXPmvVvglANEkqtVqH8Uqv"
      "0s64Mqv8EsAopmY2U7AoW189R+meCPx8wq/BCCaSJZl84DlwJbtHn4VfkICaDLuvjfwtjYO/"
      "zkKv5AAZoE0TRc0rv5btGn4PwJcpfALCWB2eDPwxjb8XOuAj7n7FQq/"
      "kABmgVqttrAxvZ7fpuG/TIWfkABmiTzPDwL2aMfwA5cq/GIitCBIE8iy7EXu/"
      "hCwczuGX9N+oRnALJGmqbn74cBftFn4zwUuU/iFBDCLuHuZ4qm/UpuF/"
      "9IQwnqNkJAAZolKpWLAccCftlH4P67wCwmgBZjZtmb2/"
      "jY5juuA84BLFH4hAcwyWZZFZnYqsH2bhP984CKFX0gAreFPgBOY+29S1gEXABcq/"
      "EICaM3Vv+Tuy4DQJuH/rMIvJIAWkef5K4Ej5vjqv17hFxJA66/+fWZ2FrC1wi8kgB7D3XcD/"
      "qYNwv8ZhV/MlD4dgumTpul8itd9F81h+D9lZp+J41jhF5oBtJjXAfvNcfg/rfALCaD19/"
      "5bULzuu2COwn8hoPALCWCO7v33B/aZo/B/Djhf9/yi2eh14GlQqVQWR1H0XWD3uQi/"
      "mX0yjuN1GgmhGUCLqdVqFkXRu4GlcxD+ixR+"
      "IQHMIXmeb02xXXZfq8MPnKfwCwlgjhgaGjLgSOC/tjj8F7v7eSEEhV9IAHN49U+AZbRusY/"
      "R8H8iSRKFX0gAc0WlUonc/STgpS0M/"
      "yUKv2glehJwMjNG0Q7AyS2S5IiZXQ58XNN+oRnAHJNlWQS8H9imVeE3s4+"
      "p8BMSQHvwcuBYZv85iRHgcjP7aLlcHtZhFxLA3F/"
      "9S+5+JtDfgvBfofALCaCNcPedgENn+eo/"
      "Gv6PxHGs8AsJoB1I03Qexeu+Wyr8QgLoPfYC3jbL4b9S4RcSQPtd/edTvO67cJbD/"
      "2GFX7QLeg7gj7wZeONshd/MrjGzD6vwE5oBtBlZli0Ezgbmz9KV/"
      "wtmtlzhFxJAG+Lu7wD2nK3wR1H0oXK5vFZHWrQbPb8gSJZlL3L37wOvVviFZgA9RLVaNXc/"
      "DHjVLIT/WoVfSABtTKlU6gfOoLmv+44A15ZKpbMUfiEBtO/U3/I8Pw74syaH/4ulUums/"
      "v5+hV9IAG3MNhRv/DXrGNQb4f+gwi8kgPa+94/c/VRghyaG/7o8z3XlFxJAu2Nm/"
      "wU4geZ8CzIa/tMHBgZe0CklJID2vvcvUazzlzQj/O7+lZGREYVfSAAdwispVvq1ZoQ/z/"
      "Nl22yzjcIvJIB2J03TPnc/C9i6CdP+r+R5vmxwcFDhFxJAh7AL8M4mhP9v6/"
      "W6wi8kgA66+s+neOFn8UzDryu/kAA6j32A/WcY/"
      "uvzPF82MDDwvE4dIQF0CFmWLaBY7GPBDML/VXc/"
      "TeEXEkCH4e77N2YAmxv+G4H3J0mi8IuuoutfB65UKoujKPoOsMfmhN/"
      "dbwTelyTJczpdRLfR1UuCZVlm7v4uivZ/c8L/"
      "NYVf6BagQ8nzfCvgg5shutHwn6rwCwmgA6nVamZmRwKv2Ix7foVfSACdjLsH4HQ2bbGPOvD3"
      "Cr+QADr86g+cBLx0U+4YGuE/"
      "JYSg8IueoCtLwDzPdwBO3gTB5cBNZnZqHMcKv9AMoFPJsiyiWOln200I/zfM7L1xHK/"
      "RKSE0A+hs/gw4juk945AD34ii6L3lclnhF5oBdPjVv+TuZwL90wz/"
      "zY3wP6tTQWgG0Pm8Cjh0Glf/3MxujqLopP7+foVfaAbQ6VSr1Xnuvhx40TSv/Aq/"
      "kAC65R9iZnsCb59G+G9R+IXoIgE0Fvs4B1g4jfCfqHt+IbprBvCmxq+pwr/"
      "S3U9S+IXoIgHUarWFFEt9zZ8i/Kvc/YQkSZ7RkAvRRQLI8/ztwF5ThR84PkmSpzXcQvz/"
      "dPSCIGmavgh4EFg6VfhDCAq/"
      "EN00A3jqqacMOATYaZLw36bwC9GlAujr6+sHzmTD131Hw3+cwi9EFwqgsdTXscDLJwj/7Qq/"
      "EN09A9gG+MC4zz8a/mMVfiGmOZPutA+cpmnk7icD248L/11mdnwcxwq/EF08A3gpxWo/"
      "0Zjw321mx8RxPKQhFaJLZwBpmpaAZUAyNvxRFB1dLpcVfiG6fAbwCuAoiucXFH4hekUAWZb1"
      "UazxvxXgwN15niv8QvSCANx9F+Bdjf+8292PGRgYUPiF6HYBpGk6D1hO8brv3e5+"
      "dJIkNQ2fEL0xA9gH2A+4V+EXonm0/"
      "ctAaZouAO6k2LXniBBCpmETondmAPsCa4EjFX4hmktbPwdQqVQWAq82s+"
      "PiOE41XEL0ENVqdds0TZfoSAghhBBCCCGEEEIIIYQQm8F/AidYDd/"
      "hAAzXAAAAAElFTkSuQmCC";


};

#endif // MOREAPPS_H
