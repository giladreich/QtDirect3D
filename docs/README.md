
<p align="center"><img src="/media/qt_dx_cube.png" width=256 height=256></p>

[![Project Status](https://github.com/giladreich/QtDirect3D/workflows/build/badge.svg)](https://github.com/giladreich/QtDirect3D/actions) [![Pull Requests Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](http://makeapullrequest.com)

---

# Qt Direct3D / DirectX Widgets

This project contains Direct3D widgets to use within the Qt Framework for DirectX 9, 10, 11 and 12.

There are also Direct3D widgets that supports [Dear ImGui](https://github.com/ocornut/imgui) that you can find within this project.


## Contents

- [Getting Started](#getting-started)
  * [Prerequisites](#prerequisites)
  * [Using the Widget](#using-the-widget)
  * [Handling Close Event](#handling-close-event)
- [Show Case](#show-case)
- [Motivation](#motivation)
- [Contributing](#contributing)
- [Authors](#authors)
- [License](#license)

## Getting Started

Clone the repository:

`git clone --recursive https://github.com/giladreich/QtDirect3D.git`

The main interesting directories within this project are `source` and `examples`:


* `source` - Containing the widgets that you can copy to your projects - depending on the Direct3D version you use. Note that under each Direct3D version, there are also ImGui widgets - depending on your use case, you can decide which widget to use.

* `examples` - Containing Qt projects that integrate the widgets - showing real examples of how to use and interact with the widgets.


#### Prerequisites

* Visual Studio 2019 - Also note that if you prefer to use Visual Studio as the IDE, rather than QtCreator, make sure to also install [Qt VS Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools-19123) extension.

* Qt 5.15.0 - msvc2019 & msvc2019_64 (might work with other Qt versions that uses [QtMsBuild](https://www.qt.io/blog/2018/01/24/qt-visual-studio-new-approach-based-msbuild)).

* CMake - You can install any version greater than v12.

#### Using the Widget

In your Qt Widgets Application project, do the following steps:

* Copy into your project the widget you chose to use and add it as an existing item.
* In your `MainWindow.ui` file, you will need to promote the `centeralWidget` to the widget you copied.
* For simplicity and purpose of this example, rename `centeralWidget` to `view`.
* Compile in order to let Qt uic & moc compiler do their magic and have proper intellisense.

At this point you should have a basic setup to get started and interact with the widget.

In the `MainWindow.h` file, you will need to create the following Qt slots:
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
    <td>Update the scene, i.e. change vertices positions.</td>
  </tr>
  <tr>
    <td>void render()</td>
    <td>Present the scene.</td>
  </tr>
  <tr>
    <td>void renderUI()</td>
    <td>Present ImGui windows.</td>
  </tr>
</table>

In your `MainWindow.cpp` file, connect the Widget's signals to our previously created slots:
```cpp
connect(ui->view, &QDirect3DXXWidget::deviceInitialized, this, &MainWindow::init);
connect(ui->view, &QDirect3DXXWidget::ticked, this, &MainWindow::tick);
connect(ui->view, &QDirect3DXXWidget::rendered, this, &MainWindow::render);
connect(ui->view, &QDirect3DXXWidget::renderedUI, this, &MainWindow::renderUI);
```

That's it! At this point you can start adding your own implementation to it.

Note that the scene will not start processing frames immediately, because it gives us better control in the MainWindow when we are ready to start. A good practice would be to add this line in the `MainWindow::init` function:

```cpp
QTimer::singleShot(500, this, [&] { ui->view->run(); });
```

We give it a short delay of 500 milliseconds in case things are still initializing in the background before we start processing frames.

##### Handling Close Event

To properly close the application and clean up any used resources, we override and manipulate the [closeEvent](http://doc.qt.io/archives/qt-4.8/qcloseevent.html). When the user exit the application, we call `event->ignore()` to postpone the close event and then we call `release` on our widget. Lastly, giving the application a short delay of 500 milliseconds before we finally accept the close event:

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

## Show Case

Using the widget with ImGui and creating a basic scene:

![Qt and ImGui](/media/Qt_and_ImGui.gif)

Game Engine editor using the widget (YouTube playlist):

[![YouTube Playlist](https://i.imgur.com/3wEI0cl.jpg)](https://www.youtube.com/playlist?list=PLDwf9d3YRfPEEA7RwwMjd3O8UzhY5-abR)

![FX Editor](https://i.imgur.com/JvoPR66.png)

![Flag Editor](https://i.imgur.com/eVxLK5X.png)


## Motivation

I've been working with [MFC](https://en.wikipedia.org/wiki/Microsoft_Foundation_Class_Library) GUI applications for quite a while and also used Qt in other projects of mine. When it comes to Desktop GUI applications, I think there is no question or doubt that using Qt is probably a better choice for many reasons.

While trying to port some of the MFC applications to use Qt, I noticed that Qt only provides QOpenGLWidget, but no QDirect3DWidget. This is the point where I started researching the subject and realized that it can be done in different ways.

For simplicity and portability reasons, I decided to disable the rendering engine Qt internally are using for painting into the widget surface. This allowed me to draw into the surface of the widget without other rendering engines interfering with the scene. Having this achieved, made it exciting using the power of Qt, whilst using Direct3D API as the rendering engine.


## Contributing

Pull-Requests are greatly appreciated should you like to contribute to the project.

Same goes for opening issues; if you have any suggestions, feedback or you found any bugs, please do not hesitate to open an [issue](https://github.com/giladreich/QtDirect3D/issues).


## Authors

* **Gilad Reich** - *Initial work* - [giladreich](https://github.com/giladreich)

See also the list of [contributors](https://github.com/giladreich/QtDirect3D/graphs/contributors) who participated in this project.


## License

This project is licensed under the MIT License - see the [license](/docs/LICENSE) for more details.
