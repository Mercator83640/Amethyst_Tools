#include "imagefilter_mainwindow.h"
#include "ui_imagefilter_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QThread>

#include "lib_hyperfocaltreatment.h"

ImageFilter_MainWindow::ImageFilter_MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImageFilter_MainWindow)
{
    ui->setupUi(this);
}

ImageFilter_MainWindow::~ImageFilter_MainWindow()
{
    delete ui;
}


void ImageFilter_MainWindow::on_pbOpenImages_clicked()
{
    QStringList ListfileName;

//    ListfileName = QFileDialog::getOpenFileNames(this,
//        tr("Images"), "D:\\adretek\\AdReTek_Projects\\2017-02-18-10-50", tr("Images(*.jpg)"));

    ListfileName = QFileDialog::getOpenFileNames(this,
        tr("Images"), "D:\\adretek\\AdReTek_Projects\\Portage_Qt\\focus-stack-master\\examples\\pcb", tr("Images(*.jpg)"));




    QString filename ;
    int indexcvmat = 0 ;

    std::vector<cv::Mat> MatInputStack ;
//    MatInputStack.clear();
    foreach (filename ,  ListfileName)
    {
        cv::Mat img = cv::imread(filename.toStdString(), IMREAD_COLOR);
        MatInputStack.push_back(img); // conserve les images dans les plans images
    }

//    CImImage Imim ;

//    Imim.ConversionRGBtoGrey(MatInputStack[0], m_matImage) ;
//            QImage image(m_matImage.data, m_matImage.cols, m_matImage.rows, m_matImage.step, QImage::Format_Grayscale8);
//        //    QImage image = img2.rgbSwapped();

//            ui->labelImage->setAlignment(Qt::AlignCenter);
//            ui->labelImage->setPixmap(QPixmap::fromImage(image.scaled(QSize(ui->labelImage->width(),ui->labelImage->height()), Qt::KeepAspectRatio, Qt::FastTransformation)));
//return ;


//    foreach (filename ,  ListfileName)
//    {
//        qDebug() << filename ;
//        cv::Mat img = cv::imread(filename.toStdString(), IMREAD_COLOR);
//        if(img.empty())
//        {
//            qDebug() << "Could not read the image: " << filename ;
//            return ;
//        }
//        m_matImageFocus[indexcvmat] = img.clone();
//        indexcvmat ++ ;
//    }

    FOCUS_INFO MethodFocus ;
    MethodFocus.m_method = ui->cbTypeTraitement->currentIndex() ; //FOCUS_PROJ_LAP
    MethodFocus.m_tileWidth = 32;
    MethodFocus.m_tileHeight = 20;
//    MethodFocus.m_tileWidth = 160;
//    MethodFocus.m_tileHeight = 120;
    MethodFocus.m_uniform = true ;
    MethodFocus.m_bSharpen = false ;

    CImageFilter *mFocusProjection = new CImageFilter(MethodFocus);

    mFocusProjection->setMatInputStack(MatInputStack) ;
    m_matImage = MatInputStack[0].clone();
    mFocusProjection->setMatOuput(&m_matImage) ;

//    mFocusProjection->focusProjectionXY();

    QThread* thread = new QThread( );

     // move the task object to the thread BEFORE connecting any signal/slots
     mFocusProjection->moveToThread(thread);

     connect(thread, SIGNAL(started()), mFocusProjection, SLOT(focusProjectionXY()));
     connect(mFocusProjection, SIGNAL(focusFinished()), thread, SLOT(quit()));
     connect(mFocusProjection, SIGNAL(focusFinished()), this, SLOT(DisplayFocusResult()));

     // automatically delete thread and task object when work is done:
     connect(mFocusProjection, SIGNAL(focusFinished()), mFocusProjection, SLOT(deleteLater()));
     connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

     thread->start();



  //     mFocusProjection.focusProjectionXY(MatInputStack, m_matImage, &MethodFocus);



   // focusProjectionXY_Laplacien(MatInputStack, m_matImage, true);

/*
    QString image_path = "D:\\adretek\\DataXtal\\rawimage3\\haifaaaaaa88-gthumb-gwdata1200-ghdata1200-gfitdatamax.jpg" ;
    cv::Mat img = cv::imread(image_path.toStdString(), IMREAD_COLOR);
    if(img.empty())
    {
        qDebug() << "Could not read the image: " << image_path ;
        return ;
    }

*/
//    QImage img2(m_matImage.data, m_matImage.cols, m_matImage.rows, m_matImage.step, QImage::Format_RGB888);
//    QImage image = img2.rgbSwapped();

//    ui->labelImage->setAlignment(Qt::AlignCenter);
//    ui->labelImage->setPixmap(QPixmap::fromImage(image.scaled(QSize(ui->labelImage->width(),ui->labelImage->height()), Qt::KeepAspectRatio, Qt::FastTransformation)));

}

void ImageFilter_MainWindow::DisplayFocusResult()
{

    QImage image = cvMatToQImage(m_matImage);

//    QImage img2(m_matImage.data, m_matImage.cols, m_matImage.rows, m_matImage.step, QImage::Format_RGB888);
//    QImage image = img2.rgbSwapped();

    ui->labelImage->setAlignment(Qt::AlignCenter);
    ui->labelImage->setPixmap(QPixmap::fromImage(image.scaled(QSize(ui->labelImage->width(),ui->labelImage->height()), Qt::KeepAspectRatio, Qt::FastTransformation)));

}


void ImageFilter_MainWindow::focusProjectionXY_Laplacien(std::vector<cv::Mat> MatInputStack, cv::Mat &MatOuput, bool bSharpen)
{
    cv::Mat ResMat ;
    cv::Mat tempMat ;
    cv::Mat ResGaussianL0 ;
    cv::Mat ResGaussianL1 ;
    std::vector<cv::Mat> ResGaussianList ;
    std::vector<cv::Mat> ResLaplacianListL0 ;
    std::vector<cv::Mat> ResLaplacianListL1 ;
    cv::Mat ResLaplacianL0 ;
    cv::Mat ResLaplacianL1 ;

    long i ;

    if ( (long)MatInputStack.size() < 1)
    {
            return  ;
    }


    if ( (long)MatInputStack.size() == 1)
    {
        MatOuput = MatInputStack[0].clone();
            return  ;
    }

    for(i=0; i<(long)MatInputStack.size(); i++)
    {
        KernelGaussian5x5(MatInputStack[i], ResMat);
        tempMat = ResMat.clone();
        // resize
        cv::Mat MatResize = cv::Mat(tempMat.rows/2 , tempMat.cols/2,tempMat.type(), cv::Scalar(0,0,0,0));
     //   ResMat.resize(MatResize.size()) ;
        cv::resize(tempMat, ResMat, MatResize.size());
        tempMat = ResMat.clone();

        ResGaussianL0 = ResMat.clone();
        ResGaussianList.push_back(tempMat);

        KernelGaussian5x5(tempMat, ResMat);
        tempMat = ResMat.clone();
        // resize
        cv::Mat MatResize0 = cv::Mat(tempMat.rows/2 , tempMat.cols/2,tempMat.type(), cv::Scalar(0,0,0,0));
        cv::resize(tempMat, ResMat, MatResize0.size());

        ResGaussianL1 = ResMat.clone() ;

        cv::Mat MatResize1 = cv::Mat(ResGaussianL0.rows , ResGaussianL0.cols,ResGaussianL0.type(), cv::Scalar(0,0,0,0));
        cv::resize(ResGaussianL1, MatResize1, MatResize1.size());

        tempMat = ResGaussianL0 - MatResize1 ; // laplacien L1
        ResLaplacianListL1.push_back(tempMat); // on stocke les n Lapalaicans L1

        cv::Mat MatResize2 = cv::Mat(MatInputStack[i].rows , MatInputStack[i].cols,MatInputStack[i].type(), cv::Scalar(0,0,0,0));
        cv::resize(ResGaussianL0, MatResize2, MatResize2.size());

        tempMat = MatInputStack[i] - MatResize2 ; // laplacien L0
        ResLaplacianListL0.push_back(tempMat); // on stocke les n Lapalaicans
    }

    // on reconstruit
    tempMat = ResLaplacianListL1[0].clone();
    for ( long j = 1 ; j < (long)ResLaplacianListL1.size(); j++)
    {
        ResMat = cv::max(tempMat, ResLaplacianListL1[j]);
        tempMat = ResMat.clone();
    }
    ResLaplacianL1 = ResMat.clone();

    tempMat = ResLaplacianListL0[0].clone();
    for ( long j = 1 ; j < (long)ResLaplacianListL0.size(); j++)
    {
        ResMat = cv::max(tempMat, ResLaplacianListL0[j]);
        tempMat = ResMat.clone();
    }
    ResLaplacianL0 = ResMat.clone();

    cv::Mat MatResize = cv::Mat(ResGaussianL0.rows , ResGaussianL0.cols,ResGaussianL0.type(), cv::Scalar(0,0,0,0));
    cv::resize(ResLaplacianL1, MatResize, MatResize.size());

    int type ;
    if ( ResGaussianL0.type() == CV_8UC1 )
        type = CV_32FC1;
    else
        type = CV_32FC3;
    cv::Mat MatSum = cv::Mat(ResGaussianL0.rows, ResGaussianL0.cols, type);
    cv::Mat MatFF = cv::Mat(ResGaussianL0.rows, ResGaussianL0.cols, type);

    for ( long j = 0 ; j < (long)ResGaussianList.size(); j++)
    {
        ResGaussianList[j].convertTo(MatFF, type, 1.0 / 255.0);
        MatSum = MatSum + MatFF;
    }
    MatSum /= (long)ResGaussianList.size() ;
    MatSum.convertTo(tempMat, ResGaussianL0.type(),  255.0);

    tempMat = tempMat + MatResize ; // Level 0

    cv::Mat MatResize2 = cv::Mat(MatInputStack[0].rows , MatInputStack[0].cols,MatInputStack[0].type(), cv::Scalar(0,0,0,0));
    cv::resize(tempMat, MatResize2, MatResize2.size());
    tempMat = ResLaplacianL0 + MatResize2 ;

    if (bSharpen)
    {
        Sharpen5x5(tempMat, MatOuput);
    }
    else
        MatOuput = tempMat.clone();

}

bool ImageFilter_MainWindow::KernelGaussian5x5(cv::Mat MatInput, cv::Mat &MatOut)
{
    cv::Point anchor;
    double delta;
    int ddepth;
    int kernel_size;

    anchor = cv::Point( -1, -1 );
    delta = 0;
    ddepth = CV_32F ;
    kernel_size = 5;
    float Ratio = 256.0f;

    cv::Mat  kernelL = (cv::Mat_<float>(5,5) <<
            1.0/Ratio,  4.0/Ratio, 6.0/Ratio, 4.0/Ratio, 1.0/Ratio,
            4.0/Ratio, 16.0/Ratio, 24.0/Ratio, 16.0/Ratio, 4.0/Ratio,
            6.0/Ratio, 24.0/Ratio, 36.0/Ratio, 24.0/Ratio, 6.0/Ratio,
            4.0/Ratio, 16.0/Ratio, 24.0/Ratio, 16.0/Ratio, 4.0/Ratio,
            1.0/Ratio,  4.0/Ratio, 6.0/Ratio, 4.0/Ratio, 1.0/Ratio
            );

    filter2D(MatInput, MatOut, MatInput.type() , kernelL );
    kernelL.release();
    return true ;
}

bool ImageFilter_MainWindow::Sharpen5x5(cv::Mat MatInput, cv::Mat &MatOut)
{
    cv::Point anchor;
    double delta;
    int ddepth;
    int kernel_size;

    anchor = cv::Point( -1, -1 );
    delta = 0;
    ddepth = CV_32F ;
    kernel_size = 5;

    cv::Mat  kernelL = (cv::Mat_<float>(5,5) <<
            -1.0/20,  -3.0/20, -4.0/20, -3.0/20, -1.0/20,
            -3.0/20, 0.0/20, 6.0/20, 0.0/20, -3.0/20,
            -4.0/20, 6.0/20, 40.0/20, 6.0/20, -4.0/20,
            -3.0/20, 0.0/20, 6.0/20, 0.0/20, -3.0/20,
            -1.0/20, -3.0/20, -4.0/20, -3.0/20, -1.0/20
            );


    filter2D(MatInput, MatOut, MatInput.type() , kernelL );

    kernelL.release();

    return true ;
}

void ImageFilter_MainWindow::on_cbTypeTraitement_currentIndexChanged(int index)
{

}


void ImageFilter_MainWindow::on_pbVersionLib_clicked()
{
    Lib_HyperFocalTreatment LibHF ;

    qDebug() << LibHF.GetVersion();
}


void ImageFilter_MainWindow::on_pb_WaveletFolder_clicked()
{
    //
    CurrentRow = 0 ;
    CurrentCol = 1 ;

    mFocusProjection = nullptr ;
    ui->progressbar_wavelet->setRange(0,96);

    WaveletFolder_Start() ;
}


void ImageFilter_MainWindow::WaveletFolder_Start()
{
    QString ls = "D:\\adretek\\AdReTek_Projects\\Portage_Qt\\Hyperfocal\\sample\\2022-11-25-09-55";       // where ls is the executable "ls"
    QString Filter = QString("JLA_RCFDI_MA1_504_%1_%2*.jpg").arg(QChar(65+CurrentRow)).arg(CurrentCol,2,10,QLatin1Char('0')) ;
    QString NameOutput = QString("JLA_RCFDI_MA1_504_%1_%2_HF.jpg").arg(QChar(65+CurrentRow)).arg(CurrentCol,2,10,QLatin1Char('0')) ;

    ui->progressbar_wavelet->setValue(CurrentRow*12 + CurrentCol);
    QDir dir(ls);
    FOCUS_INFO MethodFocus ;
    MethodFocus.m_method =  FOCUS_PROJ_WAVELET_LIB; //FOCUS_PROJ_LAP
    MethodFocus.m_NameOutput = QString("%1\\res\\%2").arg(ls).arg(NameOutput);

    mFocusProjection = new CImageFilter(MethodFocus);


    QThread* thread = new QThread( );

     // move the task object to the thread BEFORE connecting any signal/slots
     mFocusProjection->moveToThread(thread);

     connect(thread, SIGNAL(started()), mFocusProjection, SLOT(focusProjectionXY()));
     connect(mFocusProjection, SIGNAL(focusFinished()), thread, SLOT(quit()));
     connect(mFocusProjection, SIGNAL(focusFinished()), this, SLOT(WaveletFolder_Finish()));

     // automatically delete thread and task object when work is done:
  //   connect(mFocusProjection, SIGNAL(focusFinished()), mFocusProjection, SLOT(deleteLater()));
     connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));


     std::vector<cv::Mat> MatInputStack ;
     foreach( const QFileInfo& entry,
              dir.entryInfoList( QStringList() << Filter, QDir::Files | QDir::Hidden | QDir::NoSymLinks ) ) {
//               qDebug() << entry.filePath() ;
         cv::Mat img = cv::imread(entry.filePath().toStdString(), IMREAD_COLOR);
         MatInputStack.push_back(img); // conserve les images dans les plans images
     }


     mFocusProjection->setMatInputStack(MatInputStack) ;
     m_matImage = MatInputStack[0].clone();
     mFocusProjection->setMatOuput(&m_matImage) ;


      thread->start();

}

void ImageFilter_MainWindow::WaveletFolder_Finish()
{
    qDebug() << "slot Finished" ;
//    if ( mFocusProjection != nullptr)
//    {
//        QImage image = cvMatToQImage((cv::Mat)(mFocusProjection->getMatOuput()->clone()));
//        ui->labelImage->setAlignment(Qt::AlignCenter);
//        ui->labelImage->setPixmap(QPixmap::fromImage(image.scaled(QSize(ui->labelImage->width(),ui->labelImage->height()), Qt::KeepAspectRatio, Qt::FastTransformation)));
//    }
    mFocusProjection->deleteLater();

    if ( CurrentCol < 12 )
    {
        CurrentCol++;
        qDebug() << CurrentRow << " " << CurrentCol ;
        WaveletFolder_Start() ;
    }
    else
    {
        if ( CurrentRow < 7)
        {
            CurrentRow ++ ;
            CurrentCol = 1 ;
            qDebug() << CurrentRow << " " << CurrentCol ;
            WaveletFolder_Start() ;
        }
    }
}
