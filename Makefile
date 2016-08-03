# libSolace build file
# Please note currently project use cmake as a build system and
# this file is just a convenience wrapper for common tasks.
PROJECT = solace

# Project directory layout
BUILD_DIR = build
ANALYZE_DIR = build-analyze
INCLUDE_DIR = include
SRC_DIR = libsolace
TEST_DIR = test


MODULE_HEADERS = ${INCLUDE_DIR}/*
MODULE_SRC = ${SRC_DIR}/*

GENERATED_MAKE = ${BUILD_DIR}/Makefile

LIBNAME = libsolace.a
LIB_TAGRET = ${BUILD_DIR}/${LIBNAME}

TESTNAME = test_solace
TEST_TAGRET = $(BUILD_DIR)/$(TEST_DIR)/$(TESTNAME)

DOC_DIR = doc
DOC_TARGET = $(DOC_DIR)/html

COVERAGE_REPORT = coverage.json

# First tagret that starts not with '.'' - is a default target to run
.PHONY: codecheck verify clean ANALYZE_MAKE

all: lib


#-------------------------------------------------------------------------------
# CMake wrapper
#-------------------------------------------------------------------------------
${BUILD_DIR}:
	mkdir -p ${BUILD_DIR}

$(GENERATED_MAKE): ${BUILD_DIR}
	cd ${BUILD_DIR} && cmake ..


#-------------------------------------------------------------------------------
# Build the project
#-------------------------------------------------------------------------------
$(LIB_TAGRET): ${GENERATED_MAKE}
	$(MAKE) -C ${BUILD_DIR} solace

lib: $(LIB_TAGRET)

#-------------------------------------------------------------------------------
# Build unit tests
#-------------------------------------------------------------------------------
$(TEST_TAGRET): ${GENERATED_MAKE}
	$(MAKE) -C ${BUILD_DIR} $(TESTNAME)

test: $(LIB_TAGRET) $(TEST_TAGRET)
	cd ${BUILD_DIR} && ./${TEST_DIR}/$(TESTNAME)

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
$(ANALYZE_DIR):
	mkdir -p ${ANALYZE_DIR}

ANALYZE_MAKE: ${ANALYZE_DIR}
	cd ${ANALYZE_DIR} && cmake -DCMAKE_C_COMPILER=$(shell which scan-build) ..

libs/cppcheck:
	#./libs/dependencies
	git clone --depth 3 https://github.com/danmar/cppcheck.git libs/cppcheck

libs/FlintPlusPlus:
	git clone --depth 3 https://github.com/L2Program/FlintPlusPlus.git libs/FlintPlusPlus

libs/cppcheck/cppcheck: libs/cppcheck
	$(MAKE) -j2 -C libs/cppcheck cppcheck

libs/FlintPlusPlus/flint/flint++: libs/FlintPlusPlus
	$(MAKE) -j2 -C libs/FlintPlusPlus/flint

cpplint: $(MODULE_HEADERS) $(MODULE_SRC)
	cpplint --recursive --exclude=${TEST_DIR}/ci/* ${INCLUDE_DIR} ${SRC_DIR} ${TEST_DIR}

#	--enable=style,unusedFunctions,exceptNew,exceptRealloc,possibleError 
#	cppcheck --std=c++11 --enable=all -v -I $(MODULE_HEADERS) $(MODULE_SRC) 
cppcheck: $(MODULE_HEADERS) $(MODULE_SRC) libs/cppcheck/cppcheck
	#--inconclusive
	libs/cppcheck/cppcheck --std=c++11 --std=posix -D __linux__ --inline-suppr -q --error-exitcode=2 \
	--enable=warning,performance,portability,missingInclude,information,unusedFunction \
	-I include -i test/ci ${SRC_DIR} ${TEST_DIR} examples

flint: $(MODULE_HEADERS) $(MODULE_SRC) libs/FlintPlusPlus/flint/flint++
	libs/FlintPlusPlus/flint/flint++ -v -r ${SRC_DIR} ${TEST_DIR} examples/

scan-build: ANALYZE_MAKE
	cd $(ANALYZE_DIR) && scan-build $(MAKE)

tidy:
	clang-tidy -checks=llvm-*,modernize-*,clang-analyzer-*,-modernize-pass-by-value -header-filter=.* \
	${SRC_DIR}/*.cpp -- -I${INCLUDE_DIR} -Ilibs/fmt/ -std=c++14



codecheck: cpplint flint cppcheck #scan-build



#-------------------------------------------------------------------------------
# Runtime Quality Control
#-------------------------------------------------------------------------------

verify: $(TEST_TAGRET)
	# > 3.10 (not avaliable on trusty) --expensive-definedness-checks=yes
	valgrind --trace-children=yes --track-fds=yes --read-var-info=yes --redzone-size=128 --error-exitcode=4 \
	--tool=exp-sgcheck $(TEST_TAGRET)

	valgrind --trace-children=yes --track-fds=yes --read-var-info=yes --redzone-size=128 --error-exitcode=3 \
	--tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --partial-loads-ok=no \
	$(TEST_TAGRET)

$(COVERAGE_REPORT):
	coveralls --exclude build --exclude libs --exclude tests --gcov-options '\-lp' --dump $(COVERAGE_REPORT)

coverage: $(COVERAGE_REPORT)

#-------------------------------------------------------------------------------
# Insatall
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Cleanup
#-------------------------------------------------------------------------------

# Phony targets

clean:
	$(RM) -rf $(DOC_TARGET) $(BUILD_DIR)
