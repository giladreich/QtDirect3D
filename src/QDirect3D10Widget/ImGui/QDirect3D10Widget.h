/*
 *
 */

#pragma once


#include <QWidget>
#include <QTimer>

#include <d3d10.h>
#include <DirectXMath.h>
#include <D3Dcompiler.h>

class QDirect3D10Widget : public QWidget
{
    Q_OBJECT

public:
    QDirect3D10Widget(QWidget * parent);
    ~QDirect3D10Widget();

    void release();
    void resetEnvironment();

private:
    bool init();

    void beginScene();
    void endScene();

    void tick();
    void render();
    void uiRender();

// Qt Events
private:
    bool event(QEvent * event) override;
    void showEvent(QShowEvent * event) override;
    QPaintEngine * paintEngine() const override;
    void paintEvent(QPaintEvent * event) override;
    void resizeEvent(QResizeEvent * event) override;

    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#if QT_VERSION >= 0x050000
    bool nativeEvent(const QByteArray & eventType, void * message, long * result) override;
#else
    bool winEvent(MSG * message, long * result) override;
#endif


Q_SIGNALS:
    void deviceInitialized(bool success);

    void eventHandled();
    void widgetResized();

    void ticked();
    void rendered();
    void uiRendered();


private Q_SLOTS:
    void onFrame();
    void onReset();

// Getters / Setters
public:
    HWND const & nativeHandle() const { return m_hWnd; }

    ID3D10Device * device() const { return m_pDevice; }
    IDXGISwapChain * swapChain() { return m_pSwapChain; }
    ID3D10RenderTargetView* TargetView() const { return m_pRTView; }

    bool renderActive() const { return m_bRenderActive; }
    void setRenderActive(bool active) { m_bRenderActive = active; }

    DirectX::XMVECTORF32 * BackColor() { return &m_BackColor; }


private:
    ID3D10Device*            m_pDevice;
    IDXGISwapChain*          m_pSwapChain;
    ID3D10RenderTargetView*  m_pRTView;


    QTimer                    m_qTimer;

    HWND                      m_hWnd;
    bool                      m_bDeviceInitialized;

    bool                      m_bRenderActive;

    DirectX::XMVECTORF32      m_BackColor;

};

// Utils
#define ReleaseObject(object) if((object) != nullptr) { object->Release(); object = nullptr; }
