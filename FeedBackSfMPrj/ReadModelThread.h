#pragma once
#include <QThread>
#include <memory>
#include <QString>
#include <QDebug>
#include "Reconstruction.h"  

class ReadModelThread : public QThread {
    Q_OBJECT

public:
    ReadModelThread(std::shared_ptr<Reconstruction> reconstruction, const std::string& modelPath)
        : reconstruction_(reconstruction), modelPath_(modelPath) {}

protected:
    void run() override {
        // Load the model on the worker thread.
        reconstruction_->Read(modelPath_);
        
        // Notify the UI when loading completes.
        emit finishedReading();
    }

signals:
    void finishedReading();  

private:
    std::shared_ptr<Reconstruction> reconstruction_;
    std::string modelPath_;
};
