#include "updatescheduler.h"
//------------------------------------------------------------------------------------------------------------//
TUpdateScheduler::TUpdateScheduler(QObject *parent) :
    QObject(parent)
{
    UpdateJsonUrl = "http://siptest.dev/version.php";

    WebCtrl = new QNetworkAccessManager;

    UpdateRequestTimer = new QTimer;

    StartAppUpdateTime = 15*1000; /*10*60*1000;*/ // TODO: Вернуть правильный интервал! // 10 минут по 60 секунд по 1000 миллисекунд
    SchedulerTime.setHMS(15, 0, 0);

    // выполняется при старте приложения
    QTimer::singleShot(StartAppUpdateTime, this, SLOT(CheckUpdate()));

    connect(UpdateRequestTimer, SIGNAL(timeout()), this, SLOT(CheckUpdateTime()));
    UpdateRequestTimer->start(30*60*1000); // один раз в 30 минут тикает

    connect(WebCtrl, SIGNAL(finished(QNetworkReply*)), this, SLOT(JsonFileDownloaded(QNetworkReply*)));
}
//------------------------------------------------------------------------------------------------------------//
TUpdateScheduler::~TUpdateScheduler()
{
    delete UpdateRequestTimer;
    delete WebCtrl;
}
//------------------------------------------------------------------------------------------------------------//
void TUpdateScheduler::CheckUpdateTime()
{
    QTime CheckTime = QTime::currentTime();
    if( CheckTime < SchedulerTime ){ return; }

    CheckUpdate();
}
//------------------------------------------------------------------------------------------------------------//
void TUpdateScheduler::CheckUpdate()
{
    QNetworkRequest Request;
    Request.setUrl(QUrl(UpdateJsonUrl));

    WebCtrl->get(Request);
}
//------------------------------------------------------------------------------------------------------------//
void TUpdateScheduler::CheckCurrVersion(const QByteArray &DownloadedData)
{
    QJsonDocument JDoc = QJsonDocument::fromJson(DownloadedData);
    QJsonObject   JObj = JDoc.object();

    if(JObj.empty()){return;}

    QString CurAppVersion(APPLICATION_VERSION);

    QString AppFile       = JObj.take("file").toString();
    QString NewAppVersion = JObj.take("version").toString();
    QString ReleaseNotes  = JObj.take("release_notes").toString();

/*
    QFile file("log.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << CurAppVersion << "\n";
    out << NewAppVersion << "\n";
    out << QString::compare(CurAppVersion, NewAppVersion) << "\n";
    file.close();
*/

    // TODO проверить правильность работы QString::compare при стравлении строк версий
    if(QString::compare(CurAppVersion, NewAppVersion) < 0){
        emit NeedUpdate(AppFile, ReleaseNotes);
    }
}
//------------------------------------------------------------------------------------------------------------//
void TUpdateScheduler::JsonFileDownloaded(QNetworkReply* Reply)
{
    if(Reply->error() == QNetworkReply::NoError){
        CheckCurrVersion(Reply->readAll());
    }
    Reply->deleteLater();
}
//------------------------------------------------------------------------------------------------------------//