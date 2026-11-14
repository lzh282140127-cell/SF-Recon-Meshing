#pragma once

#include "baselogging.h"

#include <QtGui>
#include <QtWidgets>
#include <fstream>
#include <iostream>

class LogWidget : public QWidget {
public:
	explicit LogWidget(QWidget* parent, int max_num_blocks = 500000);

	void Append(const std::string& text);
	void Flush();
	void Clear();
	void SaveLog();

private:
	QMutex mutex_;
	std::string text_queue_;
	QPlainTextEdit* text_box_;
	std::unique_ptr<google::LogSink> log_sink_;
	std::unique_ptr<std::streambuf> redirector_;
};
