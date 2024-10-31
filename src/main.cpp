#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QScreen>
#include <QTimer>
#include <QDir>
#include <QTextStream>
#include <QIcon>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

#include "mainwindow.h"
#include "Logger.h"
#include "about.h"
#include "FileSearch.h"

void applyStyleSheet(QApplication &app) {
    QFile file(":/stylesheet.qss");
    if (!file.exists()) {
        LOG_ERROR("未找到样式表");
        return;
    }
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        LOG_INFO("加载样式表");
        QTextStream stream(&file);
        QString styleSheet = stream.readAll();
        app.setStyleSheet(styleSheet);
        file.close();
    } else {
        LOG_ERROR("无法加载样式表");
    }
}

int main(int argc, char *argv[]) {
    // 设置插件路径以确保加载 SQLite 驱动
    QString pluginPath = "/opt/homebrew/Cellar/qt/6.7.2/share/qt/plugins";
    QCoreApplication::addLibraryPath(pluginPath);
    qDebug() << "插件路径：" << QCoreApplication::libraryPaths();

    QApplication app(argc, argv);

    // 测试 SQLite 驱动是否可用
    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        LOG_ERROR("SQLite 驱动不可用。请检查插件路径和 Qt 安装。");
        return -1;
    } else {
        LOG_INFO("成功加载 SQLite 驱动。");
    }

    applyStyleSheet(app);
    app.setWindowIcon(QIcon(":/logo.png")); // 确保图标路径正确

    MainWindow w;
    w.show();

    QString settingsFile = QDir::currentPath() + "/settings.ini";
    QSettings settings(settingsFile, QSettings::IniFormat);

    bool showAbout = settings.value("showAbout", true).toBool();

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
    }

    if (about) {
        QObject::connect(&w, &MainWindow::mainWindowClosed, about.get(), &QWidget::close);
    }

    return app.exec();
}
