#ifndef SENDBYDIALOG_H
#define SENDBYDIALOG_H

#include <QMainWindow>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

class readDialog : public QObject
{
    Q_OBJECT

public:
    readDialog(QWidget *parent = 0);

signals:
    void sendInformation(QString, QString);
    void warningNotMyDialog();
    void sendProgress(int);
    void SopenMyDialog(QDateTime);
    void SreadComplete();

public slots:
    void readAndSendDialog(QString path, QDateTime startSendTime, int readBaseSide, int spead);

private:
    void changeHexToAsc(QString str, QByteArray &senddata);

    int convertHexChart(char ch);

};

namespace Ui {
class sendByDialog;
}

class sendByDialog : public QMainWindow
{
    Q_OBJECT

public:
    explicit sendByDialog(QWidget *parent = 0);
    ~sendByDialog();
    readDialog* dialogRead;

signals:
    void sendFile(QString, QDateTime, int, int);
    void sendInformation(QString, QString);
    void SendTask();
    void SopenWrongDialog();
    void SopenRightDialog(QDateTime);
    void SrepeatComplete();

private slots:
    void on_BselectDialogPath_clicked();

    void on_Bconfirm_clicked();

    void on_Bcancel_clicked();

private:
    Ui::sendByDialog *ui;
    QDateTime startSendTime;
    int lineNum;
    int readBaseSide;
};

#endif // SENDBYDIALOG_H
