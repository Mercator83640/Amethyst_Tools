#ifndef IMAGEFILTER_MAINWINDOW_H
#define IMAGEFILTER_MAINWINDOW_H

#include <QMainWindow>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


using namespace cv;

#include "qimagetocvmat.h"

#include "cimagefilter.h"


QT_BEGIN_NAMESPACE
namespace Ui { class ImageFilter_MainWindow; }
QT_END_NAMESPACE




class ImageFilter_MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ImageFilter_MainWindow(QWidget *parent = nullptr);
    ~ImageFilter_MainWindow();

public slots:
    void DisplayFocusResult();
protected slots:
    void WaveletFolder_Finish();
    void WaveletFolder_Start();
private slots:
    void on_pbOpenImages_clicked();

    void on_cbTypeTraitement_currentIndexChanged(int index);

    void on_pbVersionLib_clicked();

    void on_pb_WaveletFolder_clicked();

private:
    Ui::ImageFilter_MainWindow *ui;

    CImageFilter *mFocusProjection ;


    cv::Mat m_matImage;
    cv::Mat m_matImageCapture;
    cv::Mat m_matImageFocus[20];
    cv::Mat m_matImageThumbnail;

    int CurrentRow ;
    int CurrentCol ;

    bool Sharpen5x5(cv::Mat MatInput, cv::Mat &MatOut);
    bool KernelGaussian5x5(cv::Mat MatInput, cv::Mat &MatOut);
    void focusProjectionXY_Laplacien(std::vector<cv::Mat> MatInputStack, cv::Mat &MatOuput, bool bSharpen);
};
#endif // IMAGEFILTER_MAINWINDOW_H
