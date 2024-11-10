#ifndef UNITY_TOUCH_PANEL_API_PLUGIN_H
#define UNITY_TOUCH_PANEL_API_PLUGIN_H

#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <windows.h>
#include "ConnectionProperties.h"

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

class DLL_EXPORT UnityTouchPanelApiPlugin
{
private:
    static constexpr size_t MAX_TOUCH_POINTS = 10;
    static constexpr size_t DATA_POINTS = 3;
    std::atomic<bool> reading; // Controls the read loop
    std::thread readThread;
    std::unique_ptr<uint16_t[]> buffer;
    std::unique_ptr<uint16_t[]> externalBuffer;
    std::mutex connectionMutex;
    const ConnectionProperties connectionProperties;
    const HWND unityWindow; // Window handle for touch input
    void clearBuffers();
    void readLoop(const std::function<void(uint16_t *)> dataCallback,
                  const std::function<void(std::string)> errorCallback);

public:
    UnityTouchPanelApiPlugin();
    UnityTouchPanelApiPlugin(
        const int polling_rate_ms,
        void *window_handle)
        : reading(false), unityWindow(reinterpret_cast<HWND>(window_handle)),
          connectionProperties{
              polling_rate_ms

          }
    {
        buffer = std::make_unique<uint16_t[]>(MAX_TOUCH_POINTS * DATA_POINTS);
        externalBuffer = std::make_unique<uint16_t[]>(MAX_TOUCH_POINTS * DATA_POINTS);
    }
    ~UnityTouchPanelApiPlugin();

    bool connect();
    void read(const std::function<void(uint16_t *)> dataCallback,
              const std::function<void(std::string)> errorCallback);
    bool disconnect();
    bool isConnected() const;
    bool isReading();
    void stopReading();
};

#endif // UNITY_TOUCH_PANEL_API_PLUGIN_H