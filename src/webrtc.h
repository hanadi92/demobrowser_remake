#ifndef WEBRTC_H
#define WEBRTC_H

#include <QTabWidget>
#include <QWebEnginePage>

// trigger start/stop events of the WebRTC API through JavaScript while hidden

class WebRTC : public QWebEnginePage
{
    Q_OBJECT
public:
    WebRTC(QTabWidget *parent = 0);
    ~WebRTC();
};

#endif // WEBRTC_H
