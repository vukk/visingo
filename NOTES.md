
some TODO notes:
- limit webserver to webroot
- enable webchannel only on localhost? for future proofing
  - or make newTab command accept a bool whether or not to enable,
    then enable on main and visualization





error messages
qt.network.ssl: QSslSocket: cannot resolve SSLv2_client_method
qt.network.ssl: QSslSocket: cannot resolve SSLv2_server_method
see https://github.com/opencor/opencor/issues/516
and https://bugreports.qt.io/browse/QTBUG-42115
and 

error message "setNativeLocks failed: Resource temporarily unavailable"
see https://bugreports.qt.io/browse/QTBUG-43454


adding js developer tools?
https://qt.gitorious.org/qt/qtwebengine/source/2282c6605bc554ca610239f3dbc4d6d067c25763:tests/auto/widgets/qwebengineinspector/tst_qwebengineinspector.cpp#L47
https://getfirebug.com/firebuglite


- MAC OS X
change qmake config at $QTDIR/5.x/clang_64/mkspecs/macx-clang/qmake.conf
to have
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
instead of 10.7
