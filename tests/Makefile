vpath %.c ../src

CC=gcc
SGDK?=/opt/gendev/sgdk

ifeq ($(DEBUG),1)
	GDB=gdbserver host:2345
else
	GDB=
endif

INCLUDE_DIRS+=-I$(SGDK)/inc
INCLUDE_DIRS+=-I$(SGDK)/res
INCLUDE_DIRS+=-I.
INCLUDE_DIRS+=-I../res
INCLUDE_DIRS+=-I../src
INCLUDE_DIRS+=-I../src/mw
INCLUDE_DIRS+=-I$(CMOCKA_DIR)/include

CFLAGS=$(INCLUDE_DIRS) \
	-DUNIT_TESTS \
	-ggdb \
	-O0 \
	-Wall \
	-std=c11 \
	-Wextra \
	-Werror \
	-Wno-unused-parameter \
	-Wno-builtin-declaration-mismatch \
	-Wno-unused-function \
	-Wno-char-subscripts

 # list of function signature to mock
MOCKS=midi_process
MD_MOCKS=VDP_drawText

LDFLAGS=-ggdb -ffreestanding -lcmocka -L$(CMOCKA_DIR)/build/src -Wl,-rpath=$(CMOCKA_DIR)/build/src
LDFLAGS+=$(foreach MOCK,$(MD_MOCKS),-Wl,--wrap=$(MOCK))

UNIT_TEST_LDFLAGS=$(LDFLAGS) $(foreach MOCK,$(MOCKS),-Wl,--wrap=$(MOCK))

SRC=$(shell find ../src/* -maxdepth 0 -type f -name '*.c' ! -name '*main.c*' -printf '%f\n')
SRC+=$(shell find ../src/mw/* -maxdepth 0 -type f -name '*.c' ! -name '*main.c*' -printf 'mw/%f\n')
COMMON_TEST_SRC=$(shell find * -maxdepth 0 -type f -name '*.c' -print)
UNIT_TEST_SRC=$(shell find ./unit/* -maxdepth 0 -type f -name '*.c' -print)

SRC_OBJ=$(patsubst %.c,obj/%.o,$(SRC))
COMMON_TEST_OBJ=$(patsubst %.c,obj/%.o,$(COMMON_TEST_SRC))
UNIT_TEST_OBJ=$(patsubst %.c,obj/%.o,$(UNIT_TEST_SRC))
BIN_DIR=bin
OBJ_DIR=obj

CMOCKA_TAR=cmocka-1.1.2.tar.xz
CMOCKA_DIR=cmocka

UNIT_TESTS_TARGET=$(BIN_DIR)/unit_tests

all: clean-target unit

unit: $(UNIT_TESTS_TARGET)
	$(GDB) ./$(UNIT_TESTS_TARGET)
.PHONY: unit

$(SRC_OBJ): | $(OBJ_DIR) $(CMOCKA_DIR)

$(OBJ_DIR):
	mkdir -p $@
	mkdir -p $@/mw
	mkdir -p $@/unit
	mkdir -p $@/system

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(UNIT_TESTS_TARGET): $(SRC_OBJ) $(UNIT_TEST_OBJ) $(COMMON_TEST_OBJ)
	mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(UNIT_TEST_LDFLAGS)

$(CMOCKA_DIR):
	mkdir -p $@
	tar -xf $(CMOCKA_TAR) -C $@ --strip-components 1
	mkdir -p $@/build
	@(cd $@/build && cmake ../ && make -s)

clean-target:
	rm -rf $(UNIT_TESTS_TARGET) $(OBJ_DIR)

clean: clean-target
	rm -rf $(CMOCKA_DIR)
