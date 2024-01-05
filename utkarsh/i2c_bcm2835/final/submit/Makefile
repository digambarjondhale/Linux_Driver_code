SUBDIRS := $(wildcard */.)

.PHONY: all $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	@echo "Making $@..."

	$(MAKE) -C $@ -f Makefile

clean:
	@for dir in $(SUBDIRS); do\
                $(MAKE) clean -C $$dir;\
        done

