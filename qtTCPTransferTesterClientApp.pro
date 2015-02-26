QT += network widgets

HEADERS       = client.h \
    myLabel.h
SOURCES       = client.cpp \
                main.cpp \
    myLabel.cpp

# install
target.path = $$_PRO_FILE_PWD_/bin
INSTALLS += target

FORMS += \
    frmTcpClient.ui
