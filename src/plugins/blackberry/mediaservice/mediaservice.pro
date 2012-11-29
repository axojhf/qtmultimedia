TARGET = qtmedia_blackberry
QT += multimedia-private gui-private
CONFIG += no_private_qt_headers_warning

PLUGIN_TYPE=mediaservice
load(qt_plugin)

LIBS += -lmmrndclient -lstrm -lscreen

HEADERS += \
    bbserviceplugin.h \
    bbmediaplayerservice.h \
    bbmediaplayercontrol.h \
    bbmetadata.h \
    bbutil.h \
    bbvideowindowcontrol.h

SOURCES += \
    bbserviceplugin.cpp \
    bbmediaplayerservice.cpp \
    bbmediaplayercontrol.cpp \
    bbmetadata.cpp \
    bbutil.cpp \
    bbvideowindowcontrol.cpp

OTHER_FILES += blackberry_mediaservice.json
