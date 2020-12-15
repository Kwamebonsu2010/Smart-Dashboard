/*
 * @brief The smartmirror file will be used to define the methods that will be used to contain the data in smartmirror.cpp.
 *
 * @file smartmirror.h
 * @author Nelaven Subaskaran, Ajith Rahugnanam, Jaspreet Sandhu, Filip Durca, & Kwame Bonsu
 * @version 1.1 12/02/2020
 */

#ifndef SMARTMIRROR_H
#define SMARTMIRROR_H

#include <QMainWindow>
#include <QNetworkAccessManager> // to make requests
#include <QNetworkReply> // to handle replies
#include <QPixmap> // to work with images

QT_BEGIN_NAMESPACE
namespace Ui { class SmartMirror; }
QT_END_NAMESPACE

class SmartMirror : public QMainWindow
{
    Q_OBJECT

public:
    // Constructor for the SmartMirror.
    SmartMirror(QWidget *parent = nullptr);
    // Destructor for the SmartMirror constructor.
    ~SmartMirror();

// private class members used to contain the SmartMirror's data
private:
    QString myMemeApiUrl = "https://api.imgflip.com/get_memes"; // url to get the memes JSON data
    QString newsURL = "https://newsapi.org/v2/top-headlines?country=ca&apiKey=98ed9ef3ca8e49e498644cf341afd037";
    QString defaultNews = "https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcQ_5QaNylWhFr3MEqOmsiDH4vC7NKMxrpuQVQ&usqp=CAU";
    QString sportsURLnext = "https://www.thesportsdb.com/api/v1/json/1/eventsnext.php?id=";
    QString sportsURLlast = "https://www.thesportsdb.com/api/v1/json/1/eventslast.php?id=";
    QString sportsURLTeam = "https://www.thesportsdb.com/api/v1/json/1/lookupteam.php?id=";
    QString sportsURLLeague = "https://www.thesportsdb.com/api/v1/json/1/search_all_teams.php?l=";
    QString weatherURL = "https://api.openweathermap.org/data/2.5/onecall?lat=42.9849&lon=-81.2453&exclude=minutely,hourly&units=metric&appid=2f7b5a1f0aad5869516abea7a356a1e0";

    QString weatherIcon1URL = "http://openweathermap.org/img/wn/";
    QString weatherIcon2URL = "@4x.png";

    QString stockURL = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=";
    QString stockKey = "&apikey=6YFOES5D400R2Z5V";

    Ui::SmartMirror* ui; // allow you access to all the widgets

private slots:
    void on_weather_clicked();
    void on_sports_clicked();
    void on_stocks_clicked();
    void on_news_clicked();
    void on_memes_clicked();
    void jsonNewsFinished(QNetworkReply*);
    void jsonMemeFinished(QNetworkReply*);
    void jsonSportsNextFinished(QNetworkReply*);
    void jsonSportsLastFinished(QNetworkReply*);
    void jsonSportsTeamFinished(QNetworkReply*);
    void jsonSportsLeagueFinished(QNetworkReply*);
    void badgeFinished(QNetworkReply*);
    void weatherIconFinished(QNetworkReply*);
    void imageFinished(QNetworkReply*);
    void stocksFinished(QNetworkReply*);
    void jsonWeatherFinished(QNetworkReply*);
    void on_leagueBox_currentIndexChanged(const QString &arg1);
    void on_teamBox_currentIndexChanged(int index);
    void on_stockEnter_clicked();
    void screenClear();
};
#endif // SMARTMIRROR_H
