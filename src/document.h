#pragma once

#include <QObject>
#include <QString>

class GModel;
class LoadThread;
class SaveThread;

class Document : public QObject {
    Q_OBJECT;

public:
    Document();
    ~Document();

    void destroy();
    void create();
    void load(const QString & file_name);
    void save(const QString & file_name);

    bool hasFile() const;
    QString getFileName() const;

signals:
    void loadFinished();
    void saveFinished();

public slots:
    void onLoadFinished();
    void onSaveFinished();

protected:
    LoadThread * load_thread;
    LoadThread * save_thread;

    bool has_file;
    GModel * gmodel;
};
