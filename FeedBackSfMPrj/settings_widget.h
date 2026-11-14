#pragma once
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>

class SettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget* parent = nullptr);
    void setFilePath(const QString& filePath);

private slots:
    void loadFile();
    void saveFile();

private:
    QString m_filePath;
    QTextEdit* m_textEdit;
};
