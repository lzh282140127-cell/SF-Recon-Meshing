#include "settings_widget.h"
#include "settings.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

SettingsWidget::SettingsWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_textEdit = new QTextEdit(this);
    layout->addWidget(m_textEdit);

    // Set up signals for text changes
    connect(m_textEdit, &QTextEdit::textChanged, this, &SettingsWidget::saveFile);

    setLayout(layout);
}

void SettingsWidget::setFilePath(const QString& filePath) {
    m_filePath = filePath;
    loadFile();
}

void SettingsWidget::loadFile() {
    if (m_filePath.isEmpty())
        return;

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file: " + file.errorString());
        return;
    }

    QTextStream in(&file);
    m_textEdit->setPlainText(in.readAll());
    file.close();
}

void SettingsWidget::saveFile() {
    if (m_filePath.isEmpty())
        return;

    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot save file: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    out << m_textEdit->toPlainText();
    file.close();

    EasyIni* settings = EasyIni::GetPtr();
    settings->fresh();
}
