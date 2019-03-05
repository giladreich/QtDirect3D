/*
 *
 */

#include "MainWindow.h"

#include <Windows.h>

#include <QDesktopWidget>
#include <QStyle>
#include <QDebug>
#include <QTime>
#include <QMessageBox>
#include <QCloseEvent>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass)
{
    ui->setupUi(this);

    adjustWindowSize();

    connect(ui->view, &QDirect3D10Widget::deviceInitialized, this, &MainWindow::init);
    connect(ui->view, &QDirect3D10Widget::ticked, this, &MainWindow::tick);
    connect(ui->view, &QDirect3D10Widget::rendered, this, &MainWindow::render);
}

MainWindow::~MainWindow() = default;

void MainWindow::adjustWindowSize()
{
    resize(1280, 800);
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            size(),
            qApp->desktop()->availableGeometry()
        )
    );
}

bool MainWindow::init(bool success)
{
    if (!success)
        return false;

    ui->view->setRenderActive(true);

    m_bWindowInit = true;
    disconnect(ui->view, &QDirect3D10Widget::deviceInitialized, this, &MainWindow::init);
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