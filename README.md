
<p align="center"><img src="pictures/qt_dx_cube.png" width=256 height=256></p>

# Qt Direct3D / DirectX Widget

This project contains Direct3D/DirectX widgets to use within the Qt Framework for DirectX 9, 10, 11 and 12.

There are also Direct3D widgets that supports [Dear ImGui](https://github.com/ocornut/imgui), so you can decide whether you want to use only Direct3D widget or Direct3D widget with ImGui.


## Getting Started

`git clone --recursive https://github.com/giladreich/QtDirect3D`

Under the `src` directory you'll find the widgets sources for each version. If you want the ImGui widget, you'll find it under `src/QDirect3D*Widget/ImGui`.

Under the `examples` directory there are few samples to demostrate how to interact with the widget and how easy it is to get started.

When you start a new `Qt GUI Application` project, do the following steps:

* Copy the widget you want to use into your project and add it as existing item.
* In your `MainWindow.ui` file you'll need to promote the `centeralWidget` when you start a fresh project so it will draw the Direct3D widget.
* Compile and run. 

At this point you should have a basic Direct3D scene.

NOTE: If you're using the demo projects, make sure you have `$(QTDIR)` environment variable to the qt version/kit you have.

### Using the Widget

In the `MainWindow.h` file, you'll need to create 3(4 if using ImGui) slots:
```cpp
public slots:
	bool init(bool success);
	void tick();
	void render();
	void uiRender(); // ImGui
```

In your `MainWindow.cpp` file, connect the signals and slots with the widget:
```cpp
connect(ui->view, &QDirect3D11Widget::deviceInitialized, this, &MainWindow::init);
connect(ui->view, &QDirect3D11Widget::ticked, this, &MainWindow::tick);
connect(ui->view, &QDirect3D11Widget::rendered, this, &MainWindow::render);
connect(ui->view, &QDirect3D11Widget::uiRendered, this, &MainWindow::uiRender);
```

Then do your own implemenetation to the following slots:
* `init` - Additional initialization. True is passed from the widget if the 3d device and scene successfully created.
* `tick` - Scene update (i.e moving vertices positions).
* `render` - Drawing.
* `uiRender` - ImGui widgets drawing.

At this point everything should be up and running.

##### Handling close event

To properly clean up the application, we override the [closeEvent](http://doc.qt.io/archives/qt-4.8/qcloseevent.html), call `release` and give it atleast 500 miliseconds delay before we close the application:

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

### Prerequisites

* Qt msvc compiler (I use msvc141, but should work also with older versions).
* Qt VS Plugin(if you want to use the VS solutions). You could also load the demos in `QtCreator` by loading the `CMakeLists.txt` in the root of the project.

## Show Case

Using the widget with ImGui and creating a scene:

![Qt and ImGui](/pictures/Qt_and_ImGui.gif)


## Motivation

I've been working with [MFC](https://en.wikipedia.org/wiki/Microsoft_Foundation_Class_Library) GUI Applications for quiet a while now and at the same time used Qt.

When it comes to Desktop GUI Application, I think there is no question or doubt that using Qt is the best decision for many reasons.

In my spare time I made few more projects using the MFC framework because they are easier to use when working with the WinAPI, because I wanted to render 3D stuff using DirectX.

Qt always seem to me more of a OpenGL thing, the fact that they only provide `QOpenGLWidget` build-in and not `QDirect3DWidget` and it's based on OpenGL in general.

I therefore decided to challenge myself using DirectX in Qt and was happy to see the end-results.

## Contributing

Pull-Requests are more than welcome and will be very appreciated. So feel free to contribute if you want to improve the project or fix any bugs.

Same goes for opening issues, if you have any suggestions, feedback or you found any bugs, please do not hesitate to open an [issue](https://github.com/giladreich/QtDirect3D/issues).

## Authors

* **Gilad Reich** - *Initial work* - [giladreich](https://github.com/giladreich)

See also the list of [contributors](https://github.com/giladreich/QtDirect3D/graphs/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

