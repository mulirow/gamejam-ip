# game code directory name
GAMEDIR := src

# binary output directory name
OUTPUTDIR := bin

# name for the output(binary) of the game code
GAMENAME := game

# if you want debug information about memory e.g. memory leaks use -fsanitize=address
# add C flags, like: CFLAGS := -Wall -Werror -Wconversion -Wextra
CFLAGS := 

# add load flags, others like -pthread
LDLIB := -lm -lallegro -lallegro_image -lallegro_primitives -lallegro_font -lallegro_ttf -lallegro_audio -lallegro_acodec

CC := gcc -std=c17
RM := rm -f
MK := mkdir -p

EXT := c

GAMESOURCES := $(shell find $(GAMEDIR) -type f -name *.$(EXT))
GAMEOBJS := $(subst .$(EXT),.o,$(GAMESOURCES))

build: mkdirs buildGAME clean

all: mkdirs buildGAME clean runGAME

run: runGAME

buildGAME: $(GAMEOBJS) 
	@echo "\n  Linking $(GAMENAME)..."
	$(CC) -o $(OUTPUTDIR)/$(GAMENAME) $(GAMEOBJS) $(LDLIB) $(CFLAGS)
	@echo "\n"

%.o : %.$(EXT)	
	$(CC) -c $< -o $@ $(CFLAGS)

mkdirs:
	$(MK) $(OUTPUTDIR)

clean:
	@echo "  Cleaning..."
	$(RM) $(GAMEOBJS) 

runGAME:
	@echo "\n  Starting to run $(GAMENAME)...\n"; ./$(OUTPUTDIR)/$(GAMENAME)
