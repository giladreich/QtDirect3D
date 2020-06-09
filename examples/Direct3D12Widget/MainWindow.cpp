/*
 *
 */
#include "MainWindow.h"

#include <QStyle>
#include <QDebug>
#include <QTime>
#include <QScreen>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget * parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass)
    , m_WindowSize(QSize(1280, 800))
    , m_pCbxDoFrames(new QCheckBox(this))
{
    ui->setupUi(this);
    m_pScene = ui->view;

    adjustWindowSize();
    addToolbarWidgets();
    connectSlots();
}

MainWindow::~MainWindow() = default;

void MainWindow::adjustWindowSize()
{
    resize(m_WindowSize.width(), m_WindowSize.height());
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(),
                                    qApp->screens().first()->availableGeometry()));
}

void MainWindow::addToolbarWidgets()
{
    // Add CheckBox to tool-bar to stop/continue frames execution.
    m_pCbxDoFrames->setText("Do Frames");
    m_pCbxDoFrames->setChecked(true);
    connect(m_pCbxDoFrames, &QCheckBox::stateChanged, [&] {
        if (m_pCbxDoFrames->isChecked())
            m_pScene->continueFrames();
        else
            m_pScene->pauseFrames();
    });
    ui->mainToolBar->addWidget(m_pCbxDoFrames);
}

void MainWindow::connectSlots()
{
    connect(m_pScene, &QDirect3D12Widget::deviceInitialized, this, &MainWindow::init);
    connect(m_pScene, &QDirect3D12Widget::ticked, this, &MainWindow::tick);
    connect(m_pScene, &QDirect3D12Widget::rendered, this, &MainWindow::render);

    // NOTE: Additionally, you can listen to some basic IO events.
    // connect(m_pScene, &QDirect3D12Widget::keyPressed, this, &MainWindow::onKeyPressed);
    // connect(m_pScene, &QDirect3D12Widget::mouseMoved, this, &MainWindow::onMouseMoved);
    // connect(m_pScene, &QDirect3D12Widget::mouseClicked, this, &MainWindow::onMouseClicked);
    // connect(m_pScene, &QDirect3D12Widget::mouseReleased, this,
    // &MainWindow::onMouseReleased);
}

void MainWindow::init(bool success)
{
    if (!success)
    {
        QMessageBox::critical(this, "ERROR", "Direct3D widget initialization failed.",
                              QMessageBox::Ok);
        return;
    }

    // TODO: Add here your extra initialization here.
    // ...

    // Start processing frames with a short delay in case things are still initializing/loading
    // in the background.
    QTimer::singleShot(500, this, [&] { m_pScene->run(); });
    disconnect(m_pScene, &QDirect3D12Widget::deviceInitialized, this, &MainWindow::init);
}

void MainWindow::tick()
{
    // TODO: Update the scene here.
    // m_pMesh->Tick();
}

void MainWindow::render(ID3D12GraphicsCommandList * cl)
{
    // TODO: Use the command list pointer to queue items to be rendered.
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    event->ignore();
    m_pScene->release();
    QTime dieTime = QTime::currentTime().addMSecs(500);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    event->accept();
}
