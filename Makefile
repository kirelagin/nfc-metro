PACKAGE = metro

CXX      ?= gcc
CXXFLAGS +=
LDFLAGS  +=

exe_hdr   = $(wildcard src/*.h)
exe_src   = $(wildcard src/*.c)
exe_libs  = -lnfc

EXEEXT    =
LIBEXT    = .so

.PHONY: all check clean

all: $(PACKAGE)$(EXEEXT)

$(PACKAGE)$(EXEEXT): $(patsubst %.c, %.o, $(exe_src))
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(exe_libs) -o $@

clean:
	rm -f src/*.o src/*.d $(PACKAGE)$(EXEEXT)

%.o : %.c
	$(CXX) $(CXXFLAGS) -MD -c $< -o $(patsubst %.c, %.o, $<)

ifneq "$(MAKECMDGOALS)" "clean"
deps  = $(patsubst %.c, %.d, $(exe_src))
-include $(deps)
endif
