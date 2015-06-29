TEMPLATE = app
TARGET = visingo
QT += webenginewidgets webchannel network widgets printsupport

qtHaveModule(uitools):!embedded: QT += uitools
else: DEFINES += QT_NO_UITOOLS

macx: CONFIG -= app_bundle


FORMS += \
    addbookmarkdialog.ui \
    bookmarks.ui \
    cookies.ui \
    cookiesexceptions.ui \
    downloaditem.ui \
    downloads.ui \
    history.ui \
    passworddialog.ui \
    proxy.ui \
    settings.ui

HEADERS += \
    autosaver.h \
    bookmarks.h \
    browsermainwindow.h \
    chasewidget.h \
    downloadmanager.h \
    edittableview.h \
    edittreeview.h \
    featurepermissionbar.h\
    history.h \
    modelmenu.h \
    searchlineedit.h \
    settings.h \
    squeezelabel.h \
    tabwidget.h \
    toolbarsearch.h \
    urllineedit.h \
    visingoapp.h \
    webview.h \
    xbel.h

SOURCES += \
    autosaver.cpp \
    bookmarks.cpp \
    browsermainwindow.cpp \
    chasewidget.cpp \
    downloadmanager.cpp \
    edittableview.cpp \
    edittreeview.cpp \
    featurepermissionbar.cpp\
    history.cpp \
    modelmenu.cpp \
    searchlineedit.cpp \
    settings.cpp \
    squeezelabel.cpp \
    tabwidget.cpp \
    toolbarsearch.cpp \
    urllineedit.cpp \
    visingoapp.cpp \
    webview.cpp \
    xbel.cpp \
    main.cpp

RESOURCES += data/data.qrc htmls/htmls.qrc
RESOURCES += jquery.qrc


contains(DEFINES, QWEBENGINEPAGE_SETNETWORKACCESSMANAGER) {
    HEADERS += cookiejar.h networkaccessmanager.h
    SOURCES += cookiejar.cpp networkaccessmanager.cpp
}

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

# TODO: change icons etc.
win32 {
   RC_FILE = browser.rc
}

# TODO: change icons etc.
mac {
    ICON = browser.icns
    QMAKE_INFO_PLIST = Info_mac.plist
    TARGET = Visingo
}

# TODO: change icons etc.
EXAMPLE_FILES = Info_mac.plist browser.icns browser.ico browser.rc






# install
target.path = /usr/local/bin/visingo # TODO change, configurable
INSTALLS += target







### clingo ###

# clingo requires C++11 (0x)
CONFIG += c++11

DEFINES += NDEBUG=1 WITH_PYTHON=1 WITH_THREADS=1

INCLUDEPATH += $$PWD/clingo/app/shared/include
INCLUDEPATH += $$PWD/clingo/libgringo
INCLUDEPATH += $$PWD/clingo/libclasp
INCLUDEPATH += $$PWD/clingo/libprogram_opts
INCLUDEPATH += $$PWD/clingo/libprogram_opts/src
INCLUDEPATH += $$PWD/clingo/app/clingo/src

INCLUDEPATH += $$PWD/clingo/build/release
DEPENDPATH += $$PWD/clingo/build/release

unix: LIBS += $$PWD/clingo/build/release/app/clingo/src/clingo_app.o
unix: LIBS += $$PWD/clingo/build/release/app/clingo/src/clasp/clasp_app.o

unix: PRE_TARGETDEPS += $$PWD/clingo/build/release/libshared.a
unix: PRE_TARGETDEPS += $$PWD/clingo/build/release/libgringo.a
unix: PRE_TARGETDEPS += $$PWD/clingo/build/release/libclasp.a
unix: PRE_TARGETDEPS += $$PWD/clingo/build/release/libprogram_opts.a

unix: LIBS += -L$$PWD/clingo/build/release/ -lshared -lgringo -lclasp -lprogram_opts


# TODO: make clingo library and require only that header, not the turtles all the way down

# MODIFY HERE: clingo requirements
win32 {
   # TODO
}

macx {
    # The following should be correct for homebrew installed
    # tbb, lua 5.2 and python 2.7

    # TBB
    LIBS += /usr/local/lib/libtbb.dylib
    INCLUDEPATH += /usr/local/Cellar/tbb/4.3-20150611/include
    DEPENDPATH += /usr/local/Cellar/tbb/4.3-20150611/include
    # Lua
    LIBS += /usr/local/lib/liblua.5.2.dylib
    INCLUDEPATH += /usr/local/include/lua5.2
    DEPENDPATH += /usr/local/include/lua5.2
    # Python
    INCLUDEPATH += /usr/include/python2.7
    LIBS += -lpython2.7
}

unix:!macx: {
    INCLUDEPATH += /usr/include/python2.7 /usr/include/lua5.1
    INCLUDEPATH += /home/kuuranne/opt/tbb43_20150424oss/include
    LIBS += -L/home/kuuranne/opt/tbb43_20150424oss/lib/intel64/gcc4.4
    LIBS += -lpython2.7 -llua5.1 -ltbb
}

# EOF
