/*
 *
 */
#pragma comment(lib, "d3d10.lib")

#include "QDirect3D10Widget.h"

#include <QDebug>
#include <QEvent>
#include <QWheelEvent>

#include "imgui.h"
#include "backends/imgui_impl_dx10.h"
#include "backends/imgui_impl_win32.h"

constexpr int FPS_LIMIT    = 60.0f;
constexpr int MS_PER_FRAME = (int)((1.0f / FPS_LIMIT) * 1000.0f);

QDirect3D10Widget::QDirect3D10Widget(QWidget * parent)
    : QWidget(parent)
    , m_pDevice(Q_NULLPTR)
    , m_pSwapChain(Q_NULLPTR)
    , m_pRTView(Q_NULLPTR)
    , m_hWnd(reinterpret_cast<HWND>(winId()))
    , m_bDeviceInitialized(false)
    , m_bRenderActive(false)
    , m_bStarted(false)
    , m_BackColor{0.0f, 0.135f, 0.481f, 1.0f}
{
    qDebug() << "[QDirect3D10Widget::QDirect3D10Widget] - Widget Handle: " << m_hWnd;

    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);

    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_NativeWindow);

    // Setting these attributes to our widget and returning null on paintEngine event
    // tells Qt that we'll handle all drawing and updating the widget ourselves.
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
}

QDirect3D10Widget::~QDirect3D10Widget() {}

void QDirect3D10Widget::release()
{
    m_bDeviceInitialized = false;
    disconnect(&m_qTimer, &QTimer::timeout, this, &QDirect3D10Widget::onFrame);
    m_qTimer.stop();

    ImGui_ImplDX10_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    ReleaseObject(m_pRTView);
    ReleaseObject(m_pSwapChain);
    ReleaseObject(m_pDevice);
}

void QDirect3D10Widget::run()
{
    m_qTimer.start(MS_PER_FRAME);
    m_bRenderActive = m_bStarted = true;
}

void QDirect3D10Widget::pauseFrames()
{
    if (!m_qTimer.isActive() || !m_bStarted) return;

    disconnect(&m_qTimer, &QTimer::timeout, this, &QDirect3D10Widget::onFrame);
    m_qTimer.stop();
    m_bRenderActive = false;
}

void QDirect3D10Widget::continueFrames()
{
    if (m_qTimer.isActive() || !m_bStarted) return;

    connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D10Widget::onFrame);
    m_qTimer.start(MS_PER_FRAME);
    m_bRenderActive = true;
}

void QDirect3D10Widget::showEvent(QShowEvent * event)
{
    if (!m_bDeviceInitialized)
    {
        m_bDeviceInitialized = init();
        emit deviceInitialized(m_bDeviceInitialized);
    }

    QWidget::showEvent(event);
}

bool QDirect3D10Widget::init()
{
    DXGI_SWAP_CHAIN_DESC sd               = {};
    sd.BufferCount                        = 2;
    sd.BufferDesc.Width                   = width();
    sd.BufferDesc.Height                  = height();
    sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                       = m_hWnd;
    sd.SampleDesc.Count                   = 1;
    sd.SampleDesc.Quality                 = 0;
    sd.Windowed                           = TRUE;
    sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    UINT iCreateFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    iCreateFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr =
        D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, iCreateFlags,
                                      D3D10_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice);
    // Can't create hardware device, try software.
    if (hr != S_OK)
    {
        DXCall(D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_SOFTWARE, NULL,
                                             iCreateFlags, D3D10_SDK_VERSION, &sd,
                                             &m_pSwapChain, &m_pDevice));
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // ImGuiIO & io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsClassic();
    ImGui_ImplWin32_Init(m_hWnd);
    ImGui_ImplDX10_Init(m_pDevice);

    resetEnvironment();

    connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D10Widget::onFrame);

    return true;
}

void QDirect3D10Widget::onFrame()
{
    // The ImGui and scene frames will always be rendered so the user can interact with the gui
    // even if m_bRenderActive is false. But we are not going to update the scene so it remains
    // frozen.
    if (m_bRenderActive) tick();

    beginScene();
    render();
    renderUI();
    endScene();
}

void QDirect3D10Widget::beginScene()
{
    m_pDevice->OMSetRenderTargets(1, &m_pRTView, NULL);
    m_pDevice->ClearRenderTargetView(m_pRTView, reinterpret_cast<const float *>(&m_BackColor));
}

void QDirect3D10Widget::endScene()
{
    if (FAILED(m_pSwapChain->Present(1, 0))) { onReset(); }
}

void QDirect3D10Widget::tick()
{
    // TODO: Update your scene here. For aesthetics reasons, only do it here if it's an
    // important component, otherwise do it in the MainWindow.
    // m_pCamera->Tick();

    emit ticked();
}

void QDirect3D10Widget::render()
{
    // TODO: Present your scene here. For aesthetics reasons, only do it here if it's an
    // important component, otherwise do it in the MainWindow.
    // m_pCamera->Apply();

    emit rendered();
}

void QDirect3D10Widget::renderUI()
{
    ImGui_ImplDX10_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    emit renderedUI();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX10_RenderDrawData(ImGui::GetDrawData());
}

void QDirect3D10Widget::onReset()
{
    ID3D10Texture2D * pBackBuffer;
    ReleaseObject(m_pRTView);
    DXCall(m_pSwapChain->ResizeBuffers(0, width(), height(), DXGI_FORMAT_UNKNOWN, 0));
    DXCall(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
    DXCall(m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRTView));
    ReleaseObject(pBackBuffer);
}

void QDirect3D10Widget::resetEnvironment()
{
    // TODO: Add your own custom default environment, i.e:
    // m_pCamera->resetCamera();

    onReset();

    if (!m_bRenderActive) tick();
}

void QDirect3D10Widget::wheelEvent(QWheelEvent * event)
{
    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && event->angleDelta().x() == 0)
    {
        // TODO: Update your camera position based on the delta value.
    }
    else if (event->angleDelta().x() !=
             0) // horizontal scrolling - mice with another side scroller.
    {
        ImGui::GetIO().MouseWheelH += (float)(event->angleDelta().y() / WHEEL_DELTA);
    }
    else if (event->angleDelta().y() != 0)
    {
        ImGui::GetIO().MouseWheel += (float)(event->angleDelta().y() / WHEEL_DELTA);
    }

    QWidget::wheelEvent(event);
}

QPaintEngine * QDirect3D10Widget::paintEngine() const
{
    return Q_NULLPTR;
}

void QDirect3D10Widget::paintEvent(QPaintEvent * event) {}

void QDirect3D10Widget::resizeEvent(QResizeEvent * event)
{
    if (m_bDeviceInitialized)
    {
        onReset();
        emit widgetResized();
    }

    QWidget::resizeEvent(event);
}

bool QDirect3D10Widget::event(QEvent * event)
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

                if (nativeParent && nativeParent != this &&
                    ::GetFocus() == reinterpret_cast<HWND>(nativeParent->winId()))
                    ::SetFocus(m_hWnd);
            }
            break;
        case QEvent::KeyPress:
            emit keyPressed((QKeyEvent *)event);
            break;
        case QEvent::MouseMove:
            if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
                emit mouseMoved((QMouseEvent *)event);
            break;
        case QEvent::MouseButtonPress:
            if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) &&
                !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
                emit mouseClicked((QMouseEvent *)event);
            break;
        case QEvent::MouseButtonRelease:
            if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) &&
                !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
                emit mouseReleased((QMouseEvent *)event);
            break;
    }

    return QWidget::event(event);
}

extern LRESULT
    ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT QDirect3D10Widget::WndProc(MSG * pMsg)
{
    // Process wheel events using Qt's event-system.
    if (pMsg->message == WM_MOUSEWHEEL || pMsg->message == WM_MOUSEHWHEEL) return false;

    if (ImGui_ImplWin32_WndProcHandler(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam))
        return true;

    return false;
}

#if QT_VERSION >= 0x050000
bool QDirect3D10Widget::nativeEvent(const QByteArray & eventType,
                                    void *             message,
                                    long *             result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);

#    ifdef Q_OS_WIN
    MSG * pMsg = reinterpret_cast<MSG *>(message);
    return WndProc(pMsg);
#    endif

    return QWidget::nativeEvent(eventType, message, result);
}

#else // QT_VERSION < 0x050000
bool QDirect3D10Widget::winEvent(MSG * message, long * result)
{
    Q_UNUSED(result);

#    ifdef Q_OS_WIN
    MSG * pMsg = reinterpret_cast<MSG *>(message);
    return WndProc(pMsg);
#    endif

    return QWidget::winEvent(message, result);
}
#endif // QT_VERSION >= 0x050000
