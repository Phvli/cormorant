DEFINE    = \
			USE_OPENGL \
			DEBUG \

			# DEBUG_LIGHTWEIGHT \


FILES     = \
			main \
			game/game_logic \
			game/menu \
			game/cutscene \
			game/entity \
			game/entity/create \
			game/entity/component \
			game/entity/activation/timed \
			game/entity/activation/distance \
			game/entity/ai/missile \
			game/entity/ai/spawner \
			game/entity/graphics/dynamic \
			game/entity/graphics/rigid \
			game/entity/graphics/sprite \
			game/entity/physics/dynamic \
			game/terrain \
			game/terrain/node \
			game/terrain/chunk \
            $(ENGINE) $(UTIL) $(MATH) $(GFX)

ENGINE    =	\
            core/engine \
			core/message \
			core/engines/core \
			core/engines/logger \
			core/engines/input \
			core/engines/video \
			core/engines/sound \

UTIL      =	\
			core/util/config \
			core/util/file \
			core/util/string \

MATH      = \
			math/util \
			math/vec2 \
			math/vec3 \
			math/vec4 \
			math/line2 \
			math/tri2 \
			math/tri3 \
			math/poly2 \
			math/poly3 \
			math/mat4 \
			math/quat \

GFX       = \
			gfx/sprite \
			gfx/font \
			gfx/3d/scene \
			gfx/3d/skybox \
			gfx/3d/model \
			gfx/3d/mesh \
			gfx/3d/primitives \
			gfx/3d/material \
			gfx/3d/texture \
			gfx/3d/program \
			gfx/3d/shader \
			gfx/3d/formats/obj \
			gfx/3d/scenery/buildings \
			gfx/3d/scenery/clouds \
			gfx/3d/scenery/rocks \
			gfx/3d/scenery/trees \

RESFILE   = ../res/game.res

# CFLAGS    = -Wall -ansi -pedantic -Werror
# LDFLAGS   = -mwindows -lmingw32 -lSDL2main -lSDL2_mixer -lSDL2_image -lSDL2

# L_SDL     = -mwindows -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_image
L_SDL     = -mwindows -lmingw32 -lSDL2main -lSDL2_mixer -lSDL2_image -lSDL2
L_GL      = -lglew32 -lopengl32
# L_GL      = -lgl -lglew32
# L_GL      = -lopengl32 -lglu32 -lgdi32 -lglew32
CFLAGS    = -Wall -ansi -pedantic -Werror
# CFLAGS    = -Wall -ansi -pedantic -Werror -Wl,-subsystem,windows
LDFLAGS   = -lmingw32 $(L_SDL) $(L_GL) -static-libgcc -static-libstdc++


# CFLAGS    = -Wall -ansi -pedantic -Werror
# LDFLAGS   = -mwindows -lmingw32 -lSDL2main -lSDL2_mixer -lSDL2_image -lSDL2

# CFLAGS    = $(DEBUGGING) -Wall -ansi -pedantic -Werror -Wl,-subsystem,windows
# LDFLAGS   = $(DEBUGGING) -lwinmm -shared-libgcc -shared-libstdc++


#LDFLAGS = -mwindows -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_image -shared-libstdc++
# -static -static-libgcc -static-libstdc++
# LDFLAGS = -shared-libstdc++

CC        = g++
SRCEXT    = cpp

OBJDIR    = ../obj

OBJS      = $(patsubst %,$(OBJDIR)/%.o,$(FILES))

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo $(PROJECT) - $(APP):
	$(CC) $(OBJS) $(RESFILE) $(LDFLAGS) -o $(TARGET)
	
$(OBJDIR)/%.o: %.$(SRCEXT)
	@echo $<:
	$(CC) $(CFLAGS) $(addprefix -D, $(DEFINE)) $< -o $@ -c
	@echo --------------------------------------------------------------------------------
