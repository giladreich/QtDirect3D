

#include "MainWindow.h"


#include <QDebug>
#include <QTime>
#include <QMessageBox>
#include <QCloseEvent>


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindowClass)
{
	ui->setupUi(this);

	connect(ui->view, &QDirect3D9Widget::deviceInitialized, this, &MainWindow::init);
	connect(ui->view, &QDirect3D9Widget::ticked, this, &MainWindow::tick);
	connect(ui->view, &QDirect3D9Widget::rendered, this, &MainWindow::render);
}

bool MainWindow::init(bool success)
{
	if (!success)
		return false;

	

	m_bWindowInit = true;
	disconnect(ui->view, &QDirect3D9Widget::deviceInitialized, this, &MainWindow::init);
	return true;
}

void MainWindow::tick()
{

}

void MainWindow::render()
{

}

void MainWindow::closeEvent(QCloseEvent * event)
{
	event->ignore();
	ui->view->release();
	m_bWindowClosing = true;
	QTime dieTime = QTime::currentTime().addMSecs(500);
	while (QTime::currentTime() < dieTime)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

	event->accept();
}