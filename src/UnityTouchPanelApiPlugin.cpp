#include "UnityTouchPanelApiPlugin.h"
#include <windows.h>
#include <iostream>

bool UnityTouchPanelApiPlugin::connect()
{
    clearBuffers();
    return isConnected();
}

void UnityTouchPanelApiPlugin::read(const std::function<void(uint16_t *)> dataCallback,
                                    const std::function<void(std::string)> errorCallback)
{
    stopReading();
    reading.store(true);
    clearBuffers();

    readThread = std::thread(
        [this, dataCallback, errorCallback]()
        { readLoop(dataCallback, errorCallback); });
}

void UnityTouchPanelApiPlugin::readLoop(const std::function<void(uint16_t *)> dataCallback,
                                        const std::function<void(std::string)> errorCallback)
{
    try
    {
        MSG msg;
        uint16_t *raw_buffer = buffer.get();
        uint16_t *external_buffer = externalBuffer.get();
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

            for (size_t i = 0; i < touchCount; i++)
            {
                raw_buffer[i * DATA_POINTS + 0] = static_cast<uint16_t>(touchInputs[i].x);
                raw_buffer[i * DATA_POINTS + 1] = static_cast<uint16_t>(touchInputs[i].y);
                raw_buffer[i * DATA_POINTS + 2] = static_cast<uint16_t>(touchInputs[i].dwFlags);
            }

            std::memcpy(external_buffer, raw_buffer, MAX_TOUCH_POINTS * DATA_POINTS * sizeof(uint16_t));

            dataCallback(external_buffer);
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

void UnityTouchPanelApiPlugin::clearBuffers()
{
    buffer.reset(new uint16_t[MAX_TOUCH_POINTS * DATA_POINTS]);
    externalBuffer.reset(new uint16_t[MAX_TOUCH_POINTS * DATA_POINTS]);
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