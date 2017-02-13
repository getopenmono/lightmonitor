#ifndef app_controller_h
#define app_controller_h

#include <mono.h>
#include <mbed.h>
#include "FilteredAnalogIn.h"
#include "hysteresis_trigger.h"
#include "internet.h"
#include <power_saver.h>

using namespace mono;

class Notification
{
public:
    String Title;
    String Message;
    DateTime eventTime;
    bool isSent;

    Notification()
    {
        isSent = true;
    }

    Notification(String title, String message)
    {
        Title = title;
        Message = message;
        isSent = false;
        eventTime = DateTime::now();
    }
};

class AppController : public mono::IApplication {
public:

    static const char* PushoverUserId;
    static const char* PushoverTokenId;
    static const char* PushoverDeviceId;

    static const PinName samplePins[1];
    static const int numberOfPins = 1;

    io::FilteredAnalogIn<8> sensorPin;
    io::HysteresisTrigger sensorTrigger;
    mono::ui::TextLabelView valueLbl, statusTxt;
    ScheduledTask task, pinTask;
    Timer pinTim, statusTimer;
    Internet internet;
    network::HttpPostClient pclient;
    Notification current, pending;
    PowerSaver pwrsaver;

    AppController();

    void sampleTask();
    void samplePin();
    uint16_t value();

    void setErrorState();
    void _setErrorState();
    void clearErrorState();
    void _clearErrorState();
    
    void sendNotification(String title, String message);
    void _connectedSendNotification();
    void _connectError();
    void _notificationSent(network::INetworkRequest::CompletionEvent *);
    void _notificationError(network::INetworkRequest::ErrorEvent*);
    uint16_t _httpBodyLength();
    void _httpBodyContent(char *body);
    void _httpResponse(const network::HttpClient::HttpResponseData &);
    
    
    void setStatus(String text, mono::display::Color color = ui::View::StandardTextColor);
    void clearStatus();

    void monoWakeFromReset();
    void monoWillGotoSleep();
    void monoWakeFromSleep();

};

#endif /* app_controller_h */
