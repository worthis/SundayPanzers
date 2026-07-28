.PHONY: all win switch clean

all: win switch

win:
	@echo "==> Creating build directory..."
	@mkdir -p build/win
	@echo "==> Building for Windows (C++)..."
	x86_64-w64-mingw32-g++ -I source -I/opt/raylib/win/include -O2 -Wall $(shell find source -name '*.cpp') -o build/win/my_game.exe -L/opt/raylib/win/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static
	@echo "==> Copying resources to build/win/..."
	@rm -rf build/win/data
	@cp -r data build/win/data 2>/dev/null || echo "    [!] Папка data/ не найдена в корне."
	@echo "==> Done! Run: ./build/win/my_game.exe"

switch:
	@echo "==> Building for Switch (C++)..."
	@$(MAKE) -f Makefile.switch
	@echo "==> Switch build complete: my_game.nro"

clean:
	@rm -rf build
	@rm -f my_game.nro my_game.nso my_game.npdm my_game.elf my_game.nacp