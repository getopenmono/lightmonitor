
#ifndef internet_h
#define internet_h

#include <power_aware_interface.h>
#include <mbed.h>
#include <wireless/module_communication.h>
#include <mono.h>

using namespace mono;

class Internet : public mono::power::IPowerAware {
protected:
    
    bool connected;
    mbed::SPI rpSpi;
    mono::redpine::ModuleSPICommunication spiComm;
    mbed::FunctionPointer connectHandler;
    mbed::FunctionPointer errorHandler;
    
public:
    
    Internet();
    
    void connect(String ssid, String pass);
    
    template <typename Class>
    void setConnectCallback(Class *cnxt, void(Class::*method)())
    {
        connectHandler.attach<Class>(cnxt, method);
    }
    
    template <typename Class>
    void setErrorCallback(Class *cnxt, void(Class::*method)())
    {
        errorHandler.attach<Class>(cnxt, method);
    }
    
    bool isConnected() const;
    
    void onSystemPowerOnReset();
    void onSystemEnterSleep();
    void onSystemWakeFromSleep();
    void OnSystemBatteryLow();
    
    void onNetworkReady();
    void onNetworkError();
};

#endif /* internet_h */
