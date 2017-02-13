#include "app_controller.h"

using mono::geo::Rect;

const char* AppController::PushoverUserId = "";
const char* AppController::PushoverTokenId = "";
const char* AppController::PushoverDeviceId = "";
const char* PushoverUrl = "http://api.pushover.net/1/messages.json";

const int sampleIntervalSecs = 1;

#ifndef WIFI_SSID
#define WIFI_SSID ""
#define WIFI_PASS ""
#endif

const PinName AppController::samplePins[1] = { J_RING1 };

AppController::AppController() :
    sensorPin(J_RING1),
    sensorTrigger(10000, 3000),
    valueLbl(Rect(10, 10, 156, 90), ""),
    statusTxt(Rect(10,220-30,156,30), ""),
    statusTimer(4000, true)
{
    valueLbl.setAlignment(ui::TextLabelView::ALIGN_CENTER);
    valueLbl.show();

    statusTxt.setAlignment(ui::TextLabelView::ALIGN_CENTER);
    statusTxt.show();

    sensorTrigger.setUpperTriggerCallback<AppController>(this, &AppController::clearErrorState);
    sensorTrigger.setLowerTriggerCallback<AppController>(this, &AppController::setErrorState);

    pinTask.setTask<AppController>(this, &AppController::samplePin);
    
    //pinTim.setCallback<AppController>(this, &AppController::samplePin);
    //pinTim.Start();
    
    statusTimer.setCallback<AppController>(this, &AppController::clearStatus);
}

void AppController::sampleTask()
{
    samplePin();
}

void AppController::samplePin()
{
    uint16_t value = sensorPin.read_u16();

    //printf("%d;%d\r\n", sensorPin.mbed::AnalogIn::read_u16(), value);
    valueLbl.setText(String::Format("%05d", value));
    sensorTrigger.check(value);
    
    pinTask.reschedule(DateTime::now().addSeconds(sampleIntervalSecs));
}

void AppController::setErrorState()
{
    IApplicationContext::Instance->PowerManager->__shouldWakeUp = true;
    async<AppController>(this, &AppController::_setErrorState);
}

void AppController::_setErrorState()
{
    valueLbl.setText(display::RedColor);
    valueLbl.scheduleRepaint();
    setStatus("Below threshold!", display::RedColor);
    printf("triggering notification for value: %d\r\n", sensorPin.value());
    sendNotification("Sensor below threshold!", "Light level is below threshold!");
}

void AppController::clearErrorState()
{
    IApplicationContext::Instance->PowerManager->__shouldWakeUp = true;
    async<AppController>(this, &AppController::_clearErrorState);
}

void AppController::_clearErrorState()
{
    valueLbl.setText(display::GreenColor);
    valueLbl.scheduleRepaint();
    setStatus("Level OK");
    sendNotification("Sensor OK!", "Light level is restored!");
}

void AppController::sendNotification(String title, String message)
{
//    printf("send notice deactivated\r\n");
//    return;
    
    if (current.isSent == false)
    {
        printf("notification will be pending!\r\n");
        pending = Notification(title, message);
        return;
    }

    printf("Creating notification!\r\n");
    current = Notification(title, message);
    pwrsaver.deactivate();
    
    if (internet.isConnected())
    {
        printf("already connected - sent right away!\r\n");
        _connectedSendNotification();
    }
    else
    {
        printf("not connected - init %s wifi!\r\n", WIFI_SSID);
        internet.setConnectCallback<AppController>(this, &AppController::_connectedSendNotification);
        internet.setErrorCallback<AppController>(this, &AppController::_connectError);
        setStatus("initing wifi...");
        internet.connect(WIFI_SSID, WIFI_PASS);
    }
}

void AppController::_connectedSendNotification()
{
    pwrsaver.deactivate();
    printf("sending notification...\r\n");
    setStatus("sending notice...");
    pclient = network::HttpPostClient(PushoverUrl, "application/x-www-form-urlencoded\r\n");
    pclient.setBodyDataCallback<AppController>(this, &AppController::_httpBodyContent);
    pclient.setBodyLengthCallback<AppController>(this, &AppController::_httpBodyLength);
    pclient.setErrorCallback<AppController>(this, &AppController::_notificationError);
    pclient.setDataReadyCallback<AppController>(this, &AppController::_httpResponse);
    pclient.setCompletionCallback<AppController>(this, &AppController::_notificationSent);
    pclient.post();
}

void AppController::_connectError()
{
    printf("Wifi connect error!\r\n");
    setStatus("Wifi connect error!", display::RedColor);
}

void AppController::_notificationSent(network::INetworkRequest::CompletionEvent *)
{
    printf("Notification sent!\r\n");
    setStatus("sent!");
    current = Notification();

    if (pending.isSent == false) {
        printf("There is a pending notification! sent it\r\n");
        current = pending;
        pending = Notification();
        _connectedSendNotification();
    }
    {
        pwrsaver.undim();
    }
}

void AppController::_notificationError(network::INetworkRequest::ErrorEvent*)
{
    printf("notification http error!\r\n");
    setStatus("http send error!", display::RedColor);

    if (pending.isSent == false) {
        current = pending;
        pending = Notification();
        _connectedSendNotification();
    }
}

uint16_t AppController::_httpBodyLength()
{
    return snprintf(0, 0, "token=%s&user=%s&device=%s&title=%s&message=%s",
                    PushoverTokenId, PushoverUserId, PushoverUserId,
                    current.Title(), current.Message());
}

void AppController::_httpBodyContent(char *body)
{
    snprintf(body, _httpBodyLength(), "token=%s&user=%s&device=%s&title=%s&message=%s",
             PushoverTokenId, PushoverUserId, PushoverUserId,
             current.Title(), current.Message());
}


void AppController::setStatus(String text, mono::display::Color color)
{
    statusTimer.Stop();
    statusTxt.setText(color);
    statusTxt.setText(text);
    statusTimer.Start();
    pwrsaver.deactivate();
}
void AppController::clearStatus()
{
    statusTxt.setText("");
    pwrsaver.undim();
}

void AppController::_httpResponse(const network::HttpClient::HttpResponseData &resp)
{
    printf("%s", resp.bodyChunk());
}

void AppController::monoWakeFromReset()
{
    // set the filter initial value to a sample (that is not filtered)
    sensorPin.clear(sensorPin.mbed::AnalogIn::read_u16());
    sampleTask();
    
    pwrsaver.undim();
}

void AppController::monoWillGotoSleep()
{
}

void AppController::monoWakeFromSleep()
{
    valueLbl.scheduleRepaint();
    statusTxt.scheduleRepaint();
    pwrsaver.undim();
}
