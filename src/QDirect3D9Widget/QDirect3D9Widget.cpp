/*
 * 
 */

#include "QDirect3D9Widget.h"

#include <QDebug>
#include <QEvent>
#include <QMessageBox>
#include <QDateTime>

#include <iostream>



QDirect3D9Widget::QDirect3D9Widget(QWidget *parent)
	: QWidget(parent)
	, m_hWnd(reinterpret_cast<HWND>(winId()))
	, m_bDeviceInitialized(false)
{
	qDebug() << "[QDirect3D9Widget::QDirect3D9Widget] - Widget Handle: " << m_hWnd;

	QPalette pal = palette();
	pal.setColor(QPalette::Background, Qt::black);
	setAutoFillBackground(true);
	setPalette(pal);

	setFocusPolicy(Qt::StrongFocus);
	setAttribute(Qt::WA_NativeWindow);

	// Setting these attributes to our widget and returning nullptr on paintEngine event 
	// tells Qt that we'll handle all drawing and updating the widget ourselves.
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);

	connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D9Widget::onFrame);
	m_qTimer.start(5);
}

QDirect3D9Widget::~QDirect3D9Widget()
{
}

void QDirect3D9Widget::release()
{
	disconnect(&m_qTimer, &QTimer::timeout, this, &QDirect3D9Widget::onFrame);
	m_qTimer.stop();
	m_lpD3DDev->Release();
}

void QDirect3D9Widget::showEvent(QShowEvent * event)
{
	if (!m_bDeviceInitialized)
	{
		m_bDeviceInitialized = init();
		emit deviceInitialized(m_bDeviceInitialized);
	}

	QWidget::showEvent(event);
}

bool QDirect3D9Widget::init()
{
	if ((m_lpD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
	{
		QMessageBox::critical(this, tr("ERROR"), tr("Failed to create Direct3D."), QMessageBox::Ok);
		return false;
	}

	memset(&m_DevParam, 0, sizeof(m_DevParam));
	m_DevParam.Windowed = TRUE;
	m_DevParam.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_DevParam.BackBufferFormat = D3DFMT_UNKNOWN;
	m_DevParam.EnableAutoDepthStencil = TRUE;
	m_DevParam.AutoDepthStencilFormat = D3DFMT_D16;
	m_DevParam.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync
	//m_DevParam.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // Present without vsync, maximum unthrottled framerate

	if (m_lpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_DevParam, &m_lpD3DDev) < 0)
	{
		m_lpD3D->Release();
		QMessageBox::critical(this, tr("ERROR"), tr("Failed to create Direct3D device."), QMessageBox::Ok);
		return false;
	}

	onReset();

	return true;
}

void QDirect3D9Widget::onFrame()
{
	tick();
	render();
}

void QDirect3D9Widget::tick()
{
	//m_pCamera->Tick();

	emit ticked(); // Signal the parent to do it's own update before we start rendering.
}

void QDirect3D9Widget::render()
{
	D3DCOLOR bColor = D3DCOLOR_RGBA((int)(0.45f*255.0f), (int)(0.55f*255.0f), (int)(0.60f*255.0f), (int)(1.00f*255.0f));
	m_lpD3DDev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, bColor, 1.0f, 0);

	m_lpD3DDev->BeginScene();

	//m_pCamera->Apply();

	emit rendered(); // Signal the parent to do it's own rendering before we presenting the scene.

	m_lpD3DDev->EndScene();
	//RECT rc = { rect().left(), rect().top(), rect().right(), rect().bottom() };
	HRESULT hr = m_lpD3DDev->Present(NULL, NULL, NULL, NULL);
	if (hr == D3DERR_DEVICELOST && m_lpD3DDev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		onReset();
	}
}

void QDirect3D9Widget::onReset()
{
	m_lpD3DDev->Reset(&m_DevParam);
}

void QDirect3D9Widget::resetEnvironment()
{
	//m_pCamera->resetCamera();
}

QPaintEngine * QDirect3D9Widget::paintEngine() const
{
	return nullptr;
}

void QDirect3D9Widget::paintEvent(QPaintEvent * event)
{ }

void QDirect3D9Widget::resizeEvent(QResizeEvent* event)
{
	if (m_bDeviceInitialized)
	{
		m_DevParam.BackBufferWidth = width();
		m_DevParam.BackBufferHeight = height();
		onReset();
		emit widgetResized();
	}
	QWidget::resizeEvent(event);
}

bool QDirect3D9Widget::event(QEvent * event)
{
	switch (event->type())
	{
	// Workaround for https://bugreports.qt.io/browse/QTBUG-42183 to get key strokes.
	case QEvent::Enter:
	case QEvent::FocusIn:
	case QEvent::FocusAboutToChange:
		if (/*this->hasFocus() && */::GetFocus() != m_hWnd)
		{
			QWidget* nativeParent = this;
			while (true)
			{
				if (nativeParent->isWindow())
					break;

				auto parent = nativeParent->nativeParentWidget();
				if (!parent)
					break;

				nativeParent = parent;
			}

			if (nativeParent && nativeParent != this && ::GetFocus() == reinterpret_cast<HWND>(nativeParent->winId()))
				::SetFocus(m_hWnd);
		}
		break;
	}

	return QWidget::event(event);
}

LRESULT QDirect3D9Widget::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	return FALSE;
}

#if QT_VERSION >= 0x050000
bool QDirect3D9Widget::nativeEvent(const QByteArray & eventType, void * message, long * result)
{
	Q_UNUSED(eventType);
	Q_UNUSED(result);

#ifdef Q_OS_WIN
	MSG * pMsg = reinterpret_cast< MSG * >(message);
	return WndProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
#endif

	return QWidget::nativeEvent(eventType, message, result);
}

#else // QT_VERSION < 0x050000
bool QDirect3D9Widget::winEvent(MSG * message, long * result)
{
	Q_UNUSED(result);

#ifdef Q_OS_WIN
	MSG * pMsg = reinterpret_cast< MSG * >(message);
	return WndProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
#endif

	return QWidget::winEvent(message, result);
}
#endif // QT_VERSION >= 0x050000