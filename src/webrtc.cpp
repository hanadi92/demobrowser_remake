#include "webrtc.h"

WebRTC::WebRTC(QTabWidget *parent)
    : QWebEnginePage(parent)
{
    // load my hosted webRTCAPI script
    load(QUrl(""));
}

WebRTC::~WebRTC()
{
}
