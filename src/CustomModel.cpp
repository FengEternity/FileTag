#include "CustomModel.h"
#include "Logger.h"
#include <QDebug>

CustomModel::CustomModel(QObject *parent)
        : QAbstractListModel(parent) {}

int CustomModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return fileList.size();
}

QVariant CustomModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= fileList.size() || role != Qt::DisplayRole)
        return QVariant();
    return fileList.at(index.row());
}

void CustomModel::addFile(const QString &filePath) {
    beginInsertRows(QModelIndex(), fileList.size(), fileList.size());
    fileList << filePath;
    endInsertRows();
    // qDebug() << "File added to model: " << filePath;
    // Logger::instance().log("File added to model: " + filePath);
}

void CustomModel::clear() {
    beginResetModel();
    fileList.clear();
    endResetModel();
    // qDebug() << "Model cleared.";
    // Logger::instance().log("Model cleared.");
}
