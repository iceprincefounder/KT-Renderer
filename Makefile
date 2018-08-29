
LOGFILE=$(LOGPATH)`date +'%H:%M:%S'`

SRC_DIR := ./src
OBJ_DIR := ./out
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

CXX = g++
CXXFLAGS = -O3 -Wall

.PHONY: clean default install start

default: kt-render

kt-render: clean start $(OBJ_FILES)
	@echo [${LOGFILE}] "--Build $@"
	@$(CXX) -o ${OBJ_DIR}/ktRender $(OBJ_FILES)
	@echo [${LOGFILE}] "--Done!"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo [${LOGFILE}] "--Build $< -> $@  "
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo [${LOGFILE}] "--Done!"

install:
	@echo [${LOGFILE}] "--Run the tester..."
	@${OBJ_DIR}/ktRender ${OBJ_DIR}/output.ppm
	@# xdg-open out/output.ppm
	@# open ./output.ppm # MacOS setting
	@echo [${LOGFILE}] "--Output file:${OBJ_DIR}/output.ppm"
	@echo [${LOGFILE}] "--Done!"

start: ; @echo [${LOGFILE}] "--Start to comple...!"

clean: ; @rm -f $(OBJ_FILES) out/output.ppm
