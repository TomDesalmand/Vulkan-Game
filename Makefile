UNAME_S := $(shell uname -s)
CURDIR := $(shell pwd)

CXX       := g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
LOG_LEVEL ?= 2

INCLUDE_DIRS := include

ifeq ($(UNAME_S), Darwin)
	LDFLAGS := -lvulkan -lglfw -lm -L/opt/homebrew/lib -L/usr/local/lib
	CPPFLAGS += -I/opt/homebrew/include -I/usr/local/include
else
	INCLUDES :=
	LDFLAGS  := -lvulkan -lglfw -lm
endif

CPPFLAGS += -DLOG_LEVEL=$(LOG_LEVEL)
CPPFLAGS += -DUSE_STB_TRUETYPE
CPPFLAGS += $(addprefix -I,$(INCLUDE_DIRS))
CPPFLAGS += $(INCLUDES)

RM := rm -f
MKDIR_P := mkdir -p

NAME := project

SRC := $(wildcard *.cpp) \
       $(wildcard source/*.cpp) \
       $(wildcard source/window/*.cpp) \
       $(wildcard source/pipeline/*.cpp) \
       $(wildcard source/devices/*.cpp) \
       $(wildcard source/game/*.cpp) \
       $(wildcard source/systems/*.cpp) \
       $(wildcard source/input/*.cpp) \
       $(wildcard source/utils/*.cpp)

OBJ := $(SRC:.cpp=.o)

VERT_SHADERS := $(wildcard shaders/*.vert)
FRAG_SHADERS := $(wildcard shaders/*.frag)

VERT_SPVS := $(VERT_SHADERS:.vert=.vert.spv)
FRAG_SPVS := $(FRAG_SHADERS:.frag=.frag.spv)

SHADERS_BIN := $(VERT_SPVS) $(FRAG_SPVS)

all: maybe_bear

maybe_bear:
	@if ! command -v bear >/dev/null 2>&1 ; then \
		echo "bear not found in PATH; running normal build"; \
		$(MAKE) --no-print-directory build; \
	elif [ -f compile_commands.json ]; then \
		echo "compile_commands.json already exists; running normal build"; \
		$(MAKE) --no-print-directory build; \
	elif [ -n "$$BEAR_CAPTURE" ]; then \
		$(MAKE) --no-print-directory build; \
	else \
		echo "Capturing compile commands with bear (one-time build) ..."; \
		BEAR_CAPTURE=1 exec bear -- $(MAKE) --no-print-directory build; \
	fi

build: $(NAME) shaders

$(NAME): $(OBJ)
	@echo -e '\e[1m\e[32mLinking $(NAME)\e[0m'
	@$(CXX) $(OBJ) -o $(NAME) $(LDFLAGS)
	@echo -e '\e[1m\e[32mCompiled $(NAME)\e[0m'

%.o: %.cpp
	@echo -e '\e[2mCompiling $< -> $@\e[0m'
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

shaders: $(SHADERS_BIN)
	@echo -e '\e[1m\e[32mCompiled Shaders\e[0m'

%.vert.spv: %.vert
	@echo -e '\e[2mCompiling shader $< -> $@\e[0m'
	@$(MKDIR_P) $(dir $@)
	@glslc $< -o $@

%.frag.spv: %.frag
	@echo -e '\e[2mCompiling shader $< -> $@\e[0m'
	@$(MKDIR_P) $(dir $@)
	@glslc $< -o $@

clean:
	@$(RM) $(OBJ)
	@$(RM) $(wildcard *.d)

fclean: clean
	@$(RM) $(NAME)
	@$(RM) $(wildcard shaders/*.spv)
	@$(RM) compile_commands.json

re: fclean all

.PHONY: all maybe_bear build clean fclean re shaders
