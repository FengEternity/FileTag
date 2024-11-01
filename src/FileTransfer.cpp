#include "FileTransfer.h"
#include "ui_FileTransfer.h"

FileTransfer::FileTransfer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FileTransfer),
    tcpSocket(new QTcpSocket(this)),
    selectedFilePath(""),
    bytesSent(0),
    totalBytes(0),
    lastBytesSent(0),
    isConnected(false),
    connectButton(ui->connectButton),
    portNumberEdit(ui->portNumberEdit),
    connectionStatusLabel(ui->connectionStatusLabel),
    fileTransferProgressBar(ui->progressBar)
{
    ui->setupUi(this);

    // 连接按钮的点击信号到对应的槽函数
    connect(ui->selectFileButton, SIGNAL(clicked()), this, SLOT(onSelectFileButtonClicked()));
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(onConnectButtonClicked()));
    connect(ui->sendFileButton, SIGNAL(clicked()), this, SLOT(onSendFileButtonClicked()));

    // 连接TCP套接字的相关信号到槽函数
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(onTcpSocketConnected()));
    connect(tcpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(onTcpSocketBytesWritten(qint64)));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(onTcpSocketDisconnected()));
}

FileTransfer::~FileTransfer()
{
    delete ui;
    delete tcpSocket;
    delete connectButton;
    delete portNumberEdit;
    delete connectionStatusLabel;
    delete fileTransferProgressBar;
}

void FileTransfer::onSelectFileButtonClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", QDir::homePath());
    if (!filePath.isEmpty()) {
        // 设置文件路径标签显示选择的文件路径
        ui->filePathLabel->setText("文件路径：" + filePath);

        // 记录选择的文件路径
        selectedFilePath = filePath;

        // 发出文件选择完成信号
        emit fileSelected(selectedFilePath);

        // 如果已经连接，启用发送文件按钮
        if (isConnected) {
            ui->sendFileButton->setEnabled(true);
        }
    }
}

void FileTransfer::onConnectButtonClicked()
{
    QString targetAddress = ui->targetAddressEdit->text();
    QString portNumberStr = ui->portNumberEdit->text();

    bool ok;
    int portNumber = portNumberStr.toInt(&ok);

    if (!ok) {
        QMessageBox::warning(this, "警告", "端口号输入错误，请输入有效的整数端口号！", QMessageBox::Ok);
        return;
    }

    tcpSocket->connectToHost(targetAddress, portNumber);
    if (tcpSocket->waitForConnected()) {
        // 连接成功后的相关处理，比如更新连接状态显示、启用发送文件按钮等
        isConnected = true;
        ui->connectionStatusLabel->setText("已连接");
        ui->sendFileButton->setEnabled(true);

        // 可以在这里进行其他连接成功后的初始化操作，比如获取一些服务器端返回的初始信息等（如果有需要）
    } else {
        // 连接失败后的处理，比如弹出提示框告知用户、更新连接状态显示等
        QMessageBox::warning(this, "省略号", "无法连接到目标地址，请检查网络或目标地址是否正确！", QMessageBox::Ok);
        isConnected = false;
        ui->connectionStatusLabel->setText("未连接");
    }
}

void FileTransfer::onSendFileButtonClicked()
{
    QString filePath = selectedFilePath;
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择文件！", QMessageBox::Ok);
        return;
    }

    if (!isConnected) {
        QMessageBox::warning(this, "警告", "请先连接到目标地址！", QMessageBox::Ok);
        return;
    }

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QString fileName = file.fileName();
        QDataStream out(tcpSocket);
        out << fileName;

        // 获取文件总字节数
        totalBytes = file.size();

        QByteArray buffer = file.readAll();
        out << buffer;
        file.close();

        // 发出文件传输开始信号（这里可根据需求添加，比如在界面上显示传输开始等）

        // 启动定时器，定期检查传输进度
        QTimer *progressTimer = new QTimer(this);
        connect(progressTimer, SIGNAL(timeout()), this, SLOT(checkTransferProgress()));
        progressTimer->start(100); // 每隔100毫秒检查一次进度

        // 等待文件传输完成或出现错误
        while (tcpSocket->state() == QTcpSocket::ConnectedState && bytesSent < totalBytes) {
            // 这里可以添加一些延迟或其他处理逻辑，比如定期检查传输进度
            // 目前简单等待 until 传输完成
        }

        // 停止定时器
        progressTimer->stop();
        delete progressTimer;

        if (bytesSent == totalBytes) {
            // 传输成功，发出文件传输完成信号
            emit fileTransferFinished(true);
        } else {
            // 传输失败，发出文件传输完成信号
            emit fileTransferFinished(false);
        }
    } else {
        QMessageBox::warning(this, "警告", "无法打开文件！", QMessageBox::Ok);
        // 发出文件传输完成信号，传输失败
        emit fileTransferFinished(false);
    }
}

void FileTransfer::onTcpSocketConnected()
{
    // 更新连接状态显示
    ui->connectionStatusLabel->setText("已连接");
    // 启用发送文件按钮
    ui->sendFileButton->setEnabled(true);
}

void FileTransfer::onTcpSocketBytesWritten(qint64 bytes)
{
    // 更新已发送的字节数
    bytesSent += bytes;

    // 发出文件传输进度信号
    emit fileTransferProgress(bytesSent, totalBytes);

    // 更新文件传输进度条的值
    fileTransferProgressBar->setValue(static_cast<int>((bytesSent * 100) / totalBytes));

    // 判断是否传输完成
    if (bytesSent == totalBytes)
        emit fileTransferFinished(true);
}

void FileTransfer::onTcpSocketDisconnected()
{
    // 更新连接状态显示
    ui->connectionStatusLabel->setText("已与目标地址断开连接");
    // 启用发送文件按钮
    ui->sendFileButton->setEnabled(false);
    isConnected = false;
}

void FileTransfer::checkTransferProgress()
{
    if (tcpSocket->state() == QTcpSocket::ConnectedState) {
        // 这里不再获取tcpSocket->bytesWritten()，而是通过判断是否有新的字节写入来更新bytesSent
        if (lastBytesSent < bytesSent) {
            // 只有当已发送字节数有变化且大于上次记录的值时才更新并发出进度信号
            // 更新上次发送进度信号时的已发送字节数
            lastBytesSent = bytesSent;

            // 发出文件传输进度信号
            emit fileTransferProgress(bytesSent, totalBytes);

            // 更新文件传输进度条的值
            fileTransferProgressBar->setValue(static_cast<int>((bytesSent * 100) / totalBytes));
        }
    }
}