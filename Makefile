# Generated Makefile for libsolace: Mon 11 Sep 02:40:38 AEST 2017
PREFIX = /usr
dbg = -g


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

LIBNAME = lib$(PROJECT).a
LIB_TAGRET = $(BUILD_DIR)/$(SRC_DIR)/$(LIBNAME)

TESTNAME = test_$(PROJECT)
TEST_TAGRET = $(BUILD_DIR)/$(TEST_DIR)/$(TESTNAME)

DOC_DIR = doc
DOC_TARGET = $(DOC_DIR)/html

COVERAGE_REPORT = coverage.info

# First tagret that starts not with '.'' - is a default target to run
.PHONY: codecheck verify ANALYZE_MAKE

all: lib


#-------------------------------------------------------------------------------
# CMake wrapper
#-------------------------------------------------------------------------------
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(GENERATED_MAKE): ${BUILD_DIR}
	cd ${BUILD_DIR} && cmake ..

release: ${BUILD_DIR}
	cd ${BUILD_DIR} && cmake -DCMAKE_BUILD_TYPE=Release ..
	$(MAKE) -C ${BUILD_DIR}

debug: ${BUILD_DIR}
	cd ${BUILD_DIR} && cmake -DCMAKE_BUILD_TYPE=Debug ..
	$(MAKE) -C ${BUILD_DIR}

${BUILD_DIR}/build_coverage: ${BUILD_DIR}
	cd ${BUILD_DIR} && cmake -DCOVERALLS=ON -DCMAKE_BUILD_TYPE=Debug ..
	$(MAKE) -C ${BUILD_DIR}
	$(shell touch ${BUILD_DIR}/build_coverage)

#-------------------------------------------------------------------------------
# Build the project
#-------------------------------------------------------------------------------
$(LIB_TAGRET): $(GENERATED_MAKE)
	$(MAKE) -C ${BUILD_DIR} $(PROJECT)

lib: $(LIB_TAGRET)

# Build the project with coverage
build_coverage: $(BUILD_DIR)/build_coverage

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

tools/cppcheck:
	# Getting latest cppcheck
	git clone --depth 1 https://github.com/danmar/cppcheck.git tools/cppcheck

tools/FlintPlusPlus:
	git clone --depth 1 https://github.com/L2Program/FlintPlusPlus.git tools/FlintPlusPlus

tools/cppcheck/cppcheck: tools/cppcheck
	git -C tools/cppcheck pull
	$(MAKE) -j2 -C tools/cppcheck cppcheck

tools/FlintPlusPlus/flint/flint++: tools/FlintPlusPlus
	git -C tools/FlintPlusPlus pull
	$(MAKE) -j2 -C tools/FlintPlusPlus/flint


cpplint: $(MODULE_HEADERS) $(MODULE_SRC)
	cpplint --recursive --exclude=${TEST_DIR}/ci/* ${INCLUDE_DIR} ${SRC_DIR} ${TEST_DIR}


# TODO: Extra cppcheck options to consider: --enable=style --inconclusive
cppcheck: $(MODULE_HEADERS) $(MODULE_SRC) tools/cppcheck/cppcheck
	tools/cppcheck/cppcheck --std=c++11 --std=posix -D __linux__ --inline-suppr -q --error-exitcode=2 \
	--enable=warning,performance,portability,missingInclude,information,unusedFunction \
	-I include -i test/ci ${SRC_DIR} ${TEST_DIR} examples


flint: $(MODULE_HEADERS) $(MODULE_SRC) tools/FlintPlusPlus/flint/flint++
	tools/FlintPlusPlus/flint/flint++ -v -r ${SRC_DIR} ${TEST_DIR} examples/

scan-build: ANALYZE_MAKE
	cd $(ANALYZE_DIR) && scan-build $(MAKE)

tidy:
	clang-tidy -checks=llvm-*,modernize-*,clang-analyzer-*,-modernize-pass-by-value -header-filter=.* \
	${SRC_DIR}/*.cpp -- -I${INCLUDE_DIR} -Ilibs/fmt/ -std=c++14



codecheck: cpplint flint cppcheck #scan-build



#-------------------------------------------------------------------------------
# Runtime Quality Control
#-------------------------------------------------------------------------------

valgrind-sgcheck: $(TEST_TAGRET)
	# > 3.10 (not avaliable on trusty) --expensive-definedness-checks=yes --read-var-info=yes
	valgrind --trace-children=yes --track-fds=yes --redzone-size=128 --error-exitcode=4 \
	--tool=exp-sgcheck $(TEST_TAGRET)

valgrind-memcheck: $(TEST_TAGRET)
	valgrind --trace-children=yes --track-fds=yes --redzone-size=128 --error-exitcode=3 \
	--tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --partial-loads-ok=no \
	$(TEST_TAGRET)


# valgrind-sgcheck is broken for now :'(
verify: valgrind-memcheck


$(COVERAGE_REPORT): $(BUILD_DIR)/build_coverage
	# capture coverage info
	lcov --directory . --capture --output-file $@
	# filter out system and test code
	lcov --remove coverage.info 'test/*' '/usr/*' 'libs/*' --output-file $@

coverage: $(COVERAGE_REPORT)


coverage_report: $(COVERAGE_REPORT)
	lcov --list $(COVERAGE_REPORT)


#-------------------------------------------------------------------------------
# Insatall
#-------------------------------------------------------------------------------

.PHONY: install
install: $(LIB_TAGRET)
	@install -m 644 -v -D -t $(DESTDIR)$(PREFIX)/lib $(LIB_TAGRET)
	@for i in $(shell find $(INCLUDE_DIR)/$(PROJECT) -type d); do \
		install -m 755 -v -d $(DESTDIR)$(PREFIX)/$$i; \
		install -m 644 -v -D -t $(DESTDIR)$(PREFIX)/$$i $$i/*.hpp; \
	done


.PHONY: uninstall
uninstall:
	$(RM) -f $(DESTDIR)$(PREFIX)/lib/$(LIBNAME)
	$(RM) -rf $(DESTDIR)$(PREFIX)/include/$(PROJECT)


#-------------------------------------------------------------------------------
# Packaging
#-------------------------------------------------------------------------------

.PHONY: debian
debian:
	dpkg-buildpackage -d

.PHONY: debian-clean
debian-clean:
	dpkg-buildpackage -d -T clean

#-------------------------------------------------------------------------------
# Cleanup
#-------------------------------------------------------------------------------

.PHONY: clean
clean:
	$(RM) -rf $(DOC_TARGET) $(BUILD_DIR) $(COVERAGE_REPORT)
	$(RM) tools/cppcheck/cppcheck
	$(RM) tools/FlintPlusPlus/flint/flint++
