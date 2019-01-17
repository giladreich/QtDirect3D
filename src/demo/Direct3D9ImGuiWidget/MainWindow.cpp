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


#include "imgui.h"


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindowClass)
	, m_bShowDemoWindow(true)
	, m_bShowAnotherWindow(false)
{
	ui->setupUi(this);

	adjustWindowSize();

	connect(ui->view, &QDirect3D9Widget::deviceInitialized, this, &MainWindow::init);
	connect(ui->view, &QDirect3D9Widget::ticked, this, &MainWindow::tick);
	connect(ui->view, &QDirect3D9Widget::rendered, this, &MainWindow::render);
	connect(ui->view, &QDirect3D9Widget::uiRendered, this, &MainWindow::uiRender);
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
	disconnect(ui->view, &QDirect3D9Widget::deviceInitialized, this, &MainWindow::init);
	return true;
}

void MainWindow::tick()
{
	
}

void MainWindow::render()
{

}

void MainWindow::uiRender()
{
	if (m_bShowDemoWindow)
		ImGui::ShowDemoWindow(&m_bShowDemoWindow);

	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &m_bShowDemoWindow);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &m_bShowAnotherWindow);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	if (m_bShowAnotherWindow)
	{
		ImGui::Begin("Another Window", &m_bShowAnotherWindow);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			m_bShowAnotherWindow = false;
		ImGui::End();
	}
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