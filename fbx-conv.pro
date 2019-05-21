QT -= core

CONFIG += c++11 console warn_off

INCLUDEPATH += $$(FBX_SDK_ROOT)/include

DEFINES += FBXSDK_NEW_API

win32-msvc* {
    QMAKE_CXXFLAGS_WARN_OFF -= -W0
    QMAKE_CXXFLAGS += -W3 /wd4244 /wd4100 /wd4018 /wd4189
    DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS
    FBX_SDK_LIB_PATH = $$(FBX_SDK_ROOT)/lib/vs2013
}

contains(QMAKE_HOST.arch, x86_64) {
    FBX_SDK_LIB_PATH = $$FBX_SDK_LIB_PATH/x64
} else {
    FBX_SDK_LIB_PATH = $$FBX_SDK_LIB_PATH/x86
}

CONFIG(debug, debug|release) {
    CONFIG_DIR = debug
} else {
    CONFIG_DIR = release
}

LIBS += -L$$FBX_SDK_LIB_PATH/$$CONFIG_DIR -llibfbxsdk-md

SOURCES +=  \
    src/main.cpp \
    src/modeldata/C3BFile.cpp \
    src/modeldata/FileIO.cpp \
    src/modeldata/Reference.cpp \
    src/modeldata/ReferenceTable.cpp \
    src/modeldata/Serialization.cpp \
    src/modeldata/WriteBinnary.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    src/FbxConv.h \
    src/FbxConvCommand.h \
    src/Settings.h \
    src/json/BaseJSONWriter.h \
    src/json/JSONWriter.h \
    src/json/UBJSON.h \
    src/json/UBJSONWriter.h \
    src/log/codes.h \
    src/log/log.h \
    src/log/messages.h \
    src/modeldata/Animation.h \
    src/modeldata/Attributes.h \
    src/modeldata/C3BFile.h \
    src/modeldata/FileIO.h \
    src/modeldata/Keyframe.h \
    src/modeldata/Material.h \
    src/modeldata/Mesh.h \
    src/modeldata/MeshPart.h \
    src/modeldata/Model.h \
    src/modeldata/Node.h \
    src/modeldata/NodeAnimation.h \
    src/modeldata/NodePart.h \
    src/modeldata/Reference.h \
    src/modeldata/ReferenceTable.h \
    src/readers/FbxConverter.h \
    src/readers/FbxMeshInfo.h \
    src/readers/Reader.h \
    src/readers/matrix3.h \
    src/readers/util.h
