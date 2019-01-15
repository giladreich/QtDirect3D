#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);

	bool init(bool success);
	void tick();
	void render();

private:
	void closeEvent(QCloseEvent * event) override;


private:
	Ui::MainWindowClass * ui;

	bool                      m_bWindowInit;
	bool                      m_bWindowClosing;

};
