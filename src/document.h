#pragma once

#include <QString>

class GModel;

class Document {
public:
    Document();
    ~Document();

    void destroy();
    void create();
    void load(const QString & file_name);

    bool hasFile() const;
    QString getFileName() const;

protected:
    bool has_file;
    GModel * gmodel;
};
