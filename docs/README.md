
<p align="center"><img src="/media/qt_dx_cube.png" width=256 height=256></p>

<p align="center">
    <a href="https://github.com/giladreich/QtDirect3D/actions" alt="CI Status">
        <img src="https://github.com/giladreich/QtDirect3D/workflows/build/badge.svg" /></a>
    <a href="http://makeapullrequest.com" alt="Pull Requests">
        <img src="https://img.shields.io/badge/PRs-welcome-brightgreen.svg?logo=pre-commit" /></a>
    <a href="https://www.qt.io/" alt="Qt">
        <img src="https://img.shields.io/badge/CMake-Qt-brightgreen.svg?logo=cmake" /></a>
</p>

---

# Qt Direct3D / DirectX Widgets

This project contains Direct3D widgets that can be used within the Qt Framework for DirectX 9, 10, 11 and 12.

There are also Direct3D widgets included that support [Dear ImGui](https://github.com/ocornut/imgui).

## Contents

- [Getting Started](#getting-started)
  * [Building Examples](#building-examples)
  * [Using the Widget](#using-the-widget)
  * [Handling Close Event](#handling-close-event)
- [Preview](#preview)
- [Motivation](#motivation)
- [Contributing](#contributing)
- [Authors](#authors)
- [License](#license)

## Getting Started

Clone the repository:

`git clone --recursive https://github.com/giladreich/QtDirect3D.git`

The main directories are `source` and `examples`:

<table width=100%>
    <tr>
        <th>Directory</th>
        <th>Description</th>
    </tr>
    <tr>
        <td>source</td>
        <td>Qt custom widgets that you can copy to your projects depending on the Direct3D version you use. Note that
            under each Direct3D version, there is also the same widget with ImGui integration.</td>
    </tr>
    <tr>
        <td>examples</td>
        <td>Showing how to integrate and interact with the widget.</td>
    </tr>
</table>

#### Building Examples

* [Building with Visual Studio](/docs/BUILD.md/#building-with-visual-studio).
* [Building with CMake](/docs/BUILD.md/#building-with-cmake).
* [Building with QtCreator](/docs/BUILD.md/#building-with-qtcreator).

#### Using the Widget

In your Qt Widgets Application project, do the following steps:

* Copy the widget into your project and add it as an existing item.
* Open `MainWindow.ui` with QtDesigner and promote the `centeralWidget` to the widget you copied.
* Rename `centeralWidget` to `view`.
* Compile to let Qt's uic & moc compiler to do their magic and have proper IntelliSense.

At this point you should have a basic setup to get started and interact with the widget.

Open `MainWindow.h` file and create the following Qt slots:
```cpp
public slots:
    void init(bool success);
    void tick();
    void render();
    void renderUI();
```
<table width=100%>
  <tr>
    <th>Slot</th>
    <th>Remarks</th>
  </tr>
  <tr>
    <td>bool init(bool success)</td>
    <td>Additional initialization step. success will be true if the widget successfully initialized.</td>
  </tr>
  <tr>
    <td>void tick()</td>
    <td>Update the scene, e.g. change vertices positions.</td>
  </tr>
  <tr>
    <td>void render()</td>
    <td>Present the scene.</td>
  </tr>
  <tr>
    <td>void renderUI()</td>
    <td>Present and manage ImGui windows.</td>
  </tr>
</table>

Open `MainWindow.cpp` and connect the Widget's signals to the previously created slots:

```cpp
connect(ui->view, &QDirect3DXXWidget::deviceInitialized, this, &MainWindow::init);
connect(ui->view, &QDirect3DXXWidget::ticked, this, &MainWindow::tick);
connect(ui->view, &QDirect3DXXWidget::rendered, this, &MainWindow::render);
connect(ui->view, &QDirect3DXXWidget::renderedUI, this, &MainWindow::renderUI);
```

As a final step, add the following code to the end of the `MainWindow::init` function:

```cpp
QTimer::singleShot(500, this, [&] { ui->view->run(); });
```

A short delay of 500 milliseconds before the frames are executed ensures that all Qt's internal signals and slots have finished processing.

##### Handling Close Event

It is necessary to override the [closeEvent](http://doc.qt.io/archives/qt-4.8/qcloseevent.html) in order to properly release any used resources:

```cpp
void MainWindow::closeEvent(QCloseEvent * event)
{
	event->ignore();

	ui->view->release();
	m_bWindowClosing = true;
	QTime dieTime = QTime::currentTime().addMSecs(500);
	while (QTime::currentTime() < dieTime)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

	event->accept();
}
```
Calling the function `event->ignore()` at the beginning will postpone the `closeEvent` allowing for calling the widget's function `release`. Lastly, giving the application a short delay of 500 milliseconds before accepting the `closeEvent`.

## Preview

Using the widget with imgui and creating a basic scene:

<img src="/media/Qt_and_ImGui.gif" alt="Basic Scene" width="100%" />

Game Engine editor using the widget (YouTube playlist):

<a href="https://www.youtube.com/playlist?list=PLDwf9d3YRfPEEA7RwwMjd3O8UzhY5-abR" alt="Basic Scene">
    <img src="https://i.imgur.com/3wEI0cl.jpg" width="100%" /></a>

<img src="https://i.imgur.com/JvoPR66.png" alt="FX Editor" width="100%" />

<img src="https://i.imgur.com/eVxLK5X.png" alt="Flag Editor" width="100%" />


## Motivation

I've been working with both [MFC](https://en.wikipedia.org/wiki/Microsoft_Foundation_Class_Library) GUI applications as well as Qt in various projects.

Whilst rewriting some of the MFC applications to use Qt, I noticed that Qt only provides QOpenGLWidget, but no QDirect3DWidget. I therefore began to research different ways to accomplish this.

I realized Qt's internal rendering engine needed to be disabled in order to render into the widget's surface without the internal engine's interference. Once this was achieved, it was thrilling to use the power of Qt, whilst also using Direct3D as the graphics API.


## Contributing

Pull-Requests are greatly appreciated should you like to contribute to the project.

Same goes for opening issues; if you have any suggestions, feedback or you found any bugs, please do not hesitate to open an [issue](https://github.com/giladreich/QtDirect3D/issues).


## Authors

* **Gilad Reich** - *Initial work* - [giladreich](https://github.com/giladreich)

See also the list of [contributors](https://github.com/giladreich/QtDirect3D/graphs/contributors) who participated in this project.


## License

This project is licensed under the MIT License - see the [license](/docs/LICENSE) for more details.

