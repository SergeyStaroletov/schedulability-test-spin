QT = core

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    lisp_parce.cpp

INCLUDEPATH += /opt/homebrew/Cellar/ecl/24.5.10/include
#INCLUDEPATH += /Users/sergey/Projects/ECRTS2025/ecl/build
INCLUDEPATH += /opt/homebrew/Cellar/gmp/6.3.0/include

LIBS += -L /opt/homebrew/Cellar/ecl/24.5.10/lib -lecl
#LIBS += -L/Users/sergey/Projects/ECRTS2025/ecl/build -lecl

#LIBS += -L /opt/homebrew/Cellar/gmp/6.3.0/lib -lgmp
