#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QScreen>
#include <QTimer>
#include "mainwindow.h"
#include "Logger.h"
#include "about.h"
#include <QDebug>

void applyStyleSheet(QApplication &app) {
    QFile file(":/stylesheet.qss");
    if (!file.exists()) {
        LOG_ERROR("未找到样式表");
        // qDebug() << "未找到样式表";
        return;
    }
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        LOG_INFO("加载样式表");
        // qDebug() << "加载样式表";
        QTextStream stream(&file);
        QString styleSheet = stream.readAll();
        app.setStyleSheet(styleSheet);
        file.close();
    } else {
        LOG_ERROR("无法加载样式表");
        // qDebug() << ("无法加载样式表");
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    applyStyleSheet(app);

    app.setWindowIcon(QIcon(":/logo.png")); // 确保图标路径正确

    MainWindow w;
    w.show();

    QString settingsFile = QDir::currentPath() + "/settings.ini";
    QSettings settings(settingsFile, QSettings::IniFormat);

    bool showAbout = settings.value("showAbout", true).toBool();
    // qDebug() << "showAbout:" << showAbout;

    std::unique_ptr<About> about;
    if (showAbout) {
        about = std::make_unique<About>(settingsFile);
        about->resize(400, 300);
        QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
        int x = (screenGeometry.width() - about->width()) / 2;
        int y = (screenGeometry.height() - about->height()) / 2;
        about->move(x, y);
        about->show();
        about->raise();
        about->activateWindow();
        // qDebug() << "About window shown";
    }

    if (about) {
        QObject::connect(&w, &MainWindow::mainWindowClosed, about.get(), &QWidget::close);
    }

    return app.exec();
}
