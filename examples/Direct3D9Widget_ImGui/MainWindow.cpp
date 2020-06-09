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

#include "imgui.h"

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
    connect(m_pScene, &QDirect3D9Widget::deviceInitialized, this, &MainWindow::init);
    connect(m_pScene, &QDirect3D9Widget::ticked, this, &MainWindow::tick);
    connect(m_pScene, &QDirect3D9Widget::rendered, this, &MainWindow::render);
    connect(m_pScene, &QDirect3D9Widget::renderedUI, this, &MainWindow::renderUI);

    // NOTE: Additionally, you can listen to some basic IO events.
    // connect(m_pScene, &QDirect3D9Widget::keyPressed, this, &MainWindow::onKeyPressed);
    // connect(m_pScene, &QDirect3D9Widget::mouseMoved, this, &MainWindow::onMouseMoved);
    // connect(m_pScene, &QDirect3D9Widget::mouseClicked, this, &MainWindow::onMouseClicked);
    // connect(m_pScene, &QDirect3D9Widget::mouseReleased, this, &MainWindow::onMouseReleased);
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
    ImGui::StyleColorsClassic();
    // ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Start processing frames with a short delay in case things are still initializing/loading
    // in the background.
    QTimer::singleShot(500, this, [&] { m_pScene->run(); });
    disconnect(m_pScene, &QDirect3D9Widget::deviceInitialized, this, &MainWindow::init);
}

void MainWindow::tick()
{
    // TODO: Update the scene here.
    // m_pMesh->Tick();
}

void MainWindow::render()
{
    // TODO: Present the scene here.
    // m_pMesh->Render();
}

void MainWindow::renderUI()
{
    static bool bShowDemoWindow    = true;
    static bool bShowAnotherWindow = false;
    if (bShowDemoWindow) ImGui::ShowDemoWindow(&bShowDemoWindow);

    {
        static float f       = 0.0f;
        static int   counter = 0;

        ImGui::Begin(
            "Hello, world!"); // Create a window called "Hello, world!" and append into it.
        ImGui::Text("This is some useful text."); // Display some text (you can use a format
                                                  // strings too)
        ImGui::Checkbox("Demo Window",
                        &bShowDemoWindow); // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &bShowAnotherWindow);

        // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

        // Edit 4 floats representing a color
        ImGui::ColorEdit4("BackColor", (float *)m_pScene->BackColor(),
                          ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR |
                              ImGuiColorEditFlags_PickerHueWheel);

        // Buttons return true when clicked (most widgets return true when edited/activated)
        if (ImGui::Button("Button")) counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    if (bShowAnotherWindow)
    {
        // Pass a pointer to our bool variable (the window will have a closing button that will
        // clear the bool when clicked)
        ImGui::Begin("Another Window", &bShowAnotherWindow);
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me")) bShowAnotherWindow = false;
        ImGui::End();
    }
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
