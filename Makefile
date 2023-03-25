DEMOS_SRC = $(wildcard src/*.c)
DEMOS = $(patsubst src/%.c, public/%.html, $(DEMOS_SRC))
DEMOS_URLS = $(patsubst public/%.html, /%.html, $(DEMOS))

CFLAGS = 
FCFLAGS =

ifeq ($(PROD),1)
  CFLAGS += -Oz --closure=1 -flto
  FCFLAGS += -sMINIFY_HTML -sEVAL_CTORS
endif

.PHONY: all
all: build/ public/ site

.PHONY: run
run: all
	emrun public/index.html

.PHONY: site
site: $(DEMOS)
	> public/index.html
	for w in $(DEMOS_URLS); do echo "<a href=\"$${w}\">$${w}</a>" >> public/index.html; done

.PHONY: clean
clean:
	rm -rf build/
	rm -rf public/

build/:
	mkdir build
public/:
	mkdir public

public/%.html: src/%.c build/libsokol.a build/libcimgui.a
	emcc -o $@ $< $(CFLAGS) $(FCFLAGS) \
		--shell-file sokol/shell.html \
		-sNO_FILESYSTEM=1 \
		-sASSERTIONS=0 \
		-sMALLOC=emmalloc \
		-Isokol -Icimgui -Iinclude \
		build/libsokol.a build/libcimgui.a
ifeq ($(PROD),1)
	wasm-opt $(patsubst %.html, %.wasm, $@) -o $(patsubst %.html, %.wasm, $@) \
		--dce --gufa --flatten --rereloop -Oz --gufa --dce -Oz --converge -Oz
endif

build/libsokol.a:
	emcc -c sokol/sokol.c -o build/libsokol.o $(CFLAGS) -std=gnu11 -Isokol -Icimgui
	emar qc build/libsokol.a build/libsokol.o
	emranlib build/libsokol.a

build/libcimgui.a:
	em++ -std=gnu++11 -Isokol -Icimgui $(CFLAGS) \
		-c cimgui/cimgui.cpp -o build/cimgui.o
	em++ -std=gnu++11 -Isokol -Icimgui $(CFLAGS) \
		-c cimgui/imgui/imgui.cpp -o build/imgui.o
	em++ -std=gnu++11 -Isokol -Icimgui $(CFLAGS) \
		-c cimgui/imgui/imgui_widgets.cpp -o build/imgui_widgets.o
	em++  -std=gnu++11 -Isokol -Icimgui $(CFLAGS) \
		-c cimgui/imgui/imgui_draw.cpp -o build/imgui_draw.o
	em++ -std=gnu++11 -Isokol -Icimgui $(CFLAGS) \
		-c cimgui/imgui/imgui_tables.cpp -o build/imgui_tables.o
	em++ -std=gnu++11 -Isokol -Icimgui $(CFLAGS) \
		-c cimgui/imgui/imgui_demo.cpp -o build/imgui_demo.o
		
	emar qc build/libcimgui.a \
		build/cimgui.o \
		build/imgui.o \
		build/imgui_widgets.o \
		build/imgui_draw.o \
		build/imgui_tables.o \
		build/imgui_demo.o \
	
	emranlib build/libcimgui.a