#pragma once

#include "basethreading.h"

#include <QtCore>
#include <QtWidgets>
#include <memory>

class ThreadControlWidget : public QWidget {
public:
    explicit ThreadControlWidget(QWidget* parent);

    void StartThread(const QString& progress_text,
        bool stoppable,
        std::unique_ptr<Thread> thread);
    void StartFunction(const QString& progress_text,
        const std::function<void()>& func);

private:
    QProgressDialog* progress_bar_;
    QAction* destructor_;
    std::unique_ptr<Thread> thread_;
};
