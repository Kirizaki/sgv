#include "../include/mainwindow.hpp"
#include "ui_mainwindow.h"

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Tworzymy render window
    renderWindow_ = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

    // Podpinamy do widgetu z .ui
    ui->vtkWidget->setRenderWindow(renderWindow_);

    // Renderer
    renderer_ = vtkSmartPointer<vtkRenderer>::New();
    renderWindow_->AddRenderer(renderer_);

    renderer_->SetBackground(0.1, 0.2, 0.3);
}

MainWindow::~MainWindow()
{
    delete ui;
}
