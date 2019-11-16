######################################################################
# QtReversi
######################################################################

TEMPLATE = app
TARGET = QtReversi
INCLUDEPATH += .
TEMPLATE += app
QT += gui quick core widgets

# Input
HEADERS += game.h
SOURCES += main.cpp
RESOURCES += img.qrc
