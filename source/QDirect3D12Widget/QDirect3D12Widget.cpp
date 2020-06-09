/*
 *
 */
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "QDirect3D12Widget.h"

#include <QDebug>
#include <QEvent>
#include <QWheelEvent>

using Microsoft::WRL::ComPtr;

constexpr int FPS_LIMIT    = 60.0f;
constexpr int MS_PER_FRAME = (int)((1.0f / FPS_LIMIT) * 1000.0f);

QDirect3D12Widget::QDirect3D12Widget(QWidget * parent)
    : QWidget(parent)
    , m_iCurrFrameIndex(0)
    , m_pDevice(Q_NULLPTR)
    , m_pFactory(Q_NULLPTR)
    , m_pSwapChain(Q_NULLPTR)
    , m_pCommandQueue(Q_NULLPTR)
    , m_pCommandAllocators{}
    , m_pCommandList(Q_NULLPTR)
    , m_pRTVDescHeap(Q_NULLPTR)
    , m_iRTVDescSize(0)
    , m_pRTVResources{}
    , m_RTVDescriptors{}
    , m_pSrvDescHeap(Q_NULLPTR)
    , m_hSwapChainEvent(Q_NULLPTR)
    , m_hFenceEvent(Q_NULLPTR)
    , m_pFence(Q_NULLPTR)
    , m_iFenceValues{}
    , m_hWnd(reinterpret_cast<HWND>(winId()))
    , m_bDeviceInitialized(false)
    , m_bRenderActive(false)
    , m_bStarted(false)
    , m_BackColor{0.0f, 0.135f, 0.481f, 1.0f}
{
    qDebug() << "[QDirect3D12Widget::QDirect3D12Widget] - Widget Handle: " << m_hWnd;

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

QDirect3D12Widget::~QDirect3D12Widget() {}

void QDirect3D12Widget::release()
{
    m_bDeviceInitialized = false;
    disconnect(&m_qTimer, &QTimer::timeout, this, &QDirect3D12Widget::onFrame);
    m_qTimer.stop();

    waitForGpu();

    for (UINT i = 0; i < FRAME_COUNT; i++) ReleaseObject(m_pRTVResources[i]);
    ReleaseObject(m_pSwapChain);
    ReleaseHandle(m_hSwapChainEvent);
    ReleaseObject(m_pCommandQueue);
    for (UINT i = 0; i < FRAME_COUNT; i++) ReleaseObject(m_pCommandAllocators[i]);
    ReleaseObject(m_pCommandList);
    ReleaseObject(m_pRTVDescHeap);
    ReleaseObject(m_pSrvDescHeap);
    ReleaseObject(m_pFence);
    ReleaseHandle(m_hFenceEvent);
    ReleaseObject(m_pDevice);
    ReleaseObject(m_pFactory);
}

void QDirect3D12Widget::run()
{
    m_qTimer.start(MS_PER_FRAME);
    m_bRenderActive = m_bStarted = true;
}

void QDirect3D12Widget::pauseFrames()
{
    if (!m_qTimer.isActive() || !m_bStarted) return;

    disconnect(&m_qTimer, &QTimer::timeout, this, &QDirect3D12Widget::onFrame);
    m_qTimer.stop();
    m_bRenderActive = false;
}

void QDirect3D12Widget::continueFrames()
{
    if (m_qTimer.isActive() || !m_bStarted) return;

    connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D12Widget::onFrame);
    m_qTimer.start(MS_PER_FRAME);
    m_bRenderActive = true;
}

void QDirect3D12Widget::showEvent(QShowEvent * event)
{
    if (!m_bDeviceInitialized)
    {
        m_bDeviceInitialized = init();
        emit deviceInitialized(m_bDeviceInitialized);
    }

    QWidget::showEvent(event);
}

bool QDirect3D12Widget::init()
{
    create3DDevice();

    resetEnvironment();

    connect(&m_qTimer, &QTimer::timeout, this, &QDirect3D12Widget::onFrame);

    return true;
}

void QDirect3D12Widget::create3DDevice()
{
    UINT factoryFlags = 0;

#ifdef _DEBUG
    {
        ComPtr<ID3D12Debug> dx12Debug;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dx12Debug))))
        {
            dx12Debug->EnableDebugLayer();

            // Enable additional debug layers.
            factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    DXCall(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_pFactory)));

    // Try and get hardware adapter compatible with d3d12, if not found, use wrap.
    ComPtr<IDXGIAdapter1> adapter;
    getHardwareAdapter(m_pFactory, adapter.GetAddressOf());
    if (!adapter) DXCall(m_pFactory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)));

    DXCall(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice)));

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;
    DXCall(m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue)));

    // Describe and create the swap chain.
    {
        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.BufferCount           = FRAME_COUNT;
        sd.Width                 = width();
        sd.Height                = height();
        sd.Format                = DXGI_FORMAT_R8G8B8A8_UNORM;
        // sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        sd.Flags              = 0;
        sd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count   = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Scaling            = DXGI_SCALING_NONE;
        sd.Stereo             = FALSE;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSd = {};
        fsSd.Windowed                        = TRUE;

        ComPtr<IDXGISwapChain1> swapChain1;
        DXCall(m_pFactory->CreateSwapChainForHwnd(m_pCommandQueue, m_hWnd, &sd, &fsSd,
                                                  Q_NULLPTR, swapChain1.GetAddressOf()));
        DXCall(swapChain1->QueryInterface(IID_PPV_ARGS(&m_pSwapChain)));
        m_iCurrFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
    }

    // Create render target view(RTV) descriptor heaps and handles.
    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
    rtvDesc.NumDescriptors             = FRAME_COUNT;
    rtvDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    DXCall(m_pDevice->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&m_pRTVDescHeap)));
    m_iRTVDescSize =
        m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        m_pRTVDescHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < FRAME_COUNT; i++)
    {
        m_RTVDescriptors[i] = rtvHandle;
        DXCall(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pRTVResources[i])));
        m_pDevice->CreateRenderTargetView(m_pRTVResources[i], Q_NULLPTR, m_RTVDescriptors[i]);
        rtvHandle.Offset(1, m_iRTVDescSize);
    }

    // Create shader resource view(SRV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC srvDesc = {};
    srvDesc.NumDescriptors             = 1;
    srvDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    DXCall(m_pDevice->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&m_pSrvDescHeap)));

    // Create command allocator for each frame.
    for (UINT i = 0; i < FRAME_COUNT; i++)
    {
        DXCall(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                 IID_PPV_ARGS(&m_pCommandAllocators[i])));
    }

    // Create command list. We don't create PSO here, so we set it to Q_NULLPTR to use the
    // default PSO. Command list by default set on recording state when created, therefore we
    // close it for now.
    DXCall(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                        m_pCommandAllocators[m_iCurrFrameIndex], Q_NULLPTR,
                                        IID_PPV_ARGS(&m_pCommandList)));
    DXCall(m_pCommandList->Close());

    // Create synchronized objects.
    DXCall(m_pDevice->CreateFence(m_iFenceValues[m_iCurrFrameIndex], D3D12_FENCE_FLAG_NONE,
                                  IID_PPV_ARGS(&m_pFence)));
    m_iFenceValues[m_iCurrFrameIndex]++;

    m_hFenceEvent = CreateEvent(Q_NULLPTR, FALSE, FALSE, Q_NULLPTR);
    if (!m_hFenceEvent) DXCall(HRESULT_FROM_WIN32(GetLastError()));

    // DXCall(m_pSwapChain->SetMaximumFrameLatency(FRAME_COUNT));
    // m_hSwapChainEvent = m_pSwapChain->GetFrameLatencyWaitableObject();

    // Wait for the GPU to complete our setup before proceeding.
    waitForGpu();
}

void QDirect3D12Widget::onFrame()
{
    if (m_bRenderActive) tick();

    beginScene();
    render();
    endScene();
}

void QDirect3D12Widget::beginScene()
{
    DXCall(m_pCommandAllocators[m_iCurrFrameIndex]->Reset());
    DXCall(m_pCommandList->Reset(m_pCommandAllocators[m_iCurrFrameIndex], Q_NULLPTR));

    m_pCommandList->ResourceBarrier(
        1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRTVResources[m_iCurrFrameIndex],
                                                 D3D12_RESOURCE_STATE_PRESENT,
                                                 D3D12_RESOURCE_STATE_RENDER_TARGET));
}

void QDirect3D12Widget::endScene()
{
    m_pCommandList->ResourceBarrier(
        1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRTVResources[m_iCurrFrameIndex],
                                                 D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                 D3D12_RESOURCE_STATE_PRESENT));

    DXCall(m_pCommandList->Close());
    m_pCommandQueue->ExecuteCommandLists(
        1, reinterpret_cast<ID3D12CommandList * const *>(&m_pCommandList));

    DXCall(m_pSwapChain->Present(1, 0));

    moveToNextFrame();
}

void QDirect3D12Widget::tick()
{
    // TODO: Update your scene here. For aesthetics reasons, only do it here if it's an
    // important component, otherwise do it in the MainWindow.
    // m_pCamera->Tick();

    emit ticked();
}

void QDirect3D12Widget::render()
{
    // Start recording the render commands
    m_pCommandList->ClearRenderTargetView(m_RTVDescriptors[m_iCurrFrameIndex],
                                          reinterpret_cast<const float *>(&m_BackColor), 0,
                                          Q_NULLPTR);
    m_pCommandList->OMSetRenderTargets(1, &m_RTVDescriptors[m_iCurrFrameIndex], FALSE,
                                       Q_NULLPTR);
    m_pCommandList->SetDescriptorHeaps(1, &m_pSrvDescHeap);

    // TODO: Present your scene here. For aesthetics reasons, only do it here if it's an
    // important component, otherwise do it in the MainWindow.
    // m_pCamera->Apply();

    emit rendered(m_pCommandList);
}

void QDirect3D12Widget::onReset()
{
    // TODO(Gilad): FIXME: this needs to be done in a synchronized manner. Need to look at
    // DirectX-12 samples here: https://github.com/microsoft/DirectX-Graphics-Samples how to
    // properly do this without leaking memory.
    pauseFrames();
    resizeSwapChain(width(), height());
    continueFrames();
}

void QDirect3D12Widget::cleanupRenderTarget()
{
    waitForGpu();

    for (UINT i = 0; i < FRAME_COUNT; i++)
    {
        ReleaseObject(m_pRTVResources[i]);
        m_iFenceValues[i] = m_iFenceValues[m_iCurrFrameIndex];
    }
}

void QDirect3D12Widget::createRenderTarget()
{
    for (UINT i = 0; i < FRAME_COUNT; i++)
    {
        DXCall(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pRTVResources[i])));
        m_pDevice->CreateRenderTargetView(m_pRTVResources[i], Q_NULLPTR, m_RTVDescriptors[i]);
    }
}

void QDirect3D12Widget::waitForGpu()
{
    DXCall(m_pCommandQueue->Signal(m_pFence, m_iFenceValues[m_iCurrFrameIndex]));

    DXCall(m_pFence->SetEventOnCompletion(m_iFenceValues[m_iCurrFrameIndex], m_hFenceEvent));
    WaitForSingleObject(m_hFenceEvent, INFINITE);

    m_iFenceValues[m_iCurrFrameIndex]++;
}

void QDirect3D12Widget::moveToNextFrame()
{
    const UINT64 currentFenceValue = m_iFenceValues[m_iCurrFrameIndex];
    DXCall(m_pCommandQueue->Signal(m_pFence, currentFenceValue));

    m_iCurrFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
    if (m_pFence->GetCompletedValue() < m_iFenceValues[m_iCurrFrameIndex])
    {
        DXCall(
            m_pFence->SetEventOnCompletion(m_iFenceValues[m_iCurrFrameIndex], m_hFenceEvent));
        WaitForSingleObject(m_hFenceEvent, INFINITE);
    }

    m_iFenceValues[m_iCurrFrameIndex] = currentFenceValue + 1;
}

void QDirect3D12Widget::resizeSwapChain(int width, int height)
{
    // ReleaseHandle(m_hSwapChainEvent);
    cleanupRenderTarget();

    if (m_pSwapChain)
    {
        DXCall(m_pSwapChain->ResizeBuffers(FRAME_COUNT, width, height,
                                           DXGI_FORMAT_R8G8B8A8_UNORM, 0));
    }
    else
    {
        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.BufferCount           = FRAME_COUNT;
        sd.Width                 = width;
        sd.Height                = height;
        sd.Format                = DXGI_FORMAT_R8G8B8A8_UNORM;
        // sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        sd.Flags              = 0;
        sd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count   = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Scaling            = DXGI_SCALING_NONE;
        sd.Stereo             = FALSE;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSd = {};
        fsSd.Windowed                        = TRUE;

        ComPtr<IDXGISwapChain1> swapChain;
        DXCall(m_pFactory->CreateSwapChainForHwnd(m_pCommandQueue, m_hWnd, &sd, &fsSd,
                                                  Q_NULLPTR, swapChain.GetAddressOf()));
        DXCall(swapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain)));

        // DXCall(m_pSwapChain->SetMaximumFrameLatency(FRAME_COUNT));
        // m_hSwapChainEvent = m_pSwapChain->GetFrameLatencyWaitableObject();
    }

    createRenderTarget();

    m_iCurrFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

void QDirect3D12Widget::getHardwareAdapter(IDXGIFactory2 *  pFactory,
                                           IDXGIAdapter1 ** ppAdapter)
{
    ComPtr<IDXGIAdapter1> adapter;
    *ppAdapter = Q_NULLPTR;

    for (UINT adapterIndex = 0;
         DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter);
         ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        // Skip software adapter.
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

        // Check to see if the adapter supports Direct3D 12, but don't create the actual device
        // yet.
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                        _uuidof(ID3D12Device), Q_NULLPTR)))
            break;
    }

    *ppAdapter = adapter.Detach();
}

void QDirect3D12Widget::resetEnvironment()
{
    // TODO: Add your own custom default environment, i.e:
    // m_pCamera->resetCamera();

    onReset();

    if (!m_bRenderActive) tick();
}

void QDirect3D12Widget::wheelEvent(QWheelEvent * event)
{
    if (event->angleDelta().x() == 0)
    {
        // TODO: Update your camera position based on the delta value.
    }
    else if (event->angleDelta().x() !=
             0) // horizontal scrolling - mice with another side scroller.
    {
        // MouseWheelH += (float)(event->angleDelta().y() / WHEEL_DELTA);
    }
    else if (event->angleDelta().y() != 0)
    {
        // MouseWheel += (float)(event->angleDelta().y() / WHEEL_DELTA);
    }

    QWidget::wheelEvent(event);
}

QPaintEngine * QDirect3D12Widget::paintEngine() const
{
    return Q_NULLPTR;
}

void QDirect3D12Widget::paintEvent(QPaintEvent * event) {}

void QDirect3D12Widget::resizeEvent(QResizeEvent * event)
{
    if (m_bDeviceInitialized)
    {
        onReset();
        emit widgetResized();
    }

    QWidget::resizeEvent(event);
}

bool QDirect3D12Widget::event(QEvent * event)
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
            emit mouseMoved((QMouseEvent *)event);
            break;
        case QEvent::MouseButtonPress:
            emit mouseClicked((QMouseEvent *)event);
            break;
        case QEvent::MouseButtonRelease:
            emit mouseReleased((QMouseEvent *)event);
            break;
    }

    return QWidget::event(event);
}

LRESULT QDirect3D12Widget::WndProc(MSG * pMsg)
{
    // Process wheel events using Qt's event-system.
    if (pMsg->message == WM_MOUSEWHEEL || pMsg->message == WM_MOUSEHWHEEL) return false;

    return false;
}

#if QT_VERSION >= 0x050000
bool QDirect3D12Widget::nativeEvent(const QByteArray & eventType,
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
bool QDirect3D12Widget::winEvent(MSG * message, long * result)
{
    Q_UNUSED(result);

#    ifdef Q_OS_WIN
    MSG * pMsg = reinterpret_cast<MSG *>(message);
    return WndProc(pMsg);
#    endif

    return QWidget::winEvent(message, result);
}
#endif // QT_VERSION >= 0x050000
