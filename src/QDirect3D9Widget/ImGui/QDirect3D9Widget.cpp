/*
 *
 */

#pragma comment(lib, "d3d9.lib")


#include "QDirect3D9Widget.h"

#include <QDebug>
#include <QEvent>
#include <QMessageBox>
#include <QDateTime>

#include <iostream>

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#define RENDER_FRAME_MSECONDS 16


QDirect3D9Widget::QDirect3D9Widget(QWidget *parent)
    : QWidget(parent)
    , m_pDevice(nullptr)
    , m_pD3D(nullptr)
    , m_hWnd(reinterpret_cast<HWND>(winId()))
    , m_bDeviceInitialized(false)
    , m_bRenderActive(false)
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

    // Activate the timer
    connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D9Widget::onFrame);
    m_qTimer.start(RENDER_FRAME_MSECONDS);
    m_bRenderActive = true;
}

QDirect3D9Widget::~QDirect3D9Widget()
{
}

void QDirect3D9Widget::release()
{
    m_bDeviceInitialized = false;
    disconnect(&m_qTimer, &QTimer::timeout, this, &QDirect3D9Widget::onFrame);
    m_qTimer.stop();

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    m_pDevice->Release();
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
    if ((m_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
    {
        QMessageBox::critical(this, tr("ERROR"), tr("Failed to create Direct3D."), QMessageBox::Ok);
        return false;
    }

    memset(&m_PresentParams, 0, sizeof(m_PresentParams));
    m_PresentParams.Windowed = true;
    m_PresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
    m_PresentParams.BackBufferFormat = D3DFMT_UNKNOWN;
    m_PresentParams.EnableAutoDepthStencil = true;
    m_PresentParams.AutoDepthStencilFormat = D3DFMT_D16;
    m_PresentParams.BackBufferWidth = width();
    m_PresentParams.BackBufferHeight = height();
    m_PresentParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    m_PresentParams.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    HRESULT hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
                                      D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                      &m_PresentParams, &m_pDevice);
    if (hr != D3D_OK)
    {
        m_pD3D->Release();
        QMessageBox::critical(this, tr("ERROR"), tr("Failed to create Direct3D device."), QMessageBox::Ok);
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImGuiIO & io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsClassic();
    ImGui_ImplWin32_Init(m_hWnd);
    ImGui_ImplDX9_Init(m_pDevice);

    resetEnvironment();

    return true;
}

void QDirect3D9Widget::onFrame()
{
    // The ImGui and scene frames will always be rendered so the user can interact with the gui even if m_bRenderActive is false.
    // But we are not going to update the scene so it remains frozen.
    if (m_bRenderActive) tick();

    beginScene();
    render();
    uiRender();
    endScene();
}

void QDirect3D9Widget::beginScene()
{
    m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(155, 0, 51, 102), 1.0f, 0);
    m_pDevice->BeginScene();
}

void QDirect3D9Widget::endScene()
{
    m_pDevice->EndScene();
    RECT rc = { rect().left(), rect().top(), rect().right(), rect().bottom() };
    HRESULT hr = m_pDevice->Present(&rc, &rc, m_hWnd, NULL);
    if (hr == D3DERR_DEVICELOST && m_pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
    {
        onReset();
    }
}

void QDirect3D9Widget::tick()
{
    // TODO: Do your own widget updating before emitting ticked, i.e:
    //m_pCamera->Tick();

    emit ticked(); // Signals the parent to do it's own update before we start rendering.
}

void QDirect3D9Widget::render()
{
    // TODO: Do your own widget rendering before emitting rendered, i.e:
    //m_pCamera->Apply();

    emit rendered(); // Signals the parent to do it's own rendering before we presenting the scene.
}

void QDirect3D9Widget::uiRender()
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    emit uiRendered(); // Signals the parent to add it's own ImGui widgets.

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void QDirect3D9Widget::onReset()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    m_PresentParams.BackBufferWidth = width();
    m_PresentParams.BackBufferHeight = height();
    m_pDevice->Reset(&m_PresentParams);
    ImGui_ImplDX9_CreateDeviceObjects();
}

void QDirect3D9Widget::resetEnvironment()
{
    // TODO: Do your own custom default environment, i.e:
    //m_pCamera->resetCamera();
    m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(155, 0, 51, 102), 1.0f, 0);
    m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
    m_pDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);
    m_pDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);

    onReset();

    if (!m_bRenderActive) tick();
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
        // To make sure that we always have focus on the widget when we enter the rect area.
    case QEvent::Enter:
    case QEvent::FocusIn:
    case QEvent::FocusAboutToChange:
        if (::GetFocus() != m_hWnd)
        {
            QWidget * nativeParent = this;
            while (true)
            {
                if (nativeParent->isWindow()) break;

                QWidget * parent = nativeParent->nativeParentWidget();
                if (!parent) break;

                nativeParent = parent;
            }

            if (nativeParent && nativeParent != this && ::GetFocus() == reinterpret_cast<HWND>(nativeParent->winId()))
                ::SetFocus(m_hWnd);
        }
        break;
    }

    return QWidget::event(event);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT QDirect3D9Widget::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    // NOTE(Gilad): Windows messages can be handled here or you can also use the build-in Qt events.
    //switch (msg)
    //{
    //}

    return false;
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