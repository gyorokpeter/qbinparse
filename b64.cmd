setlocal
cls
@call config.cmd
@if not exist libq64.a (
    echo create libq64.a as per https://code.kx.com/q/interfaces/using-c-functions/#windows-mingw-64
    :: nm -p q64.lib|egrep 'T [^.]'|sed 's/0* T //' >>q64.def
    exit /b 1
)
g++ --std=c++20 -Wall -shared qbinparse.cpp -m64 -I%KX_KDB_PATH%/c/c -L. -lq64 -o qbinparse_w64.dll -static -static-libgcc -static-libstdc++
