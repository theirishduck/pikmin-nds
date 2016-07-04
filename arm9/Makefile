#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary files
# all directories are relative to this makefile
#---------------------------------------------------------------------------------
BUILD		:=	build
NAMESPACES := entities physics ai debug
NAMESPACE_SOURCES := $(addprefix source/,$(NAMESPACES))
SOURCES		:=	gfx source $(NAMESPACE_SOURCES)
# The .. here causes the project root to be a valid include path, for explicit
# including of files from the ARM9 side when required.
INCLUDES	:=	include source build ..
DATA		:= data
BLEND := art
TEXTURES := $(CURDIR)/art/textures
NITRODIR := $(CURDIR)/nitrofs

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb -mthumb-interwork

CFLAGS	:=	-g -Wall -O2\
			-march=armv5te -mtune=arm946e-s -fomit-frame-pointer\
			-ffast-math \
			$(ARCH)

CFLAGS	+=	$(INCLUDE) -DARM9
CXXFLAGS	:=	$(CFLAGS) -fno-rtti -fno-exceptions -std=c++11

ASFLAGS	:=	-g $(ARCH) -march=armv5te -mtune=arm946e-s

LDFLAGS	=	-specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=	-lfilesystem -lfat -lnds9

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(LIBNDS)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export ARM9ELF	:=	$(CURDIR)/$(TARGET).elf
export DEPSDIR := $(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
					$(foreach dir,$(BLEND),$(CURDIR)/$(dir))

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
BLENDFILES_BONE	:=	$(foreach dir,$(BLEND),$(notdir $(wildcard $(dir)/*.bone.blend)))
BLENDFILES_VERTEX	:=	$(foreach dir,$(BLEND),$(notdir $(wildcard $(dir)/*.vertex.blend)))
TEXTUREFILES := $(foreach dir,$(TEXTURES),$(notdir $(wildcard $(dir)/*.*.png)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) $(CPPFILES:.cpp=.o) \
									$(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export NITROFILES := $(addprefix $(NITRODIR)/textures/,$(notdir $(TEXTUREFILES:.png=))) \
				$(addprefix $(NITRODIR)/actors/,$(notdir $(BLENDFILES_BONE:.bone.blend=.dsgx))) \
				$(addprefix $(NITRODIR)/actors/,$(notdir $(BLENDFILES_VERTEX:.vertex.blend=.dsgx)))

.PHONY: $(BUILD) clean clean-nitrofs clean-actors

#---------------------------------------------------------------------------------
$(BUILD): $(NITROFILES)
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) *.elf *.nds* *.bin $(NITRODIR)

clean-nitrofs:
	@rm -rf $(NITRODIR)

clean-actors:
	@rm -rf $(NITRODIR)/actors

$(NITRODIR)/actors/%.dsgx : $(BLEND)/%.vertex.blend
		@mkdir -p $(NITRODIR)/actors
		bash -c 'set -o pipefail; python3 ../tools/blender2dsgx.py --animation=vertex --vtx10 --output $@ $< 2>&1 | sed -f supress-blender-output.sed'

$(NITRODIR)/actors/%.dsgx : $(BLEND)/%.bone.blend
	@mkdir -p $(NITRODIR)/actors
	bash -c 'set -o pipefail; python3 ../tools/blender2dsgx.py --animation=bone --vtx10 --output $@ $< 2>&1 | sed -f supress-blender-output.sed'

# We need one of these for every paletted format, largely because make doesn't
# seem to support multiple wildcard fields, and I'm *far* too lazy to come up
# with a clever hack. -zeta

$(NITRODIR)/textures/%.2bpp : $(TEXTURES)/%.2bpp.png
	@mkdir -p $(NITRODIR)/textures
	hy ../tools/dtex.hy $< to 2bpp palette at $(@:.2bpp=.pal)
	hy ../tools/dtex.hy $< to 2bpp at $@

$(NITRODIR)/textures/%.t2bpp : $(TEXTURES)/%.t2bpp.png
	@mkdir -p $(NITRODIR)/textures
	hy ../tools/dtex.hy $< to 2bpp palette at $(@:.t2bpp=.pal)
	hy ../tools/dtex.hy $< to 2bpp at $@

$(NITRODIR)/textures/%.4bpp : $(TEXTURES)/%.4bpp.png
	@mkdir -p $(NITRODIR)/textures
	hy ../tools/dtex.hy $< to 4bpp palette at $(@:.4bpp=.pal)
	hy ../tools/dtex.hy $< to 4bpp at $@

$(NITRODIR)/textures/%.t4bpp : $(TEXTURES)/%.t4bpp.png
	@mkdir -p $(NITRODIR)/textures
	hy ../tools/dtex.hy $< to 4bpp palette at $(@:.t4bpp=.pal)
	hy ../tools/dtex.hy $< to 4bpp at $@

$(NITRODIR)/textures/%.8bpp : $(TEXTURES)/%.8bpp.png
	@mkdir -p $(NITRODIR)/textures
	hy ../tools/dtex.hy $< to 8bpp palette at $(@:.8bpp=.pal)
	hy ../tools/dtex.hy $< to 8bpp at $@

$(NITRODIR)/textures/%.t8bpp : $(TEXTURES)/%.t8bpp.png
	@mkdir -p $(NITRODIR)/textures
	hy ../tools/dtex.hy $< to 8bpp palette at $(@:.t8bpp=.pal)
	hy ../tools/dtex.hy $< to 8bpp at $@

$(NITRODIR)/textures/%.a3i5 : $(TEXTURES)/%.a3i5.png
	@mkdir -p $(NITRODIR)/textures
	hy ../tools/dtex.hy $< to a3i5 palette at $(@:.a3i5=.pal)
	hy ../tools/dtex.hy $< to a3i5 at $@

$(NITRODIR)/textures/%.a5i3 : $(TEXTURES)/%.a5i3.png
	@mkdir -p $(NITRODIR)/textures
	hy ../tools/dtex.hy $< to a5i3 palette at $(@:.a5i3=.pal)
	hy ../tools/dtex.hy $< to a5i3 at $@

#---------------------------------------------------------------------------------
else

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(ARM9ELF) :	$(OFILES)
	@echo linking $(notdir $@)
	@$(LD)  $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	%_bin.h:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

%.dsgx.o %_dsgx.h:	%.dsgx
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPSDIR)/*.d

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
