QT += network widgets

HEADERS       = client.h
SOURCES       = client.cpp \
                main.cpp

# install
target.path = $$_PRO_FILE_PWD_/bin
INSTALLS += target
