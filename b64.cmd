setlocal
cls
@call config.cmd
@if not exist libq64.a (
    echo create libq64.a with mklib64.cmd
    exit /b 1
)
g++ --std=c++20 -Wall -shared qbinparse.cpp -m64 -I%KX_KDB_PATH%/c/c -L. -lq64 -o qbinparse_w64.dll -static -static-libgcc -static-libstdc++
