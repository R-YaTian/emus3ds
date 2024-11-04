TOPDIR ?= $(CURDIR)

.PHONY: pot

TARGET		:= emus3ds
APP_TITLE	:= High compatibility emulators for 3DS
APP_VERSION	:= R2

CFILES	:=	$(wildcard src/3ds/*.c) \
            $(wildcard src/cores/temperpce/3ds/*.c) \
            $(wildcard src/cores/picodrive/3ds/*.c)

CPPFILES :=	$(wildcard src/3ds/*.cpp) \
            $(wildcard src/cores/temperpce/3ds/*.cpp) \
            $(wildcard src/cores/virtuanes/3ds/*.cpp) \
            $(wildcard src/cores/picodrive/3ds/*.cpp)

po/%/emus3ds.po: $(TOPDIR)/$(notdir $(TARGET)).pot
	@echo "Building $@"
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	@if [ -f $@ ]; \
	then \
		msgmerge --backup=off -qU $@ $< && touch $@; \
	else \
		msginit -i $< --no-translator -l $*.UTF-8 -o $@; \
	fi

pot: $(CFILES) $(CPPFILES)
	@echo "Building $(TARGET).pot"
	xgettext -o $(TARGET).pot $^ -kgetText -c \
		--from-code="UTF-8" \
		--copyright-holder="R-YaTian" \
		--package-name="$(APP_TITLE)" \
		--package-version="$(APP_VERSION)" \
		--msgid-bugs-address="gmtianya@gmail.com"
