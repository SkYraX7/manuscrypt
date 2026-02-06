@echo off
REM Build script for manuscrypt on Windows with MinGW

REM Try to find MinGW and add to PATH if not already available
where gcc >nul 2>&1
if errorlevel 1 (
    echo gcc not found in PATH. Searching for MinGW...

    REM Check common MinGW installation locations
    if exist "C:\msys64\mingw64\bin\gcc.exe" (
        echo Found MinGW at C:\msys64\mingw64
        set "PATH=C:\msys64\mingw64\bin;%PATH%"
    ) else if exist "C:\MinGW\bin\gcc.exe" (
        echo Found MinGW at C:\MinGW
        set "PATH=C:\MinGW\bin;%PATH%"
    ) else if exist "C:\Program Files\MinGW\bin\gcc.exe" (
        echo Found MinGW at C:\Program Files\MinGW
        set "PATH=C:\Program Files\MinGW\bin;%PATH%"
    ) else if exist "C:\mingw64\bin\gcc.exe" (
        echo Found MinGW at C:\mingw64
        set "PATH=C:\mingw64\bin;%PATH%"
    ) else (
        echo ERROR: Could not find MinGW installation.
        echo Please install MinGW or add it to your PATH.
        echo Expected locations: C:\msys64\mingw64, C:\MinGW, C:\Program Files\MinGW, C:\mingw64
        exit /b 1
    )
)

echo Building manuscrypt...

gcc -Wall -Wextra -O2 -std=c11 -I"C:/Program Files/OpenSSL-Win64/include" -c keygen.c -o keygen.o
if errorlevel 1 goto error

gcc -Wall -Wextra -O2 -std=c11 -I"C:/Program Files/OpenSSL-Win64/include" -c keystream.c -o keystream.o
if errorlevel 1 goto error

gcc -Wall -Wextra -O2 -std=c11 -I"C:/Program Files/OpenSSL-Win64/include" -c cipher.c -o cipher.o
if errorlevel 1 goto error

gcc -Wall -Wextra -O2 -std=c11 -I"C:/Program Files/OpenSSL-Win64/include" -c fileio.c -o fileio.o
if errorlevel 1 goto error

gcc -Wall -Wextra -O2 -std=c11 -I"C:/Program Files/OpenSSL-Win64/include" -c main.c -o main.o
if errorlevel 1 goto error

echo Linking...
gcc keygen.o keystream.o cipher.o fileio.o main.o -o manuscrypt.exe -L"C:/Program Files/OpenSSL-Win64/lib" -lssl -lcrypto
if errorlevel 1 goto error

echo.
echo Build successful! Run manuscrypt.exe to start.
goto end

:error
echo.
echo Build failed! Check that MinGW and OpenSSL are installed correctly.
exit /b 1

:end
