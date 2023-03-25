DEMOS_SRC = $(wildcard src/*.c)
DEMOS = $(patsubst src/%.c, public/%.html, $(DEMOS_SRC))

.PHONY: all
all: build/ public/ build/libsokol.a build/libcimgui.a $(DEMOS)

.PHONY: clean
clean:
	rm -rf build/
	rm -rf public/

build/:
	mkdir build
public/:
	mkdir public

public/%.html: src/%.c
	emcc -o $@ $< \
		--shell-file sokol/shell.html \
		-sNO_FILESYSTEM=1 \
		-sASSERTIONS=0 \
		-sMALLOC=emmalloc \
		--closure=1 \
		-Isokol -Icimgui -Iinclude \
		build/libsokol.a build/libcimgui.a

build/libsokol.a:
	emcc -c sokol/sokol.c -o build/libsokol.o -std=gnu11 -Isokol -Icimgui
	emar qc build/libsokol.a build/libsokol.o
	emranlib build/libsokol.a

build/libcimgui.a:
	em++ -std=gnu++11 -Isokol -Icimgui \
		-c cimgui/cimgui.cpp -o build/cimgui.o
	em++ -std=gnu++11 -Isokol -Icimgui \
		-c cimgui/imgui/imgui.cpp -o build/imgui.o
	em++ -std=gnu++11 -Isokol -Icimgui \
		-c cimgui/imgui/imgui_widgets.cpp -o build/imgui_widgets.o
	em++  -std=gnu++11 -Isokol -Icimgui \
		-c cimgui/imgui/imgui_draw.cpp -o build/imgui_draw.o
	em++ -std=gnu++11 -Isokol -Icimgui \
		-c cimgui/imgui/imgui_tables.cpp -o build/imgui_tables.o
	em++ -std=gnu++11 -Isokol -Icimgui \
		-c cimgui/imgui/imgui_demo.cpp -o build/imgui_demo.o
		
	emar qc build/libcimgui.a \
		build/cimgui.o \
		build/imgui.o \
		build/imgui_widgets.o \
		build/imgui_draw.o \
		build/imgui_tables.o \
		build/imgui_demo.o \
	
	emranlib build/libcimgui.a