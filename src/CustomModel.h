#ifndef CUSTOMMODEL_H
#define CUSTOMMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class CustomModel : public QAbstractListModel {
Q_OBJECT

public:
    CustomModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void addFile(const QString &filePath);
    void clear();

private:
    QStringList fileList;
};

#endif // CUSTOMMODEL_H
