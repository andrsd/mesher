#pragma once

#include <QObject>
#include <QString>

class GModel;
class LoadThread;

class Document : public QObject {
    Q_OBJECT;

public:
    Document();
    ~Document();

    void destroy();
    void create();
    void load(const QString & file_name);

    bool hasFile() const;
    QString getFileName() const;

signals:
    void loadFinished();

public slots:
    void onLoadFinished();

protected:
    LoadThread * load_thread;

    bool has_file;
    GModel * gmodel;
};
