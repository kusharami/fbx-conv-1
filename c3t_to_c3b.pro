VERSION = 1.0.1

QT -= gui
DEFINES += "APP_VERSION=\"\\\"$$VERSION\\\"\""
TARGET = c3t_to_c3b
TEMPLATE = app
CONFIG += c++11 console warn_off

INCLUDEPATH += $$PWD/src $$(FBX_SDK_ROOT)/include
DEFINES += FBXSDK_NEW_API

unix {
    QMAKE_CXXFLAGS_WARN_OFF -= -w
    QMAKE_CXXFLAGS += -Wall
}

*-clang {
	QMAKE_CXXFLAGS += \
		-Wno-pragma-pack \
		-Wno-varargs \
		-Wno-null-dereference \
		-Wno-reorder \
		-Wno-unused-value \
		-Wno-unused-variable \
		-Wno-delete-non-virtual-dtor \
		-Wno-format-security \
		-Wno-unsequenced \
		-Wno-format \
		-Wno-parentheses
}

macx {
	FBX_SDK_LIB_PATH = $$(FBX_SDK_ROOT)/lib/clang
	LIBS += -framework Cocoa
}

win32-msvc* {
    QMAKE_CXXFLAGS_WARN_OFF -= -W0
    QMAKE_CXXFLAGS += -W3 /wd4244 /wd4100 /wd4018 /wd4189
    DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS
    FBX_SDK_LIB_PATH = $$(FBX_SDK_ROOT)/lib/vs2013
	contains(QMAKE_HOST.arch, x86_64) {
		FBX_SDK_LIB_PATH = $$FBX_SDK_LIB_PATH/x64
	} else {
		FBX_SDK_LIB_PATH = $$FBX_SDK_LIB_PATH/x86
	}
}

CONFIG(debug, debug|release) {
    CONFIG_DIR = debug
} else {
    CONFIG_DIR = release
}

LIBS += -L$$FBX_SDK_LIB_PATH/$$CONFIG_DIR
macx:LIBS += -lfbxsdk
win32-msvc*:LIBS += -llibfbxsdk-md

SOURCES +=  \
    src/c3t_to_c3b/main.cpp \
    src/modeldata/C3BFile.cpp \
    src/modeldata/FileIO.cpp \
    src/modeldata/Reference.cpp \
    src/modeldata/ReferenceTable.cpp \
    src/modeldata/Serialization.cpp \
    src/modeldata/WriteBinnary.cpp

HEADERS += \
    src/json/BaseJSONWriter.h \
    src/json/JSONWriter.h \
    src/json/UBJSON.h \
    src/json/UBJSONWriter.h \
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
    src/readers/Reader.h \
    src/readers/matrix3.h \
    src/readers/util.h
