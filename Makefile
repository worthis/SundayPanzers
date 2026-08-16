.PHONY: all win switch clean

all: win switch

win:
	@echo "==> Creating build directory..."
	@mkdir -p build/win
	@echo "==> Compiling Windows resources (icon + version info)..."
	x86_64-w64-mingw32-windres -I . meta/app.rc -O coff -o build/app_rc.o
	@echo "==> Building for Windows (C++)..."
	x86_64-w64-mingw32-g++ -I source -I/opt/raylib/win/include -O2 -Wall $(shell find source -name '*.cpp') build/app_rc.o -o build/win/sunday_panzers.exe -L/opt/raylib/win/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static
	@echo "==> Copying resources to build/win/..."
	@rm -rf build/win/data
	@cp -r data build/win/data 2>/dev/null || echo "    [!] Папка data/ не найдена в корне."
	@echo "==> Done! Run: ./build/win/sunday_panzers.exe"

switch:
	@echo "==> Building for Switch (C++)..."
	@$(MAKE) -f Makefile.switch
	@echo "==> Copying artifacts to build/switch/..."
	@mkdir -p build/switch
	@cp -f sunday_panzers.nro build/switch/sunday_panzers.nro
	@rm -rf build/switch/data
	@cp -r data build/switch/data 2>/dev/null || echo "    [!] Папка data/ не найдена в корне."
	@echo "==> Switch build complete: build/switch/sunday_panzers.nro"

clean:
	@rm -rf build
	@rm -f sunday_panzers.nro sunday_panzers.nso sunday_panzers.npdm sunday_panzers.elf sunday_panzers.nacp