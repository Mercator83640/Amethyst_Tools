#include "mainwindow.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QPixmap>
#include <QtConcurrent>
#include <QSettings>
#include <QStandardPaths>
#include <QDateTime>

#include "lib_hyperfocaltreatment.h"
#include "focusstack.hh"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    buildUi();
    loadSettings();

    connect(m_browseInput, &QPushButton::clicked, this, &MainWindow::onBrowseInput);
    connect(m_browseOutput, &QPushButton::clicked, this, &MainWindow::onBrowseOutput);
    connect(m_run, &QPushButton::clicked, this, &MainWindow::onRun);
    connect(m_stackSlider, &QSlider::valueChanged, this, &MainWindow::onStackIndexChanged);
    connect(&m_watcher, &QFutureWatcher<RunResult>::finished, this, &MainWindow::onRunFinished);

    resize(1600, 950);
    setWindowTitle("HyperFocal Tester - Enhanced");

    connect(m_preset, &QComboBox::currentTextChanged,
            this, &MainWindow::onPresetChanged);

    auto setCustomPreset = [this]()
    {
        if (m_preset->currentText() != "Custom")
            m_preset->setCurrentText("Custom");
    };

    connect(m_consistency, qOverload<int>(&QSpinBox::valueChanged), this, setCustomPreset);
    connect(m_denoise, qOverload<double>(&QDoubleSpinBox::valueChanged), this, setCustomPreset);
    connect(m_reference, qOverload<int>(&QSpinBox::valueChanged), this, setCustomPreset);
    connect(m_threads, qOverload<int>(&QSpinBox::valueChanged), this, setCustomPreset);
    connect(m_batchsize, qOverload<int>(&QSpinBox::valueChanged), this, setCustomPreset);
    connect(m_jpgQuality, qOverload<int>(&QSpinBox::valueChanged), this, setCustomPreset);

    connect(m_saveResult, &QCheckBox::toggled, this, setCustomPreset);
    connect(m_disableOpenCL, &QCheckBox::toggled, this, setCustomPreset);
    connect(m_noWhiteBalance, &QCheckBox::toggled, this, setCustomPreset);
    connect(m_noContrast, &QCheckBox::toggled, this, setCustomPreset);
    connect(m_globalAlign, &QCheckBox::toggled, this, setCustomPreset);
    connect(m_fullResolutionAlign, &QCheckBox::toggled, this, setCustomPreset);
    connect(m_alignKeepSize, &QCheckBox::toggled, this, setCustomPreset);
    connect(m_noCrop, &QCheckBox::toggled, this, setCustomPreset);
    connect(m_verbose, &QCheckBox::toggled, this, setCustomPreset);
    connect(m_saveSteps, &QCheckBox::toggled, this, setCustomPreset);
    connect(m_disableAlignment, &QCheckBox::toggled, this, setCustomPreset);

    connect(m_disableAlignment, &QCheckBox::toggled, this, &MainWindow::updateAlignmentUi);
}

void MainWindow::loadSettings()
{
    QSettings settings("Adretek", "HyperFocalTester");

    const QString inputDir = settings.value("paths/inputDir").toString();
    const QString outputFile = settings.value("paths/outputFile").toString();

    m_inputDir->setText(inputDir);
    m_outputFile->setText(outputFile);

    m_consistency->setValue(settings.value("options/consistency", 2).toInt());
    m_denoise->setValue(settings.value("options/denoise", 2.0).toDouble());
    m_saveResult->setChecked(settings.value("options/saveResult", true).toBool());

    m_reference->setValue(settings.value("options/reference", -1).toInt());
    m_threads->setValue(settings.value("options/threads", 1).toInt());
    m_batchsize->setValue(settings.value("options/batchsize", 8).toInt());
    m_jpgQuality->setValue(settings.value("options/jpgQuality", 95).toInt());

    m_disableOpenCL->setChecked(settings.value("options/disableOpenCL", true).toBool());
    m_noWhiteBalance->setChecked(settings.value("options/noWhiteBalance", true).toBool());
    m_noContrast->setChecked(settings.value("options/noContrast", true).toBool());
    m_globalAlign->setChecked(settings.value("options/globalAlign", false).toBool());
    m_fullResolutionAlign->setChecked(settings.value("options/fullResolutionAlign", false).toBool());
    m_alignKeepSize->setChecked(settings.value("options/alignKeepSize", true).toBool());
    m_noCrop->setChecked(settings.value("options/noCrop", false).toBool());
    m_verbose->setChecked(settings.value("options/verbose", false).toBool());
    m_saveSteps->setChecked(settings.value("options/saveSteps", false).toBool());

    m_disableAlignment->setChecked(settings.value("options/disableAlignment", true).toBool());

    if (!inputDir.isEmpty() && QDir(inputDir).exists())
        loadFolderPreview(inputDir);

    const QString preset = settings.value("options/preset", "Balanced").toString();
    const int presetIndex = m_preset->findText(preset);
    if (presetIndex >= 0)
        m_preset->setCurrentIndex(presetIndex);


    if (preset != "Custom")
        applyPreset(preset);

    updateAlignmentUi();
}

void MainWindow::saveSettings() const
{
    QSettings settings("Adretek", "HyperFocalTester");

    settings.setValue("paths/inputDir", m_inputDir->text().trimmed());
    settings.setValue("paths/outputFile", m_outputFile->text().trimmed());

    settings.setValue("options/consistency", m_consistency->value());
    settings.setValue("options/denoise", m_denoise->value());
    settings.setValue("options/saveResult", m_saveResult->isChecked());

    settings.setValue("options/reference", m_reference->value());
    settings.setValue("options/threads", m_threads->value());
    settings.setValue("options/batchsize", m_batchsize->value());
    settings.setValue("options/jpgQuality", m_jpgQuality->value());

    settings.setValue("options/disableOpenCL", m_disableOpenCL->isChecked());
    settings.setValue("options/noWhiteBalance", m_noWhiteBalance->isChecked());
    settings.setValue("options/noContrast", m_noContrast->isChecked());
    settings.setValue("options/globalAlign", m_globalAlign->isChecked());
    settings.setValue("options/fullResolutionAlign", m_fullResolutionAlign->isChecked());
    settings.setValue("options/alignKeepSize", m_alignKeepSize->isChecked());
    settings.setValue("options/noCrop", m_noCrop->isChecked());
    settings.setValue("options/verbose", m_verbose->isChecked());
    settings.setValue("options/saveSteps", m_saveSteps->isChecked());
    settings.setValue("options/disableAlignment", m_disableAlignment->isChecked());

     settings.setValue("options/preset", m_preset->currentText());
}

void MainWindow::buildUi()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);

    auto *inputLayout = new QHBoxLayout();
    m_inputDir = new QLineEdit(this);
    m_browseInput = new QPushButton("Browse input...", this);
    inputLayout->addWidget(new QLabel("Input folder:", this));
    inputLayout->addWidget(m_inputDir);
    inputLayout->addWidget(m_browseInput);

    auto *outputLayout = new QHBoxLayout();
    m_outputFile = new QLineEdit(this);
    m_browseOutput = new QPushButton("Browse output...", this);
    outputLayout->addWidget(new QLabel("Output image:", this));
    outputLayout->addWidget(m_outputFile);
    outputLayout->addWidget(m_browseOutput);

    auto *optionsBox = new QGroupBox("Options", this);
    auto *optionsLayout = new QGridLayout(optionsBox);

    m_consistency = new QSpinBox(this);
    m_consistency->setRange(0, 2);
    m_consistency->setValue(2);

    m_denoise = new QDoubleSpinBox(this);
    m_denoise->setRange(0.0, 10.0);
    m_denoise->setDecimals(1);
    m_denoise->setSingleStep(0.5);
    m_denoise->setValue(2.0);

    m_reference = new QSpinBox(this);
    m_reference->setRange(-1, 999);
    m_reference->setValue(-1);
    m_reference->setToolTip("-1 = middle image");

    m_threads = new QSpinBox(this);
    m_threads->setRange(1, 64);
    m_threads->setValue(1);

    m_batchsize = new QSpinBox(this);
    m_batchsize->setRange(1, 32);
    m_batchsize->setValue(8);

    m_jpgQuality = new QSpinBox(this);
    m_jpgQuality->setRange(0, 100);
    m_jpgQuality->setValue(95);

    m_disableAlignment = new QCheckBox("Disable alignment", this);
    m_disableAlignment->setChecked(true);

    m_saveResult = new QCheckBox("Save output image", this);
    m_saveResult->setChecked(true);

    m_disableOpenCL = new QCheckBox("Disable OpenCL", this);
    m_disableOpenCL->setChecked(true);

    m_noWhiteBalance = new QCheckBox("No white balance", this);
    m_noWhiteBalance->setChecked(true);

    m_noContrast = new QCheckBox("No contrast compensation", this);
    m_noContrast->setChecked(true);

    m_globalAlign = new QCheckBox("Global align", this);
    m_fullResolutionAlign = new QCheckBox("Full resolution align", this);
    m_alignKeepSize = new QCheckBox("Align keep size", this);
    m_alignKeepSize->setChecked(true);

    m_noCrop = new QCheckBox("No crop", this);
    m_verbose = new QCheckBox("Verbose", this);
    m_saveSteps = new QCheckBox("Save steps", this);

    int row = 0;
    optionsLayout->addWidget(new QLabel("Consistency:", this), row, 0);
    optionsLayout->addWidget(m_consistency, row, 1);
    optionsLayout->addWidget(new QLabel("Denoise:", this), row, 2);
    optionsLayout->addWidget(m_denoise, row, 3);
    optionsLayout->addWidget(new QLabel("Reference:", this), row, 4);
    optionsLayout->addWidget(m_reference, row, 5);
    ++row;

    optionsLayout->addWidget(new QLabel("Threads:", this), row, 0);
    optionsLayout->addWidget(m_threads, row, 1);
    optionsLayout->addWidget(new QLabel("Batch size:", this), row, 2);
    optionsLayout->addWidget(m_batchsize, row, 3);
    optionsLayout->addWidget(new QLabel("JPG quality:", this), row, 4);
    optionsLayout->addWidget(m_jpgQuality, row, 5);
    ++row;

    optionsLayout->addWidget(m_saveResult, row, 0, 1, 2);
    optionsLayout->addWidget(m_disableOpenCL, row, 2, 1, 2);
    optionsLayout->addWidget(m_verbose, row, 4, 1, 2);
    ++row;

    optionsLayout->addWidget(m_noWhiteBalance, row, 0, 1, 2);
    optionsLayout->addWidget(m_noContrast, row, 2, 1, 2);
    optionsLayout->addWidget(m_saveSteps, row, 4, 1, 2);
    ++row;

    optionsLayout->addWidget(m_disableAlignment, row, 0, 1, 2);
    optionsLayout->addWidget(m_globalAlign, row, 2, 1, 2);
    optionsLayout->addWidget(m_fullResolutionAlign, row, 4, 1, 2);
    ++row;

    optionsLayout->addWidget(m_alignKeepSize, row, 0, 1, 2);
    optionsLayout->addWidget(m_noCrop, row, 2, 1, 2);
    optionsLayout->setColumnStretch(6, 1);

    m_run = new QPushButton("Run HyperFocus", this);
    m_status = new QLabel("Ready", this);
    m_benchmark = new QLabel("Time: -", this);

    auto *statusLayout = new QHBoxLayout();
    statusLayout->addWidget(m_run);
    statusLayout->addSpacing(12);
    statusLayout->addWidget(m_status);
    statusLayout->addStretch();
    statusLayout->addWidget(m_benchmark);

    auto *contentLayout = new QGridLayout();

    auto *stackBox = new QGroupBox("Input stack", this);
    auto *stackLayout = new QVBoxLayout(stackBox);
    m_stackList = new QListWidget(this);
    m_stackSlider = new QSlider(Qt::Horizontal, this);
    m_stackSlider->setMinimum(0);
    m_stackSlider->setMaximum(0);
    m_stackImage = new QLabel("No stack loaded", this);
    m_stackImage->setMinimumSize(420, 320);
    m_stackImage->setAlignment(Qt::AlignCenter);
    m_stackImage->setStyleSheet("QLabel { background:#111; color:white; border:1px solid #555; }");
    stackLayout->addWidget(m_stackList);
    stackLayout->addWidget(m_stackSlider);
    stackLayout->addWidget(m_stackImage, 1);

    auto *resultBox = new QGroupBox("Merged result", this);
    auto *resultLayout = new QVBoxLayout(resultBox);
    m_resultImage = new QLabel("No result", this);
    m_resultImage->setMinimumSize(560, 420);
    m_resultImage->setAlignment(Qt::AlignCenter);
    m_resultImage->setStyleSheet("QLabel { background:#111; color:white; border:1px solid #555; }");
    resultLayout->addWidget(m_resultImage);

    auto *depthBox = new QGroupBox("Depth preview", this);
    auto *depthLayout = new QVBoxLayout(depthBox);
    m_depthImage = new QLabel("No depth preview", this);
    m_depthImage->setMinimumSize(260, 220);
    m_depthImage->setAlignment(Qt::AlignCenter);
    m_depthImage->setStyleSheet("QLabel { background:#111; color:white; border:1px solid #555; }");
    depthLayout->addWidget(m_depthImage);

    contentLayout->addWidget(stackBox, 0, 0);
    contentLayout->addWidget(resultBox, 0, 1);
    contentLayout->addWidget(depthBox, 0, 2);

    contentLayout->setColumnStretch(0, 2);
    contentLayout->setColumnStretch(1, 4);
    contentLayout->setColumnStretch(2, 1);

    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(outputLayout);
    mainLayout->addWidget(optionsBox);
    mainLayout->addLayout(statusLayout);
    mainLayout->addLayout(contentLayout, 1);


    m_preset = new QComboBox(this);
    m_preset->addItem("Custom");
    m_preset->addItem("Fast");
    m_preset->addItem("Balanced");
    m_preset->addItem("Quality");
    m_preset->addItem("Debug");

    row = 0;
    optionsLayout->addWidget(new QLabel("Preset:", this), row, 0);
    optionsLayout->addWidget(m_preset, row, 1);
    ++row;
}

void MainWindow::applyPreset(const QString &presetName)
{
    if (presetName == "Fast")
    {
        m_consistency->setValue(1);
        m_denoise->setValue(0.5);

        m_reference->setValue(2);
        m_threads->setValue(4);
        m_batchsize->setValue(4);
        m_jpgQuality->setValue(90);

        m_disableOpenCL->setChecked(true);
        m_noWhiteBalance->setChecked(true);
        m_noContrast->setChecked(true);
        m_globalAlign->setChecked(false);
        m_fullResolutionAlign->setChecked(false);
        m_alignKeepSize->setChecked(true);
        m_noCrop->setChecked(false);
        m_verbose->setChecked(false);
        m_saveSteps->setChecked(false);
    }
    else if (presetName == "Balanced")
    {
        m_consistency->setValue(2);
        m_denoise->setValue(1.0);

        m_reference->setValue(2);
        m_threads->setValue(4);
        m_batchsize->setValue(5);
        m_jpgQuality->setValue(95);

        m_disableOpenCL->setChecked(true);
        m_noWhiteBalance->setChecked(true);
        m_noContrast->setChecked(true);
        m_globalAlign->setChecked(false);
        m_fullResolutionAlign->setChecked(false);
        m_alignKeepSize->setChecked(true);
        m_noCrop->setChecked(false);
        m_verbose->setChecked(false);
        m_saveSteps->setChecked(false);
    }
    else if (presetName == "Quality")
    {
        m_consistency->setValue(2);
        m_denoise->setValue(2.0);

        m_reference->setValue(2);
        m_threads->setValue(4);
        m_batchsize->setValue(8);
        m_jpgQuality->setValue(100);

        m_disableOpenCL->setChecked(true);
        m_noWhiteBalance->setChecked(false);
        m_noContrast->setChecked(false);
        m_globalAlign->setChecked(false);
        m_fullResolutionAlign->setChecked(false);
        m_alignKeepSize->setChecked(true);
        m_noCrop->setChecked(false);
        m_verbose->setChecked(false);
        m_saveSteps->setChecked(false);
    }
    else if (presetName == "Debug")
    {
        m_consistency->setValue(2);
        m_denoise->setValue(2.0);

        m_reference->setValue(2);
        m_threads->setValue(1);
        m_batchsize->setValue(5);
        m_jpgQuality->setValue(95);

        m_disableOpenCL->setChecked(true);
        m_noWhiteBalance->setChecked(true);
        m_noContrast->setChecked(true);
        m_globalAlign->setChecked(false);
        m_fullResolutionAlign->setChecked(false);
        m_alignKeepSize->setChecked(true);
        m_noCrop->setChecked(false);
        m_verbose->setChecked(true);
        m_saveSteps->setChecked(true);
    }
}

void MainWindow::onPresetChanged(const QString &presetName)
{
    if (presetName == "Custom")
    {
        saveSettings();
        return;
    }

    applyPreset(presetName);
    saveSettings();
}

void MainWindow::setBusy(bool busy)
{
    m_inputDir->setEnabled(!busy);
    m_browseInput->setEnabled(!busy);
    m_outputFile->setEnabled(!busy);
    m_browseOutput->setEnabled(!busy);
    m_run->setEnabled(!busy);

    m_consistency->setEnabled(!busy);
    m_denoise->setEnabled(!busy);
    m_reference->setEnabled(!busy);
    m_threads->setEnabled(!busy);
    m_batchsize->setEnabled(!busy);
    m_jpgQuality->setEnabled(!busy);

    m_saveResult->setEnabled(!busy);
    m_disableOpenCL->setEnabled(!busy);
    m_noWhiteBalance->setEnabled(!busy);
    m_noContrast->setEnabled(!busy);
    m_globalAlign->setEnabled(!busy);
    m_fullResolutionAlign->setEnabled(!busy);
    m_alignKeepSize->setEnabled(!busy);
    m_noCrop->setEnabled(!busy);
    m_verbose->setEnabled(!busy);
    m_saveSteps->setEnabled(!busy);
    m_disableAlignment->setEnabled(!busy);
}

std::vector<QString> MainWindow::loadImageList(const QString &dirPath) const
{
    std::vector<QString> files;
    QDir dir(dirPath);
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp" << "*.tif" << "*.tiff";
    QFileInfoList infos = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    for (const QFileInfo &fi : infos)
        files.push_back(fi.absoluteFilePath());
    return files;
}

void MainWindow::onBrowseInput()
{
    const QString startDir =
        m_inputDir->text().trimmed().isEmpty()
            ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
            : m_inputDir->text().trimmed();

    const QString dir = QFileDialog::getExistingDirectory(this, "Select input folder", startDir);
    if (dir.isEmpty())
        return;

    m_inputDir->setText(dir);

    if (m_outputFile->text().trimmed().isEmpty())
        m_outputFile->setText(QDir(dir).filePath("hyperfocus_result.jpg"));

    loadFolderPreview(dir);
    saveSettings();
}

void MainWindow::onBrowseOutput()
{
    const QString startFile =
        m_outputFile->text().trimmed().isEmpty()
            ? QDir(m_inputDir->text().trimmed().isEmpty() ? QDir::homePath() : m_inputDir->text().trimmed())
                  .filePath("hyperfocus_result.jpg")
            : m_outputFile->text().trimmed();

    const QString file = QFileDialog::getSaveFileName(
        this,
        "Select output image",
        startFile,
        "Images (*.jpg *.png *.bmp *.tif *.tiff)");

    if (file.isEmpty())
        return;

    m_outputFile->setText(file);
    saveSettings();
}

void MainWindow::loadFolderPreview(const QString &dirPath)
{
    m_imagePaths = loadImageList(dirPath);
    m_loadedStack.clear();
    m_stackList->clear();

    for (const QString &path : m_imagePaths) {
        m_stackList->addItem(QFileInfo(path).fileName());
        cv::Mat img = cv::imread(path.toStdString(), cv::IMREAD_COLOR);
        if (!img.empty())
            m_loadedStack.push_back(img);
    }

    if (m_loadedStack.empty()) {
        m_stackSlider->setRange(0, 0);
        m_stackImage->setText("No valid images");
        return;
    }

    m_stackSlider->setRange(0, int(m_loadedStack.size()) - 1);
    m_stackSlider->setValue(0);
    onStackIndexChanged(0);
}

void MainWindow::onStackIndexChanged(int value)
{
    if (value < 0 || value >= int(m_loadedStack.size()))
        return;

    m_stackList->setCurrentRow(value);
    showMatOnLabel(m_loadedStack[size_t(value)], m_stackImage);
}

QImage MainWindow::matToQImage(const cv::Mat &mat) const
{
    if (mat.empty())
        return QImage();

    if (mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, int(rgb.step), QImage::Format_RGB888).copy();
    }

    if (mat.type() == CV_8UC1) {
        return QImage(mat.data, mat.cols, mat.rows, int(mat.step), QImage::Format_Grayscale8).copy();
    }

    return QImage();
}

void MainWindow::showMatOnLabel(const cv::Mat &mat, QLabel *label)
{
    const QImage img = matToQImage(mat);
    if (img.isNull()) {
        label->setText("Display not supported");
        return;
    }

    label->setPixmap(QPixmap::fromImage(img).scaled(
        label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

cv::Mat MainWindow::makeDepthPreviewFromStack(const std::vector<cv::Mat> &stack) const
{
    if (stack.empty())
        return cv::Mat();

    cv::Mat gray0;
    cv::cvtColor(stack.front(), gray0, cv::COLOR_BGR2GRAY);

    cv::Mat bestScore(gray0.size(), CV_32F, cv::Scalar(-1.0f));
    cv::Mat bestIndex(gray0.size(), CV_8U, cv::Scalar(0));

    for (int z = 0; z < int(stack.size()); ++z) {
        cv::Mat gray, lap, score;
        cv::cvtColor(stack[size_t(z)], gray, cv::COLOR_BGR2GRAY);
        cv::Laplacian(gray, lap, CV_32F, 3);
        score = cv::abs(lap);

        for (int y = 0; y < score.rows; ++y) {
            const float *s = score.ptr<float>(y);
            float *b = bestScore.ptr<float>(y);
            uchar *idx = bestIndex.ptr<uchar>(y);
            for (int x = 0; x < score.cols; ++x) {
                if (s[x] > b[x]) {
                    b[x] = s[x];
                    idx[x] = uchar(z);
                }
            }
        }
    }

    cv::Mat normalized, colored;
    if (stack.size() > 1)
        bestIndex.convertTo(normalized, CV_8U, 255.0 / double(stack.size() - 1));
    else
        normalized = cv::Mat(bestIndex.size(), CV_8U, cv::Scalar(0));

    cv::applyColorMap(normalized, colored, cv::COLORMAP_JET);
    return colored;
}

void MainWindow::onRun()
{
    const QString inputDir = m_inputDir->text().trimmed();
    const QString outputPath = m_outputFile->text().trimmed();

    if (inputDir.isEmpty()) {
        QMessageBox::warning(this, "HyperFocal Tester", "Select an input folder.");
        return;
    }

    saveSettings();
    loadFolderPreview(inputDir);

    if (m_imagePaths.size() < 2) {
        QMessageBox::warning(this, "HyperFocal Tester", "Need at least 2 images.");
        return;
    }

    setBusy(true);
    m_status->setText("Processing...");
    m_benchmark->setText("Time: running...");

    std::vector<QString> imageList = m_imagePaths;

    int consistency = m_consistency->value();
    double denoise = m_denoise->value();
    bool saveResult = m_saveResult->isChecked();

    int reference = m_reference->value();
    int threads = m_threads->value();
    int batchsize = m_batchsize->value();
    int jpgQuality = m_jpgQuality->value();

    bool disableOpenCL = m_disableOpenCL->isChecked();
    bool noWhiteBalance = m_noWhiteBalance->isChecked();
    bool noContrast = m_noContrast->isChecked();
    bool globalAlign = m_globalAlign->isChecked();
    bool fullResolutionAlign = m_fullResolutionAlign->isChecked();
    bool alignKeepSize = m_alignKeepSize->isChecked();
    bool noCrop = m_noCrop->isChecked();
    bool verbose = m_verbose->isChecked();
    bool saveSteps = m_saveSteps->isChecked();

    auto future = QtConcurrent::run([
                                        imageList,
                                        outputPath,
                                        consistency,
                                        denoise,
                                        saveResult,
                                        reference,
                                        threads,
                                        batchsize,
                                        jpgQuality,
                                        disableOpenCL,
                                        noWhiteBalance,
                                        noContrast,
                                        globalAlign,
                                        fullResolutionAlign,
                                        alignKeepSize,
                                        noCrop,
                                        verbose,
                                        saveSteps,
                                        this]() -> RunResult
                                    {
                                        RunResult rr;
                                        rr.outputPath = outputPath;

                                        std::vector<cv::Mat> stack;
                                        for (const QString &path : imageList) {
                                            cv::Mat img = cv::imread(path.toStdString(), cv::IMREAD_COLOR);
                                            if (!img.empty())
                                                stack.push_back(img);
                                        }

                                        if (stack.size() < 2) {
                                            rr.error = "Not enough readable images.";
                                            return rr;
                                        }

                                        Lib_HyperFocalTreatment focus;
                                        focus.setConsistencyFactor(consistency);
                                        focus.setDenoisingFactor(float(denoise));

                                        focus.InitFocusStackOption(outputPath, true);

                                        focus.stack.set_reference(reference);
                                        focus.stack.set_threads(threads);
                                        focus.stack.set_batchsize(batchsize);
                                        focus.stack.set_jpgquality(jpgQuality);
                                        focus.stack.set_disable_opencl(disableOpenCL);
                                        focus.stack.set_save_steps(saveSteps);
                                        focus.stack.set_nocrop(noCrop);
                                        //focus.stack.set_verbose(verbose);

                                        int flags = focusstack::FocusStack::ALIGN_DEFAULT;

                                        if (globalAlign)
                                            flags |= focusstack::FocusStack::ALIGN_GLOBAL;
                                        if (fullResolutionAlign)
                                            flags |= focusstack::FocusStack::ALIGN_FULL_RESOLUTION;
                                        if (alignKeepSize)
                                            flags |= focusstack::FocusStack::ALIGN_KEEP_SIZE;
                                        if (noWhiteBalance)
                                            flags |= focusstack::FocusStack::ALIGN_NO_WHITEBALANCE;
                                        if (noContrast)
                                            flags |= focusstack::FocusStack::ALIGN_NO_CONTRAST;

                                        focus.stack.set_align_flags(flags);
                                        bool disableAlignment = m_disableAlignment->isChecked();
                                        focus.stack.set_disable_alignment(disableAlignment);

                                        focus.PushListImageRaw(imageList);

                                        qint64 startMs = QDateTime::currentMSecsSinceEpoch();
                                        rr.merged = focus.HyperFocusRun();
                                        rr.elapsedMs = QDateTime::currentMSecsSinceEpoch() - startMs;

                                        rr.previewDepth = makeDepthPreviewFromStack(stack);

                                        if (saveResult && !rr.merged.empty())
                                            cv::imwrite(outputPath.toStdString(), rr.merged);

                                        if (rr.merged.empty() && rr.error.isEmpty())
                                            rr.error = "HyperFocus returned an empty image.";

                                        return rr;
                                    });

    m_watcher.setFuture(future);
}

void MainWindow::updateAlignmentUi()
{
    const bool enabled = !m_disableAlignment->isChecked();
    m_globalAlign->setEnabled(enabled);
    m_fullResolutionAlign->setEnabled(enabled);
    m_alignKeepSize->setEnabled(enabled);
    m_noWhiteBalance->setEnabled(enabled);
    m_noContrast->setEnabled(enabled);
    m_noCrop->setEnabled(enabled);
}

void MainWindow::onRunFinished()
{
    RunResult rr = m_watcher.result();
    setBusy(false);

    if (!rr.error.isEmpty()) {
        m_status->setText("Failed");
        m_benchmark->setText("Time: -");
        QMessageBox::warning(this, "HyperFocal Tester", rr.error);
        return;
    }

    showMatOnLabel(rr.merged, m_resultImage);
    showMatOnLabel(rr.previewDepth, m_depthImage);

    m_status->setText(m_saveResult->isChecked()
                          ? QString("Done - saved to %1").arg(rr.outputPath)
                          : "Done");

    m_benchmark->setText(QString("Time: %1 ms").arg(rr.elapsedMs));
}
