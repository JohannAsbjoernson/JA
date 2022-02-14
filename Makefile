RACK_DIR ?= ../..

FLAGS += -Idep/include
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard src/*.c)
SOURCES += $(wildcard src/*.h)
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)
DISTRIBUTABLES += $(wildcard README*)

include $(RACK_DIR)/plugin.mk
