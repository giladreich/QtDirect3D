/*
 *
 */

#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();

	void adjustWindowSize();

	bool init(bool success);
	void tick();
	void render();
	void uiRender();

private:
	void closeEvent(QCloseEvent * event) override;


private:
	Ui::MainWindowClass * ui;

	bool                      m_bWindowInit;
	bool                      m_bWindowClosing;

	bool m_bShowDemoWindow;
	bool m_bShowAnotherWindow;


};
