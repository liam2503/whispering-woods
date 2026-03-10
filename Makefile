OS := $(shell uname -s)

ifeq ($(OS), Darwin)
BREW_PREFIX := $(shell brew --prefix)
SFML_PATH = $(BREW_PREFIX)/Cellar/sfml@2/2.6.2_1
CPR_PATH = $(BREW_PREFIX)/opt/cpr
EXE_EXT = 
CXX = clang++
LDFLAGS = -L$(SFML_PATH)/lib -L$(CPR_PATH)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network -lcpr -lcurl
RELEASE_LDFLAGS = $(LDFLAGS)
CXXFLAGS = -std=c++17 -g -I$(SFML_PATH)/include -I$(CPR_PATH)/include -I$(SRC_DIR) -MMD -MP
RELEASE_CXXFLAGS = -std=c++17 -O3 -I$(SFML_PATH)/include -I$(CPR_PATH)/include -I$(SRC_DIR) -MMD -MP
else
SFML_PATH = C:/SFML-2.6.2
CPR_PATH = C:/msys64/mingw64
EXE_EXT = .exe
CXX = g++
LDFLAGS = -L$(SFML_PATH)/lib -L$(CPR_PATH)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network -lcpr -lcurl
RELEASE_LDFLAGS = $(LDFLAGS) -mwindows -static-libgcc -static-libstdc++
CXXFLAGS = -std=c++17 -g -I$(SFML_PATH)/include -I$(CPR_PATH)/include -I$(SRC_DIR) -MMD -MP
RELEASE_CXXFLAGS = -std=c++17 -O3 -I$(SFML_PATH)/include -I$(CPR_PATH)/include -I$(SRC_DIR) -MMD -MP
endif

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
RELEASE_DIR = release

TARGET = $(BIN_DIR)/main$(EXE_EXT)

rwildcard = $(foreach d,$(wildcard $(1)*),$(call rwildcard,$d/,$(2)) $(filter $(subst *,%,$(2)),$d))
SOURCES := $(call rwildcard,$(SRC_DIR)/,*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))
DEPS := $(OBJECTS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

release:
	$(MAKE) clean_obj
	$(MAKE) build_release

build_release: CXXFLAGS = $(RELEASE_CXXFLAGS)
build_release: $(OBJECTS)
	@mkdir -p $(RELEASE_DIR)
ifeq ($(OS), Darwin)
	@mkdir -p $(RELEASE_DIR)/game.app/Contents/MacOS
	@mkdir -p $(RELEASE_DIR)/game.app/Contents/Resources
	$(CXX) $(OBJECTS) -o $(RELEASE_DIR)/game.app/Contents/MacOS/gameBin $(RELEASE_LDFLAGS)
	cp -r Assets $(RELEASE_DIR)/game.app/Contents/Resources/
	@echo '#!/bin/bash' > $(RELEASE_DIR)/game.app/Contents/MacOS/game
	@echo 'cd "$$(dirname "$$0")/../Resources"' >> $(RELEASE_DIR)/game.app/Contents/MacOS/game
	@echo 'exec "$$(dirname "$$0")/gameBin"' >> $(RELEASE_DIR)/game.app/Contents/MacOS/game
	@chmod +x $(RELEASE_DIR)/game.app/Contents/MacOS/game
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > $(RELEASE_DIR)/game.app/Contents/Info.plist
	@echo '<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">' >> $(RELEASE_DIR)/game.app/Contents/Info.plist
	@echo '<plist version="1.0"><dict>' >> $(RELEASE_DIR)/game.app/Contents/Info.plist
	@echo '<key>CFBundleExecutable</key><string>game</string>' >> $(RELEASE_DIR)/game.app/Contents/Info.plist
	@echo '<key>CFBundleName</key><string>game</string>' >> $(RELEASE_DIR)/game.app/Contents/Info.plist
	@echo '<key>CFBundleIdentifier</key><string>com.teamkitsune.whisperingwood</string>' >> $(RELEASE_DIR)/game.app/Contents/Info.plist
	@echo '<key>CFBundlePackageType</key><string>APPL</string>' >> $(RELEASE_DIR)/game.app/Contents/Info.plist
	@echo '</dict></plist>' >> $(RELEASE_DIR)/game.app/Contents/Info.plist
else
	$(CXX) $(OBJECTS) -o $(RELEASE_DIR)/game$(EXE_EXT) $(RELEASE_LDFLAGS)
	cp $(SFML_PATH)/bin/sfml-graphics-2.dll $(RELEASE_DIR)/
	cp $(SFML_PATH)/bin/sfml-window-2.dll $(RELEASE_DIR)/
	cp $(SFML_PATH)/bin/sfml-audio-2.dll $(RELEASE_DIR)/
	cp $(SFML_PATH)/bin/sfml-system-2.dll $(RELEASE_DIR)/
	cp $(SFML_PATH)/bin/openal32.dll $(RELEASE_DIR)/
	cp -r Assets $(RELEASE_DIR)/
endif

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

clean_obj:
	rm -rf $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(RELEASE_DIR)

.PHONY: all release build_release clean clean_obj