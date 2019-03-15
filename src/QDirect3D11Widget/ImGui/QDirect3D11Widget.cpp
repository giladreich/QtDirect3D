/*
 *
 */

#pragma comment(lib, "d3d11.lib")


#include "QDirect3D11Widget.h"

#include <QDebug>
#include <QEvent>
#include <QMessageBox>
#include <QDateTime>

#include <exception>

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#define RENDER_FRAME_MSECONDS 16


QDirect3D11Widget::QDirect3D11Widget(QWidget *parent)
    : QWidget(parent)
    , m_pDevice(nullptr)
    , m_pDeviceContext(nullptr)
    , m_pSwapChain(nullptr)
    , m_pRTView(nullptr)
    , m_hWnd(reinterpret_cast<HWND>(winId()))
    , m_bDeviceInitialized(false)
    , m_bRenderActive(false)
    , m_BackColor{ 0.0f, 0.135f, 0.481f, 1.0f }
{
    qDebug() << "[QDirect3D11Widget::QDirect3D11Widget] - Widget Handle: " << m_hWnd;

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
}

QDirect3D11Widget::~QDirect3D11Widget()
{ }

void QDirect3D11Widget::release()
{
    m_bDeviceInitialized = false;
    disconnect(&m_qTimer, &QTimer::timeout, this, &QDirect3D11Widget::onFrame);
    m_qTimer.stop();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    ReleaseObject(m_pRTView);
    ReleaseObject(m_pSwapChain);
    ReleaseObject(m_pDeviceContext);
    ReleaseObject(m_pDevice);
}

void QDirect3D11Widget::showEvent(QShowEvent * event)
{
    if (!m_bDeviceInitialized)
    {
        m_bDeviceInitialized = init();
        emit deviceInitialized(m_bDeviceInitialized);
    }

    QWidget::showEvent(event);
}

bool QDirect3D11Widget::init()
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width();
    sd.BufferDesc.Height = height();
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT iCreateFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    iCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    DXCall(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE,
                                         NULL, iCreateFlags,
                                         featureLevels, _countof(featureLevels),
                                         D3D11_SDK_VERSION, &sd,
                                         &m_pSwapChain, &m_pDevice,
                                         &featureLevel, &m_pDeviceContext));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImGuiIO & io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsClassic();
    ImGui_ImplWin32_Init(m_hWnd);
    ImGui_ImplDX11_Init(m_pDevice, m_pDeviceContext);

    resetEnvironment();

    // Activates the timer to render frames
    connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D11Widget::onFrame);
    m_qTimer.start(RENDER_FRAME_MSECONDS);
    m_bRenderActive = true;

    return true;
}

void QDirect3D11Widget::onFrame()
{
    // The ImGui and scene frames will always be rendered so the user can interact with the gui even if m_bRenderActive is false.
    // But we are not going to update the scene so it remains frozen.
    if (m_bRenderActive) tick();

    beginScene();
    render();
    uiRender();
    endScene();
}

void QDirect3D11Widget::beginScene()
{
    m_pDeviceContext->OMSetRenderTargets(1, &m_pRTView, NULL);
    m_pDeviceContext->ClearRenderTargetView(m_pRTView, reinterpret_cast<const float *>(&m_BackColor));
}

void QDirect3D11Widget::endScene()
{
    if (FAILED(m_pSwapChain->Present(1, 0)))
    {
        onReset();
    }
}

void QDirect3D11Widget::tick()
{
    // TODO: Do your own widget updating before emitting ticked, i.e:
    //m_pCamera->Tick();

    emit ticked(); // Signals the parent to do it's own update before we start rendering.
}

void QDirect3D11Widget::render()
{
    // TODO: Do your own widget rendering before emitting rendered, i.e:
    //m_pCamera->Apply();

    emit rendered(); // Signals the parent to do it's own rendering before we presenting the scene.
}

void QDirect3D11Widget::uiRender()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    emit uiRendered(); // Signals the parent to add it's own ImGui widgets.

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void QDirect3D11Widget::onReset()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    ReleaseObject(m_pRTView);
    DXCall(m_pSwapChain->ResizeBuffers(0, width(), height(), DXGI_FORMAT_UNKNOWN, 0));
    DXCall(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
    DXCall(m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRTView));
    ReleaseObject(pBackBuffer);
}

void QDirect3D11Widget::resetEnvironment()
{
    // TODO: Do your own custom default environment, i.e:
    //m_pCamera->resetCamera();

    onReset();

    if (!m_bRenderActive) tick();
}

QPaintEngine * QDirect3D11Widget::paintEngine() const
{
    return nullptr;
}

void QDirect3D11Widget::paintEvent(QPaintEvent * event)
{ }

void QDirect3D11Widget::resizeEvent(QResizeEvent* event)
{
    if (m_bDeviceInitialized)
    {
        onReset();
        emit widgetResized();
    }

    QWidget::resizeEvent(event);
}

bool QDirect3D11Widget::event(QEvent * event)
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

LRESULT QDirect3D11Widget::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
bool QDirect3D11Widget::nativeEvent(const QByteArray & eventType, void * message, long * result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);

#ifdef Q_OS_WIN
    MSG * pMsg = reinterpret_cast<MSG *>(message);
    return WndProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
#endif

    return QWidget::nativeEvent(eventType, message, result);
}

#else // QT_VERSION < 0x050000
bool QDirect3D11Widget::winEvent(MSG * message, long * result)
{
    Q_UNUSED(result);

#ifdef Q_OS_WIN
    MSG * pMsg = reinterpret_cast<MSG *>(message);
    return WndProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
#endif

    return QWidget::winEvent(message, result);
}
#endif // QT_VERSION >= 0x050000