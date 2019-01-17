# Qt Direct3D / DirectX Widget

This project contains Direct3D widgets for DirectX 9, 10 and 11 to use within the Qt Framework.

There are also Direct3D widgets that supports [Dear ImGui](https://github.com/ocornut/imgui) within this project, so you can decide whether you want to use only Direct3D widget or with ImGui.


## Getting Started

`git clone https://github.com/giladreich/QtDirect3D`

If you're going to use the widget with `ImGui` support, you'll need to clone the submodule by:

`git submodule update --init --recursive`

Under the `src` directory you'll find the widgets sources.

Under the `src/demo` directory there are few samples to demostrate how to interact with the widget and how easy it is to get started.

When you start a new `Qt GUI Application` project, do the following steps:

* Copy the widget you want to use into your project and add it as existing item.
* Make sure you set the `Include` directories for `DirectX` and `ImGui`(if you use). For `Linker` add the following library: `d3d9.lib` | `d3d10.lib` | `d3d11.lib`(depending on your directx version).
* In your `MainWindow.ui` file you'll need to promote the `centeralWidget` when you start a fresh project so it will draw the Direct3D widget.
* Compile and run and at this point you should have a basic Direct3D scene.

NOTE: If you're using the demo projects, make sure you have `$(QTDIR)` environment variable to the qt version/kit you have.

### Using the Widget

In the `MainWindow.h` file, you'll need to create 3(4 if using ImGui) slots:
```cpp
public Q_SLOTS:
	bool init(bool success);
	void tick();
	void render();
	void uiRender(); // ImGui
```

In your `MainWindow.cpp` file, write the implementation for the follow slots:
* `init` - true is passed from the widget if successful 3d device creation and scene and you can add your addtional initialization to the scene.
* `tick` - to update the scene (i.e moving vertices positions).
* `render` - do your drawing.
* `uiRender` - Draw your ImGui widgets.

Next step is to connect the signal and slots of the widget:
```cpp
connect(ui->view, &QDirect3D9Widget::deviceInitialized, this, &MainWindow::init);
connect(ui->view, &QDirect3D9Widget::ticked, this, &MainWindow::tick);
connect(ui->view, &QDirect3D9Widget::rendered, this, &MainWindow::render);
connect(ui->view, &QDirect3D9Widget::uiRendered, this, &MainWindow::uiRender);
```

To clean up the application properly, we override the [closeEvent](http://doc.qt.io/archives/qt-4.8/qcloseevent.html), call `release` and give it atleast 500 miliseconds delay before we close the application:

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
At this point everything should be up and running.

### Prerequisites

* Qt msvc compiler (I use msvc141, but should work also on older versions).
* Qt VS Plugin(optional if you want to use the demo's).

## Show Case

Using the widget with ImGui and creating a scene:

![Qt and ImGui](/pictures/Qt_and_ImGui.gif)


## Motivation

I've been working with [MFC](https://en.wikipedia.org/wiki/Microsoft_Foundation_Class_Library) GUI Applications for quiet a while now and at the same time used Qt.

When it comes to Desktop GUI Application, I think there is no question that using Qt is the best decision for many reasons.

In my spare time I made few more projects using the MFC framework because they are easier to use when working with the WinAPI, since I wanted to render 3D stuff using Direct3D.

Qt always seem to me more of a OpenGL thing(which it is), the fact that they only have `QOpenGLWidget` build-in and not `QDirect3DWidget` and it's based on OpenGL in general.

I therefore decided to challenge myself using Direct3D in Qt and I was happy to see the end-results.

## Contributing

Pull-Requests are more than welcome and will be very appreciated. So feel free to contribute if you want to improve the project or fix any bugs.

Same goes for opening issues, if you have any suggestions or you found any bugs, please do not hesitate to open an [issue](https://github.com/giladreich/QtDirect3D/issues).

## Authors

* **Gilad Reich** - *Initial work* - [giladreich](https://github.com/giladreich)

See also the list of [contributors](https://github.com/giladreich/QtDirect3D/graphs/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

