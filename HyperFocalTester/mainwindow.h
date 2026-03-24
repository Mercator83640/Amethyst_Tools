#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFutureWatcher>
#include <opencv2/opencv.hpp>
#include <QSettings>
#include <QComboBox>

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QPushButton;
class QListWidget;
class QSlider;
class QSpinBox;
class QCheckBox;
class QDoubleSpinBox;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onBrowseInput();
    void onBrowseOutput();
    void onRun();
    void onRunFinished();
    void onStackIndexChanged(int value);

private:
    void loadSettings();
    void saveSettings() const;

private:
    struct RunResult {
        cv::Mat merged;
        cv::Mat previewDepth;
        qint64 elapsedMs = 0;
        QString outputPath;
        QString error;
    };

    void buildUi();
    void setBusy(bool busy);
    void loadFolderPreview(const QString &dirPath);
    std::vector<QString> loadImageList(const QString &dirPath) const;
    QImage matToQImage(const cv::Mat &mat) const;
    void showMatOnLabel(const cv::Mat &mat, QLabel *label);
    cv::Mat makeDepthPreviewFromStack(const std::vector<cv::Mat> &stack) const;
    void applyPreset(const QString &presetName);
    void updateAlignmentUi();
    void onPresetChanged(const QString &presetName);

    QLineEdit *m_inputDir = nullptr;
    QPushButton *m_browseInput = nullptr;
    QLineEdit *m_outputFile = nullptr;
    QPushButton *m_browseOutput = nullptr;
    QPushButton *m_run = nullptr;

    QComboBox *m_preset = nullptr;

    QListWidget *m_stackList = nullptr;
    QSlider *m_stackSlider = nullptr;
    QLabel *m_stackImage = nullptr;
    QLabel *m_resultImage = nullptr;
    QLabel *m_depthImage = nullptr;
    QLabel *m_status = nullptr;
    QLabel *m_benchmark = nullptr;
    QSpinBox *m_consistency = nullptr;
    QDoubleSpinBox *m_denoise = nullptr;
    QCheckBox *m_saveResult = nullptr;
    QCheckBox *m_disableAlignment = nullptr;

    std::vector<QString> m_imagePaths;
    std::vector<cv::Mat> m_loadedStack;
    QFutureWatcher<RunResult> m_watcher;

    QSpinBox *m_reference = nullptr;
    QSpinBox *m_threads = nullptr;
    QSpinBox *m_batchsize = nullptr;
    QSpinBox *m_jpgQuality = nullptr;

    QCheckBox *m_disableOpenCL = nullptr;
    QCheckBox *m_noWhiteBalance = nullptr;
    QCheckBox *m_noContrast = nullptr;
    QCheckBox *m_globalAlign = nullptr;
    QCheckBox *m_fullResolutionAlign = nullptr;
    QCheckBox *m_alignKeepSize = nullptr;
    QCheckBox *m_noCrop = nullptr;
    QCheckBox *m_verbose = nullptr;
    QCheckBox *m_saveSteps = nullptr;
};

#endif
