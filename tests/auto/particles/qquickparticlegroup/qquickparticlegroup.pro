CONFIG += testcase
TARGET = tst_qquickparticlegroup
SOURCES += tst_qquickparticlegroup.cpp
macx:CONFIG -= app_bundle

include (../../shared/util.pri)
TESTDATA = data/*

QT += core-private gui-private v8-private qml-private quick-private testlib

