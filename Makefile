# libSolace build file
# Please note currently project use cmake as a build system and
# this file is just a convenience wrapper for common tasks.
PROJECT = solace


MODULE_HEADERS = include/*
MODULE_SRC = src/*
BUILD_DIR = build
GENERATED_MAKE = ${BUILD_DIR}/Makefile

LIBNAME = libsolace.a
LIB_TAGRET = ${BUILD_DIR}/${LIBNAME}

TESTNAME = test_solace
TEST_TAGRET = $(BUILD_DIR)/$(TESTNAME)

DOC_DIR = doc
DOC_TARGET = $(DOC_DIR)/html


# First tagret that starts not with '.'' - is a default target to run
.PHONY: verify clean ${GENERATED_MAKE}

all: lib


#-------------------------------------------------------------------------------
# CMake wrapper
#-------------------------------------------------------------------------------
${BUILD_DIR}:
	mkdir -p ${BUILD_DIR}

${GENERATED_MAKE}: ${BUILD_DIR}
	cd ${BUILD_DIR} && cmake ..


#-------------------------------------------------------------------------------
# Build lib
#-------------------------------------------------------------------------------
lib: $(LIB_TAGRET)


$(LIB_TAGRET): ${GENERATED_MAKE}
	$(MAKE) -C ${BUILD_DIR} solace


#-------------------------------------------------------------------------------
# Build unit tests
#-------------------------------------------------------------------------------
test: $(LIB_TAGRET) $(TEST_TAGRET)
	cd ${BUILD_DIR} && ./$(TESTNAME)


$(TEST_TAGRET): ${GENERATED_MAKE}
	$(MAKE) -C ${BUILD_DIR} $(TESTNAME)


#-------------------------------------------------------------------------------
# Build docxygen documentation
#-------------------------------------------------------------------------------
$(DOC_TARGET):
	doxygen doc/Doxyfile

# Build project doxygen docs doxyfile.inc 
doc: $(MODULE_HEADERS) $(MODULE_SRC) $(DOC_TARGET)


#-------------------------------------------------------------------------------
# Code quality assurance
#-------------------------------------------------------------------------------

cpplint: $(MODULE_HEADERS) $(MODULE_SRC)
	cpplint --recursive --exclude=test/ci/* include/ src/ test/

#	--enable=style,unusedFunctions,exceptNew,exceptRealloc,possibleError 
#	cppcheck --std=c++11 --enable=all -v -I $(MODULE_HEADERS) $(MODULE_SRC) 
cppcheck: $(MODULE_HEADERS) $(MODULE_SRC)
	#--inconclusive
	cppcheck --std=c++11 --std=posix -D __linux__  -q --error-exitcode=2 \
	--enable=warning,performance,portability,information,unusedFunction,missingInclude \
	-I include -i test/ci src test examples


verify: cpplint cppcheck


#-------------------------------------------------------------------------------
# Insatall
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Cleanup
#-------------------------------------------------------------------------------

# Phony targets

clean:
	$(RM) -rf $(DOC_TARGET) $(BUILD_DIR)
