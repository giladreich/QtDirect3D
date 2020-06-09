
## Build Instructions

Before getting started, make sure to install everything that is specified in the [Prerequisites](/docs/README.md/#prerequisites).

### Building using CMake

Once you have Qt installed, just like the `Developer Command Prompt for VS` to setup environment variables to the terminal, Qt provides their own environment tools in:

`%AppData%\Microsoft\Windows\Start Menu\Programs\Qt\5.15.0`

Start by launching `Qt 5.15.0 (MSVC 2019 32-bit)` and then run the following commands in the root directory of the project:

```sh
mkdir build-x86 && cd build-x86
cmake .. -G "Visual Studio 16 2019" -A Win32
cmake --build . --config Release -- /M
windeployqt .\bin\x86-vc142\Release\
```

For x64 builds, launch `Qt 5.15.0 (MSVC 2019 64-bit)` and then:

```sh
mkdir build-x64 && cd build-x64
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release -- /M
windeployqt .\bin\x64-vc142\Release\
```

Note that you can launch the generated solution file from the terminal. By doing that, it will pass-forward all the environment variables to IDE, without having to manually change any environment variables in your system.

### Building using Visual Studio

As previously mentioned in the [Prerequisites](#prerequisites), you will need to install `Qt VS Tools` extension and configure it to the path of your installed Qt-Kits (msvc2019 & msvc2019_64).

Also make sure to add one of the Qt-Kits bin directory to your PATH environment variable. Alternatively, you can launch the `VS IDE` (devenv.exe) from the `Qt 5.15.0 (MSVC 2019 XX-bit)` terminal. By doing so, you should have all the required environment variables to debug the project.

### Building using QtCreator

In QtCreator you need to make sure to properly [configure your Qt-Kits](https://doc.qt.io/qtcreator/creator-targets.html). Once you have that done, you can load the project using the `CMakeLists.txt` file in the root directory of the project.

If everything went right, you should be able to compile and debug the project.