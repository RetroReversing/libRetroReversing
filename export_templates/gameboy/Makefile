roms := {{ GAME_NAME }}.gb

rom_obj := \
data.o \
main.o \
common.o \
jumps.o \

game_obj        := $(rom_obj:.o=_rr.o)



### Build tools

ifeq (,$(shell which sha1sum))
SHA1 := shasum
else
SHA1 := sha1sum
endif

RGBDS ?=
RGBASM  ?= $(RGBDS)rgbasm
RGBFIX  ?= $(RGBDS)rgbfix
RGBGFX  ?= $(RGBDS)rgbgfx
RGBLINK ?= $(RGBDS)rgblink


### Build targets

.SUFFIXES:
.SECONDEXPANSION:
.PRECIOUS:
.SECONDARY:
.PHONY: all red clean tidy compare tools

all: $(roms)
game:        {{ GAME_NAME }}.gb

clean: tidy
	find gfx \( -iname '*.1bpp' -o -iname '*.2bpp' -o -iname '*.pic' \) -delete

tidy:
	rm -f $(roms) $(game_obj) $(roms:.gb=.map) $(roms:.gb=.sym) rgbdscheck.o
	$(MAKE) clean -C tools/

compare: $(roms)
	@$(SHA1) -c roms.sha1

tools:
	$(MAKE) -C tools/


RGBASMFLAGS = -h -L -Weverything
# Create a sym/map for debug purposes if `make` run with `DEBUG=1`
ifeq ($(DEBUG),1)
RGBASMFLAGS += -E
endif

$(gameobj):        RGBASMFLAGS += -D _libRR

rgbdscheck.o: common/rgbdscheck.asm
	$(RGBASM) -o $@ $<

# The dep rules have to be explicit or else missing files won't be reported.
# As a side effect, they're evaluated immediately instead of when the rule is invoked.
# It doesn't look like $(shell) can be deferred so there might not be a better way.
define DEP
$1: $2 $$(shell tools/scan_includes $2) | rgbdscheck.o
	$$(RGBASM) $$(RGBASMFLAGS) -o $$@ $$<
endef

# Build tools when building the rom.
# This has to happen before the rules are processed, since that's when scan_includes is run.
ifeq (,$(filter clean tidy tools,$(MAKECMDGOALS)))

$(info $(shell $(MAKE) -C tools))

# Dependencies for objects (drop _red and _blue from asm file basenames)
$(foreach obj, $(game_obj), $(eval $(call DEP,$(obj),$(obj:_rr.o=.asm))))

endif


%.asm: ;


game_pad        = 0x00

game_opt        = -jsv -n 0 -k 01 -l 0x33 -m 0x13 -r 03 -t "{{ GAME_NAME }}"

%.gb: $$(game_obj) layout.link
	$(RGBLINK) -p $(game_pad) -d -m $*.map -n $*.sym -l layout.link -o $@ $(filter %.o,$^)
	$(RGBFIX) -p $(game_pad) $(game_opt) $@

%.gbc: $$(game_obj) layout.link
	$(RGBLINK) -p $(game_pad) -d -m $*.map -n $*.sym -l layout.link -o $@ $(filter %.o,$^)
	$(RGBFIX) -p $(game_pad) $(game_opt) $@


### Misc file-specific graphics rules
gfx/tilesets/%.2bpp: tools/gfx += --trim-whitespace


### Catch-all graphics rules

%.png: ;

%.2bpp: %.png
	$(RGBGFX) $(rgbgfx) -o $@ $<
	$(if $(tools/gfx),\
		tools/gfx $(tools/gfx) -o $@ $@)

%.1bpp: %.png
	$(RGBGFX) $(rgbgfx) -d1 -o $@ $<
	$(if $(tools/gfx),\
		tools/gfx $(tools/gfx) -d1 -o $@ $@)

%.pic: %.2bpp
	tools/pkmncompress $< $@