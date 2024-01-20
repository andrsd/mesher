#pragma once

#include <QDialog>

class QVBoxLayout;
class QTextEdit;

class TextBoxDialog : public QDialog {
public:
    explicit TextBoxDialog(QWidget * parent = nullptr);
    ~TextBoxDialog() override;

    void loadTextFromFile(const QString & file_name);

protected:
    QVBoxLayout * layout;
    QTextEdit * text;
};
