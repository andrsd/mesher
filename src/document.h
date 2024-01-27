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
    ~Document() override;

    void destroy();
    void create();
    void load(const QString & file_name);
    void save();

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
    SaveThread * save_thread;

    bool has_file;
    GModel * gmodel;
};
