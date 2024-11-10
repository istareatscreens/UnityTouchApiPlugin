/*
  Need to wrap the UnityTouchPanelApiPlugin class functions with extern functions
  to be able to interact with it in unity
  reference: https://stackoverflow.com/a/36574077
*/
#ifndef __PLUGIN_WRAPPER_H
#define __PLUGIN_WRAPPER_H

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#include <vector>
#include <memory>
#include "UnityTouchPanelApiPlugin.h"

// Declare callback
typedef int(__stdcall *DataReceivedCallback)(const int *);
typedef int(__stdcall *EventCallback)(std::string);

typedef std::unique_ptr<UnityTouchPanelApiPlugin> PluginPtr;

extern "C"
{

  // instantiation
  extern DLL_EXPORT UnityTouchPanelApiPlugin *Initialize(
      int device_classification,
      void *window_handle,
      int polling_rate_ms);

  // Need to passed intantiated object returned from Initialize to execute these function
  extern DLL_EXPORT void Dispose(UnityTouchPanelApiPlugin *obj);
  extern DLL_EXPORT bool Connect(UnityTouchPanelApiPlugin *obj);
  extern DLL_EXPORT void Read(UnityTouchPanelApiPlugin *obj,
                              DataReceivedCallback data_received,
                              EventCallback event_callback);
  extern DLL_EXPORT bool Disconnect(UnityTouchPanelApiPlugin *obj);
  extern DLL_EXPORT bool IsConnected(UnityTouchPanelApiPlugin *obj);
  extern DLL_EXPORT bool IsReading(UnityTouchPanelApiPlugin *obj);
  extern DLL_EXPORT void StopReading(UnityTouchPanelApiPlugin *obj);
  extern DLL_EXPORT int PluginLoaded();
  extern DLL_EXPORT void ReloadPlugin();
}

#endif // __PLUGIN_WRAPPER_H
