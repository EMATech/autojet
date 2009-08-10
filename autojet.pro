TEMPLATE = app

FORMS =	src/configuration.ui \
		src/configwriter.ui \
		src/mainwindow.ui \
		src/maximum.ui \
		src/minimum.ui \
		src/create.ui \
		src/select.ui

HEADERS =	src/cdwriter.h \
			src/configuration.h \
			src/autojet.h \
			src/create.h

SOURCES =	src/cdwriter.cpp \
			src/configuration.cpp \
			src/main.cpp \
			src/autojet.cpp \
			src/create.cpp

RESOURCES = src/autojet.qrc

TARGET = autojet

TRANSLATIONS = autojet_fr.ts

INSTALLS += target
target.path = /usr/local/bin

OBJECTS_DIR  = tmp
MOC_DIR      = tmp
DESTDIR      = bin

CONFIG += release \
          warn_on \
          qt \
          thread 
