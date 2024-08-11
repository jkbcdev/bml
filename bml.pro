QT += core widgets
TARGET = build/bml
TEMPLATE = app

SOURCES +=  src/*.cpp

HEADERS +=  include/*.h \
            include/*.hpp

INCLUDEPATH += include

include(external/qmarkdowntextedit/qmarkdowntextedit.pri)

