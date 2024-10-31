#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QFileDialog>
#include <QMessageBox>
#include <QDataStream>
#include <QTimer>
#include "ui_FileTransfer.h"

class FileTransfer : public QMainWindow
{
    Q_OBJECT

public:
    explicit FileTransfer(QWidget *parent = 0);
    ~FileTransfer();

    private slots:
        void onSelectFileButtonClicked();
    void onConnectButtonClicked();
    void onSendFileButtonClicked();
    void onTcpSocketConnected();
    void onTcpSocketBytesWritten(qint64 bytes);
    void onTcpSocketDisconnected();
    void checkTransferProgress();

    signals:
        void fileSelected(const QString &selectedFilePath);
    void fileTransferProgress(qint64 bytesSent, qint64 totalBytes);
    void fileTransferFinished(bool success);

private:
    Ui::FileTransfer *ui;
    QTcpSocket *tcpSocket;
    QString selectedFilePath;
    qint64 bytesSent;
    qint64 totalBytes;
    qint64 lastBytesSent;
    bool isConnected;

    QPushButton *connectButton;
    QLineEdit *portNumberEdit;
    QLabel *connectionStatusLabel;

    // 添加对文件传输进度条的引用
    QProgressBar *fileTransferProgressBar;
};

#endif // FILETRANSFER_H