
## Prerequisites

* [Qt 5.15.0](https://www.qt.io/download) - install the following versions `msvc2019` & `msvc2019_64` (might work with other Qt versions that uses [QtMsBuild](https://www.qt.io/blog/2018/01/24/qt-visual-studio-new-approach-based-msbuild)).

* [Visual Studio 2019](https://visualstudio.microsoft.com/downloads/) - also note that when using Visual Studio as the IDE, it requires to install [Qt VS Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools-19123) extension.

* [CMake](https://cmake.org/download/) - this is optional for building with cmake, requires to install any version greater than v12.

### Building with CMake

Once you have Qt installed, just like the `Developer Command Prompt for VS` to setup environment variables in the terminal session, Qt provides their own environment tools:

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

Note that you should launch the generated solution file from the terminal. By doing that, it will pass-forward all the environment variables to the IDE without having to manually configure system's environment variables.

### Building with Visual Studio

As mentioned in the [Prerequisites](#prerequisites), you will need to install `Qt VS Tools` extension and configure it to the path of your installed Qt-Kits (`msvc2019` & `msvc2019_64`).

Under the `examples` directory, there are multiple VS solution files depending on the use case.

### Building with QtCreator

In QtCreator you need to make sure to properly [configure your Qt-Kits](https://doc.qt.io/qtcreator/creator-targets.html). Then you can load the project using the `CMakeLists.txt` file in the root directory of the project.
