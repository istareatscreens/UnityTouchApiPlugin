#include "UnityTouchPanelApiPlugin.h"
#include <windows.h>
#include <iostream>

bool UnityTouchPanelApiPlugin::connect()
{
    return isConnected();
}

void UnityTouchPanelApiPlugin::read(const std::function<void(const int *)> dataCallback,
                                    const std::function<void(std::string)> errorCallback)
{
    stopReading();
    reading.store(true);

    readThread = std::thread(
        [this, dataCallback, errorCallback]()
        { readLoop(dataCallback, errorCallback); });
}

void UnityTouchPanelApiPlugin::readLoop(const std::function<void(const int *)> dataCallback,
                                        const std::function<void(std::string)> errorCallback)
{
    try
    {
        MSG msg;
        int touchDataBuffer[MAX_TOUCH_POINTS * DATA_POINTS];
        int pollingRateMs = connectionProperties.polling_rate_ms;

        while (reading.load(std::memory_order_relaxed))
        {
            if (!PeekMessage(&msg, unityWindow, WM_TOUCH, WM_TOUCH, PM_REMOVE))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(pollingRateMs));
                continue;
            }
            TOUCHINPUT touchInputs[MAX_TOUCH_POINTS];
            int touchCount = MAX_TOUCH_POINTS;

            HTOUCHINPUT hTouchInput = reinterpret_cast<HTOUCHINPUT>(msg.lParam);

            if (!GetTouchInputInfo(hTouchInput, MAX_TOUCH_POINTS, touchInputs, sizeof(TOUCHINPUT)))
            {
                errorCallback("Failed to read touch data.");
                std::this_thread::sleep_for(std::chrono::milliseconds(pollingRateMs));
                continue;
            }

            for (int i = 0; i < touchCount; i++)
            {
                buffer[i * DATA_POINTS + 0] = touchInputs[i].x;
                buffer[i * DATA_POINTS + 1] = touchInputs[i].y;
                buffer[i * DATA_POINTS + 2] = touchInputs[i].dwFlags;
            }
            dataCallback(touchDataBuffer);
            CloseTouchInputHandle(hTouchInput);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(pollingRateMs));
    }
    catch (...)
    {
        errorCallback("An error occurred while reading touch data.");
        disconnect();
    }
}

bool UnityTouchPanelApiPlugin::disconnect()
{
    stopReading();
    return true;
}

bool UnityTouchPanelApiPlugin::isConnected() const
{
    return IsWindow(unityWindow);
}

bool UnityTouchPanelApiPlugin::isReading()
{
    std::lock_guard<std::mutex> lock(connectionMutex);
    return reading.load();
}

void UnityTouchPanelApiPlugin::stopReading()
{
    std::lock_guard<std::mutex> lock(connectionMutex);
    if (reading.load())
    {
        reading.store(false);
        if (readThread.joinable())
        {
            readThread.join();
        }
    }
}

UnityTouchPanelApiPlugin::~UnityTouchPanelApiPlugin()
{
    disconnect();
}