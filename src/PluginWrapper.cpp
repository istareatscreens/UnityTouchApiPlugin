#include "PluginWrapper.h"
#include "UnityTouchPanelApiPlugin.h"
#include <mutex>

static std::vector<UnityTouchPanelApiPlugin *> &getPluginInstances()
{
    static std::vector<UnityTouchPanelApiPlugin *> pluginInstances;
    return pluginInstances;
}

// Using a map was causing runtime crashes so switched to array
static std::vector<int> &getDeviceClassifications()
{
    static std::vector<int> deviceClassifications;
    return deviceClassifications;
}

extern "C"
{
    DLL_EXPORT UnityTouchPanelApiPlugin *Initialize(
        int device_classification,
        void *window_handle,

        int polling_rate_ms)
    {
        auto &pluginInstances = getPluginInstances();
        auto &deviceClassifications = getDeviceClassifications();

        for (size_t i = 0; i < deviceClassifications.size(); i++)
        {
            if (deviceClassifications[i] != device_classification)
            {
                continue;
            }
            UnityTouchPanelApiPlugin *plugin = pluginInstances[i];
            if (!plugin)
            {
                break;
            }
            plugin->stopReading();
            return plugin;
        }

        // Create new instance and store it in the smart pointer array
        auto newPlugin = new UnityTouchPanelApiPlugin(
            polling_rate_ms,
            window_handle

        );

        getPluginInstances().push_back(newPlugin);
        getDeviceClassifications().push_back(device_classification);

        return newPlugin;
    }

    DLL_EXPORT void Dispose(UnityTouchPanelApiPlugin *obj)
    {
        if (!obj)
        {
            return;
        }

        auto &pluginInstances = getPluginInstances();
        auto &deviceClassifications = getDeviceClassifications();

        // Iterate over both pluginInstances and deviceClassifications simultaneously
        for (size_t i = 0; i < pluginInstances.size(); ++i)
        {
            if (pluginInstances[i] != obj)
            {
                continue;
            }
            pluginInstances.erase(pluginInstances.begin() + i);
            deviceClassifications.erase(deviceClassifications.begin() + i);
            return;
        }
    }

    DLL_EXPORT bool Connect(UnityTouchPanelApiPlugin *obj)
    {
        if (!obj)
        {
            return false;
        }
        return obj->connect();
    }

    DLL_EXPORT void Read(UnityTouchPanelApiPlugin *obj, DataReceivedCallback data_received, EventCallback event_callback)
    {
        if (!obj)
        {
            return;
        }

        obj->read(
            [data_received](const int *data)
            { data_received(data); },
            [event_callback](std::string error)
            { event_callback(error); });
    }

    DLL_EXPORT bool Disconnect(UnityTouchPanelApiPlugin *obj)
    {
        if (!obj)
        {
            return false;
        }

        return obj->disconnect();
    }

    DLL_EXPORT bool IsConnected(UnityTouchPanelApiPlugin *obj)
    {
        if (!obj)
        {
            return false;
        }

        return obj->isConnected();
    }

    DLL_EXPORT bool IsReading(UnityTouchPanelApiPlugin *obj)
    {
        if (!obj)
        {
            return false;
        }

        return obj->isReading();
    }

    DLL_EXPORT void StopReading(UnityTouchPanelApiPlugin *obj)
    {
        if (!obj)
        {
            return;
        }
        obj->stopReading();
    }

    // Use to test if Unity is correctly loading the plugin
    DLL_EXPORT int PluginLoaded()
    {
        return 1;
    }

    DLL_EXPORT void ReloadPlugin()
    {
        getPluginInstances().clear();
    }
}