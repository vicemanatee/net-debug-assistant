#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QString>
#include <model.h>
#include <model_client.h>
#include <sendontime.h>
#include <QTimer>
#include <model_udpconnnect.h>
#include <sendbydialog.h>

namespace Ui {
class controller;
}

class repeatSendMsg : public QObject
{
    Q_OBJECT

public:
    repeatSendMsg(QWidget* parent = 0);
    QTimer* TrepeatSendMsg;

private:
    int repeatTime;


signals:
    void timeToSend();

public slots:
    void getRepeatTime(int);

};

class controller : public QMainWindow
{
    Q_OBJECT

public:
    model* SCmodel;
    model_client* Cmodel;

    QString head;
    QString processedMsg;
    QString realMsg;
    int lengthOfget;

    explicit controller(QWidget *parent = 0);
    ~controller();
    Ui::controller *ui;

private slots:

    void on_BnetSetCleanHostIP_clicked();

    void on_BnetSetPasteHostIP_clicked();

    void on_CBrecepSetSaveDialog_clicked();

    void on_BrecepSetSaveDialog_clicked();

    //void on_LEsendSetrepPeriod_cursorPositionChanged(int arg1, int arg2);

    void on_LEsendSetrepPeriod_textChanged();

    void on_CBsendSetclearAftSend_clicked();

    void on_BcleanDialog_clicked();

    void on_EtextSend_textChanged();

    void on_BcleanSend_clicked();

    void on_BactivateListen_clicked();

    void on_BsendSetConnect_clicked();

    void on_BsendSetBreak_clicked();

    void outputClientMessage(QByteArray msg);

    void outputServerMessage();

    void on_BsendFile_clicked();

    void on_RBsendSelASC_toggled();

    void on_RBsendSelHEX_toggled();

    void on_Bsend_clicked();

    void on_RBsendZoneClient_toggled();

    void on_RBsendZoneServer_toggled();

    void on_BsendOnTime_clicked();

    void on_CBsendSetrepPeriod_clicked();

    void sendRepeatMsg();

    void on_Bsearch_clicked();

    void on_BclearSearch_clicked();

    void on_BserverInitial_clicked();

    void on_LElocalHostAdd_textChanged(const QString &arg1);

    void on_LElocalHostPort_textChanged(const QString &arg1);

    void on_CBtypePro_currentIndexChanged(const QString &arg1);

    void getUPDMsg(QString str);

    void on_BsendByDialog_clicked();

    void repeatDialog(QString side, QString content);

    void repeatNotMyDialog();

signals:

    void SsendFile();
    void SsendRepeatTime(int);
    void SsendUdpMsg(QString);
    void SclientDisconnectFromServer();

private:
    model_UDPconnnect* udpClient;

    QString filePath;

    int periodTime;

    QByteArray userInput;

    void changeHexToAsc (QString str, QByteArray &senddata);

    char convertHexChart(char ch);

    sendOnTime* SendTextOnTime;

    repeatSendMsg* repeatSend;

    QString time;

    void sendMessage();

    sendByDialog* sendDialog;

    QString timeFormat;

    QString dialogHistory;
};



#endif // CONTROLLER_H
