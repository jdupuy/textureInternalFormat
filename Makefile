# GNU Make solution makefile autogenerated by Premake
# Type "make help" for usage help

ifndef config
  config=debug64
endif
export config

PROJECTS := default rgba2 rgba4 rgba5 r3g3b2

.PHONY: all clean help $(PROJECTS)

all: $(PROJECTS)

default: 
	@echo "==== Building default ($(config)) ===="
	@${MAKE} --no-print-directory -C . -f default.make

rgba2: 
	@echo "==== Building rgba2 ($(config)) ===="
	@${MAKE} --no-print-directory -C . -f rgba2.make

rgba4: 
	@echo "==== Building rgba4 ($(config)) ===="
	@${MAKE} --no-print-directory -C . -f rgba4.make

rgba5: 
	@echo "==== Building rgba5 ($(config)) ===="
	@${MAKE} --no-print-directory -C . -f rgba5.make

r3g3b2: 
	@echo "==== Building r3g3b2 ($(config)) ===="
	@${MAKE} --no-print-directory -C . -f r3g3b2.make

clean:
	@${MAKE} --no-print-directory -C . -f default.make clean
	@${MAKE} --no-print-directory -C . -f rgba2.make clean
	@${MAKE} --no-print-directory -C . -f rgba4.make clean
	@${MAKE} --no-print-directory -C . -f rgba5.make clean
	@${MAKE} --no-print-directory -C . -f r3g3b2.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "   debug64"
	@echo "   release64"
	@echo "   debug32"
	@echo "   release32"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   default"
	@echo "   rgba2"
	@echo "   rgba4"
	@echo "   rgba5"
	@echo "   r3g3b2"
	@echo ""
	@echo "For more information, see http://industriousone.com/premake/quick-start"
