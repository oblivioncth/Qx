//! [0]
// Create a taskbar button interface
TaskbarButton* button = new TaskbarButton(widget);

// Connect the window to the widgets window
button->setWindow(widget->windowHandle());

// Change the button's overlay icon
button->setOverlayIcon(QIcon(":/loading.png"));

// Change the button's progress indicator
button->setProgressState(Qx::TaskbarButton::ProgressState::Normal);
progress->setValue(50);
//! [0]