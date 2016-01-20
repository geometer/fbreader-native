ROOTDIR = $(shell pwd)

include makefiles/opts.mk

ZL_CORE_DIRS = constants drm encoding filesystem image language library logger unix util xml
ZL_CORE_DIR_PATHS = $(patsubst %, src/common/zlibrary/core/%, $(ZL_CORE_DIRS))
ZL_CORE_INCLUDE = $(patsubst %, -I $(ROOTDIR)/%, $(ZL_CORE_DIR_PATHS))

ZL_CORE_DIRS_EXTRA = filesystem/zip unix/filesystem unix/library xml/expat
ZL_CORE_DIR_PATHS_EXTRA = $(patsubst %, src/common/zlibrary/core/%, $(ZL_CORE_DIRS_EXTRA))

ZL_TEXT_DIRS = fonts model json
ZL_TEXT_DIR_PATHS = $(patsubst %, src/common/zlibrary/text/%, $(ZL_TEXT_DIRS))
ZL_TEXT_INCLUDE = $(ZL_CORE_INCLUDE) $(patsubst %, -I $(ROOTDIR)/%, $(ZL_TEXT_DIR_PATHS))

FBREADER_DIRS = bookmodel formats library
FBREADER_DIR_PATHS = $(patsubst %, src/common/fbreader/%, $(FBREADER_DIRS))
FBREADER_INCLUDE = $(ZL_TEXT_INCLUDE) $(patsubst %, -I $(ROOTDIR)/%, $(FBREADER_DIR_PATHS))

FORMAT_DIRS = css doc fb2 html oeb pdb rtf txt util xhtml
FORMAT_DIR_PATHS = $(patsubst %, src/common/fbreader/formats/%, $(FORMAT_DIRS))

all:
	@for dir in $(ZL_CORE_DIR_PATHS) $(ZL_CORE_DIR_PATHS_EXTRA); do \
		if [ -d $$dir ]; then \
			if ! $(MAKE) -C $$dir -f $(ROOTDIR)/makefiles/subdir.mk ROOTDIR=$(ROOTDIR) INCLUDE="$(ZL_CORE_INCLUDE)" $@; then \
				exit 1; \
			fi; \
		fi; \
	done;
	@for dir in $(ZL_TEXT_DIR_PATHS); do \
		if [ -d $$dir ]; then \
			if ! $(MAKE) -C $$dir -f $(ROOTDIR)/makefiles/subdir.mk ROOTDIR=$(ROOTDIR) INCLUDE="$(ZL_TEXT_INCLUDE)" $@; then \
				exit 1; \
			fi; \
		fi; \
	done;
	@for dir in $(FBREADER_DIR_PATHS) $(FORMAT_DIR_PATHS); do \
		if [ -d $$dir ]; then \
			if ! $(MAKE) -C $$dir -f $(ROOTDIR)/makefiles/subdir.mk ROOTDIR=$(ROOTDIR) INCLUDE="$(ZL_TEXT_INCLUDE)" $@; then \
				exit 1; \
			fi; \
		fi; \
	done;

#install: all do_install
#
#do_install:
#	@for dir in $(ZLIBDIRS) $(APPDIRS); do \
#		if [ -d $$dir ]; then \
#			cd $$dir; make $@; cd $(ROOTDIR); \
#		fi; \
#	done
#
#do_install_dev:
#	@for dir in $(ZLIBDIRS); do \
#		if [ -d $$dir ]; then \
#			cd $$dir; make $@; cd $(ROOTDIR); \
#		fi; \
#	done

clean:
	@for dir in $(ZL_CORE_DIR_PATHS) $(ZL_CORE_DIR_PATHS_EXTRA) $(ZL_TEXT_DIR_PATHS) $(FBREADER_DIR_PATHS) $(FORMAT_DIR_PATHS); do \
		if [ -d $$dir ]; then \
			if ! $(MAKE) -C $$dir -f $(ROOTDIR)/makefiles/subdir.mk ROOTDIR=$(ROOTDIR) $@; then \
				exit 1; \
			fi; \
		fi; \
	done;

distclean: clean
