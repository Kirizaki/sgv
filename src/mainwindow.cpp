#include "../include/mainwindow.hpp"
#include "ui_mainwindow.h"

#include <vtkRenderWindowInteractor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include "itkImage.h"
#include <vtkImageImport.h>
#include "itkVTKImageExport.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // create render window
    renderWindow_ = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

    // connect widget
    ui->vtkWidget->setRenderWindow(renderWindow_);

    // create viewer
    imageViewer_ = vtkSmartPointer<vtkImageViewer2>::New();

    // create render window (Qt-compatible)
    renderWindow_ = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->vtkWidget->setRenderWindow(renderWindow_);

    // attach render window to the viewer
    imageViewer_->SetRenderWindow(renderWindow_);

    connect(ui->actionOpen_DICOM, &QAction::triggered, this, &MainWindow::openDicom);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openDicom()
{
    // open DICOM series dir
    QString dir = QFileDialog::getExistingDirectory(
        this, "Select DICOM Folder", QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
        return;

    try {
        // ---- Define ITK image type ----
        using PixelType = signed short;                 // DICOM voxels are usually stored as signed 16-bit integers
        constexpr unsigned int Dimension = 3;           // We are working with 3D volumes (x, y, z)
        using ImageType = itk::Image<PixelType, Dimension>; // ITK image type to hold the DICOM volume

        // ---- Reader setup ----
        using ReaderType = itk::ImageSeriesReader<ImageType>; // Reads a series of 2D images into one 3D volume
        auto dicomIO = itk::GDCMImageIO::New();               // ITK reader for DICOM format (using GDCM backend)

        // ---- Generate list of DICOM files ----
        auto nameGenerator = itk::GDCMSeriesFileNames::New(); // Helper to gather filenames belonging to a DICOM series
        nameGenerator->SetUseSeriesDetails(true);             // Ensure series are grouped properly by metadata
        nameGenerator->SetDirectory(dir.toStdString());       // Directory selected by user

        // ---- Get list of available series ----
        auto seriesUID = nameGenerator->GetSeriesUIDs();      // Query all unique series in the folder
        if (seriesUID.empty()) {
            QMessageBox::warning(this, "Error", "No DICOM series found."); // Bail out if folder has no DICOM series
            return;
        }

        // ---- Pick first series ----
        std::string seriesIdentifier = seriesUID.begin()->c_str();
        auto fileNames = nameGenerator->GetFileNames(seriesIdentifier); // Get ordered file list for chosen series

        // ---- Create and run reader ----
        auto reader = ReaderType::New();
        reader->SetImageIO(dicomIO);       // Use GDCM for DICOM parsing
        reader->SetFileNames(fileNames);   // Provide all files for this series
        reader->Update();                  // Actually load the 3D image into memory

        // ---- ITK → VTK pipeline connection ----
        auto itkExporter = itk::VTKImageExport<ImageType>::New(); // Export ITK image
        itkExporter->SetInput(reader->GetOutput());               // Connect exporter to reader output
        auto vtkImporter = vtkSmartPointer<vtkImageImport>::New();// Importer for VTK side

        // Manually connect all callbacks between exporter and importer
        itkExporter->Update();
        vtkImporter->SetUpdateInformationCallback(itkExporter->GetUpdateInformationCallback());
        vtkImporter->SetPipelineModifiedCallback(itkExporter->GetPipelineModifiedCallback());
        vtkImporter->SetWholeExtentCallback(itkExporter->GetWholeExtentCallback());
        vtkImporter->SetSpacingCallback(itkExporter->GetSpacingCallback());
        vtkImporter->SetOriginCallback(itkExporter->GetOriginCallback());
        vtkImporter->SetScalarTypeCallback(itkExporter->GetScalarTypeCallback());
        vtkImporter->SetNumberOfComponentsCallback(itkExporter->GetNumberOfComponentsCallback());
        vtkImporter->SetPropagateUpdateExtentCallback(itkExporter->GetPropagateUpdateExtentCallback());
        vtkImporter->SetUpdateDataCallback(itkExporter->GetUpdateDataCallback());
        vtkImporter->SetDataExtentCallback(itkExporter->GetDataExtentCallback());
        vtkImporter->SetBufferPointerCallback(itkExporter->GetBufferPointerCallback());
        vtkImporter->SetCallbackUserData(itkExporter->GetCallbackUserData());
        vtkImporter->Update(); // Finish pipeline connection

        // ---- Feed the data into VTK viewer ----
        imageViewer_->SetInputData(vtkImporter->GetOutput());        // Give the image to vtkImageViewer2
        imageViewer_->SetRenderWindow(renderWindow_);                // Render window (Qt OpenGL widget)
        imageViewer_->SetupInteractor(ui->vtkWidget->interactor());  // Link to Qt interactor

        // ---- Initialize slice ----
        int minSlice = imageViewer_->GetSliceMin();                  // First slice index
        int maxSlice = imageViewer_->GetSliceMax();                  // Last slice index
        int initSlice = (minSlice + maxSlice) / 2;                   // Start at the middle slice
        imageViewer_->SetSlice(initSlice);

        // Debug info for console
        std::cout << "Slice range: " << minSlice << " - " << maxSlice
                  << ", starting at: " << initSlice << std::endl;

        imageViewer_->Render(); // Render first image

        // ---- Connect slider to slice control ----
        ui->horizontalSlider->setMinimum(minSlice);  // Slider min = first slice
        ui->horizontalSlider->setMaximum(maxSlice);  // Slider max = last slice
        ui->horizontalSlider->setValue(initSlice);   // Slider starts in middle

        // Whenever slider moves, update slice in VTK viewer
        connect(ui->horizontalSlider, &QSlider::valueChanged, this, [this](int value) {
            imageViewer_->SetSlice(value);
            imageViewer_->Render();
        });
    } catch (itk::ExceptionObject &ex) {
        QMessageBox::critical(this, "ITK Error", ex.what());
    }
}
