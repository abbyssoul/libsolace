
TESTS_BUILD_DIR = $(BLD_DIR)/test
TESTS_OUT_DIR = $(OUT_DIR)/test

TESTS_SRC := $(shell find $(CURDIR) -name "*.cpp")
TESTS_OBJS = $(TESTS_SRC:$(CURDIR)/%.cpp=$(TESTS_BUILD_DIR)/%.o)

TEST_TARGET = $(TESTS_OUT_DIR)/$(PROJECT).test

include $(BUILD_ROOT)/Makefile.inc
LDLIBS += `pkg-config --libs cppunit`

# Default target
all: $(TEST_TARGET)


$(TESTS_BUILD_DIR):
	@mkdir -p $(TESTS_BUILD_DIR)

$(TESTS_OUT_DIR):
	@mkdir -p $(TESTS_OUT_DIR)

$(TESTS_BUILD_DIR)/%.o: $(CURDIR)/%.cpp $(TESTS_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# $(MODULE_OBJS)
$(TEST_TARGET): $(TESTS_OUT_DIR) $(TESTS_OBJS) 
	$(CXX) $(LDFLAGS) -o $@ $(TESTS_OBJS) $(LDLIBS)
