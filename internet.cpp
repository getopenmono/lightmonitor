
#include "internet.h"
#include <wireless/redpine_module.h>


Internet::Internet() :
    rpSpi(RP_SPI_MOSI, RP_SPI_MISO, RP_SPI_CLK),
    spiComm(rpSpi, NC, RP_nRESET, RP_INTERRUPT)
{
    connected = false;
    IApplicationContext::Instance->PowerManager->AppendToPowerAwareQueue(this);
}

void Internet::connect(String ssid, String pass)
{
    if (connected)
        return;

    mono::redpine::Module::setNetworkReadyCallback<Internet>(this, &Internet::onNetworkReady);
    mono::redpine::Module::setConnectFailedCallback<Internet>(this, &Internet::onNetworkError);
    
    if (mono::redpine::Module::initialize(&spiComm))
        mono::redpine::Module::setupWifiOnly(ssid, pass);
    else
        printf("Could not connect to Wifi!\r\n");
}

bool Internet::isConnected() const
{
    return connected;
}

void Internet::onSystemPowerOnReset()
{
}
void Internet::onSystemEnterSleep()
{
    connected = false;
}
void Internet::onSystemWakeFromSleep()
{
    connected = false;
}
void Internet::OnSystemBatteryLow()
{
    
}

void Internet::onNetworkReady()
{
    connected = true;
    connectHandler.call();
}

void Internet::onNetworkError()
{
    connected = false;
    errorHandler.call();
}
