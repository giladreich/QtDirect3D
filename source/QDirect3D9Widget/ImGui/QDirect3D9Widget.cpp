/*
 *
 */
#pragma comment(lib, "d3d9.lib")

#include "QDirect3D9Widget.h"

#include <QDebug>
#include <QEvent>
#include <QWheelEvent>

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

constexpr int FPS_LIMIT    = 60.0f;
constexpr int MS_PER_FRAME = (int)((1.0f / FPS_LIMIT) * 1000.0f);

QDirect3D9Widget::QDirect3D9Widget(QWidget * parent)
    : QWidget(parent)
    , m_pDevice(Q_NULLPTR)
    , m_pD3D(Q_NULLPTR)
    , m_hWnd(reinterpret_cast<HWND>(winId()))
    , m_bDeviceInitialized(false)
    , m_bRenderActive(false)
    , m_bStarted(false)
    , m_BackColor{0.0f, 0.135f, 0.481f, 1.0f}
{
    qDebug() << "[QDirect3D9Widget::QDirect3D9Widget] - Widget Handle: " << m_hWnd;

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

QDirect3D9Widget::~QDirect3D9Widget() {}

void QDirect3D9Widget::release()
{
    m_bDeviceInitialized = false;
    disconnect(&m_qTimer, &QTimer::timeout, this, &QDirect3D9Widget::onFrame);
    m_qTimer.stop();

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    ReleaseObject(m_pDevice);
}

void QDirect3D9Widget::run()
{
    m_qTimer.start(MS_PER_FRAME);
    m_bRenderActive = m_bStarted = true;
}

void QDirect3D9Widget::pauseFrames()
{
    if (!m_qTimer.isActive() || !m_bStarted) return;

    disconnect(&m_qTimer, &QTimer::timeout, this, &QDirect3D9Widget::onFrame);
    m_qTimer.stop();
    m_bRenderActive = false;
}

void QDirect3D9Widget::continueFrames()
{
    if (m_qTimer.isActive() || !m_bStarted) return;

    connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D9Widget::onFrame);
    m_qTimer.start(MS_PER_FRAME);
    m_bRenderActive = true;
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
        qDebug() << "[QDirect3D9Widget::init]: Failed to create Direct3D interface.";
        return false;
    }

    memset(&m_PresentParams, 0, sizeof(m_PresentParams));
    m_PresentParams.Windowed                   = true;
    m_PresentParams.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
    m_PresentParams.BackBufferFormat           = D3DFMT_UNKNOWN;
    m_PresentParams.EnableAutoDepthStencil     = true;
    m_PresentParams.AutoDepthStencilFormat     = D3DFMT_D16;
    m_PresentParams.BackBufferWidth            = width();
    m_PresentParams.BackBufferHeight           = height();
    m_PresentParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    m_PresentParams.PresentationInterval       = D3DPRESENT_INTERVAL_DEFAULT;

    HRESULT hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
                                      D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_PresentParams,
                                      &m_pDevice);
    // Can't create hardware device, try software.
    if (hr != S_OK)
    {
        DXCall(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, m_hWnd,
                                    D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_PresentParams,
                                    &m_pDevice));
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // ImGuiIO & io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsClassic();
    ImGui_ImplWin32_Init(m_hWnd);
    ImGui_ImplDX9_Init(m_pDevice);

    resetEnvironment();

    connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D9Widget::onFrame);

    return true;
}

void QDirect3D9Widget::onFrame()
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

void QDirect3D9Widget::beginScene()
{
    D3DCOLOR clearColor =
        D3DCOLOR_ARGB(155, (int)(m_BackColor.r * 255.0f), (int)(m_BackColor.g * 255.0f),
                      (int)(m_BackColor.b * 255.0f));
    DXCall(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0f, 0));
    DXCall(m_pDevice->BeginScene());
}

void QDirect3D9Widget::endScene()
{
    DXCall(m_pDevice->EndScene());
    RECT    rc = {rect().left(), rect().top(), rect().right(), rect().bottom()};
    HRESULT hr = m_pDevice->Present(&rc, &rc, m_hWnd, NULL);
    if (hr == D3DERR_DEVICELOST && m_pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
    { onReset(); }
}

void QDirect3D9Widget::tick()
{
    // TODO: Update your scene here. For aesthetics reasons, only do it here if it's an
    // important component, otherwise do it in the MainWindow.
    // m_pCamera->Tick();

    emit ticked();
}

void QDirect3D9Widget::render()
{
    // TODO: Present your scene here. For aesthetics reasons, only do it here if it's an
    // important component, otherwise do it in the MainWindow.
    // m_pCamera->Apply();

    emit rendered();
}

void QDirect3D9Widget::renderUI()
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    emit renderedUI();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void QDirect3D9Widget::onReset()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    m_PresentParams.BackBufferWidth  = width();
    m_PresentParams.BackBufferHeight = height();
    DXCall(m_pDevice->Reset(&m_PresentParams));
    ImGui_ImplDX9_CreateDeviceObjects();
}

void QDirect3D9Widget::resetEnvironment()
{
    // TODO: Add your own custom default environment, i.e:
    // m_pCamera->resetCamera();

    D3DCOLOR clearColor =
        D3DCOLOR_ARGB(155, (int)(m_BackColor.r * 255.0f), (int)(m_BackColor.g * 255.0f),
                      (int)(m_BackColor.b * 255.0f));
    DXCall(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0f, 0));
    DXCall(m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE));
    DXCall(m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
    DXCall(m_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false));
    DXCall(m_pDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE));
    DXCall(m_pDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE));

    onReset();

    if (!m_bRenderActive) tick();
}

void QDirect3D9Widget::wheelEvent(QWheelEvent * event)
{
    if (!ImGui::IsAnyWindowHovered() && event->angleDelta().x() == 0)
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

QPaintEngine * QDirect3D9Widget::paintEngine() const
{
    return Q_NULLPTR;
}

void QDirect3D9Widget::paintEvent(QPaintEvent * event) {}

void QDirect3D9Widget::resizeEvent(QResizeEvent * event)
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

                if (nativeParent && nativeParent != this &&
                    ::GetFocus() == reinterpret_cast<HWND>(nativeParent->winId()))
                    ::SetFocus(m_hWnd);
            }
            break;
        case QEvent::KeyPress:
            emit keyPressed((QKeyEvent *)event);
            break;
        case QEvent::MouseMove:
            if (!ImGui::IsAnyWindowHovered()) emit mouseMoved((QMouseEvent *)event);
            break;
        case QEvent::MouseButtonPress:
            if (!ImGui::IsAnyWindowHovered() && !ImGui::IsAnyWindowFocused())
                emit mouseClicked((QMouseEvent *)event);
            break;
        case QEvent::MouseButtonRelease:
            if (!ImGui::IsAnyWindowHovered() && !ImGui::IsAnyWindowFocused())
                emit mouseReleased((QMouseEvent *)event);
            break;
    }

    return QWidget::event(event);
}

extern LRESULT
    ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// NOTE: Native windows messages can be handled here or you can also use the build-in Qt
// events.
LRESULT QDirect3D9Widget::WndProc(MSG * pMsg)
{
    // Process wheel events using Qt's event-system.
    if (pMsg->message == WM_MOUSEWHEEL || pMsg->message == WM_MOUSEHWHEEL) return false;

    if (ImGui_ImplWin32_WndProcHandler(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam))
        return true;

    return false;
}

#if QT_VERSION >= 0x050000
bool QDirect3D9Widget::nativeEvent(const QByteArray & eventType, void * message, long * result)
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
bool QDirect3D9Widget::winEvent(MSG * message, long * result)
{
    Q_UNUSED(result);

#    ifdef Q_OS_WIN
    MSG * pMsg = reinterpret_cast<MSG *>(message);
    return WndProc(pMsg);
#    endif

    return QWidget::winEvent(message, result);
}
#endif // QT_VERSION >= 0x050000
