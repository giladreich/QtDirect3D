/*
 *
 */
#pragma once

#include <stdexcept>

#include <QWidget>
#include <QTimer>

#include <d3d9.h>
#include <D3Dcompiler.h>

class QDirect3D9Widget : public QWidget
{
    Q_OBJECT

public:
    QDirect3D9Widget(QWidget * parent);
    ~QDirect3D9Widget();

    void release();
    void resetEnvironment();

    void run();
    void pauseFrames();
    void continueFrames();

private:
    bool init();

    void beginScene();
    void endScene();

    void tick();
    void render();
    void renderUI();

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
    void rendered();
    void renderedUI();

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

    IDirect3DDevice9 *      device() const { return m_pDevice; }
    D3DPRESENT_PARAMETERS * devicePresentParams() { return &m_PresentParams; }
    IDirect3D9 *            direct3D() const { return m_pD3D; }

    bool renderActive() const { return m_bRenderActive; }
    void setRenderActive(bool active) { m_bRenderActive = active; }

    D3DCOLORVALUE * BackColor() { return &m_BackColor; }

private:
    IDirect3DDevice9 *    m_pDevice;
    IDirect3D9 *          m_pD3D;
    D3DPRESENT_PARAMETERS m_PresentParams;

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
