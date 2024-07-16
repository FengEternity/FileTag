#include "TagController.h"
#include "MultiSelectDialog.h"
#include "Logger.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>

TagController::TagController(const std::string &filename, FileTagSystem &fileTagSystem, QObject *parent)
        : QObject(parent), tagManager(filename), fileTagSystem(fileTagSystem) {}

void TagController::addTag(QWidget *parentWidget) {
    QString filePath = QFileDialog::getOpenFileName(parentWidget, "选择文件", "", "所有文件 (*)");
    if (filePath.isEmpty()) {
        return;
    }
    QString tag = QInputDialog::getText(parentWidget, "添加标签", "请输入标签:");

    if (!filePath.isEmpty() && !tag.isEmpty()) {
        fileTagSystem.addTags(filePath.toStdString(), tag.toStdString());
        QMessageBox::information(parentWidget, "标签已添加", "标签已添加到文件: " + filePath);
        Logger::instance().log("标签已添加到文件: " + filePath);
        emit tagsUpdated();
    }
}

void TagController::searchTag(QWidget *parentWidget) {
    QString tag = QInputDialog::getText(parentWidget, "搜索标签", "请输入标签:");

    if (!tag.isEmpty()) {
        std::vector<std::string> files = fileTagSystem.searchFilesByTag(tag.toStdString());
        QStringList fileList;
        for (const auto &file : files) {
            fileList.append(QString::fromStdString(file));
        }
        emit displayFiles(fileList);  // 显示文件列表
    }
}

void TagController::removeTag(QWidget *parentWidget) {
    QString tag = QInputDialog::getText(parentWidget, "删除标签", "请输入标签:");

    if (!tag.isEmpty()) {
        std::vector<std::string> files = fileTagSystem.searchFilesByTag(tag.toStdString());
        if (files.empty()) {
            QMessageBox::information(parentWidget, "无文件", "没有文件包含此标签。");
            Logger::instance().log("没有文件包含此标签。");
            return;
        }

        QStringList fileList;
        for (const auto &file : files) {
            fileList.append(QString::fromStdString(file));
        }

        MultiSelectDialog dialog(fileList, parentWidget);
        if (dialog.exec() == QDialog::Accepted) {
            QStringList selectedFiles = dialog.selectedItems();
            if (selectedFiles.contains("删除所有文件")) {
                for (const auto &file : files) {
                    fileTagSystem.removeTag(file, tag.toStdString());
                }
                QMessageBox::information(parentWidget, "标签已删除", "标签已从所有文件删除。");
                Logger::instance().log("标签已从所有文件删除。");
            } else {
                for (const auto &selectedFile : selectedFiles) {
                    fileTagSystem.removeTag(selectedFile.toStdString(), tag.toStdString());
                }
                QMessageBox::information(parentWidget, "标签已删除", "标签已从选中的文件中删除。");
                Logger::instance().log("标签已从选中的文件中删除。");
            }
            emit tagsUpdated();
        }
    }
}

void TagController::updateTag(QWidget *parentWidget) {
    QString filePath = QInputDialog::getText(parentWidget, "更新标签", "请输入文件路径:");
    QString oldTag = QInputDialog::getText(parentWidget, "更新标签", "请输入旧标签:");
    QString newTag = QInputDialog::getText(parentWidget, "更新标签", "请输入新标签:");

    if (!filePath.isEmpty() && !oldTag.isEmpty() && !newTag.isEmpty()) {
        fileTagSystem.updateTag(filePath.toStdString(), oldTag.toStdString(), newTag.toStdString());
        QMessageBox::information(parentWidget, "标签已更新", "文件中的标签已更新: " + filePath);
        Logger::instance().log("文件中的标签已更新: " + filePath);
        emit tagsUpdated();
    }
}
