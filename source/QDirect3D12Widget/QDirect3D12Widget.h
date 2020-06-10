/*
 *
 */
#pragma once

#include <stdexcept>

#include <QWidget>
#include <QTimer>

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"

class QDirect3D12Widget : public QWidget
{
    Q_OBJECT

public:
    QDirect3D12Widget(QWidget * parent);
    ~QDirect3D12Widget();

    void release();
    void resetEnvironment();

    void run();
    void pauseFrames();
    void continueFrames();

private:
    bool init();
    void create3DDevice();
    void getHardwareAdapter(IDXGIFactory2 * pFactory, IDXGIAdapter1 ** ppAdapter);
    void resizeSwapChain(int width, int height);
    void cleanupRenderTarget();
    void createRenderTarget();

    void beginScene();
    void endScene();

    void tick();
    void render();

    void waitForGpu();
    void moveToNextFrame();

    // Qt Events
private:
    bool           event(QEvent * event) override;
    void           showEvent(QShowEvent * event) override;
    QPaintEngine * paintEngine() const override;
    void           paintEvent(QPaintEvent * event) override;
    void           resizeEvent(QResizeEvent * event) override;
    void           wheelEvent(QWheelEvent * event) override;

    LRESULT WINAPI WndProc(MSG * pMsg);

#if QT_VERSION >= 0x050000
    bool nativeEvent(const QByteArray & eventType, void * message, long * result) override;
#else
    bool winEvent(MSG * message, long * result) override;
#endif

signals:
    void deviceInitialized(bool success);

    void eventHandled();
    void widgetResized();

    void ticked();
    void rendered(ID3D12GraphicsCommandList * cl);

    void keyPressed(QKeyEvent *);
    void mouseMoved(QMouseEvent *);
    void mouseClicked(QMouseEvent *);
    void mouseReleased(QMouseEvent *);

private slots:
    void onFrame();
    void onReset();

    // Getters / Setters
public:
    HWND const & nativeHandle() const { return m_hWnd; }

    ID3D12Device *              device() const { return m_pDevice; }
    IDXGISwapChain *            swapChain() { return m_pSwapChain; }
    ID3D12GraphicsCommandList * commandList() const { return m_pCommandList; }

    bool renderActive() const { return m_bRenderActive; }
    void setRenderActive(bool active) { m_bRenderActive = active; }

    D3DCOLORVALUE * BackColor() { return &m_BackColor; }

private:
    // Pipeline objects.
    static int const FRAME_COUNT = 3;
    UINT             m_iCurrFrameIndex;

    ID3D12Device *              m_pDevice;
    IDXGIFactory4 *             m_pFactory;
    IDXGISwapChain3 *           m_pSwapChain;
    ID3D12CommandQueue *        m_pCommandQueue;
    ID3D12CommandAllocator *    m_pCommandAllocators[FRAME_COUNT];
    ID3D12GraphicsCommandList * m_pCommandList;

    ID3D12DescriptorHeap *      m_pRTVDescHeap;
    UINT                        m_iRTVDescSize; // May vary from device to device.
    ID3D12Resource *            m_pRTVResources[FRAME_COUNT];
    D3D12_CPU_DESCRIPTOR_HANDLE m_RTVDescriptors[FRAME_COUNT];
    ID3D12DescriptorHeap *      m_pSrvDescHeap;

    // Synchronization objects.
    HANDLE        m_hSwapChainEvent;
    HANDLE        m_hFenceEvent;
    ID3D12Fence * m_pFence;
    UINT64        m_iFenceValues[FRAME_COUNT];

    // Widget objects.
    QTimer m_qTimer;

    HWND m_hWnd;
    bool m_bDeviceInitialized;

    bool m_bRenderActive;
    bool m_bStarted;

    D3DCOLORVALUE m_BackColor;
};

// ############################################################################
// ############################## Utils #######################################
// ############################################################################
#define ReleaseObject(object)                                                                 \
    if ((object) != Q_NULLPTR)                                                                \
    {                                                                                         \
        object->Release();                                                                    \
        object = Q_NULLPTR;                                                                   \
    }
#define ReleaseHandle(object)                                                                 \
    if ((object) != Q_NULLPTR)                                                                \
    {                                                                                         \
        CloseHandle(object);                                                                  \
        object = Q_NULLPTR;                                                                   \
    }

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr)
        : std::runtime_error(HrToString(hr))
        , m_hr(hr)
    {
    }
    HRESULT Error() const { return m_hr; }

private:
    const HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr)) { throw HrException(hr); }
}

#define DXCall(func) ThrowIfFailed(func)
