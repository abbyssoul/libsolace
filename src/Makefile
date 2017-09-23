
PROJECT_BUILD_DIR = $(BLD_DIR)/$(PROJECT)
PROJECT_OUT_DIR = $(OUT_DIR)/$(PROJECT)

PROJECT_SRC := $(shell find $(CURDIR) -name "*.cpp")
PROJECT_OBJS = $(PROJECT_SRC:$(CURDIR)/%.cpp=$(PROJECT_BUILD_DIR)/%.o)

PROJECT_TARGET = $(PROJECT_OUT_DIR)/$(PROJECT)

include $(BUILD_ROOT)/Makefile.inc
# LDLIBS += `pkg-config --libs cppunit`
# LDFLAGS += -shared

# Default target
all: $(PROJECT_TARGET)


$(PROJECT_BUILD_DIR):
	@mkdir -p $(PROJECT_BUILD_DIR)

$(PROJECT_OUT_DIR): 
	@mkdir -p $(PROJECT_OUT_DIR)

$(PROJECT_BUILD_DIR)/%.o: $(CURDIR)/%.cpp $(PROJECT_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# $(MODULE_OBJS)
$(PROJECT_TARGET): $(PROJECT_OUT_DIR) $(PROJECT_OBJS) 
	$(CXX) $(LDFLAGS) -o $@ $(TESTS_OBJS) $(LDLIBS)

