/*
 * @author Nelaven Subaskaran, Ajith Rahugnanam, Jaspreet Sandhu, Filip Durca, & Kwame Bonsu
 *
 * @brief The smartmirror file will be used to contain a the data and define the methods declared in smartmirror.h.
 *
 * This is the main file of our project in which each of the features have been implemented. It makes use of
 * several methods to ensure the functionality of each of the user stories as outlined in the Jira Kanban Board.
 *
 * @file smartmirror.cpp
 *
 * @version 1.1 12/02/2020
 */

#include "smartmirror.h"
#include "ui_smartmirror.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <cstdlib>
#include <string>
#include <ctime>
#include <cmath>
#include <QPainter>
#include <QMessageBox>

// Store Articles and Article Images
QStringList newsArticles;
QStringList newsImages;
int articleNum = 0;
// Variable to help the ImageFinished know when it is being called by news or memes
int dif = 0;
// Store sports team ID, League ID, and array of teams
QString teamID = "134880";
QString leagueID = "NBA";
QJsonArray teamArray;

// Name of the meme
QString memeName;

using namespace std;

/* @author Kwame Bonsu
 *
 * @brief This function recieves the JSON from the request
 *
 * The JSON file is parsed, storing the article titles and article image URLs
 *
 * @params QNetworkReply* reply is the reply to the network request
 *
 * @return void
 */
void SmartMirror::jsonNewsFinished(QNetworkReply* reply) {
    QString strReply = (QString)reply->readAll();
    // Parse Json and store each headline and url to image
    QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray jsonArticles = jsonObject["articles"].toArray();
    QJsonArray jsonImages = jsonObject["articles"].toArray();
    // Store headlines
    foreach(const QJsonValue & value, jsonArticles) {
        QJsonObject obj = value.toObject();
        newsArticles.append(obj["title"].toString());
    }
    // Store image URLs
    foreach(const QJsonValue & value, jsonImages) {
        QJsonObject obj = value.toObject();
        newsImages.append(obj["urlToImage"].toString());
    }
}
/* @author Ajith Rahugnanam
 *
 * @brief This function enables stock functionality of program
 *
 * The function parses the JSON object retrieved from the API
 * and displays the stocks information to the UI.
 *
 * @param QNetworkReply a pointer, the reply to the API request
 *
 * @return void
 */
void SmartMirror::stocksFinished(QNetworkReply* reply) {
    // Parse JSON Object from API reply
    QString stockList;
    QString strReply = (QString)reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();

    // Get todays date and foprmat it
    // to find todays respective data
    int year = QDate::currentDate().year();
    int month = QDate::currentDate().month();
    int day = QDate::currentDate().day();
    day -= 1;
    qDebug() << day;
    QString fullDate;
    QString helper;
    //CHECK CODE
    if (day < 10) {
        helper = "0";
        helper += QString::number(day);
    }

    // combine Year-Month-Day
    fullDate = "" + QString::number(year) + "-" + QString::number(month) + "-" + helper + "";

    // Get stock ticker symbol from Data
    QString stockName = jsonObject["Meta Data"].toObject()["2. Symbol"].toString();
    ui->stockTicker->setText("Stock: " + stockName);
    // Get Stocks Opening price for today
    QString openPrice = jsonObject["Time Series (Daily)"].toObject()[fullDate].toObject()["1. open"].toString();
    ui->stockOpen->setText("Open: " + openPrice);
    // Get Stocks Highest price for today
    QString highPrice = jsonObject["Time Series (Daily)"].toObject()[fullDate].toObject()["2. high"].toString();
    ui->stockHigh->setText("High: " + highPrice);
    // Get Stocks Lowest price for today
    QString lowPrice = jsonObject["Time Series (Daily)"].toObject()[fullDate].toObject()["3. low"].toString();
    ui->stockLow->setText("Low: " + lowPrice);
    // Append all data collected into one String
    stockList.append("Stock: " + stockName + "\n");
    stockList.append("Open: " + openPrice + "\n");
    stockList.append("High: " + highPrice + "\n");
    stockList.append("Low: " + lowPrice);
}

/*
 * @author Nelaven Subaskaran
 *
 * @brief Get the JSON data for memes once the request is complete.
 *
 * @param reply will provide the data response from a specific URL
 *
 * @return void
 */
void SmartMirror::jsonMemeFinished(QNetworkReply *reply) {
    // list of image urls for memes
    QStringList memesList;
    // list of image names for memes
    QStringList namesList;
    // store the JSON data as a string
    QString strReply = (QString)reply->readAll();
    // convert the data back to QByteArray and store it as a JSON document
    QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
    // Parse the JSON response to an object
    QJsonObject jsonObject = jsonResponse.object();
    // Get the data object from the JSON response
    QJsonObject dataObj = jsonObject["data"].toObject();
    // Inside the dataObj, there will be a memes object
    QJsonArray jsonArray = dataObj["memes"].toArray();

    // Go through each meme and access its URL property and add it to the memesList
    foreach (const QJsonValue & value, jsonArray) {
        QJsonObject obj = value.toObject();
        memesList.append(obj["url"].toString());
        namesList.append(obj["name"].toString());
    }

    // memesList most likely be empty if the user does not have a stable internet connection
    if (memesList.isEmpty()) {
        QMessageBox::information(this, tr("SmartMirror"), tr("No internet connection found. Please check your connection."));
    } else {
        // Generate a random number to get a meme
        srand((int)time(0)); // need to seed the random number generator
        int i = (rand() % memesList.size()) + 0;
        // Send network requests to get the contents of the URL for the specific meme
        QNetworkAccessManager *displayMeme = new QNetworkAccessManager(this);
        // Connect the signal to the slot (provide the network response in imageFinished)
        connect(displayMeme, &QNetworkAccessManager::finished, this, &SmartMirror::imageFinished);
        const QUrl url = QUrl(memesList.at(i));
        memeName = namesList.at(i);
        // Send the request to the network
        QNetworkRequest request(url);
        // Get the data from the request sent
        displayMeme->get(request);
    }
}

/*
 * @brief This method is used to display the selected teams next game date and competitor
 *
 * Using the SportsDB API, the function gets a json file containing the next 5 games of the selected
 * team. It then checks if the next game is available. This is done to maintain functionality in the
 * off-season. It then accesses the first game from the json file and displays the information, checking
 * for which team is home and which team is away. It then updates the label to display the information to the user
 *
 * @author Filip Durca
 *
 * @param QNetworkReply* reply is the reply from the network
 *
 * @return void
 */
void SmartMirror::jsonSportsNextFinished(QNetworkReply* reply) {
    QString strReply = (QString)reply->readAll();
    QString gameInfo = "Next Game:\n";

    // Parse Json
    QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();

    // Check if next game is available
    if (!jsonObject["events"].isArray()){
        gameInfo.append("Not available :(");
        ui->nextGame->setText(gameInfo);
        return;
    }

    // Continue parsing if next game exists
    QJsonArray jsonGames = jsonObject["events"].toArray();
    QJsonObject game = jsonGames.at(0).toObject();

    // Turn game info into a string
    gameInfo.append(game["dateEvent"].toString());
    gameInfo.append("\n");

    // Check if home or away
    if (game["strHomeTeam"] == ui->teamBox->currentText()){
        gameInfo.append(game["strHomeTeam"].toString());
        gameInfo.append(" vs ");
        gameInfo.append(game["strAwayTeam"].toString());
        gameInfo.append("\n");
        gameInfo.append(game["intHomeScore"].toString());
        gameInfo.append(" - ");
        gameInfo.append(game["intAwayScore"].toString());
    } else {
        gameInfo.append(game["strAwayTeam"].toString());
        gameInfo.append(" @ ");
        gameInfo.append(game["strHomeTeam"].toString());
        gameInfo.append("\n");
        gameInfo.append(game["intAwayScore"].toString());
        gameInfo.append(" - ");
        gameInfo.append(game["intHomeScore"].toString());
    }

    ui->nextGame->setText(gameInfo);
}

/*
 * @brief This method is used to display the selected teams previous game and score
 *
 * Using the SportsDB API, the function gets a json file containing the last 5 games of the selected
 * team. It then accesses the first game from the json file, the most recent, and displays the information,
 * checking for which team is home and which team is away. It then updates the label to display the
 * information to the user.
 *
 * @author Filip Durca
 *
 * @param QNetworkReply* reply is the reply from the network
 *
 * @return void
 */
void SmartMirror::jsonSportsLastFinished(QNetworkReply* reply) {

    QString strReply = (QString)reply->readAll();
    // Parse Json
    QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray jsonGames = jsonObject["results"].toArray();
    QJsonObject game = jsonGames.at(0).toObject();
    QString gameInfo = "Last Game:\n";

    // Turn game info into a string
    gameInfo.append(game["dateEventLocal"].toString());
    gameInfo.append("\n");

    // Check if home or away
    if (game["strHomeTeam"] == ui->teamBox->currentText()){
        gameInfo.append(game["strHomeTeam"].toString());
        gameInfo.append(" vs ");
        gameInfo.append(game["strAwayTeam"].toString());
        gameInfo.append("\n");
        gameInfo.append(game["intHomeScore"].toString());
        gameInfo.append(" - ");
        gameInfo.append(game["intAwayScore"].toString());
    } else {
        gameInfo.append(game["strAwayTeam"].toString());
        gameInfo.append(" @ ");
        gameInfo.append(game["strHomeTeam"].toString());
        gameInfo.append("\n");
        gameInfo.append(game["intAwayScore"].toString());
        gameInfo.append(" - ");
        gameInfo.append(game["intHomeScore"].toString());
    }

    ui->lastGame->setText(gameInfo);
}

/*
 * @brief This method is used to display the selected teams icon
 *
 * Using the SportsDB API, the function gets a json file of the user selection team. It then searches
 * the json file for the api link to the png file of the teams badge. Using this new link, it then
 * sends a network request to the badgefinished method, which displays the icon.
 *
 * @author Filip Durca
 *
 * @param QNetworkReply* reply is the reply from the network
 *
 * @return void
 */
void SmartMirror::jsonSportsTeamFinished(QNetworkReply* reply) {
    QString strReply = (QString)reply->readAll();
    // Parse Json
    QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray jsonArray = jsonObject["teams"].toArray();
    QJsonObject jsonTeam = jsonArray.at(0).toObject();
    QString badge = jsonTeam["strTeamBadge"].toString();
    // Send badge to be displayed
    QNetworkAccessManager* displayBadge = new QNetworkAccessManager(this);
    connect(displayBadge, &QNetworkAccessManager::finished, this, &SmartMirror::badgeFinished);
    QNetworkRequest request((QUrl)badge);
    displayBadge->get(request);
}

/*
 * @brief This method is used to change the teams available for selection based on the user league selection
 *
 * Using the SportsDB API, the function gets a json file containing every team within the users league. It then
 * gets the array of the team dictionary objects and updates the team selection drop down menu accordingly.
 *
 * @author Filip Durca
 *
 * @param QNetworkReply* reply is the reply from the network
 *
 * @return void
 */
void SmartMirror::jsonSportsLeagueFinished(QNetworkReply* reply) {
    QString strReply = (QString)reply->readAll();
    //Get all teams in the league
    QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    teamArray = jsonObject["teams"].toArray();
    QJsonObject jsonTeam;

    //Clear the combo box
    ui->teamBox->clear();

    //Change the teams displayed in the team drop down
    for (int i = 0; i < teamArray.size(); i++){
        jsonTeam = teamArray.at(i).toObject();
        ui->teamBox->addItem(jsonTeam["strTeam"].toString());
    }
}

/* @author Jaspreet Sandhu
 *
 * @brief This function enables weather functionality.
 *
 * This function is used to parse the JSON object and render the appropriate
 * ui elements to the screen. The labels are filled with data retrieved
 * from the API at the time of request.
 *
 * @param QNetworkReply pointer, which 'stores' the API request's reply.
 *
 * @return void, as all processing is done within the function.
 */
void SmartMirror::jsonWeatherFinished(QNetworkReply * reply) {

    QString result = (QString)(reply->readAll());
    QJsonDocument jsonResponse = QJsonDocument::fromJson(result.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();

    // Below is to get Current Weather
    QJsonObject currentObj = jsonObject["current"].toObject();
    QJsonArray weatherArr = currentObj["weather"].toArray();

    QJsonObject weatherObj = weatherArr.at(0).toObject();

    QString weatherMain = weatherObj.value("main").toString();
    QString icon = weatherObj.value("icon").toString();

    QString iconString = weatherIcon1URL + icon + weatherIcon2URL;

    double temp = currentObj.value("temp").toDouble();
    double feels = currentObj.value("feels_like").toDouble();
    int temperature = (int)round(temp);
    int feelsLike = (int)round(feels);

    std::string tempText = std::to_string(temperature) + "°C";
    std::string feelsText = std::to_string(feelsLike) + "°C";

    ui->city->setText(QString::fromStdString("London, ON"));
    ui->tempLabel->setText(QString::fromStdString(tempText));
    ui->weatherDesc->setText(weatherMain);
    ui->feelsLabel->setText(QString::fromStdString("Feels Like"));
    ui->feelsValue->setText(QString::fromStdString(feelsText));


    // Weather Icon
    QNetworkAccessManager* displayWeatherIcon = new QNetworkAccessManager(this);
    connect(displayWeatherIcon, &QNetworkAccessManager::finished, this, &SmartMirror::weatherIconFinished);
    QNetworkRequest request((QUrl)iconString);
    displayWeatherIcon->get(request);


    // Below is to get 3-Day Weather Forecast
    QJsonArray dailyArr = jsonObject["daily"].toArray();

    QJsonObject day1Obj = dailyArr.at(0).toObject();
    QJsonObject day2Obj = dailyArr.at(1).toObject();
    QJsonObject day3Obj = dailyArr.at(2).toObject();

    double day1Double = day1Obj.value("temp").toObject().value("day").toDouble();
    int day1Int = (int)round(day1Double);

    double day2Double = day2Obj.value("temp").toObject().value("day").toDouble();
    int day2Int = (int)round(day2Double);

    double day3Double = day3Obj.value("temp").toObject().value("day").toDouble();
    int day3Int = (int)round(day3Double);

    std::string day1Str = to_string(day1Int) + "°C";
    std::string day2Str = to_string(day2Int) + "°C";
    std::string day3Str = to_string(day3Int) + "°C";

    std::string days[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    time_t now;
    struct tm nowLocal;
    now = time(NULL);
    nowLocal = *localtime(&now);

    int td = nowLocal.tm_wday;
    std::string tdStr = days[td];

    ui->forecastTitle->setText(QString::fromStdString("3-Day Forecast"));
    ui->dayOne->setText(QString::fromStdString(days[(td+1)%7] + ": " + day1Str));
    ui->dayTwo->setText(QString::fromStdString(days[(td+2)%7] + ": " + day2Str));
    ui->dayThree->setText(QString::fromStdString(days[(td+3)%7] + ": " + day3Str));

    return;
}


/*
 * @author Nelaven Subaskaran
 *
 * @brief Get the actual image to display for the user.
 *
 * @param reply will provide the data response from a specific URL
 *
 * @return void
 */
void SmartMirror::imageFinished(QNetworkReply *reply) {
    // Store the image in pix
    QPixmap pix;
    pix.loadFromData(reply->readAll());
    QPixmap scaled_img = pix.scaled(ui->meme1->width(), ui->meme1->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // Show the image to the user and scale it to fit the screen
    ui->meme1->setPixmap(scaled_img);
    if (dif == 0) {
        // Show the name of the meme
        ui->memeName->setText(memeName);
    }
}

/*
 * @brief This method is used to display the selected teams icon
 *
 * Using the SportsDB API, the function gets the png file using the link from the
 * jsonSportsTeamFinished method. It then displays the image on the team header.
 *
 * @author Filip Durca
 *
 * @param QNetworkReply* reply is the reply from the network
 *
 * @return void
 */
void SmartMirror::badgeFinished(QNetworkReply* reply) {
    QPixmap pix;
    pix.loadFromData(reply->readAll());
    QPixmap scaled_img = pix.scaled(ui->weatherIcon->width(), ui->weatherIcon->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    //Display the scaled image
    ui->teamHeader->setPixmap(scaled_img);
}


/*
 * @brief This method is used to display the appropriate weather icon
 *
 * Using the OpenWeatherMapAPI, the function uses the url to render
 * the icon's png image.
 *
 * @author Jaspreet Sandhu
 *
 * @param QNetworkReply* reply is the reply from the network
 *
 * @return void
 */
void SmartMirror::weatherIconFinished(QNetworkReply* reply) {
    QPixmap pix;
    pix.loadFromData(reply->readAll());
    QPixmap scaled_img = pix.scaled(ui->weatherIcon->width(), ui->weatherIcon->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // Show the image to the user and scale it to fit the screen
    ui->weatherIcon->setPixmap(scaled_img);
}


SmartMirror::SmartMirror(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SmartMirror)
{
    ui->setupUi(this);

    QNetworkAccessManager* newsManager = new QNetworkAccessManager(this);
    connect(newsManager, &QNetworkAccessManager::finished, this, &SmartMirror::jsonNewsFinished);
    const QUrl url = QUrl(newsURL);
    QNetworkRequest request(url);
    newsManager->get(request);

    //Make aspects invisible
    ui->teamBox->setVisible(false);
    ui->leagueBox->setVisible(false);
    ui->stockInput->setVisible(false);
    ui->stockEnter->setVisible(false);

    // Initialize Team Array
    on_leagueBox_currentIndexChanged(leagueID);
}

/* @author Jaspreet Sandhu
 *
 * @brief This funtion makes the initial request to OpenWeatherMapAPI through the url.
 *
 * It is responsible for making the GET request each time the button is clicked, using
 * the 'weatherURL' as specified in the header file. It then passes the returned JSON
 * along to the 'jsonWeatherFinished()' function for further use.
 *
 * @param N/A
 *
 * @return void
 */
void SmartMirror::on_weather_clicked()
{
    // Clear Screen
    screenClear();
    // Get Updated Weather Data
    QNetworkAccessManager *weatherManager = new QNetworkAccessManager(this);
    connect(weatherManager, &QNetworkAccessManager::finished, this, &SmartMirror::jsonWeatherFinished);
    const QUrl url = QUrl(weatherURL);
    QNetworkRequest request(url);
    weatherManager->get(request);
}


/*
 * @brief This method is used to change the screen to the sports screen
 *
 * This method is the core of the sports section of the api. It first clears the screen
 * of what was there before and then makes the drop down menus for team and league selection
 * visible. It creates the url using the team ID specified by the user selection or the default
 * (first team, atlanta hawks from the first league, the nba). It then sends requests to thesportsdb api to get the information
 * to display the teams next and last game as well as their icon.
 *
 * @author Filip Durca
 *
 * @return void
 */
void SmartMirror::on_sports_clicked()
{
    // Clear Screen
    screenClear();
    ui->teamBox->setVisible(true);
    ui->leagueBox->setVisible(true);

    // Set the urls
    QString next = sportsURLnext;
    QString last = sportsURLlast;
    QString team = sportsURLTeam;

    next.append(teamID);
    last.append(teamID);
    team.append(teamID);

    // Call the API
    // Next Game
    QNetworkAccessManager* sportsManager1 = new QNetworkAccessManager(this);
    connect(sportsManager1, &QNetworkAccessManager::finished, this, &SmartMirror::jsonSportsNextFinished);
    const QUrl urlNext = QUrl(next);
    QNetworkRequest requestNext(urlNext);
    sportsManager1->get(requestNext);

    // Last Game
    QNetworkAccessManager* sportsManager2 = new QNetworkAccessManager(this);
    connect(sportsManager2, &QNetworkAccessManager::finished, this, &SmartMirror::jsonSportsLastFinished);
    const QUrl urlLast = QUrl(last);
    QNetworkRequest requestLast(urlLast);
    sportsManager2->get(requestLast);

    // Icon
    QNetworkAccessManager* sportsManager3 = new QNetworkAccessManager(this);
    connect(sportsManager3, &QNetworkAccessManager::finished, this, &SmartMirror::jsonSportsTeamFinished);
    const QUrl urlTeam = QUrl(sportsURLTeam);
    QNetworkRequest requestTeam(team);
    sportsManager3->get(requestTeam);
}

/* @author Ajith Rahugnanam
 *
 * @brief This function gets the stock from user and calls API
 *
 * The function retreives the stock ticker symbol from the user
 * and and places it into the REST API Links, uses QNetworkRequest
 * to call API request
 *
 * @param no parameters
 *
 * @return void, calls function stocksFinished(QNetworkReply* reply)
 * which gets the return from API call
 */
void SmartMirror::on_stocks_clicked()
{
    // Clear Screen
    screenClear();
    ui->stockInput->show();
    ui->stockHeader->show();
    ui->stockEnter->show();

    QString enterStock = "Enter Stock Ticker:";
    ui->stockHeader->setText(enterStock);
    QNetworkAccessManager *stocks = new QNetworkAccessManager(this);
    connect(stocks, &QNetworkAccessManager::finished, this, &SmartMirror::stocksFinished);

    QString stockChoice = ui->stockInput->text();
    QString stockstring = stockURL+stockChoice+stockKey;
    const QUrl url = QUrl(stockstring);
    QNetworkRequest request(url);
    stocks->get(request);
}


/* @author Kwame Bonsu
 *
 * @brief This function cycles through the news articles.
 *
 * Each time the 'News' button is clicked it displays the next news article. The articles are sorted from most to least
 * recent.
 *
 * @return void, as all processing is done within the function
 */
void SmartMirror::on_news_clicked()
{
    dif = 1;
    // Clear Screen
    screenClear();
    // Get and display image. If there is none linked to the article display default news image
    QNetworkAccessManager* displayMeme = new QNetworkAccessManager(this);
    QObject::connect(displayMeme, &QNetworkAccessManager::finished, this, &SmartMirror::imageFinished);
    if (!newsImages.at(articleNum).isNull() && newsImages.at(articleNum).startsWith("https")) {
        QNetworkRequest request((QUrl)newsImages.at(articleNum));
        displayMeme->get(request);
    }
    else {
        QNetworkRequest request((QUrl)defaultNews);
        displayMeme->get(request);
    }
    // Show Article title and cycle through in order
    ui->header->setText(newsArticles.at(articleNum));
    articleNum++;
    if (articleNum == newsArticles.size()) {
        articleNum = 0;
    }
    
}

void SmartMirror::on_memes_clicked()
{
    dif = 0;
    // Clear Screen
    screenClear();
    // Send network requests to get the contents of the URL for the memes API
    QNetworkAccessManager *memes = new QNetworkAccessManager(this);
    // Connect the signal to the slot (provide the network response in jsonMemeFinished)
    connect(memes, &QNetworkAccessManager::finished, this, &SmartMirror::jsonMemeFinished);
    const QUrl url = QUrl(myMemeApiUrl);
    // Send the request to the network
    QNetworkRequest request(url);
    // Get the data from the request sent
    memes->get(request);
}

/*
 * @author Nelaven Subaskaran, Ajith Rahugnanam, Jaspreet Sandhu, Filip Durca, & Kwame Bonsu
 *
 * @brief This function clears the screen and resets all the objects.
 *
 * This method is used by each 'on_clicked()' function to clear the screen before rendering
 * any new ui elements.
 *
 * @params N/A
 *
 * @return void
 */
void SmartMirror::screenClear() {
    // Clear Screen
    ui->meme1->clear();
    ui->memeName->clear();
    ui->mainscreen->clear();
    ui->header->clear();
    ui->tempLabel->clear();
    ui->city->clear();
    ui->weatherDesc->clear();
    ui->feelsLabel->clear();
    ui->feelsValue->clear();
    ui->nextGame->clear();
    ui->lastGame->clear();
    ui->teamHeader->clear();
    ui->teamBox->setVisible(false);
    ui->leagueBox->setVisible(false);
    ui->forecastTitle->clear();
    ui->dayOne->clear();
    ui->dayTwo->clear();
    ui->dayThree->clear();
    ui->stockHeader->clear();
    ui->stockInput->hide();
    ui->stockEnter->hide();
    ui->stockOpen->clear();
    ui->stockHigh->clear();
    ui->stockLow->clear();
    ui->stockOpen->clear();
    ui->stockHigh->clear();
    ui->stockLow->clear();
    ui->stockTicker->clear();
    ui->weatherIcon->clear();
}


/*
 * @brief This method is used to change the league selection when the user selects another team
 *
 * Using the text in the drop down box, when a user changes the league it updates the leagueID. It
 * then sends a request to thesportsdb for every team within the league specified and updates the team
 * ID to the first team alphabetically in the league.
 *
 * @author Filip Durca
 *
 * @param QString &arg1 is the name of the league
 *
 * @return void
 */
void SmartMirror::on_leagueBox_currentIndexChanged(const QString &arg1){
    //Set league ID
    leagueID = arg1;

    //Change the league URL
    QString leagueURL = sportsURLLeague;
    leagueURL.append(leagueID);

    //Call upon the url
    QNetworkAccessManager* sportsManager1 = new QNetworkAccessManager(this);
    connect(sportsManager1, &QNetworkAccessManager::finished, this, &SmartMirror::jsonSportsLeagueFinished);
    const QUrl url = QUrl(leagueURL);
    QNetworkRequest requestNext(leagueURL);
    sportsManager1->get(requestNext);

}

/*
 * @brief This method is used to change the team selection when the user selects another team
 *
 * Using the index from the user selection, the team ID changes to the corresponding team in the
 * array based on the index of the users selection. It then calls the on_sports_clicked() method to
 * update the screen with the new teams information.
 *
 * @author Filip Durca
 *
 * @param int index is the index of the team within the array
 *
 * @return void
 */
void SmartMirror::on_teamBox_currentIndexChanged(int index){
    //Change the team id
    teamID = (QString) (teamArray.at(index).toObject())["idTeam"].toString();
    //Update screen unless first time being changed
    if (ui->leagueBox->isVisible()){
        on_sports_clicked();
    }
}

void SmartMirror::on_stockEnter_clicked()
{
    SmartMirror::on_stocks_clicked();
}

SmartMirror::~SmartMirror()
{
    delete ui;
}
