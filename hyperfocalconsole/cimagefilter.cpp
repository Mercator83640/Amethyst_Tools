#include "cimagefilter.h"
#include "qdebug.h"

//#include <QMessageBox>
#include <QDir>


// #include "lib_amethyst_logging.h"
// #include "logcategories.h"

//static short Resolution_ICIm;
//static short VitesseScanIm ; // 0 old version  1 new version x 3




CImageFilter::CImageFilter()
{

}

CImageFilter::CImageFilter(FOCUS_INFO pMethod)
{
    m_pMethod = pMethod ;
}

CImageFilter::~CImageFilter()
{

}

const FOCUS_INFO &CImageFilter::pMethod() const
{
    return m_pMethod;
}

void CImageFilter::setPMethod(const FOCUS_INFO &newPMethod)
{
    m_pMethod = newPMethod;
}

void CImageFilter::setMatOuput(cv::Mat *newMatOuput)
{
    MatOuput = newMatOuput;
}

void CImageFilter::setMatInputStack(std::vector<cv::Mat> newMatInputStack)
{
    MatInputStack = newMatInputStack;
}

void CImageFilter::focusProjectionXY()
{
        qDebug() << "focusProjectionXY ENTER";

        switch(m_pMethod.m_method)
        {
        case FOCUS_PROJ_MAX:          focusProjectionXY_Max(); break;
        case FOCUS_PROJ_TILE:         focusProjectionXY_Tile(); break;
        case FOCUS_PROJ_GRADIENT:     focusProjectionXY_Gradient(); break;
        case FOCUS_PROJ_MERTENS:      focusProjectionXY_Mertens(); break;
        case FOCUS_PROJ_WAVELET:      focusProjectionXY_Wavelet(); break;
        case FOCUS_PROJ_WAVELET_LIB:  focusProjectionXY_Wavelet_Lib(); break;
        default:
        case FOCUS_PROJ_LAP:          focusProjectionXY_Laplacien(); break;
        }

        qDebug() << "focusProjectionXY BEFORE emit focusFinished";
        emit focusFinished();
        qDebug() << "focusProjectionXY EXIT";
}

void CImageFilter::focusProjectionXY_Max()
{
    cv::Mat ResMat ;
    *MatOuput = MatInputStack.at(0).clone();
    for ( long i = 1 ; i < (long)MatInputStack.size(); i++)
    {
        ResMat = cv::max(*MatOuput, MatInputStack[i]);
        *MatOuput = ResMat.clone();
    }
}

void  CImageFilter::focusProjectionXY_Mertens()
{
    cv::Mat ResMat ;
    *MatOuput = MatInputStack.at(0).clone();

    std::vector<float> times;
    for ( long i = 0 ; i < (long)MatInputStack.size(); i++)
    {
        times.push_back(0.1 * (i+1));
    }

    cv::Mat response;
    cv::Ptr<cv::CalibrateDebevec> calibrate = cv::createCalibrateDebevec();
    calibrate->process(MatInputStack, response, times);
    cv::Mat hdr;
    cv::Ptr<cv::MergeDebevec> merge_debevec = cv::createMergeDebevec();
    merge_debevec->process(MatInputStack, hdr, times, response);
    cv::Mat ldr;
    cv::Ptr<cv::Tonemap> tonemap = cv::createTonemap(2.2f);
    tonemap->process(hdr, ldr);

    cv::Ptr<cv::MergeMertens> merge_mertens = cv::createMergeMertens();
    merge_mertens->process(MatInputStack, *MatOuput);

    cv::Ptr<cv::MergeRobertson> merge_robert = cv::createMergeRobertson();
    merge_robert->process(MatInputStack, ResMat, times);


    imwrite("d:\\temp\\ldr.png", ldr * 255);
    imwrite("d:\\temp\\hdr.png", hdr );
    imwrite("d:\\temp\\robert.png", ResMat );
}

void CImageFilter::focusProjectionXY_Wavelet_Lib()
{
    qDebug() << "focusProjectionXY_Wavelet_Lib ENTER";

    if (MatInputStack.empty())
    {
        qWarning() << "focusProjectionXY_Wavelet_Lib: empty stack";
        return;
    }

    if (MatOuput == nullptr)
    {
        qWarning() << "focusProjectionXY_Wavelet_Lib: MatOuput is null";
        return;
    }

  //  Lib_HyperFocalTreatment LibHF;
    Lib_HyperFocalTreatment* pLibHF = new Lib_HyperFocalTreatment();

    qDebug() << "HF output target =" << m_pMethod.m_NameOutput;

    qDebug() << "HF step 0 InitFocusStackOption";
    pLibHF->InitFocusStackOption(m_pMethod.m_NameOutput,true);

    for (size_t i = 0; i < MatInputStack.size(); ++i)
    {
        const cv::Mat& img = MatInputStack.at(i);

        qDebug() << "HF PushImageRaw" << static_cast<int>(i)
                 << "rows=" << img.rows
                 << "cols=" << img.cols
                 << "type=" << img.type();

        pLibHF->PushImageRaw(img);
    }

    qDebug() << "HF BEFORE HyperFocusStart";
    pLibHF->HyperFocusStart();
    qDebug() << "HF AFTER HyperFocusStart";

    qDebug() << "HF BEFORE HyperFocusFinalMerge";
    pLibHF->HyperFocusFinalMerge();
    qDebug() << "HF AFTER HyperFocusFinalMerge";

    qDebug() << "HF BEFORE HyperFocusWaitDone";
    pLibHF->HyperFocusWaitDone();
    qDebug() << "HF AFTER HyperFocusWaitDone";

    qDebug() << "HF BEFORE HyperFocusGetResult";
    *MatOuput = pLibHF->HyperFocusGetResult().clone();
    qDebug() << "HF AFTER HyperFocusGetResult";

    qDebug() << "focusProjectionXY_Wavelet_Lib EXIT";

    qDebug() << "HF step 3 HyperFocusWaitDone BEFORE";
    pLibHF->HyperFocusWaitDone();
    qDebug() << "HF step 3 HyperFocusWaitDone AFTER";

//    QThread::msleep(100);

    qDebug() << "focusProjectionXY_Wavelet_Lib BEFORE DELETE";
    delete pLibHF;
    qDebug() << "focusProjectionXY_Wavelet_Lib AFTER DELETE";

    qDebug() << "focusProjectionXY_Wavelet_Lib EXIT WITHOUT DELETE";
}


void CImageFilter::setHyperFocusLib(Lib_HyperFocalTreatment* pLibHF)
{
    m_pLibHF = pLibHF;
}

/*
void CImageFilter::focusProjectionXY_Wavelet_Lib()
{
    qDebug() << "focusProjectionXY_Wavelet_Lib ENTER";
    qDebug() << "stack size =" << MatInputStack.size();
    qDebug() << "output ptr =" << (MatOuput != nullptr);
    qDebug() << "output name =" << m_pMethod.m_NameOutput;

    if (MatInputStack.empty())
    {
        qWarning() << "focusProjectionXY_Wavelet_Lib: empty stack";
        return;
    }

    if (MatOuput == nullptr)
    {
        qWarning() << "focusProjectionXY_Wavelet_Lib: MatOuput is null";
        return;
    }

    for (size_t i = 0; i < MatInputStack.size(); ++i)
    {
        const cv::Mat &img = MatInputStack.at(i);
        qDebug() << "img" << static_cast<int>(i)
                 << "empty=" << img.empty()
                 << "rows=" << img.rows
                 << "cols=" << img.cols
                 << "type=" << img.type()
                 << "channels=" << img.channels()
                 << "depth=" << img.depth()
                 << "continuous=" << img.isContinuous();
    }

    Lib_HyperFocalTreatment LibHF;

    qDebug() << "HF step 1 PushImageRaw(0)";
    LibHF.PushImageRaw(MatInputStack.at(0));

    qDebug() << "HF step 2 InitFocusStackOption";
    LibHF.InitFocusStackOption(m_pMethod.m_NameOutput);

    qDebug() << "HF step 3 HyperFocusStart";
    LibHF.HyperFocusStart();

    for (size_t i = 1; i < MatInputStack.size(); i++)
    {
        qDebug() << "HF step 4 PushImageRaw(" << static_cast<int>(i) << ")";
        LibHF.PushImageRaw(MatInputStack.at(i));
    }

    qDebug() << "HF step 5 HyperFocusFinalMerge";
    LibHF.HyperFocusFinalMerge();

    qDebug() << "HF step 6 HyperFocusWaitDone BEFORE";
    LibHF.HyperFocusWaitDone();
    qDebug() << "HF step 6 HyperFocusWaitDone AFTER";

    qDebug() << "HF step 7 HyperFocusGetResult";
    *MatOuput = LibHF.HyperFocusGetResult().clone();

    qDebug() << "focusProjectionXY_Wavelet_Lib EXIT";
}
*/


void CImageFilter::focusProjectionXY_Wavelet()
{

}




void CImageFilter::StopProcessUser()
{
    // QMessageBox msg ;
    // msg.setText("valider");

    // msg.exec();
}

void CImageFilter::focusProjectionXY_Tile()
{
    CImImage ImIm ;
    float *fContrastMax;
    float fContrast;
    long i, j, k, childW, childH, tabW, tabH, *pIndImg ;
    long width, height ;
    long TileSize_W = m_pMethod.m_tileWidth;
    long TileSize_H = m_pMethod.m_tileHeight;

    width = MatInputStack[0].cols;
    tabW = width / TileSize_W;
    if(width % TileSize_W != 0)
        tabW++;

    height = MatInputStack[0].rows;
    tabH = height / TileSize_H;
    if(height % TileSize_H != 0)
        tabH++;

    qDebug() << "nombre de tuiles = " << tabW << " x " << tabH ;

    fContrastMax = new float[tabW * tabH];
    pIndImg = new long[tabW*tabH];

    for(i=0; i<tabW*tabH; i++)
    {
        fContrastMax[i] = 0.0f;
        pIndImg[i] = 0;
    }

    //Calcul du contraste local
    for(k=0; k<(long)MatInputStack.size(); k++)
    {
        for(i=0; i<height; i+=TileSize_H)
        {
            for(j=0; j<width; j+=TileSize_W)
            {
                childW = TileSize_W;
                childH = TileSize_H;

                if(i+TileSize_H >= height)
                    childH = height - 1 - i;
                if(j+TileSize_W >= width)
                    childW = width - 1 - j;

                // micro image
                cv::Mat subImage(MatInputStack[k], cv::Rect(j, i, childW, childH));
                cv::Mat subGreyImage(subImage.rows, subImage.cols, CV_8U);
                ImIm.ConversionRGBtoGrey(subImage, subGreyImage) ;
              //  imshow("grey ?", subGreyImage) ;
                // mesure entropy
                fContrast = ImIm.GetContrasteEdgeHisto(subGreyImage, cv::Mat());
            //   qCDebug(logAcq) << "contraste f = " << fContrast ;
                if(fContrast > fContrastMax[(i/childH)*tabW+(j/childW)])
                {
                    fContrastMax[(i/childH)*tabW+(j/childW)] = fContrast;
                    pIndImg[(i/childH)*tabW+(j/childW)] = k;
                }

            }
        }
    }

//    qCDebug(logAcq) << "contraste map " ;
//    for(k=0; k<(long)MatInputStack.size(); k++)
//    {
//        qCDebug(logAcq) << pIndImg[k];
//    }

    //Création de l'image finale
    cv::Mat matttp = MatOuput->clone();
    qDebug() << "matttp to " <<  matttp.rows << " x "<< matttp.cols ;
    for(k=0; k<(long)MatInputStack.size(); k++)
    {
        for(i=0; i<tabH; i++)
        {
            for(j=0; j<tabW; j++)
            {
                if(pIndImg[i*tabW+j] == k)
                {
                childW = TileSize_W;
                childH = TileSize_H;

                if(i+TileSize_H >= height) childH = height - 1 - i;
                if(j+TileSize_W >= width)  childW = width - 1 - j;

                    cv::Mat subImage(MatInputStack[k], cv::Rect(j*TileSize_W, i*TileSize_H, childW, childH));
                 //   imshow("subimage", subImage) ;
         //           qCDebug(logAcq) << "Copy to " <<  j*TileSize_W << " x "<<  i*TileSize_H << " [" << childW << "x"  << childH ;
                    subImage.copyTo(matttp(cv::Rect(j*TileSize_W, i*TileSize_H, childW, childH)));
//                    ImIm.GetNegativeImage(matttp, *MatOuput) ;
//                    *MatOuput = matttp.clone();
                 //   imshow("mainimage", *MatOuput) ;
                 //   StopProcessUser();
                }
            }
        }
    }
    *MatOuput = matttp.clone();


    delete [] fContrastMax ;
    delete[] pIndImg;
}

void CImageFilter::focusProjectionXY_Gradient()
{
    long i, j, k, n;
    double max = -1.0;

    if ( (long)MatInputStack.size() < 1)
    {
        qDebug() << "focusProjectionXY_Gradient :: Pas d'images" ;
            return  ;
    }


    if ( (long)MatInputStack.size() == 1)
    {
        qDebug() << "focusProjectionXY_Gradient :: Unique image" ;
        *MatOuput = MatInputStack[0].clone();
        return  ;
    }
    cv::Mat cartePoids = cv::Mat(MatInputStack[0].rows, MatInputStack[0].cols, CV_32FC2);
    cv::Mat carteTmpo = cartePoids.clone();
    cv::Mat carteGradient = cartePoids.clone();

    cv::Mat KernelX = GetKernelGradient(1);
    cv::Mat KernelY = GetKernelGradient(2);
    cv::Mat KernelXY = GetKernelGradient(3);
    cv::Mat KernelYX = GetKernelGradient(4);

    carteTmpo = MatInputStack[0].clone();

    max = 255.0 ;
    MatInputStack[0].convertTo(carteTmpo, CV_32FC1, 1.0 / max);

    filter2D(carteTmpo, carteGradient, carteTmpo.type() , KernelX );
    filter2D(carteTmpo, cartePoids, carteTmpo.type() , KernelY );
    cv::max(cartePoids, carteGradient, cartePoids);
    filter2D(carteTmpo, carteGradient, carteTmpo.type() , KernelXY );
    cv::max(cartePoids, carteGradient, cartePoids);
    filter2D(carteTmpo, carteGradient, carteTmpo.type() , KernelYX );
    cv::max(cartePoids, carteGradient, cartePoids);

    cv::Mat carteClasse = cv::Mat(MatInputStack[0].rows, MatInputStack[0].cols, CV_8UC1);
    carteClasse = cv::Scalar((double)0.0);

    cv::Mat cartePoidstmp = cv::Mat(MatInputStack[0].rows, MatInputStack[0].cols, CV_32FC1); // CV_32FC2
    cv::Mat carteTmpotmp = cartePoids.clone();
    cv::Mat carteGradienttmp = cartePoids.clone();
    cv::Mat carteId = cv::Mat(MatInputStack[0].rows, MatInputStack[0].cols, CV_8UC1);
    cv::Mat carteClasseTmp = cv::Mat(MatInputStack[0].rows, MatInputStack[0].cols, CV_8UC1);

 //   qCDebug(logAcq) << "Etablisseent Carte Poids ";
 //   qCDebug(logAcq) << "type carteID" << carteId.type() << " / " <<  CV_8U << " / " << CV_8UC1 ;

    for(i=1; i<(long)MatInputStack.size(); i++)
    {
        MatInputStack[i].convertTo(carteTmpotmp, CV_32FC1, 1.0 / max);

        filter2D(carteTmpotmp, carteGradienttmp, carteTmpotmp.type() , KernelX );
        filter2D(carteTmpotmp, cartePoidstmp, carteTmpotmp.type() , KernelY );
        cv::max(cartePoidstmp, carteGradienttmp, cartePoidstmp);
        filter2D(carteTmpotmp, carteGradienttmp, carteTmpotmp.type() , KernelXY );
        cv::max(cartePoidstmp, carteGradienttmp, cartePoidstmp);
        filter2D(carteTmpotmp, carteGradienttmp, carteTmpotmp.type() , KernelYX );
        cv::max(cartePoidstmp, carteGradienttmp, cartePoidstmp);

        cv::subtract(cartePoidstmp, cartePoids, carteTmpotmp);//on fait la différence : les positif = contours qui viennent d'apparaitre


//        carteId = carteTmpotmp > 0 ;
//        carteId.convertTo(carteId, CV_8U, 255) ;
        cv::Mat carteId1 = cv::Mat(MatInputStack[0].rows, MatInputStack[0].cols, CV_8UC1);
        cv::threshold(carteTmpotmp, carteId1, 0.0001, 1.0, cv::THRESH_BINARY);
//        qCDebug(logAcq) << "Type apres tthres" <<  " " << carteId1.type();
        cv::cvtColor(carteId1, carteId1, cv::COLOR_RGB2GRAY);
        carteId1.convertTo(carteId, CV_8U, 255.0) ;
//        qCDebug(logAcq) << "Type apres convert" <<  " " << carteId.type();
//        double  minVal,  maxVal;
//        minMaxLoc(carteTmpotmp,  &minVal,  &maxVal);  //find  minimum  and  maximum  intensities
//        cv::Mat carteId1 = cv::Mat(MatInputStack[0].rows, MatInputStack[0].cols, CV_8UC1);
//        carteId.convertTo(carteId1, CV_8UC1, 255.0/(maxVal  -  minVal),  -minVal) ;
//               cv::threshold(carteTTi, carteId, 0.0001, 1.0, cv::THRESH_BINARY);
//                carteId.convertTo(carteId, CV_8U, 255) ;
  //              cv::imshow("bin",carteId);
  //              StopProcessUser();
//                cv::imwrite("d:\\temp\\inittestsaveima.tif", carteTmpotmp) ;
//                cv::imwrite("d:\\temp\\testsaveima.tif", carteId) ;
 //               qCDebug(logAcq) << "Type apres tthres" << carteTmpotmp.type() << " " << carteId.type();
        carteClasseTmp = cv::Scalar((double)i);

  //      qCDebug(logAcq) << "bitwise 1 entre type " << carteClasseTmp.type() << " " << carteClasse.type() << " " << carteId.type();
//        if ( carteClasseTmp.type() != carteClasse.type() || carteClasseTmp.type() != carteId.type())
//        {
//            qCDebug(logAcq) << "carteClasseTmp Type incoherent" << carteClasseTmp.type() << " " << carteId.type();
//        }
//        if ( carteClasseTmp.rows != carteClasse.rows || carteClasseTmp.rows != carteId.rows)
//        {
//            qCDebug(logAcq) << "carteClasseTmp rows incoherent";
//        }
//        if ( carteClasseTmp.cols != carteClasse.cols || carteClasseTmp.cols != carteId.cols)
//        {
//            qCDebug(logAcq) << "carteClasseTmp cols incoherent";
//        }


//        cv::bitwise_and(carteClasseTmp, carteId , carteClasse );
//        qCDebug(logAcq) << " size " << carteId.rows << " " <<  carteClasseTmp.rows << " " <<  carteId.cols << " " <<  carteClasseTmp.cols ;
//        qCDebug(logAcq) << "type carteID" << carteId.type() << " / " <<  CV_8U << " / " << CV_8UC1 ;
        if ( carteId.rows != carteClasseTmp.rows || carteId.cols != carteClasseTmp.cols)
        {
            return ;
        }

        cv::bitwise_and(carteClasseTmp, carteClasseTmp, carteClasse, carteId  );
        cv::bitwise_and(cartePoidstmp, cartePoidstmp , cartePoids, carteId );
    }


    if(m_pMethod.m_uniform)
    {
        //unsigned char cl;
        unsigned short nbiter = 1, Uc, U0,  U90, *U, t = 0; // U45,U135,
        cv::Mat carteClasseTmpU = carteClasse.clone();

        Uc = 3;
        U90 = U0 = 4;
     //   U45 = 3 ;
     //   U135 = 3;
        long TailleStructU = (long)MatInputStack.size() ;
        U = new unsigned short[TailleStructU];

        qDebug() << "Etablisseent Carte Classe" << carteClasseTmpU.rows << " x " << carteClasseTmpU.cols <<"  Type" << carteClasseTmpU.type();
        for(n=0; n<nbiter; n++)
        {
            for(i=1; i < carteClasse.cols -1 ; i++)
            {
                for(j=1; j<carteClasse.rows-1; j++)
                {
                 //   qCDebug(logAcq) << "i = " << i << "\tj = " << j ;
                    for(k=0; k<TailleStructU; k++)
                    {
                     //   qCDebug(logAcq) << "\t\tk = " << k ;

                        U[k] = 0;
                        //Site central
                    //     qCDebug(logAcq) << carteClasseTmpU.at<short>(j,i) << " " << i << " x " << j;
                        if(carteClasseTmpU.at<short>(j,i) == (short) k)
                            U[k] = U[k]+Uc;

                        //Site X, -X, Y et -Y
                        if(carteClasseTmpU.at<short>(j,i+1) == (short) k)//à droite
                            U[k] = U[k]+U0;
                        if(carteClasseTmpU.at<short>(j,i-1) == (short) k)//à gauche
                            U[k] = U[k]+U0;
                        if(carteClasseTmpU.at<short>(j-1,i) == (short) k)//en haut
                            U[k] = U[k]+U90;
                        if(carteClasseTmpU.at<short>(j+1,i) == (short) k)//en bas
                            U[k] = U[k]+U90;
                        //Site diagonaux
                        if(carteClasseTmpU.at<short>(j-1,i+1) == (short) k)//à 45°
                            U[k] = U[k]+Uc;
                        if(carteClasseTmpU.at<short>(j+1,i-1) == (short) k)//à 225°
                            U[k] = U[k]+Uc;
                        if(carteClasseTmpU.at<short>(j-1,i-1) == (short) k)//à 135°
                            U[k] = U[k]+Uc;
                        if(carteClasseTmpU.at<short>(j+1,i+1) == (short) k)//à 315°
                            U[k] = U[k]+Uc;

                        //Selection de la classe la plus probable

                        if(k == 0)
                        {
                           // cl = 0;
                            t = U[0];
                        }
                        else if(t<U[k])
                        {
                            t = U[k];
                            //cl = (uint)k;
                        }


                    }
                    //Actualisation de la classe

                  //  carteClasseTmpU.at<uint>(j,i) = cl;
                }
            }
        }
        delete[] U;
        carteClasse = carteClasseTmpU.clone();
    }
    //Passage des classes a l'image
 //   qCDebug(logAcq) << "Finalisation Carte Classe";
    *MatOuput = MatInputStack[0].clone();

    cv::Mat carteIdFinal =  MatInputStack[0].clone();


    for(i=1; i<(long)MatInputStack.size(); i++)
    {
        //création de la foction indicatrice : "pixel à changer"
        cv::threshold(carteClasse,carteIdFinal,i,i, cv::THRESH_BINARY);
//		MatInputStack[i].copyTo(MatOuput, carteId);
//        GetCaracteristiqueImage(MatInputStack[i]);
//        GetCaracteristiqueImage(*MatOuput);
//        GetCaracteristiqueImage(carteIdFinal);
        cv::bitwise_and(MatInputStack[i], MatInputStack[i] , *MatOuput, carteIdFinal );
    }

}

void CImageFilter::GetCaracteristiqueImage(cv::Mat imMat)
{
    qDebug() << "rows : " << imMat.rows << " cols : " << imMat.cols << " Type : " << imMat.type() ;
}

void CImageFilter::focusProjectionXY_Laplacien()
{
    CContrast cCont ;
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
        qDebug() << "Pile d'image vide " ;
            return  ;
    }


    if ( (long)MatInputStack.size() == 1)
    {
        qDebug() << "Pile d'image réduite à 1 " ;
        *MatOuput = MatInputStack.at(0).clone();
            return  ;
    }

    for(i=0; i<(long)MatInputStack.size(); i++)
    {
        cCont.KernelGaussian5x5(MatInputStack.at(i), ResMat);

        tempMat = ResMat.clone();
        // resize
        cv::Mat MatResize = cv::Mat(tempMat.rows/2 , tempMat.cols/2,tempMat.type(), cv::Scalar(0,0,0,0));
        cv::resize(tempMat, ResMat, MatResize.size());
        tempMat = ResMat.clone();

        ResGaussianL0 = ResMat.clone();
        ResGaussianList.push_back(tempMat);

        cCont.KernelGaussian5x5(tempMat, ResMat);
        tempMat = ResMat.clone();
        // resize
        cv::Mat MatResize0 = cv::Mat(tempMat.rows/2 , tempMat.cols/2,tempMat.type(), cv::Scalar(0,0,0,0));
        cv::resize(tempMat, ResMat, MatResize0.size());

        ResGaussianL1 = ResMat.clone() ;

        cv::Mat MatResize1 = cv::Mat(ResGaussianL0.rows , ResGaussianL0.cols,ResGaussianL0.type(), cv::Scalar(0,0,0,0));
        cv::resize(ResGaussianL1, MatResize1, MatResize1.size());

        tempMat = ResGaussianL0 - MatResize1 ; // laplacien L1
        ResLaplacianListL1.push_back(tempMat); // on stocke les n Lapalaicans L1

        cv::Mat MatResize2 = cv::Mat(MatInputStack.at(i).rows , MatInputStack.at(i).cols,MatInputStack.at(i).type(), cv::Scalar(0,0,0,0));
        cv::resize(ResGaussianL0, MatResize2, MatResize2.size());

        tempMat = MatInputStack.at(i) - MatResize2 ; // laplacien L0
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
//	ImIm.AfficherMat(MatSum,"MatSum");

    tempMat = tempMat + MatResize ; // Level 0

    cv::Mat MatResize2 = cv::Mat(MatInputStack.at(0).rows ,
                                 MatInputStack.at(0).cols,
                                 MatInputStack.at(0).type(),
                                 cv::Scalar(0,0,0,0));
    cv::resize(tempMat, MatResize2, MatResize2.size());
    tempMat = ResLaplacianL0 + MatResize2 ;

    if (m_pMethod.m_bSharpen)
    {
        cCont.Sharpen5x5(tempMat, *MatOuput);
    }
    else
        *MatOuput = tempMat.clone();
}

cv::Mat CImageFilter::GetKernelGradient(int direction)
{
    //si direction = 1 horizontale selon (1,0)
    //si direction = 2 verticale selon (0,1)
    //si direction = 3 selon la (1,1)
    //si direction = 4 selon (1,-1)

        cv::Mat  kernelL = (cv::Mat_<float>(3,3) <<
            0,  0, 0,
            0,  0, 0,
            0,  0, 0
            );
    if(direction < 1 || direction > 4)
        return kernelL ;
    else
    {
        switch (direction )
        {
        case 1 :
            kernelL.at<float>(0,0) = -1 ;
            kernelL.at<float>(0,1) = 0 ;
            kernelL.at<float>(0,2) = 1 ;
            kernelL.at<float>(1,0) = -2 ;
            kernelL.at<float>(1,1) = 0 ;
            kernelL.at<float>(1,2) = 2 ;
            kernelL.at<float>(2,0) = -1 ;
            kernelL.at<float>(2,1) = 0 ;
            kernelL.at<float>(2,2) = 1 ;
            break;
        case 2 :
            kernelL.at<float>(0,0) = 1 ;
            kernelL.at<float>(0,1) = 2 ;
            kernelL.at<float>(0,2) = 1 ;
            kernelL.at<float>(1,0) = 0 ;
            kernelL.at<float>(1,1) = 0 ;
            kernelL.at<float>(1,2) = 0 ;
            kernelL.at<float>(2,0) = -1 ;
            kernelL.at<float>(2,1) = -2 ;
            kernelL.at<float>(2,2) = -1 ;
            break;
        case 3 :
            kernelL.at<float>(0,0) = 0 ;
            kernelL.at<float>(0,1) = 1 ;
            kernelL.at<float>(0,2) = 2 ;
            kernelL.at<float>(1,0) = -1 ;
            kernelL.at<float>(1,1) = 0 ;
            kernelL.at<float>(1,2) = 1 ;
            kernelL.at<float>(2,0) = -2 ;
            kernelL.at<float>(2,1) = -1 ;
            kernelL.at<float>(2,2) = 0 ;
            break;
        case 4 :
            kernelL.at<float>(0,0) = 2 ;
            kernelL.at<float>(0,1) = 1 ;
            kernelL.at<float>(0,2) = 0 ;
            kernelL.at<float>(1,0) = 1 ;
            kernelL.at<float>(1,1) = 0 ;
            kernelL.at<float>(1,2) = -1 ;
            kernelL.at<float>(2,0) = 0 ;
            kernelL.at<float>(2,1) = -1 ;
            kernelL.at<float>(2,2) = -2 ;
            break;
        }
    }
    return kernelL ;
}

cv::Mat *CImageFilter::getMatOuput() const
{
    return MatOuput;
}



CContrast::CContrast(void)
{
}

CContrast::~CContrast(void)
{
}

bool CContrast::KernelGaussian5x5(cv::Mat MatInput, cv::Mat &MatOut)
{
    cv::Point anchor;
  //  double delta;
    //int ddepth;
 //   int kernel_size;

    anchor = cv::Point( -1, -1 );
   // delta = 0;
    //ddepth = CV_32F ;
  //  kernel_size = 5;
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



bool CContrast::Sharpen5x5(cv::Mat MatInput, cv::Mat &MatOut)
{
//	if (MatInput.type() == CV_8UC3) return false;

    cv::Point anchor;
//    double delta;
//    int ddepth;
//    int kernel_size;

    anchor = cv::Point( -1, -1 );
//    delta = 0;
//    ddepth = CV_32F ;
//    kernel_size = 5;

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


bool CContrast::Sharpen5x5b(cv::Mat MatInput, cv::Mat &MatOut)
{
    if (MatInput.type() == CV_8UC3) return false;

    cv::Point anchor;
 //   double delta;
//    int ddepth;
 //   int kernel_size;

    anchor = cv::Point( -1, -1 );
 //   delta = 0;
 //   ddepth = CV_32F ;
 //   kernel_size = 5;
    float Ratio = 8.0f ;

    cv::Mat  kernelL = (cv::Mat_<float>(5,5) <<
            -1.0/Ratio,  -1.0/Ratio, -1.0/Ratio, -1.0/Ratio, -1.0/Ratio,
            -1.0/Ratio, 2.0/Ratio, 2.0/Ratio, 2.0/Ratio, -1.0/Ratio,
            -1.0/Ratio, 2.0/Ratio, 8.0/Ratio, 2.0/Ratio, -1.0/Ratio,
            -1.0/Ratio, 2.0/Ratio, 2.0/Ratio, 2.0/Ratio, -1.0/Ratio,
            -1.0/Ratio, -1.0/Ratio, -1.0/Ratio, -1.0/Ratio, -1.0/Ratio
            );


    filter2D(MatInput, MatOut, MatInput.type() , kernelL );

    kernelL.release();

    return true ;
}

bool CContrast::UnsharpenMask(cv::Mat MatInput, cv::Mat &MatOut, float sigma, float FFact)
{
    if (MatInput.type() == CV_8UC3) return false;

    cv::Mat blurred ;
    cv::GaussianBlur(MatInput, blurred, cv::Size(0, 0),sigma);
    blurred *= FFact;
    MatOut =  ((1 + FFact) * MatInput) - blurred ;

    blurred.release();

    return true ;
}

bool CContrast::UnsharpenMaskEx(cv::Mat MatInput, cv::Mat &MatOut, float sigma, float FFact)
{
    if (MatInput.type() == CV_8UC3) return false;

    cv::Mat blurred = MatInput.clone() ;
    cv::Mat MatTmp ;
    cv::GaussianBlur(MatInput, blurred, cv::Size(),sigma);


    blurred *= FFact;
    cv::Mat image32F = cv::Mat(MatInput.rows, MatInput.cols, CV_32F);
    MatTmp = ( MatInput - blurred) ;
    if ( MatTmp.type() == CV_8U )
        MatTmp.convertTo(image32F , CV_32F, 1.0/255.);
    else
        MatTmp.convertTo(image32F , CV_32F, 1.0/65535.0);

    image32F =  image32F /(1.0 - FFact);

    if ( MatTmp.type() == CV_8U )
        image32F.convertTo(MatOut , CV_8U,255.0);
    else
        image32F.convertTo(MatOut , CV_16U,65535.0);

    blurred.release();

    return true ;
}

void CContrast::MelangeImagesMasque( cv::Mat Mat1, cv::Mat Mat2, cv::Mat &MatOut , cv::Mat Mask)
{
    CImImage cIm ;
    //
    cv::Mat MatMask3 = Mask.clone() ;
    cv::Mat MatMask4 ;

    MatMask4 = 255 - MatMask3 ;

    // passage en 32F
    cv::Mat FMask3 = cv::Mat::zeros(MatMask3.size(), CV_32F);
    MatMask3.convertTo(FMask3, CV_32F, 1.0/255.0 );
    cv::Mat FMask4 = cv::Mat::zeros(MatMask4.size(), CV_32F);
    MatMask4.convertTo(FMask4, CV_32F, 1.0/255.0 );

    //cIm.AfficherMat(Mat2,"MatTmp2 av");
    //cIm.AfficherMat(Mat1,"MatTmp1----");

    cv::Mat FMat2 = cv::Mat::zeros(Mat2.size(), CV_32F);
    cv::Mat FMat1 = cv::Mat::zeros(Mat1.size(), CV_32F);
    Mat1.convertTo(FMat1, CV_32F, 1.0/65535.0 );
    Mat2.convertTo(FMat2, CV_32F, 1.0/65535.0 );

    cv::multiply(FMat1, FMask3,FMat1 );
    cv::multiply(FMat2, FMask4, FMat2);

    FMat1.convertTo(Mat1, CV_16UC1, 65535.0 );
    FMat2.convertTo(Mat2, CV_16UC1, 65535.0 );

    //cIm.AfficherMat(Mat2,"MatTmp2 av MASQUE");
    //cIm.AfficherMat(Mat1,"MatTmp1----MASQUE");

    MatOut = Mat2 + Mat1 ;
}

void CContrast::MelangeImagesMasqueMax( cv::Mat Mat1, cv::Mat Mat2, cv::Mat &MatOut , cv::Mat Mask)
{
    MelangeImagesMasque( Mat1, Mat2, MatOut , Mask) ;
    for ( int i = 0 ; i < MatOut.rows ; i++)
    {
        for ( int j = 0 ; j < MatOut.cols ; j++)
        {
            if ( Mask.at<uint8_t>(i,j) == 255 )
            {
                MatOut.at<uint16_t>(i,j) = std::max(Mat1.at<uint16_t>(i,j), Mat2.at<uint16_t>(i,j));
            }
        }
    }

    //cv::Mat Matdiff ;
    //Matdiff = MatOut - Mat2 ;
    //CImImage cIm ;
    //cIm.AfficherMat(Matdiff,"Diff");
}


bool CContrast::AddRandomNoise(cv::Mat &MatInput)
{
    cv::Mat saltpepper_noise = cv::Mat::zeros(MatInput.rows, MatInput.cols,CV_16U);
    cv::randu(saltpepper_noise,0,2700);

    //CImImage cIm ;
    //cIm.AfficherMat(saltpepper_noise,"salt");
    MatInput += saltpepper_noise ;

    return true ;
}

bool CContrast::Autohisto4(cv::Mat MatInput, cv::Mat &MatOut, long Min, long Max )
{
    if (MatInput.type() == CV_8UC3) return false;
    if (MatInput.type() == CV_8UC1) return false;


    if ( MatInput.type() == CV_16U )
    {
        for(int i=0;i<MatInput.rows;i++)
        {
            for(int j=0;j<MatInput.cols;j++)
            {
                uint16_t Val = MatInput.at<uint16_t>(i,j) ;
                float Val8 = (float) 65535 * (float)(Val - Min) / (float)(Max - Min) ;
                if ( Val8 < (float)0) Val8 = (float)0 ;
                if ( Val8 > (float)65535) Val8 = (float)65535 ;
                MatOut.at<uint16_t>(i,j)= (uint16_t)Val8 ;
            }
        }
    }
    return true;
}

bool CContrast::Autohisto2(cv::Mat MatInput, cv::Mat &MatOut, long Min, long Max, float PourcentageHaut )
{
    if (MatInput.type() == CV_8UC3) return false;

   int histSize = 256;

    if ( MatInput.depth() == CV_8U) histSize = (int)pow(2.0, 8.0)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16.0) ;

        // trouver le min et le max
    float range[] = { 0, (float)histSize } ;
    const float* histRange = { range };

    bool uniform = true;
    bool accumulate = false;

    cv::Mat nb_hist;

     /// Compute the histograms:
    calcHist( &MatInput, 1, 0, cv::Mat(), nb_hist, 1, &histSize, &histRange, uniform, accumulate );
    //
    float *dest = new float[histSize];
    float MaxHigh = 0.0 ;
    double sum = 0.000 ;
    for(int i = 0; i < histSize; i++)
    {
        dest[i] = (float)(nb_hist.at<float>(i) ) ;
        if ( dest[i] > MaxHigh) MaxHigh = dest[i] ;
    }

    long lNGMin = 0 ;
    long lNGMax = 0 ;
    for (int i = 0; i < histSize; i++)
    {
        if (dest[i]>0)
        {
            lNGMin=i;
            break;
        }
    }
    if (lNGMin>0) lNGMin--; // line to the (p==0) point, not to data[min]

    for (int i = histSize-1; i >0 ; i-- )
    {
        sum += dest[i] ;
        if (sum > MatInput.rows * MatInput.cols * PourcentageHaut)
        {
            lNGMax=i;
            break;
        }
    }
    if (lNGMax<65535) lNGMax++; // line to the (p==0) point, not to data[min]

    if ( MatInput.type() == CV_16U )
    {
        for(int i=0;i<MatInput.rows;i++)
        {
            for(int j=0;j<MatInput.cols;j++)
            {
                uint16_t Val = MatInput.at<uint16_t>(i,j) ;
                float Val8 = (float) Max * (float)(Val - lNGMin) / (float)(lNGMax - lNGMin) ;
                if ( Val8 < (float)Min) Val8 = (float)Min ;
                if ( Val8 > (float)Max) Val8 = (float)Max ;
                MatOut.at<uint16_t>(i,j)= (uint16_t)Val8 ;
            }
        }
    }

    nb_hist.release();

    delete []dest ;

    return true;
}


bool CContrast::AdjustContrast(cv::Mat MatInput, cv::Mat &MatOut , short Gain, double Cutoff)
{
    if (MatInput.type() == CV_8UC3) return false;
    cv::Mat MatMask = MatInput.clone();
    CImImage ImImage;

    ImageStatistique StatStart ;
    ImImage.CalcHistoMeanMedian(MatInput, &StatStart, cv::Mat());

    MatMask = MatMask - StatStart.MinWithoutZero ;

    cv::Mat image32F = cv::Mat(MatInput.rows, MatInput.cols, CV_32F);
    MatMask.convertTo(image32F , CV_32F, 1.0/StatStart.MaxWithoutMaxRange);

//	MatMask = MatMask / StatStart.MaxWithoutMaxRange ;
    cv::exp(Gain * ( Cutoff - image32F), image32F);
    image32F = 1.0 / ( 1.0 + image32F) ;

    image32F.convertTo(MatOut , CV_16U,65535.0);

    MatMask.release();
    return true;
}

bool CContrast::AdjustContrast(cv::Mat MatInput, cv::Mat &MatOut , double Gain, double Cutoff)
{
    if (MatInput.type() == CV_8UC3) return false;
    cv::Mat MatMask = MatInput.clone();
    CImImage ImImage;

    short Type = CV_16UC1 ;
    if ( MatInput.type() == CV_8UC1 ) Type = CV_8UC1 ;

    ImageStatistique StatStart ;

    //char file[255];
    //sprintf_s(file, "calcul stat ");
    //::MessageBoxA(NULL, file, "AdjustContrast", MB_OK | MB_ICONWARNING);

    ImImage.CalcHistoMeanMedian(MatInput, &StatStart, cv::Mat());
    //sprintf_s(file, "calcul stat  StatStart.MinWithoutZero = %ld StatStart.MaxWithoutMaxRange = %ld",  StatStart.MinWithoutZero, StatStart.MaxWithoutMaxRange);
    //::MessageBoxA(NULL, file, "AdjustContrast", MB_OK | MB_ICONWARNING);



    MatMask = MatMask - StatStart.MinWithoutZero ;
//	ImImage.AfficherMat(MatMask,"MatMask Adjust cont");

    cv::Mat image32F = cv::Mat(MatInput.rows, MatInput.cols, CV_32F);
    MatMask.convertTo(image32F , CV_32F, 1.0/StatStart.MaxWithoutMaxRange);
//	ImImage.AfficherMat(MatMask,"MatMask 32 bis");

//	MatMask = MatMask / StatStart.MaxWithoutMaxRange ;
    cv::exp(Gain * ( Cutoff - image32F), image32F);
//	ImImage.AfficherMat(MatMask,"MatMask exponentiel");
    image32F = 1.0 / ( 1.0 + image32F) ;
//	ImImage.AfficherMat(MatMask,"MatMask inverse");

    if ( Type == CV_16UC1 )
    {
        image32F.convertTo(MatOut , CV_16U,65535.0);
    }
    else
    {
        image32F.convertTo(MatOut , CV_8U,255.0);
    }
//	ImImage.AfficherMat(MatOut,"MatOut sortie");

    MatMask.release();
    return true;
}


bool CContrast::AdjustContrastEx(cv::Mat MatInput, cv::Mat &MatOut , double Gain, double CutoffPlus)
{
    if (MatInput.type() == CV_8UC3) return false;
    cv::Mat MatMask = MatInput.clone();
    CImImage ImImage;

    short Type = CV_16UC1 ;
    if ( MatInput.type() == CV_8UC1 ) Type = CV_8UC1 ;

    ImageStatistique StatStart ;

    //char file[255];
    //sprintf_s(file, "calcul stat ");
    //::MessageBoxA(NULL, file, "AdjustContrast", MB_OK | MB_ICONWARNING);

    ImImage.CalcHistoMeanMedian(MatInput, &StatStart, cv::Mat());
    //sprintf_s(file, "calcul stat  StatStart.MinWithoutZero = %ld StatStart.MaxWithoutMaxRange = %ld",  StatStart.MinWithoutZero, StatStart.MaxWithoutMaxRange);
    //::MessageBoxA(NULL, file, "AdjustContrast", MB_OK | MB_ICONWARNING);
    float CuttoffMean = (float)StatStart.Mean / (float)( StatStart.MaxWithoutMaxRange - StatStart.MinWithoutZero);
    float Cutoff = CuttoffMean + (float)CutoffPlus;

    MatMask = MatMask - StatStart.MinWithoutZero ;
//	ImImage.AfficherMat(MatMask,"MatMask Adjust cont");

    cv::Mat image32F = cv::Mat(MatInput.rows, MatInput.cols, CV_32F);
    MatMask.convertTo(image32F , CV_32F, 1.0/StatStart.MaxWithoutMaxRange);
//	ImImage.AfficherMat(MatMask,"MatMask 32 bis");

//	MatMask = MatMask / StatStart.MaxWithoutMaxRange ;
    cv::exp(Gain * ( Cutoff - image32F), image32F);
//	ImImage.AfficherMat(MatMask,"MatMask exponentiel");
    image32F = 1.0 / ( 1.0 + image32F) ;
//	ImImage.AfficherMat(MatMask,"MatMask inverse");

    if ( Type == CV_16UC1 )
    {
        image32F.convertTo(MatOut , CV_16U,65535.0);
    }
    else
    {
        image32F.convertTo(MatOut , CV_8U,255.0);
    }
//	ImImage.AfficherMat(MatOut,"MatOut sortie");

    MatMask.release();
    return true;
}



bool CContrast::Autohisto1_Plaque(cv::Mat MatInput, cv::Mat &MatOut, float PourcentageHaut)
{
    if (MatInput.type() == CV_8UC3) return false;
    if (MatInput.type() == CV_8U) return false;

   int histSize = 256;

    if ( MatInput.depth() == CV_8U) histSize = (int)pow(2.0, 8.0)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16.0) ;

        // trouver le min et le max
    float range[] = { 0, (float)histSize } ;
    const float* histRange = { range };

    bool uniform = true;
    bool accumulate = false;

    cv::Mat nb_hist;

     /// Compute the histograms:
    calcHist( &MatInput, 1, 0, cv::Mat(), nb_hist, 1, &histSize, &histRange, uniform, accumulate );
    //
    float *dest = new float[histSize];
    float MaxHigh = 0.0 ;
    double sum = 0.000 ;
    for(int i = 0; i < histSize; i++)
    {
        dest[i] = (float)(nb_hist.at<float>(i) ) ;
        if ( dest[i] > MaxHigh) MaxHigh = dest[i] ;
    }

    long lNGMin = 0 ;
    long lNGMax = 0 ;
    for (int i = 0; i < histSize; i++)
    {
        if (dest[i]>0)
        {
            lNGMin=i;
            break;
        }
    }
    if (lNGMin>0) lNGMin--; // line to the (p==0) point, not to data[min]

    for (int i = histSize-1; i >0 ; i-- )
    {
        sum += dest[i] ;
        if (sum > MatInput.rows * MatInput.cols * PourcentageHaut)
        {
            lNGMax=i;
            break;
        }
    }
    if (lNGMax<65535) lNGMax++; // line to the (p==0) point, not to data[min]

    if ( MatInput.type() == CV_16U )
    {
        for(int i=0;i<MatInput.rows;i++)
        {
            for(int j=0;j<MatInput.cols;j++)
            {
                uint16_t Val = MatInput.at<uint16_t>(i,j) ;
                float Val8 = (float) 65535.0 * (float)(Val - lNGMin) / (float)(lNGMax - lNGMin) ;
                if ( Val8 < 0) Val8 = 0 ;
                if ( Val8 >65535) Val8 = 65535 ;
                MatOut.at<uint16_t>(i,j)= (uint16_t)Val8 ;
            }
        }
    }

    nb_hist.release();
    delete []dest ;

  return true ;
}


bool CContrast::Autohisto3_Plaque(cv::Mat MatInput, cv::Mat &MatOut, float PourcentageHaut , float CoeffMax )
{
    if (MatInput.type() == CV_8UC3) return false;

   int histSize = 256;

    if ( MatInput.depth() == CV_8U) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

        // trouver le min et le max
    float range[] = { 0, (float)histSize } ;
    const float* histRange = { range };

    bool uniform = true;
    bool accumulate = false;

    cv::Mat nb_hist;

     /// Compute the histograms:
    calcHist( &MatInput, 1, 0, cv::Mat(), nb_hist, 1, &histSize, &histRange, uniform, accumulate );
    //
    float *dest = new float[histSize];
    float MaxHigh = 0.0 ;
    double sum = 0.000 ;
    for(int i = 0; i < histSize; i++)
    {
        dest[i] = (float)(nb_hist.at<float>(i) ) ;
        if ( dest[i] > MaxHigh) MaxHigh = dest[i] ;
    }

    long lNGMin = 0 ;
    long lNGMax = 0 ;
    sum = 0.000 ;
    for (int i = 0; i < histSize; i++)
    {
        sum += dest[i] ;
        if (sum > PourcentageHaut * MatInput.rows * MatInput.cols)
        {
            lNGMin=i;
            break;
        }
    }
    if (lNGMin>0) lNGMin--; // line to the (p==0) point, not to data[min]

    if ( lNGMin > (int)((float)histSize * 0.88)) lNGMin = (long)(lNGMin / 1.05);
    if ( lNGMin > (int)((float)histSize * 0.86)) lNGMin = (long)(lNGMin / 1.10);

    sum = 0.000 ;
    for (int i = (int)((float)histSize * 0.98); i >0 ; i-- )
    {
        sum += dest[i] ;
        if (sum > MatInput.rows * MatInput.cols * PourcentageHaut)
        {
            lNGMax=i;
            break;
        }
    }
    if (lNGMax < histSize -1 ) lNGMax++; // line to the (p==0) point, not to data[min]

    lNGMin =(long)(lNGMin * 0.9) ;

    long ValMax = (long)(1.04 * histSize * 0.99) ;
    if ( ValMax > histSize -1) ValMax = histSize -1 ;
    lNGMax = (long)(1.1 * lNGMax) ;
    if ( lNGMax > histSize -1) lNGMax = histSize -1 ;

    ValMax = (long)((1.0 + CoeffMax) * lNGMax) ;
    if ( ValMax > (int)((float)histSize * 0.84)) ValMax = (long)(1.15 * lNGMax) ;
    if ( ValMax > (int)((float)histSize * 0.92)) ValMax = (long)(1.2 * lNGMax) ;
    ValMax = (long)((1.0 + CoeffMax/2) * lNGMax) ;

    if ( MatInput.type() == CV_16U )
    {
        for(int i=0;i<MatInput.rows;i++)
        {
            for(int j=0;j<MatInput.cols;j++)
            {
                uint16_t Val = MatInput.at<uint16_t>(i,j) ;
                float Val8 = (float) ValMax * (float)(Val - lNGMin) / (float)(lNGMax - lNGMin) ;
                if ( Val8 < 0) Val8 = 0 ;
                if ( Val8 >(float)(histSize -1)) Val8 = (float)(histSize -1 );
                MatOut.at<uint16_t>(i,j)= (uint16_t)Val8 ;
            }
        }
    }

    nb_hist.release();
    delete []dest ;

    return true ;
}

bool CContrast::Normalize(cv::Mat MatInput, cv::Mat &MatOut , cv::Mat Mask, short Method , double /*Facteur*/ )
{
    cv::normalize(MatInput, MatOut, 0, 65535, Method, -1, Mask);

    return true ;
}


long CContrast::Unsharpen(cv::Mat MatInput, cv::Mat &MatOut, long /*ValMask*/, float CoeffGaussien, float /*CoeffMultG*/, float /*CoeffMultFinal*/, float /*Decalage*/)
{
    cv::Mat Mask ;

    ImageStatistique Stat_In ;
    CImImage ImImage;

    double dMedian ;

    cv::Mat image;
    cv::Mat image32I0 ;
    cv::Mat image32IG ;
    cv::Mat image32F = cv::Mat(MatInput.rows, MatInput.cols, CV_32F);
    cv::Mat image32Fb = cv::Mat(MatInput.rows, MatInput.cols, CV_32F);
    cv::Mat MatContrastIn ;
    cv::Mat MatContrastOut ;

    ImImage.CalcHistoMeanMedian(MatInput, &Stat_In, cv::Mat());

    MatInput.convertTo(image32I0 , CV_32F, 1.0/65535.0);
    MatInput.convertTo(image32IG , CV_32F, 1.0/65535.0);

    float ValMed = (float)(Stat_In.Median ) ;
    dMedian = 1 -  ValMed / 65535.0 ;

    image32I0 -= dMedian;
    image32IG -= dMedian;

    CoeffGaussien = 2.5f; // 2
    cv::GaussianBlur(image32IG, image32IG, cv::Size(0, 0),CoeffGaussien);

    cv::Mat blurred ;
    float sigma = 10;
    cv::GaussianBlur(image32IG, blurred, cv::Size(0, 0),sigma);
    float Facteur = 0.5 ;
    blurred *= Facteur;
    image32IG = ( 1.0 + Facteur) * image32IG - blurred ;

    cv::Mat blurredInput ;
    sigma = 10.0f;
    Facteur = 0.4f ;
    cv::GaussianBlur(image32I0, blurredInput, cv::Size(0, 0),sigma);
    blurredInput *= Facteur;
    image32I0 = ( 1.0 + Facteur) * image32I0 - blurredInput ;

    cv::multiply(image32I0, image32IG, image32F);

    image32F += dMedian;

    cv::Mat MatTpm ;

    image32F.convertTo(MatTpm , CV_16U,(Stat_In.MaxWithoutMaxRange - Stat_In.MinWithoutZero ), Stat_In.MinWithoutZero);
//	image32F.convertTo(MatTpm , CV_16U,Stat_In.MaxWithoutMaxRange, Stat_In.MinWithoutZero);
    normalize(MatTpm, MatTpm, 0, 65535, cv::NORM_MINMAX);

    MatOut = MatTpm   ;

    image.release();
    image32I0.release() ;
    image32IG.release() ;
    image32F.release() ;
    image32Fb.release() ;
    MatContrastIn.release() ;
    MatContrastOut.release() ;
    MatTpm.release();
    blurredInput.release();

    return Stat_In.Median;
}

long CContrast::Unsharpen_1(cv::Mat MatInput, cv::Mat &MatOut)
{
    cv::Mat Mask ;

    ImageStatistique Stat_In ;
    CImImage ImImage;

//	double dMedian ;

    cv::Mat image;
    cv::Mat image32I0 ;
    cv::Mat image32IG ;
    cv::Mat image32F = cv::Mat(MatInput.rows, MatInput.cols, CV_32F);
    cv::Mat image32Fb = cv::Mat(MatInput.rows, MatInput.cols, CV_32F);
    cv::Mat MatContrastIn ;
    cv::Mat MatContrastOut ;

    ImImage.CalcHistoMeanMedian(MatInput, &Stat_In, cv::Mat());

    // remove outler

    cv::Mat MatTH ;

    cv::Mat element1 = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size( 5, 5), cv::Point( 0, 0 ) );

    /// Apply the specified morphology operation
    morphologyEx(MatInput, MatTH, cv::MORPH_CLOSE, element1, cv::Point(-1,-1), 1);
    morphologyEx(MatTH, MatTH, cv::MORPH_OPEN, element1, cv::Point(-1,-1), 1);

//	SIFilter_AfficherMat(MatInput, "MatInput");

    MatInput = MatTH.clone()   ;

    MatOut = MatTH   ;
//	SIFilter_AfficherMat(*MatOut, "fin");

    image.release();
    image32I0.release() ;
    image32IG.release() ;
    image32F.release() ;
    image32Fb.release() ;
    MatContrastIn.release() ;
    MatContrastOut.release() ;

    return Stat_In.Median;
}


bool  CContrast::RehaussementContour(cv::Mat MatInput, cv::Mat &MatOut, float SigmaGB, short ShiftPixel, float FactMul, short /*Etape*/)
{
    cv::Mat Blurred ;
    cv::Mat PlanDec1 ;
    cv::Mat PlanDec2 ;
    cv::Mat PlanDec3 ;
    cv::Mat PlanDec4 ;
    cv::Mat kernelN;
    cv::Mat kernelS;
    cv::Mat kernelE;
    cv::Mat kernelO;
    CImImage ImImage;

    cv::Point anchor;
    double delta;
    int ddepth;
    int kernel_size;

    anchor = cv::Point( -1, -1 );
    delta = 0;
    ddepth = -1 ;

    //char Buf[50] ;
//    uint32_t Start = GetTickCount();
//    uint32_t Delta ;

    cv::GaussianBlur(MatInput, Blurred, cv::Size(), SigmaGB, SigmaGB);
    kernel_size = 3 + 2*( (ShiftPixel - 1)%5 );

    kernelN = cv::Mat::zeros( kernel_size, kernel_size, CV_16U );
    kernelS = cv::Mat::zeros( kernel_size, kernel_size, CV_16U );
    kernelE = cv::Mat::zeros( kernel_size, kernel_size, CV_16U );
    kernelO = cv::Mat::zeros( kernel_size, kernel_size, CV_16U );

    kernelN.at<uint16_t>(0,ShiftPixel) = 1 ;
    kernelS.at<uint16_t>(2 + 2 * (ShiftPixel - 1),ShiftPixel) = 1 ;
    kernelE.at<uint16_t>(ShiftPixel,2 + 2 * (ShiftPixel - 1)) = 1 ;
    kernelO.at<uint16_t>(ShiftPixel,0) = 1 ;

//    Start = GetTickCount();
    filter2D(Blurred, PlanDec1, ddepth , kernelN, anchor, delta, cv::BORDER_DEFAULT );
    filter2D(Blurred, PlanDec2, ddepth , kernelS, anchor, delta, cv::BORDER_DEFAULT );
    filter2D(Blurred, PlanDec3, ddepth , kernelE, anchor, delta, cv::BORDER_DEFAULT );
    filter2D(Blurred, PlanDec4, ddepth , kernelO, anchor, delta, cv::BORDER_DEFAULT );
//    Delta = GetTickCount() - Start;
//	sprintf_s(Buf, _countof(Buf), _T("%d"), Delta);
//	::MessageBoxA(NULL, Buf, "rehaussement filter 2D (4x)", MB_OK | MB_ICONWARNING);

//    Start = GetTickCount();
    cv::subtract(Blurred, PlanDec1, PlanDec1);
    cv::subtract(Blurred, PlanDec2, PlanDec2);
    cv::subtract(Blurred, PlanDec3, PlanDec3);
    cv::subtract(Blurred, PlanDec4, PlanDec4);
//    Delta = GetTickCount() - Start;
//	sprintf_s(Buf, _countof(Buf), _T("%d"), Delta);
//	::MessageBoxA(NULL, Buf, "substract filter 2D (4x)", MB_OK | MB_ICONWARNING);

//    Start = GetTickCount();
    cv::add(PlanDec1, PlanDec2, PlanDec1);
    cv::add(PlanDec3, PlanDec4, PlanDec3);
    cv::add(PlanDec1, PlanDec3, PlanDec1);
//    Delta = GetTickCount() - Start;
//	sprintf_s(Buf, _countof(Buf), _T("%d"), Delta);
//	::MessageBoxA(NULL, Buf, "add filter 2D (3x)", MB_OK | MB_ICONWARNING);

//    Start = GetTickCount();
    cv::medianBlur(PlanDec1, PlanDec1, 5);
//    Delta = GetTickCount() - Start;
//	sprintf_s(Buf, _countof(Buf), _T("%d"), Delta);
//	::MessageBoxA(NULL, Buf, "median", MB_OK | MB_ICONWARNING);

//    Start = GetTickCount();
    cv::GaussianBlur(PlanDec1, PlanDec1, cv::Size(), 8);
//    Delta = GetTickCount() - Start;
//	sprintf_s(Buf, _countof(Buf), _T("%d"), Delta);
//	::MessageBoxA(NULL, Buf, "gauss", MB_OK | MB_ICONWARNING);
        //if ( Etape == 0) ImImage.ExporterMat(PlanDec1, "c:\\temp\\Dec1_0.tif");
        //if ( Etape == 1) ImImage.ExporterMat(PlanDec1, "c:\\temp\\Dec1_1.tif");

//    Start = GetTickCount();
    if ( FactMul < 2)
    {
        PlanDec1 *= FactMul ;
        MatOut = MatInput + PlanDec1 ;
    }
    else
    {
        PlanDec1 *= 2 ;
        short nbFois = (short) (FactMul / 2);
        MatOut = MatInput.clone();
        for ( short i = 0 ; i < nbFois ; i++) MatOut +=  PlanDec1 ;
    }
//    Delta = GetTickCount() - Start;
//	sprintf_s(Buf, _countof(Buf), _T("%d"), Delta);
//	::MessageBoxA(NULL, Buf, "final", MB_OK | MB_ICONWARNING);

    Blurred.release();
    PlanDec1.release();
    PlanDec2.release();
    PlanDec3.release();
    PlanDec4.release();
    kernelN.release();
    kernelS.release();
    kernelE.release();
    kernelO.release();

    return true ;
}

bool CContrast::LutLineaire(cv::Mat MatInput, cv::Mat &MatOut, long /*Min*/, long /*Max*/, bool /*bAuto*/, long MinOut, long MaxOut)
{
    CV_Assert(  MatInput.type() == CV_16U || MatInput.type() == CV_8UC1);

    if ( MatInput.type() == CV_8UC1 )
    {
//		cv::equalizeHist(MatInput, MatOut);
        cv::normalize(MatInput, MatOut, MinOut,MaxOut, cv::NORM_MINMAX);
        return true;
    }

/*
    if ( MatInput.type() == CV_16U )
    {
        CImImage ImImage;
        cv::Size tileSize;

        int tilesX_ = 1;
        int tilesY_ = 1;
        cv::Mat srcForLut;
        cv::Mat srcExt_;

        ImageStatistique Stat_In ;
        ImImage.CalcHistoMeanMedian(MatInput, &Stat_In, cv::Mat());

        try
        {
            if (MatInput.cols % tilesX_ == 0 && MatInput.rows % tilesY_ == 0)
            {
                tileSize = cv::Size(MatInput.cols / tilesX_, MatInput.rows / tilesY_);
                srcForLut = MatInput ;
            }
            else
            {
                cv::copyMakeBorder(MatInput, srcExt_, 0, tilesY_ - (MatInput.rows % tilesY_), 0, tilesX_ - (MatInput.cols % tilesX_), cv::BORDER_REFLECT_101);

                tileSize = cv::Size(srcExt_.cols / tilesX_, srcExt_.rows / tilesY_);
                srcForLut = srcExt_ ;
            }

            if ( bAuto )
            {
                Min = Stat_In.MinWithoutZero ;
                Max = Stat_In.MaxWithoutMaxRange ;
            }
            CLutLineaire_CalcSI calcLin16(srcForLut, MatOut , Min, Max, tileSize, tilesX_, tilesY_, MinOut, MaxOut);
            cv::parallel_for_(cv::Range(0, tilesX_ * tilesY_), calcLin16);
        }
        catch (std::exception& e)
        {
            ::MessageBoxA(NULL, e.what(), __FUNCTION__, MB_OK | MB_ICONWARNING);
            return false;
        }
        srcForLut.release();
        srcExt_.release();

    }
*/
    return true ;
}

bool CContrast::UnderWaterRehaussement(cv::Mat MatInput, cv::Mat &MatOut)
{
    CV_Assert(  MatInput.type() == CV_8UC3 );

    cv::Mat hls;
    std::vector<cv::Mat> planHLS ;
    std::vector<cv::Mat> planRGB ;
    CImImage ImImage;

    // Transformation HLS
    cv::cvtColor(MatInput, hls, cv::COLOR_BGR2HLS);
    cv::split(hls, planHLS);

    ImImage.AfficherMat(planHLS[0], "Hue");
    ImImage.AfficherMat(planHLS[2], "Satur");
    ImImage.AfficherMat(planHLS[1], "Lum");

    cv::split(MatInput, planRGB);



    // Rehaussement Stretching L et S

    //long Min = 0 ;
    //long Max = 256 ;
    for ( short i = 0 ; i < 3 ; i++)
    {
        cv::Mat MatTmp ;
        planRGB[i].convertTo(MatTmp, CV_8UC1);
        long Max = ImImage.GetMaxHisto(MatTmp, 1, cv::Mat(), false);
        long Min = ImImage.GetMinHisto(MatTmp, 1, cv::Mat(), false);

        LutLineaire(MatTmp, MatTmp, Min, Max, false,0, 256);
        planRGB[i] = MatTmp.clone();
    }
    ImImage.AfficherMat(planRGB[0], "Bleu");
    ImImage.AfficherMat(planRGB[2], "Rouge");
    ImImage.AfficherMat(planRGB[1], "Vert");

/*

*/






    // retour RGB
    cv::merge(planRGB, MatOut);


    return true ;
}

bool  CContrast::BBHE(cv::Mat MatInput, cv::Mat &MatOut)
{
    if (MatInput.type() == CV_8UC3) return false ;

   int histSize = 256;

    if ( MatInput.depth() == CV_8U) histSize = (int)pow(2.0, 8.0)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16.0) ;

        // trouver le min et le max
    float range[] = { 0, (float)histSize } ;
    const float* histRange = { range };

    bool uniform = true;
    bool accumulate = false;

    cv::Mat nb_hist;

     /// Compute the histograms:
    calcHist( &MatInput, 1, 0, cv::Mat(), nb_hist, 1, &histSize, &histRange, uniform, accumulate );
    //
    float *destHisto = new float[histSize];
    double *lfCdf = new double[histSize];
    for(int i = 0; i < histSize; i++)
    {
        destHisto[i] = (float)(nb_hist.at<float>(i) ) ;
        lfCdf[i] = 0.0;
    }


    m_BBHE_nLowerTotalNumber = 0;
    m_BBHE_nUpperTotalNumber = 0;

    GetImageBrightnessPara(MatInput) ;

//////
    m_BBHE_byDecomposeTh = m_BBHE_byBrightnessMean ;

    DecomposeImage(MatInput, m_BBHE_byDecomposeTh);

    //char Buf[100];
    //sprintf_s(Buf, 50, "m_BBHE_byDecomposeTh = %ld ",m_BBHE_byDecomposeTh );
    //::MessageBoxA(NULL, Buf,"debug correction", MB_OK);


    ComputeCDF(lfCdf,destHisto, histSize);

    GetEnhanceResult(MatInput, MatOut, lfCdf);

    delete [] destHisto;
    delete [] lfCdf;
    return true ;

}


void CContrast::GetEnhanceResult(cv::Mat MatInput, cv::Mat &MatOut, double lfCdf[])
{
//    int i,j;
    uint16_t Val;

   long LowerBand = m_BBHE_byDecomposeTh - m_BBHE_byBrightnessMin;
    long UpperBand = m_BBHE_byBrightnessMax - m_BBHE_byDecomposeTh - 1;

    for(int i=0;i<MatInput.rows;i++)
    {
        for(int j=0;j<MatInput.cols;j++)
        {
            Val = GetPixelValue(MatInput, i, j) ;
            if ( Val > 0)
            {
                if( Val <= m_BBHE_byDecomposeTh)
                {
                    Val = m_BBHE_byBrightnessMin +  LowerBand * lfCdf[Val];
                }
                else
                {
                    Val = m_BBHE_byDecomposeTh +1 +  UpperBand * lfCdf[Val];
                }
            }

            if ( MatInput.type() == CV_16U )
            {
                MatOut.at<uint16_t>(i,j) = Val;
            }
            else
            {
                MatOut.at<uint8_t>(i,j) = (uint8_t)Val;
            }

        }
    }

 }


void CContrast::ComputeCDF(double lfCdf[],float histogram[], long HistoSize)
{
    int i;

    lfCdf[0]    =  histogram[0];

    for( i = 1; i <= (int)m_BBHE_byDecomposeTh; i++)
    {
       lfCdf[i] =  lfCdf[i-1] + histogram[i];
    }

    for( i = 1; i <= (int)m_BBHE_byDecomposeTh; i++)
    {
       lfCdf[i] =  lfCdf[i] / m_BBHE_nLowerTotalNumber;
    }



    lfCdf[m_BBHE_byDecomposeTh+1] = histogram[m_BBHE_byDecomposeTh+1];

    for( i = m_BBHE_byDecomposeTh + 2; i < HistoSize; i++)
    {
       lfCdf[i] =  lfCdf[i-1] + histogram[i];
    }


    for( i = m_BBHE_byDecomposeTh+1; i < HistoSize; i++)
    {
       lfCdf[i] =  lfCdf[i] / m_BBHE_nUpperTotalNumber;
    }

}

void CContrast::DecomposeImage(cv::Mat MatInput, long  byDecomposTh)
{
    uint16_t Val;

    for(int i=0;i<MatInput.rows;i++)
    {
        for(int j=0;j<MatInput.cols;j++)
        {
            Val = GetPixelValue(MatInput, i, j) ;

            if( Val <= byDecomposTh)
            {
                    m_BBHE_nLowerTotalNumber ++;
            }
            if( Val > byDecomposTh)
            {
                    m_BBHE_nUpperTotalNumber ++;
            }
        }
    }
}


void CContrast::GetImageBrightnessPara(  cv::Mat MatInput)
{
//    int i;
    double ldatasum     =  0 ;
    uint16_t Val;
    m_BBHE_byBrightnessMin =  65536 ;
    m_BBHE_byBrightnessMax =  0 ;


    for(int i=0;i<MatInput.rows;i++)
    {
        for(int j=0;j<MatInput.cols;j++)
        {
            Val = GetPixelValue(MatInput, i, j) ;
            ldatasum   += Val;

            if( Val < m_BBHE_byBrightnessMin)
            {
                m_BBHE_byBrightnessMin = Val;
            }
            if( Val > m_BBHE_byBrightnessMax)
            {
                m_BBHE_byBrightnessMax = Val;
            }
        }
    }

    m_BBHE_byBrightnessMean = (long)((double)ldatasum / (double)(MatInput.rows * MatInput.cols));


}


uint16_t CContrast::GetPixelValue(cv::Mat MatInput, int Row, int Col)
{
    uint16_t Val;

    if ( MatInput.type() == CV_16U )
    {
        Val = MatInput.at<uint16_t>(Row,Col) ;
    }
    else
    {
        Val = MatInput.at<uint8_t>(Row,Col) ;
    }

    return Val ;
}



CImImage::CImImage(void)
{
}

CImImage::~CImImage(void)
{
}

void CImImage::CalcHistoMeanMedian(cv::Mat MatInput, ImageStatistique *pStat, cv::Mat Mask)
{
    if (pStat == NULL) return ;

        float MaxHigh = 0.0 ;
        long MaxHighIndice = 0 ;
        double sumMoyenne = 0.0 ;
        double sumMoyenne2 = 0.0 ;
        double sumMoyenneQ1 = 0.0 ;
        double sumMoyenneQ2 = 0.0 ;
        double sumMoyenneQ3 = 0.0 ;
        double Moyenne = 0.0 ;
        double Moyenne2 = 0.0 ;
        double MoyenneQ1 = 0.0 ;
        double MoyenneQ2 = 0.0 ;
        double MoyenneQ3 = 0.0 ;
        double PourcentQ1 = 0.0 ;
        double PourcentQ2 = 0.0 ;
        double PourcentQ3 = 0.0 ;
        double PourcentPicQ1 = 0.0 ;
        double PourcentPicQ2 = 0.0 ;
        double PourcentPicQ3 = 0.0 ;
        double indicePicQ1 = 0 ;
        double indicePicQ2 = 0 ;
        double indicePicQ3 = 0 ;
  //      long NbM2 = 0 ;
        long NbMQ1 = 0 ;
        long NbMQ2 = 0 ;
        long NbMQ3 = 0 ;
        long PicQ1 = 0 ;
        long PicQ2 = 0 ;
        long PicQ3 = 0 ;
        //double sumCarre = 0.0 ;
        bool bCalculMin = false ;
        double Variance = 0.0 ;
        double EntropyLoc = 0.0 ;
        int i ;
        long FreqPicInRangeTmp = 0 ;


    int histSize = 256;

    if ( MatInput.depth() == CV_8U) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

     short TypeMatInput = MatInput.type() ;

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( TypeMatInput == CV_8UC3 )
    {
             /// Separate the image in 3 places ( B, G and R )
          std::vector<cv::Mat> bgr_planes(3);
          split( MatInput, bgr_planes );

          /// Establish the number of bins
          /// Set the ranges ( for B,G,R) )
//		  float range[] = { 0, histSize } ;
       float range[] = {0,(float)(histSize)};
          const float* histRange = { range };

          bool uniform = true; bool accumulate = false;

          cv::Mat b_hist, g_hist, r_hist;

          /// Compute the histograms:
          calcHist( &bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
          calcHist( &bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
          calcHist( &bgr_planes[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

          return ;

    }
    //if (TypeMatInput == CV_8UC1 )
    //{
    //	  /// Establish the number of bins

    //	  /// Set the ranges ( for B,G,R) )
    ////	  float range[] = { 0, histSize } ;
    //   float range[] = {0,(float)(histSize)};
    //	  const float* histRange = { range };

    //	  bool uniform = true;
    //	  bool accumulate = false;

    //	  cv::Mat nb_hist;

    //	  /// Compute the histograms:
    //	  calcHist( &MatInput, 1, 0, cv::Mat(), nb_hist, 1, &histSize, &histRange, uniform, accumulate );
    //	  return ;
    //}

    if (TypeMatInput == CV_16UC1 || TypeMatInput == CV_8UC1 )
    {
          /// Establish the number of bins
        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;
        double n = 0;

        cv::Mat nb_hist;

        // Compute the histograms:
        calcHist( &MatInput, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];
        long ValeurLim2 = 1000 ;
        if ( TypeMatInput == CV_8UC1) ValeurLim2 = 4 ;
        long ValeurLimQ2_Min = 10000 ;
        if ( TypeMatInput == CV_8UC1) ValeurLimQ2_Min = 40 ;
        long ValeurLimQ2_Max = 55535 ;
        if ( TypeMatInput == CV_8UC1) ValeurLimQ2_Max = 217 ;
        long ValeurLimQ3_Max = 65535 ;
        if ( TypeMatInput == CV_8UC1) ValeurLimQ3_Max = 255 ;

        for( i = 0; i < histSize; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
            n += dest[i];
            if ( dest[i] > MaxHigh && i != 0 && i != histSize - 1)
            {
                MaxHigh = dest[i] ;
                MaxHighIndice = i ;
            }
            if ( i > 0 && dest[i] > 0 && !bCalculMin)
            {
                pStat->MinWithoutZero = i ;
                bCalculMin = true ;
            }
            sumMoyenne += i * dest[i] ;
            if ( i > ValeurLim2 && dest[i] > 0 )
            {
                sumMoyenne2 += i * dest[i] ;
            //    NbM2 += (long)dest[i] ;
            }
            if ( i > ValeurLimQ2_Min && i < ValeurLimQ2_Max  )
            {
                sumMoyenneQ2 += i * dest[i] ;
                NbMQ2 += (long)dest[i] ;
                if ( PicQ2 < (long)dest[i] )
                {
                    PicQ2 = (long)dest[i] ;
                    indicePicQ2 = i ;
                }
            }
            if ( i >= 0 && i < ValeurLimQ2_Min  )
            {
                sumMoyenneQ1 += i * dest[i] ;
                NbMQ1 += (long)dest[i] ;
                if ( PicQ1 < (long)dest[i] )
                {
                    PicQ1 = (long)dest[i] ;
                    indicePicQ1 = i ;
                }
            }
            if ( i >= ValeurLimQ2_Max && i <= ValeurLimQ3_Max  )
            {
                sumMoyenneQ3 += i * dest[i] ;
                NbMQ3 += (long)dest[i] ;
                if ( PicQ3 < (long)dest[i] )
                {
                    PicQ3 = (long)dest[i] ;
                    indicePicQ3 = i ;
                }
            }
    //		sumCarre += i * i * dest[i]  ;
            // calcul pic central
            if ( i > pStat->Range_Min && i < pStat->Range_Max  )
            {
                if ( dest[i] > FreqPicInRangeTmp )
                {
                    FreqPicInRangeTmp = (long)(dest[i]) ;
                    pStat->PicMaxInRange = i;
                }
            }

        }


    //FILE * pFile;
    //long lSize;
    //unsigned char * buffer;

    //_tfopen_s(&pFile,(LPCTSTR)_T("c:\\temp\\histogramme.csv"),_T("w"));

    //char Buf[50];
    //for( i = 0; i < histSize; i++)
 //   {
    //		memset(Buf, 0, 50);
    //		sprintf_s(Buf, 50, "%i;%f\r",i,dest[i]);
    //		fwrite( Buf, sizeof( char ), 50, pFile );
 //   }
    //fclose(pFile);
        if ( n == 0) return ;


        Moyenne = sumMoyenne / n;
        Moyenne2 = sumMoyenne2 / n;
        MoyenneQ1 = -1 ;
        MoyenneQ2 = -1 ;
        MoyenneQ3 = -1 ;
        PourcentQ1 = -1 ;
        PourcentQ2 = -1 ;
        PourcentQ3 = -1 ;
        PourcentPicQ1 = -1 ;
        PourcentPicQ2 = -1 ;
        PourcentPicQ3 = -1 ;
        if ( NbMQ1 != 0) MoyenneQ1 = sumMoyenneQ1 / NbMQ1;
        if ( NbMQ2 != 0) MoyenneQ2 = sumMoyenneQ2 / NbMQ2;
        if ( NbMQ3 != 0) MoyenneQ3 = sumMoyenneQ3 / NbMQ3;
        if ( NbMQ1+NbMQ2+NbMQ3 != 0) PourcentQ1 = 100.0*(double)NbMQ1 / (double)(NbMQ1+NbMQ2+NbMQ3);
        if ( NbMQ1+NbMQ2+NbMQ3 != 0) PourcentQ2 =  100.0*(double)NbMQ2 / (double)(NbMQ1+NbMQ2+NbMQ3);
        if ( NbMQ1+NbMQ2+NbMQ3 != 0) PourcentQ3 =  100.0*(double)NbMQ3 / (double)(NbMQ1+NbMQ2+NbMQ3);
        if ( NbMQ1+NbMQ2+NbMQ3 != 0 && ( MoyenneQ1 >=0 && MoyenneQ1 < histSize )) PourcentPicQ1 = PicQ1 ;
        if ( NbMQ1+NbMQ2+NbMQ3 != 0 && ( MoyenneQ2 >=0 && MoyenneQ2 < histSize )) PourcentPicQ2 =  PicQ2 ;
        if ( NbMQ1+NbMQ2+NbMQ3 != 0 && ( MoyenneQ3 >=0 && MoyenneQ3 < histSize )) PourcentPicQ3 =  PicQ3;

        double A ;
        for( i = 0; i < histSize; i++)
        {
            Variance += dest[i]  * (i - Moyenne) * (i - Moyenne);
            A = dest[i] / n ;
            if ( A != 0) EntropyLoc += A * log10(A);
        }

        A = 0 ;
        for( i = 0; i < histSize; i++)
        {
            A += dest[i] * i * i ;
        }
        Variance = A / (double)n - Moyenne * Moyenne;

        EntropyLoc *= -1 ;

        long lNGMin = 0 ;
        long lNGMax = 0 ;
        for ( i = 0; i < histSize; i++)
        {
            if (dest[i]>0)
            {
                lNGMin=i;
                break;
            }
        }
        if (lNGMin>0) lNGMin--; // line to the (p==0) point, not to data[min]

        long MinPic = 1 ;
        float SumPondere = (float)dest[1] / (MatInput.rows * MatInput.cols);
        for ( i = 2; i < histSize - 1; i++)
        {
            SumPondere += (float)(dest[i]) / (MatInput.rows * MatInput.cols);
            if (SumPondere > 0.05 && dest[i]>dest[i-1] && dest[i]>dest[i+1])
            {
                MinPic=i;
                break;
            }
        }
        pStat->MinPicWithoutZero = MinPic ;

            long MaxPic = histSize - 2 ;
            SumPondere = (float)dest[ histSize - 2] / (MatInput.rows * MatInput.cols);
            for ( i = histSize - 3; i >=0; i--)
            {
                SumPondere += (float)(dest[i]) / (MatInput.rows * MatInput.cols);
                if (SumPondere > 0.05 && dest[i]>dest[i-1] && dest[i]>dest[i+1])
                {
                    MaxPic=i;
                    break;
                }
            }
            pStat->MaxPicWithoutZero = MaxPic ;

// PourcentageMinCalcul
        SumPondere = 0.0 ;
        pStat->MinCalcul=0 ;
        for ( i = 0; i < histSize - 1; i++)
        {
            SumPondere += (float)(dest[i]) / (MatInput.rows * MatInput.cols);
            if (SumPondere * 1000 > pStat->PourcentageMinCalcul)
            {
                pStat->MinCalcul=i;
                break;
            }
        }
//  PourcentageMaxCalcul
        SumPondere = 0.0 ;
        pStat->MaxCalcul = 65535 ;
        for ( i = histSize - 2; i >=0; i--)
        {
            SumPondere += (float)(dest[i]) / (MatInput.rows * MatInput.cols);
            if (SumPondere * 1000 > pStat->PourcentageMaxCalcul)
            {
                pStat->MaxCalcul=i;
                break;
            }
        }


        for ( i = histSize - 2; i >0 ; i-- )
        {
            if ( dest[i]>0)
            {
                pStat->MaxWithoutMaxRange = i ;
                break;
            }
        }

        for ( i = histSize - 1; i >0 ; i-- )
        {
            if (dest[i]>200)
            {
                lNGMax=i;
                break;
            }
        }
        if (lNGMax<ValeurLimQ3_Max) lNGMax++; // line to the (p==0) point, not to data[min]

            pStat->Mean = (long)Moyenne ;
            pStat->Median = (lNGMax + lNGMin) / 2 ;
            //CString s ;
            //s.Format(L"Min %ld Max %ld --- moyenne = %ld -- medianne == %ld ___ Min Without Zero = %ld",lNGMin,lNGMax,*Mean, *Median, *MinWithoutZero);
            //AfxMessageBox(s);
            pStat->EcartType = (long)sqrt(Variance);
            pStat->Entropy = (float)EntropyLoc ;
            pStat->Mode = MaxHighIndice ;
            pStat->MoyenneD = (long)Moyenne2 ;
            pStat->MoyenneQ1 = (long)MoyenneQ1 ;
            pStat->MoyenneQ2 = (long)MoyenneQ2 ;
            pStat->MoyenneQ3 = (long)MoyenneQ3 ;
            pStat->PourcentQ1 = (long)PourcentQ1 ;
            pStat->PourcentQ2 = (long)PourcentQ2 ;
            pStat->PourcentQ3 = (long)PourcentQ3 ;
            pStat->PourcentPicQ1 = (long)PourcentPicQ1 ;
            pStat->PourcentPicQ2 = (long)PourcentPicQ2 ;
            pStat->PourcentPicQ3 = (long)PourcentPicQ3 ;
            pStat->indicePicQ1 = (long)indicePicQ1 ;
            pStat->indicePicQ2 = (long)indicePicQ2 ;
            pStat->indicePicQ3 = (long)indicePicQ3 ;
            delete []dest ;

            nb_hist.release();
        }
}

void CImImage::CalcHistoMeanMedianBC(cv::Mat MatInput, ImageStatistique *pStat, cv::Mat Mask)
{
    if (pStat == NULL) return ;

        float MaxHigh = 0.0 ;
        long MaxHighIndice = 0 ;
        double sumMoyenne = 0.0 ;
        double sumMoyenne2 = 0.0 ;
        double sumMoyenneQ1 = 0.0 ;
        double sumMoyenneQ2 = 0.0 ;
        double sumMoyenneQ3 = 0.0 ;
        double Moyenne = 0.0 ;
        double Moyenne2 = 0.0 ;
        double MoyenneQ1 = 0.0 ;
        double MoyenneQ2 = 0.0 ;
        double MoyenneQ3 = 0.0 ;
        double PourcentQ1 = 0.0 ;
        double PourcentQ2 = 0.0 ;
        double PourcentQ3 = 0.0 ;
        double PourcentPicQ1 = 0.0 ;
        double PourcentPicQ2 = 0.0 ;
        double PourcentPicQ3 = 0.0 ;
        double indicePicQ1 = 0 ;
        double indicePicQ2 = 0 ;
        double indicePicQ3 = 0 ;
        //long NbM2 = 0 ;
        long NbMQ1 = 0 ;
        long NbMQ2 = 0 ;
        long NbMQ3 = 0 ;
        long PicQ1 = 0 ;
        long PicQ2 = 0 ;
        long PicQ3 = 0 ;
        //double sumCarre = 0.0 ;
        bool bCalculMin = false ;
        double Variance = 0.0 ;
        double EntropyLoc = 0.0 ;
        int i ;
        long FreqPicInRangeTmp = 0 ;


    int histSize = 256;

    if ( MatInput.depth() == CV_8U) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

     short TypeMatInput = MatInput.type() ;

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( TypeMatInput == CV_8UC3 )
    {
             /// Separate the image in 3 places ( B, G and R )
          std::vector<cv::Mat> bgr_planes(3);
          split( MatInput, bgr_planes );

          /// Establish the number of bins
          /// Set the ranges ( for B,G,R) )
//		  float range[] = { 0, histSize } ;
       float range[] = {0,(float)(histSize)};
          const float* histRange = { range };

          bool uniform = true; bool accumulate = false;

          cv::Mat b_hist, g_hist, r_hist;

          /// Compute the histograms:
          calcHist( &bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
          calcHist( &bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
          calcHist( &bgr_planes[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

          return ;

    }
    //if (TypeMatInput == CV_8UC1 )
    //{
    //	  /// Establish the number of bins

    //	  /// Set the ranges ( for B,G,R) )
    ////	  float range[] = { 0, histSize } ;
    //   float range[] = {0,(float)(histSize)};
    //	  const float* histRange = { range };

    //	  bool uniform = true;
    //	  bool accumulate = false;

    //	  cv::Mat nb_hist;

    //	  /// Compute the histograms:
    //	  calcHist( &MatInput, 1, 0, cv::Mat(), nb_hist, 1, &histSize, &histRange, uniform, accumulate );
    //	  return ;
    //}

    if (TypeMatInput == CV_16UC1 || TypeMatInput == CV_8UC1 )
    {
          /// Establish the number of bins
        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

        // Compute the histograms:
        calcHist( &MatInput, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];
        long ValeurLim2 = 1000 ;
        if ( TypeMatInput == CV_8UC1) ValeurLim2 = 4 ;
        long ValeurLimQ2_Min = 10000 ;
        if ( TypeMatInput == CV_8UC1) ValeurLimQ2_Min = 40 ;
        long ValeurLimQ2_Max = 55535 ;
        if ( TypeMatInput == CV_8UC1) ValeurLimQ2_Max = 217 ;
        long ValeurLimQ3_Max = 65535 ;
        if ( TypeMatInput == CV_8UC1) ValeurLimQ3_Max = 255 ;

        double n = 0;
        for( i = 0; i < histSize; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
            n += dest[i];
            if ( dest[i] > MaxHigh && i != 0 && i != histSize - 1)
            {
                MaxHigh = dest[i] ;
                MaxHighIndice = i ;
            }
            if ( i > 0 && dest[i] > 0 && !bCalculMin)
            {
                pStat->MinWithoutZero = i ;
                bCalculMin = true ;
            }
            sumMoyenne += i * dest[i] ;
            if ( i > ValeurLim2 && dest[i] > 0 )
            {
                sumMoyenne2 += i * dest[i] ;
          //      NbM2 += (long)dest[i] ;
            }
            if ( i > ValeurLimQ2_Min && i < ValeurLimQ2_Max  )
            {
                sumMoyenneQ2 += i * dest[i] ;
                NbMQ2 += (long)dest[i] ;
                if ( PicQ2 < (long)dest[i] )
                {
                    PicQ2 = (long)dest[i] ;
                    indicePicQ2 = i ;
                }
            }
            if ( i >= 0 && i < ValeurLimQ2_Min  )
            {
                sumMoyenneQ1 += i * dest[i] ;
                NbMQ1 += (long)dest[i] ;
                if ( PicQ1 < (long)dest[i] )
                {
                    PicQ1 = (long)dest[i] ;
                    indicePicQ1 = i ;
                }
            }
            if ( i >= ValeurLimQ2_Max && i <= ValeurLimQ3_Max  )
            {
                sumMoyenneQ3 += i * dest[i] ;
                NbMQ3 += (long)dest[i] ;
                if ( PicQ3 < (long)dest[i] )
                {
                    PicQ3 = (long)dest[i] ;
                    indicePicQ3 = i ;
                }
            }
            if ( i > pStat->Range_Min && i < pStat->Range_Max  )
            {
                if ( dest[i] > FreqPicInRangeTmp )
                {
                    FreqPicInRangeTmp = (long)(dest[i]) ;
                    pStat->PicMaxInRange = i;
                }
            }

        }

        Moyenne = sumMoyenne / n;
        Moyenne2 = sumMoyenne2 / n;
        MoyenneQ1 = -1 ;
        MoyenneQ2 = -1 ;
        MoyenneQ3 = -1 ;
        PourcentQ1 = -1 ;
        PourcentQ2 = -1 ;
        PourcentQ3 = -1 ;
        PourcentPicQ1 = -1 ;
        PourcentPicQ2 = -1 ;
        PourcentPicQ3 = -1 ;
        if ( NbMQ1 != 0) MoyenneQ1 = sumMoyenneQ1 / NbMQ1;
        if ( NbMQ2 != 0) MoyenneQ2 = sumMoyenneQ2 / NbMQ2;
        if ( NbMQ3 != 0) MoyenneQ3 = sumMoyenneQ3 / NbMQ3;
        if ( NbMQ1+NbMQ2+NbMQ3 != 0) PourcentQ1 = 100.0*(double)NbMQ1 / (double)(NbMQ1+NbMQ2+NbMQ3);
        if ( NbMQ1+NbMQ2+NbMQ3 != 0) PourcentQ2 =  100.0*(double)NbMQ2 / (double)(NbMQ1+NbMQ2+NbMQ3);
        if ( NbMQ1+NbMQ2+NbMQ3 != 0) PourcentQ3 =  100.0*(double)NbMQ3 / (double)(NbMQ1+NbMQ2+NbMQ3);
        if ( NbMQ1+NbMQ2+NbMQ3 != 0 && ( MoyenneQ1 >=0 && MoyenneQ1 < histSize )) PourcentPicQ1 = PicQ1 ;
        if ( NbMQ1+NbMQ2+NbMQ3 != 0 && ( MoyenneQ2 >=0 && MoyenneQ2 < histSize )) PourcentPicQ2 =  PicQ2 ;
        if ( NbMQ1+NbMQ2+NbMQ3 != 0 && ( MoyenneQ3 >=0 && MoyenneQ3 < histSize )) PourcentPicQ3 =  PicQ3;

        double A ;
        for( i = 0; i < histSize; i++)
        {
            Variance += dest[i]  * (i - Moyenne) * (i - Moyenne);
            A = dest[i] / n ;
            if ( A != 0) EntropyLoc += A * log10(A);
        }

        A = 0 ;
        for( i = 0; i < histSize; i++)
        {
            A += dest[i] * i * i ;
        }
        Variance = A / (double)n - Moyenne * Moyenne;

        EntropyLoc *= -1 ;

        long lNGMin = 0 ;
        long lNGMax = 0 ;
        for ( i = 0; i < histSize; i++)
        {
            if (dest[i]>0)
            {
                lNGMin=i;
                break;
            }
        }
        if (lNGMin>0) lNGMin--; // line to the (p==0) point, not to data[min]

        long MinPic = 1 ;
        float SumPondere = (float)dest[1] / (MatInput.rows * MatInput.cols);
        for ( i = 2; i < histSize - 1; i++)
        {
            SumPondere += (float)(dest[i]) / (MatInput.rows * MatInput.cols);
            if (SumPondere > 0.05 && dest[i]>dest[i-1] && dest[i]>dest[i+1])
            {
                MinPic=i;
                break;
            }
        }
        pStat->MinPicWithoutZero = MinPic ;

            long MaxPic = histSize - 2 ;
            SumPondere = (float)dest[ histSize - 2] / (MatInput.rows * MatInput.cols);
            for ( i = histSize - 3; i >=0; i--)
            {
                SumPondere += (float)(dest[i]) / (MatInput.rows * MatInput.cols);
                if (SumPondere > 0.05 && dest[i]>dest[i-1] && dest[i]>dest[i+1])
                {
                    MaxPic=i;
                    break;
                }
            }
            pStat->MaxPicWithoutZero = MaxPic ;

// PourcentageMinCalcul
        SumPondere = 0.0 ;
        pStat->MinCalcul=0 ;
        for ( i = 0; i < histSize - 1; i++)
        {
            SumPondere += (float)(dest[i]) / (MatInput.rows * MatInput.cols);
            if (SumPondere * 1000 > pStat->PourcentageMinCalcul)
            {
                pStat->MinCalcul=i;
                break;
            }
        }
//  PourcentageMaxCalcul
        SumPondere = 0.0 ;
        pStat->MaxCalcul = 65535 ;
        for ( i = histSize - 2; i >=0; i--)
        {
            SumPondere += (float)(dest[i]) / (MatInput.rows * MatInput.cols);
            if (SumPondere * 1000 > pStat->PourcentageMaxCalcul)
            {
                pStat->MaxCalcul=i;
                break;
            }
        }


        for ( i = histSize - 2; i >0 ; i-- )
        {
            if ( dest[i]>0)
            {
                pStat->MaxWithoutMaxRange = i ;
                break;
            }
        }

        for ( i = histSize - 1; i >0 ; i-- )
        {
            if (dest[i]>200)
            {
                lNGMax=i;
                break;
            }
        }
        if (lNGMax<ValeurLimQ3_Max) lNGMax++; // line to the (p==0) point, not to data[min]

            pStat->Mean = (long)Moyenne ;
            pStat->Median = (lNGMax + lNGMin) / 2 ;
            //CString s ;
            //s.Format(L"Min %ld Max %ld --- moyenne = %ld -- medianne == %ld ___ Min Without Zero = %ld",lNGMin,lNGMax,*Mean, *Median, *MinWithoutZero);
            //AfxMessageBox(s);
            pStat->EcartType = (long)sqrt(Variance);
            pStat->Entropy = (float)EntropyLoc ;
            pStat->Mode = MaxHighIndice ;
            pStat->MoyenneD = (long)Moyenne2 ;
            pStat->MoyenneQ1 = (long)MoyenneQ1 ;
            pStat->MoyenneQ2 = (long)MoyenneQ2 ;
            pStat->MoyenneQ3 = (long)MoyenneQ3 ;
            pStat->PourcentQ1 = (long)PourcentQ1 ;
            pStat->PourcentQ2 = (long)PourcentQ2 ;
            pStat->PourcentQ3 = (long)PourcentQ3 ;
            pStat->PourcentPicQ1 = (long)PourcentPicQ1 ;
            pStat->PourcentPicQ2 = (long)PourcentPicQ2 ;
            pStat->PourcentPicQ3 = (long)PourcentPicQ3 ;
            pStat->indicePicQ1 = (long)indicePicQ1 ;
            pStat->indicePicQ2 = (long)indicePicQ2 ;
            pStat->indicePicQ3 = (long)indicePicQ3 ;
            delete []dest ;

            nb_hist.release();
        }
}

long CImImage::GetMinHisto(cv::Mat MatInput, long PourDixMil, cv::Mat Mask, bool bHorzZero)
{
    int i ;
    long MinIndice = 0;
    long MinWithoutZero = 0 ;
    double Sum ;

    int histSize = 256;

    if ( bHorzZero ) MinIndice = 0 ;

    if ( MatInput.depth() == CV_8UC1) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( MatInput.type() == CV_8UC3 )
    {
        return -1 ;
    }
    if (MatInput.type() == CV_8UC1 )
    {
        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
        calcHist( &MatInput, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];

        Sum = 0.0 ;
        for( i = 0; i < histSize; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
            Sum += dest[i] ;
            if ( i >= MinIndice && Sum > (double)(PourDixMil * MatInput.rows * MatInput.cols) / 10000.0 )
            {
                MinWithoutZero = i ;
                break ;
            }
        }
        delete []dest ;
    }

    if (MatInput.type() == CV_16UC1)
    {
        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
        calcHist( &MatInput, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];

        Sum = 0.0 ;
        for( i = 0; i < histSize; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
            Sum += dest[i] ;
            if ( i >= MinIndice && Sum > (double)(PourDixMil * MatInput.rows * MatInput.cols) / 10000.0 )
            {
                MinWithoutZero = i ;
                break ;
            }
        }
        delete []dest ;
    }

    histImage.release();

        return MinWithoutZero ;
}

long CImImage::GetNGLowestHisto(cv::Mat MatInput, cv::Mat Mask, long BorneMin , long BorneMax  )
{
    long PicNG = 0 ;
    int i ;
    //long MinIndice = 0;
    double MinFreq ;

    int histSize = 256;


    if ( MatInput.depth() == CV_8U) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

//	char buf[50] ;

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( MatInput.type() == CV_8UC3 )
    {
        return -1 ;
    }
    if (MatInput.type() == CV_8UC1 )
    {
          return -1;
    }

    if (MatInput.type() == CV_16UC1)
    {
        // conversion 8 bits pour accelerer
        cv::Mat Mat8b ;
        MatInput.convertTo(Mat8b, CV_8UC1, 1.0/256.0);

        histSize = (int)pow(2.0, 8)  ;

        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
        calcHist( &Mat8b, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];

        for( i = 0;  i < histSize - 1 ; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
        }

        MinFreq = dest[BorneMax/256] ;
        PicNG = BorneMax/256 ;
        for( i = (int)(BorneMax/256);  i >= (int)(BorneMin/256); i--)
        {
            if ( MinFreq > dest[i] && dest[i] > 0 )
            {
                MinFreq = dest[i] ;
                if (dest[i] < dest[i-1] && dest[i] < dest[i+1] )
                {
                    PicNG = i ;
                }
            }
        }
        PicNG *= 256 ;
        delete []dest ;

    }

//	itoa(PicNG, buf, 10);
//	::MessageBoxA(NULL, buf, "min", MB_OK | MB_ICONWARNING);

    return PicNG ;
}

long CImImage::GetPicHisto(cv::Mat MatInput, cv::Mat Mask, long BorneMin , long BorneMax, long *PicPourcent  )
{
    long PicNG = 0 ;
    int i ;
    //long MaxIndice = 0;
    //long MaxWithoutZero = 0 ;
    double MaxFreq ;

    int histSize = 256;


    if ( MatInput.depth() == CV_8U) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

//    if ( MatInput.depth() == CV_8U) MaxIndice = 255 ;
//    if ( MatInput.depth() == CV_16U) MaxIndice = 65535 ;

//	char buf[50] ;

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( MatInput.type() == CV_8UC3 )
    {
        return -1 ;
    }
    //if (MatInput.type() == CV_8UC1 )
    //{
    //	  return -1;
    //}

    if (MatInput.type() == CV_16UC1 || MatInput.type() == CV_8UC1)
    {
        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
//		AfficherMat( MatInput,"histramm");
        calcHist( &MatInput, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];

        MaxFreq = 0.0 ;

        for( i = 0;  i < histSize; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
        }

/*
    FILE * pFile;
    long lSize;
    unsigned char * buffer;

    _tfopen_s(&pFile,(LPCTSTR)_T("c:\\temp\\histogramme.csv"),_T("w"));

    for( i = 0; i < histSize; i++)
    {
            memset(buf, 0, 50);
            sprintf_s(buf, 50, "%i;%f\r",i,dest[i]);
            fwrite( buf, sizeof( char ), 50, pFile );
    }
    fclose(pFile);
*/
        long n = 0 ;
        for( i = (int)BorneMin;  i < (int)BorneMax; i++)
        {
            if ( MaxFreq < dest[i] )
            {
                MaxFreq = dest[i] ;
                PicNG = i ;

            }
            n += (long)dest[i] ;
        }

        if ( PicPourcent != NULL) *PicPourcent = (long)((100 * MaxFreq) / n);
        delete []dest ;
    //itoa(PicNG, buf, 10);
    //::MessageBoxA(NULL, buf, "max", MB_OK | MB_ICONWARNING);

    }


    return PicNG ;
}

long CImImage::GetFirstPicHisto256FromBottom(cv::Mat MatInput, cv::Mat Mask, long BorneMin , long BorneMax   )
{
    long PicNG = 0 ;
    int i ;
    //long MaxIndice = 0;
    //long MaxWithoutZero = 0 ;
    //double MaxFreq ;
    cv::Mat Mat256 = cv::Mat(MatInput.size(), CV_8UC1);

    int histSize = 256;


    if ( MatInput.depth() == CV_8U) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

//    if ( MatInput.depth() == CV_8U) MaxIndice = 255 ;
//    if ( MatInput.depth() == CV_16U) MaxIndice = 65535 ;

//	char buf[50] ;

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( MatInput.type() == CV_8UC3 )
    {
        return -1 ;
    }
    //if (MatInput.type() == CV_8UC1 )
    //{
    //	  return -1;
    //}

    if (MatInput.type() == CV_16UC1 || MatInput.type() == CV_8UC1)
    {
        if ( MatInput.type() == CV_16UC1 )
        {
            // convertir en 256 bits
            MatInput.convertTo(Mat256, CV_8UC1, 1.0/256.);

        }
        else
        {
            Mat256 = MatInput.clone() ;
        }
        //MaxIndice = 255 ;
        histSize = (int)pow(2.0, 8)  ;

        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
//		AfficherMat( MatInput,"histramm");
        calcHist( &Mat256, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];

        //MaxFreq = 0.0 ;

        for( i = 0;  i < histSize; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
        }

/*
    FILE * pFile;
    long lSize;
    unsigned char * buffer;
    char buf[50] ;

    _tfopen_s(&pFile,(LPCTSTR)_T("c:\\temp\\histogramme.csv"),_T("w"));

    for( i = 0; i < histSize; i++)
    {
            memset(buf, 0, 50);
            sprintf_s(buf, 50, "%i;%f\r",i,dest[i]);
            fwrite( buf, sizeof( char ), 50, pFile );
    }
    fclose(pFile);
*/

//		long n = 0 ;
        if ( BorneMin == 0) BorneMin = 1 ;

        for( i = (int)BorneMin;  i < (int)BorneMax; i++)
        {
            if ( dest[i-1] < dest[i] && dest[i+1] < dest[i] && dest[i] > 10000)
            {
//				MaxFreq = dest[i] ;
                PicNG = i ;
                break;
            }
//			n += dest[i] ;
        }
/*
        if ( PicPourcent != NULL)
        {
            long iMin , iMax ;
            iMin = PicNG - 5 ;
            if ( iMin < BorneMin ) iMin = BorneMin ;
            iMax = PicNG + 5 ;
            if ( iMax > BorneMax - 1 ) iMax = BorneMax - 1 ;
            *PicPourcent = 0 ;
            for ( i =  iMin ; i < iMax ; i++)
            {
                *PicPourcent += (100 * dest[i]) / n ;
            }
        }
*/
        delete []dest ;
    //char buf[50] ;
    //itoa(PicNG, buf, 10);
    //::MessageBoxA(NULL, buf, "max", MB_OK | MB_ICONWARNING);

    }


    return PicNG ;
}


long CImImage::GetPicHisto256(cv::Mat MatInput, cv::Mat Mask, long BorneMin , long BorneMax , long *PicPourcent  )
{
    long PicNG = 0 ;
    int i ;
    //long MaxIndice = 0;
    //long MaxWithoutZero = 0 ;
    double MaxFreq ;
    cv::Mat Mat256 = cv::Mat(MatInput.size(), CV_8UC1);

    int histSize = 256;


    if ( MatInput.depth() == CV_8U) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

//    if ( MatInput.depth() == CV_8U) MaxIndice = 255 ;
//    if ( MatInput.depth() == CV_16U) MaxIndice = 65535 ;

//	char buf[50] ;

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( MatInput.type() == CV_8UC3 )
    {
        return -1 ;
    }
    //if (MatInput.type() == CV_8UC1 )
    //{
    //	  return -1;
    //}

    if (MatInput.type() == CV_16UC1 || MatInput.type() == CV_8UC1)
    {
        if ( MatInput.type() == CV_16UC1 )
        {
            // convertir en 256 bits
            MatInput.convertTo(Mat256, CV_8UC1, 1.0/256.);

        }
        else
        {
            Mat256 = MatInput.clone() ;
        }
   //     MaxIndice = 255 ;
        histSize = (int)pow(2.0, 8)  ;

        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
//		AfficherMat( MatInput,"histramm");
        calcHist( &Mat256, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];

        MaxFreq = 0.0 ;

        for( i = 0;  i < histSize; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
        }

/*
    FILE * pFile;
    long lSize;
    unsigned char * buffer;
    char buf[50] ;

    _tfopen_s(&pFile,(LPCTSTR)_T("c:\\temp\\histogramme.csv"),_T("w"));

    for( i = 0; i < histSize; i++)
    {
            memset(buf, 0, 50);
            sprintf_s(buf, 50, "%i;%f\r",i,dest[i]);
            fwrite( buf, sizeof( char ), 50, pFile );
    }
    fclose(pFile);
*/

        long n = 0 ;
        for( i = (int)BorneMin;  i < (int)BorneMax; i++)
        {
            if ( MaxFreq < dest[i] )
            {
                MaxFreq = dest[i] ;
                PicNG = i ;
            }
            n += (long)dest[i] ;
        }

        if ( PicPourcent != NULL)
        {
            long iMin , iMax ;
            iMin = PicNG - 5 ;
            if ( iMin < BorneMin ) iMin = BorneMin ;
            iMax = PicNG + 5 ;
            if ( iMax > BorneMax - 1 ) iMax = BorneMax - 1 ;
            *PicPourcent = 0 ;
            for ( i =  (int)iMin ; i < (int)iMax ; i++)
            {
                *PicPourcent += (long)((100 * dest[i]) / n);
            }
        }
        delete []dest ;
    //itoa(PicNG, buf, 10);
    //::MessageBoxA(NULL, buf, "max", MB_OK | MB_ICONWARNING);

    }


    return PicNG ;
}


float CImImage::GetEntropyHisto(cv::Mat MatInput, cv::Mat Mask)
{
    float fEntropy = 0.0f ;
    int histSize = 256;

    if ( MatInput.depth() == CV_8UC1) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( MatInput.type() == CV_8UC3 )
    {
        return -1 ;
    }

    if (MatInput.type() == CV_16UC1 || MatInput.type() == CV_8UC1)
    {
        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
        calcHist( &MatInput, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        nb_hist /= MatInput.total();

        cv::Mat logP;
        cv::log(nb_hist,logP);

        fEntropy = -1*cv::sum(nb_hist.mul(logP)).val[0];

//		float *dest = new float[histSize];

    }

    return fEntropy;
}


float CImImage::GetContrasteEdgeHisto(cv::Mat MatInput, cv::Mat /*Mask*/)
{
    float fContraste = 0.0f ;
    int histSize = 256;

    if ( MatInput.depth() == CV_8UC1) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( MatInput.type() == CV_8UC3 )
    {
        qDebug() << "Pas d'image couleur :: non implementer" ;
        return -1 ;
    }

    if (MatInput.type() == CV_16UC1 || MatInput.type() == CV_8UC1)
    {
        cv::Mat MatCanny ;
        // Canny
        cv::GaussianBlur(MatInput,MatInput, cv::Size(3,3),0);
        long mean = cv::mean(MatInput).val[0];
        long Min = int(std::max(0, (int)((1.0 - 0.33) * mean)));
        long Max = int(std::min(255, (int)((1.0 + 0.33) * mean)));
    //	cv::Canny(MatInput, MatCanny, Min,Max,3,true);
        cv::Canny(MatInput, MatCanny, Min,Max);

        // compte le nombre de piwel à 255
//		double minVal;
//		double maxVal;
        int minLoc = 0;
        int maxLoc = 0;

        //cv::minMaxIdx( MatInput, &minVal, &maxVal, &minLoc, &maxLoc , MatCanny);
        //fContraste = maxVal;
        //if ( (maxLoc + minLoc) != 0.0)
        //{
        //	fContraste = (float)(maxLoc - minLoc) / (float)(maxLoc + minLoc) ;
        //}
        //else
        //{
        //	fContraste = 0.0f;
        //}

        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
        calcHist( &MatInput, 1, 0, cv::Mat(), nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];
        //long Numpix = 0;


        minLoc = -1 ;
        maxLoc = -1 ;
        for( long i = 1;  i < histSize; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
            //Numpix += (long)dest[i] ;
            if ( dest[i]  > 0.0f && minLoc == -1) minLoc = i ;
        }
        for( long i = histSize - 1;  i > 0; i--)
        {
            if ( dest[i]  > 0 && maxLoc == -1 ) maxLoc = i ;

        }
                //char Buf[50];
                //sprintf_s(Buf, _countof(Buf), "Min %d Max %d", minLoc, maxLoc);
                //::MessageBoxA(NULL, Buf,"ContrasteEdge", MB_OK);

        if ( (maxLoc + minLoc) != 0.0)
        {
            fContraste = (float)(maxLoc - minLoc) / (float)(maxLoc + minLoc) ;
        }
        else
        {
            fContraste = 0.0f;
        }
        //cv::minMaxIdx( MatInput, &minVal, &maxVal, &minLoc, &maxLoc , MatCanny);
        //fContraste = maxVal;

        /*
        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
        calcHist( &MatInput, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        double minF=0,maxF=0;
        int minL=0,maxL=0;
        cv::minMaxIdx(nb_hist, &minF, &maxF, &minL, &maxL);

        if ( (maxL + minL) != 0.0)
        {
            fContraste = (float)(maxL - minL) / (float)(maxL + minL) ;
        }
        else
        {
            fContraste = 0.0f;
        }
        */
        //nb_hist /= MatInput.total();

        //cv::Mat logP;
        //cv::log(nb_hist,logP);

        //fContraste = -1*cv::sum(nb_hist.mul(logP)).val[0];

//		float *dest = new float[histSize];

    }

    return fContraste;
}


float CImImage::GetContrasteHisto(cv::Mat MatInput, cv::Mat /*Mask*/)
{
    float fContraste = 0.0f ;
//    int histSize = 256;

//    if ( MatInput.depth() == CV_8UC1) histSize = (int)pow(2.0, 8)  ;
//    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
//    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( MatInput.type() == CV_8UC3 )
    {
        return -1 ;
    }

    if (MatInput.type() == CV_16UC1 || MatInput.type() == CV_8UC1)
    {
        double minVal;
        double maxVal;
        int minLoc;
        int maxLoc;

        cv::minMaxIdx( MatInput, &minVal, &maxVal, &minLoc, &maxLoc );
        fContraste = (float)(maxLoc - minLoc)/(float)cv::mean(MatInput).val[0];


    }

    return fContraste;
}



long CImImage::GetPourCentOverHisto256(cv::Mat MatInput, cv::Mat Mask, long Borne)
{
    //long PicNG = 0 ;
    int i ;
    //long MaxIndice = 0;
    //long MaxWithoutZero = 0 ;
    double MaxFreq = 0;
    cv::Mat Mat256 = cv::Mat(MatInput.size(), CV_8UC1);

    int histSize = 256;


    if ( MatInput.depth() == CV_8U) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

//    if ( MatInput.depth() == CV_8U) MaxIndice = 255 ;
//    if ( MatInput.depth() == CV_16U) MaxIndice = 65535 ;

//	char buf[50] ;

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( MatInput.type() == CV_8UC3 )
    {
        return -1 ;
    }

    if (MatInput.type() == CV_16UC1 || MatInput.type() == CV_8UC1)
    {
        if ( MatInput.type() == CV_16UC1 )
        {
            // convertir en 256 bits
            MatInput.convertTo(Mat256, CV_8UC1, 1.0/256.);
        }
        else
        {
            Mat256 = MatInput.clone() ;
        }
  //      MaxIndice = 255 ;
        histSize = (int)pow(2.0, 8)  ;

        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

        calcHist( &Mat256, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];

        MaxFreq = 0.0 ;

        long Numpix = 0;

        for( i = 0;  i < histSize; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
            Numpix += (long)dest[i] ;
        }


        for( i = (int)Borne;  i < histSize; i++)
        {
            MaxFreq += dest[i] ;
        }

        if (Numpix > 0) MaxFreq = 100 * (MaxFreq / Numpix) ;
        delete []dest ;

    }


    return (long)MaxFreq ;
}

long CImImage::GetMeanHisto(cv::Mat MatInput, cv::Mat Mask )
{
    int i ;
    //long MaxIndice = 0;
    //long MaxWithoutZero = 0 ;
   // double Sum ;
    double Moyenne ;

    int histSize = 256;

    if ( MatInput.depth() == CV_8U) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

//    if ( MatInput.depth() == CV_8U) MaxIndice = 255 ;
//    if ( MatInput.depth() == CV_16U) MaxIndice = 65535 ;

//	char buf[50] ;

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    Moyenne = -1 ;

    if ( MatInput.type() == CV_8UC3 )
    {
        Moyenne = -1 ;
    }

    if (MatInput.type() == CV_16UC1 || MatInput.type() == CV_8UC1)
    {
        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
        calcHist( &MatInput, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];

    //    Sum = 0.0 ;
        //long MMAX = histSize - 1 ;

        double sumMoyenne = 0.0 ;
        //double A = 0.0;
        //double Variance = 0.0 ;
        double n = 0;
        for( i = 0;  i < histSize; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
            sumMoyenne += i * dest[i] ;
            n += dest[i] ;
        }
        Moyenne = sumMoyenne / n;
        delete []dest ;

    }

    return (long)Moyenne ;
}


bool CImImage::GetMeanSDHisto(cv::Mat MatInput, cv::Mat Mask, long *Mean, double *SD )
{
    int i ;
//	double Sum ;

    int histSize = 256;

    if ( MatInput.depth() == CV_8UC1) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );


    if ( MatInput.type() == CV_8UC3 )
    {
        *Mean = -1 ;
        *SD = -1.0;
        return false  ;
    }

    if (MatInput.type() == CV_16UC1 || MatInput.type() == CV_8UC1)
    {
        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
        calcHist( &MatInput, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];

        //double Sum = 0.0 ;
        double Moyenne = 0 ;

        double sumMoyenne = 0.0 ;
        //double A = 0.0;
        double Variance = 0.0 ;
        double n = 0;
        for( i = 0;  i < histSize; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
            sumMoyenne += i * dest[i] ;
            n += dest[i] ;
        }

        Moyenne = sumMoyenne / n;
                        //char buf[512];
                        //sprintf_s(buf,  _countof(buf), "sumMoyenne = %.5f Moyenne = %.5f", sumMoyenne, Moyenne);
                        //::MessageBoxA(NULL, buf, "sumMoyenne", MB_OK);
        for( i = 0; i < histSize; i++)
        {
            Variance += dest[i]  * (i - Moyenne) * (i - Moyenne);
        }
        delete []dest ;
        *Mean = (long)Moyenne ;
        *SD = sqrt(Variance);

    }


    return true ;
}


long CImImage::GetMaxHisto(cv::Mat MatInput, long PourDixMil, cv::Mat Mask, bool bHorzMax,bool bIncludeMax, long Depth)
{
    int i ;
    long MaxIndice = 0;
    long MaxWithoutZero = 0 ;
    double Sum ;

    int histSize = 256;

    if ( MatInput.depth() == CV_8UC1) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

    if ( MatInput.depth() == CV_8UC1) MaxIndice = 255 ;
    if ( MatInput.depth() == CV_16U) MaxIndice = 65535 ;
    if ( bHorzMax && MatInput.depth() == CV_8U) MaxIndice = 254 ;
    if ( bHorzMax && MatInput.depth() == CV_16U) MaxIndice = 65534 ;

    //char buf[50] ;
    //itoa(MaxIndice, buf, 10);
    //::MessageBoxA(NULL, buf, "max1", MB_OK | MB_ICONWARNING);

     int hist_w = 1024; int hist_h = 800;
     //int bin_w = cvRound( (double) hist_w/histSize );

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( MatInput.type() == CV_8UC3 )
    {
        return -1 ;
    }
    //if (MatInput.type() == CV_8UC1 )
    //{
    //	  return -1;
    //}

    if (MatInput.type() == CV_16UC1 || MatInput.type() == CV_8UC1)
    {
        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
        calcHist( &MatInput, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];

        Sum = 0.0 ;
        long MMAX = histSize - 1 ;

        double sumMoyenne = 0.0 ;
        double A = 0.0;
        double Variance = 0.0 ;
        double n = 0;
        for( i = 0;  i < histSize; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
            sumMoyenne += i * dest[i] ;
            A += dest[i] * i * i ;
            n += dest[i] ;
        }
        double Moyenne = sumMoyenne / n;
        // calculer l'ecart type
        Variance = A / (double)n - Moyenne * Moyenne;
        double SD = sqrt(Variance);

        //char file[255] ;
        //sprintf_s(file, _countof(file), "Moyenne =%.2f   SD = %.2f", Moyenne, SD);
        //::MessageBoxA(NULL, file, "getmaxhisto", MB_OK);

        //
        if ( Depth != -1 )
        {
            MMAX = (int)pow(2.0, Depth) ;
        }
        if ( !bIncludeMax ) MMAX -= (long)SD ; // sans le (long) pour la 37  - avec pour la 46
        for( i = (int)MMAX;  i >= 0; i--)
        {
            Sum += dest[i] ;
            if ( i <= MaxIndice && Sum > (double)(PourDixMil * MatInput.rows * MatInput.cols) / 10000.0 )
            {
                MaxWithoutZero = i ;
                break ;
            }
        }
        delete []dest ;

    //itoa(MaxWithoutZero, buf, 10);
    //::MessageBoxA(NULL, buf, "maxMaxWithoutZero", MB_OK | MB_ICONWARNING);

    }

    histImage.release();

        return MaxWithoutZero ;
}

long CImImage::GetEnergyMaxHisto(cv::Mat MatInput, long EcartementAutourPic, cv::Mat Mask, bool bWithoutZero  , bool bWithoutMax, short Depth )
{
    int i ;
    long MinIndice = 0;
    long MaxIndice = 0;
    long MinIndiceP = 0;
    long MaxIndiceP = 0;
    //long MaxWithoutZero = 0 ;
    long IndicePic = 0;
    double MaxValDest = 0 ;
    double Sum ;
    double Sumtotal ;
    long ResPourcent = 0;
//		char Buf[255] ;

    int histSize = 256;



    if ( MatInput.depth() == CV_8U) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

    MaxIndice = (int)pow(2.0, Depth)  ;
    if ( bWithoutZero) MinIndice =  (long)(MaxIndice * 0.1);
    if ( bWithoutMax ) MaxIndice = (long)((double)MaxIndice * 0.8) ;

    int hist_w = 1024; int hist_h = 800;
    //int bin_w = cvRound( (double) hist_w/histSize );

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( MatInput.type() == CV_8UC3 )
    {
        return -1 ;
    }
    if (MatInput.type() == CV_8UC1 )
    {
          return -1;
    }

    if (MatInput.type() == CV_16UC1)
    {
        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
        calcHist( &MatInput, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        float *dest = new float[histSize];

        Sum = 0.0 ;
        Sumtotal = 0.0 ;
        MaxValDest = 0 ;
        IndicePic = MinIndice ;

        for( i = (int)MinIndice ;  i < (int)MaxIndice; i++)
        {
            dest[i] = (float)(nb_hist.at<float>(i) ) ;
            Sumtotal += dest[i] ;
            if (  MaxValDest < dest[i] )
            {
                MaxValDest = dest[i] ;
                IndicePic = i ;
            }
        }

/*
    FILE * pFile;
    long lSize;
    unsigned char * buffer;

    _tfopen_s(&pFile,(LPCTSTR)_T("c:\\temp\\histogramme.csv"),_T("w"));

    for( i = 0; i < histSize; i++)
    {
            memset(Buf, 0, 50);
            sprintf_s(Buf, 50, "%i;%f\r",i,dest[i]);
            fwrite( Buf, sizeof( char ), 50, pFile );
    }
    fclose(pFile);
*/



        MinIndiceP = IndicePic - EcartementAutourPic ;
        MaxIndiceP = IndicePic + EcartementAutourPic ;

        if ( MinIndiceP < MinIndice) MinIndiceP = MinIndice ;
        if ( MaxIndiceP > MaxIndice) MaxIndiceP = MaxIndice ;

        for( i =  (int)MinIndiceP ;  i <= (int)MaxIndiceP; i++)
        {
            Sum += dest[i] ;
        }
        ResPourcent = (long)(100.0 * Sum / Sumtotal);

        delete []dest ;

    }

//		sprintf_s(Buf, 255, "Max %f (%ld) ==> %ld  ----- Min = %ld  Max = %ld \r",MaxValDest, IndicePic, ResPourcent, MinIndice,MaxIndice );
//		::MessageBoxA(NULL, Buf, "indice du pic", MB_OK | MB_ICONWARNING);

//	sprintf_s(Buf, _countof(Buf), _T("%d"), ResPourcent);
//	::MessageBoxA(NULL, Buf, "Energy", MB_OK | MB_ICONWARNING);

    return ResPourcent ;
}


void CImImage::Flip(cv::Mat &MatInput, int Mode)
{
    cv::flip(MatInput, MatInput, Mode);
}

bool CImImage::ConversionRGBtoGrey(cv::Mat MatInput, cv::Mat &MatOut )
{
    if (MatInput.type() != CV_8UC3)
    {
        qDebug() << "input MAT should be RGB type" ;
        return false;
    }
    if (MatOut.type() != CV_8U)
    {
        qDebug() << "ouput MAT should be GREY type" ;
        return false;
    }

    cv::cvtColor(MatInput, MatOut, cv::COLOR_RGB2GRAY);

    return true ;
}

bool CImImage::Conversion16to8Auto(cv::Mat MatInput, cv::Mat &/*MatOut*/ )
{
    if (MatInput.type() == CV_8UC3) return false;
    if (MatInput.type() == CV_8U) return false;

    qDebug() << "Conversion16to8Auto ne fait rien : à terminer" ;
/*
    ImageStatistique StatStart ;
    CImImage Img ;
    CAutoThresholdMethod cAutoThreshold ;

    // trouver la zone centrale
    cv::Mat MAtEtirement = MatInput.clone() ;
    cAutoThreshold.GetMaskPSPIX(MatInput, MAtEtirement, false, false);

    short dilation_size = 7;
    short Iteration = 11 ;

    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,
                            cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
                            cv::Point(dilation_size, dilation_size));

    cv::erode(MAtEtirement, MAtEtirement, element, cv::Point(-1, -1), Iteration);


//	Img.AfficherMat(MAtEtirement, "mask conversion");


    //
    StatStart.PourcentageMinCalcul = 15 ; // 1.5%
    StatStart.PourcentageMaxCalcul = 4 ; // 0.4%
    Img.CalcHistoMeanMedian(MatInput, &StatStart, MAtEtirement);
    long lNGMin = StatStart.MinCalcul ;
    long lNGMax = StatStart.MaxCalcul  ;

    if ( lNGMax < 60000 ) lNGMax = 60000 ;

    MatInput -= lNGMin ;
    MatInput.convertTo(MatOut, CV_8UC1, 255./(float)(lNGMax - lNGMin + 1));
//	Img.AfficherMat(MatOut, "output");
//	::MessageBoxA(NULL, "conversion", "imgimage", MB_OK | MB_ICONWARNING);
*/
    return true ;
}


void CImImage::AfficherMat(cv::Mat /*MatInput*/, std::string /*NameWindows*/)
{
#if defined(_WINDOWS)
    cv::namedWindow( NameWindows, cv::WINDOW_NORMAL);// Create a window for display.
    cv::imshow(NameWindows, MatInput );
    if ( MatInput.rows > 800 && MatInput.cols > 600)
        cv::resizeWindow(NameWindows, 600,800);
    else
        cv::resizeWindow(NameWindows, MatInput.cols, MatInput.rows);

#endif // _WINDOWS
}

void CImImage::CloseAfficherMat(std::string NameWindows)
{
    cv::destroyWindow(NameWindows );
}

void CImImage::ExporterMat(cv::Mat /*MatInput*/, std::string /*NameWindows*/)
{
#if defined(_WINDOWS)
    cv::imwrite(NameWindows, MatInput);
#endif // _WINDOWS
}



bool CImImage::GetBinaryImage(cv::Mat MatInput, cv::Mat &MatBin, long Seuil)
{
    MatBin = MatInput < Seuil ;

    return true ;
}

bool CImImage::GetNegativeImage(cv::Mat MatInput, cv::Mat &MatOut)
{
    cv::bitwise_not(MatInput, MatOut);

    return true ;
}


bool CImImage::GetProjectionX(cv::Mat MatInput, std::vector<long>&ListeMoyenne , long Y1, long Y2, short Plan, short LargeurLissage)
{
    if ( Y1 < 0 ) return false ;
    if ( Y1 > MatInput.rows ) return false ;
    if ( Y2 < 0 ) return false ;
    if ( Y2 > MatInput.rows ) return false ;
    if ( Y2 - Y1 <= 0) return false ;

    ////////////////////////////////////////////////////

    double mean ;
    double sum ;
    long NbPix = 0 ;
    int i , j, k ;
    cv::Vec3b v  ;

    ListeMoyenne.clear();

    if ( (LargeurLissage / 2) * 2 == LargeurLissage) LargeurLissage += 1 ; // LargeurLissage doit etre impair

    if ( MatInput.type() == CV_8UC3 )
    {
        if ( Plan < PLAN_BLUE || Plan > PLAN_RATIO_R_B  ) return false;
    //FILE * pFileDebug;
    //char Buf[50] ;
    //_tfopen_s(&pFileDebug,(LPCTSTR)_T("c:\\temp\\ProjectionY.csv"),_T("w"));
        if ( Plan >= PLAN_BLUE && Plan <= PLAN_RED  )
        {
            for ( j = 0 ; j < MatInput.cols; j++)
            {
                sum = 0 ;
                NbPix = 0 ;
                for ( k = j - LargeurLissage / 2 ; k < j + LargeurLissage/2 + 1; k++)
                {
                    if ( k >= 0 && k < MatInput.cols )
                    {
                        for ( i = (int)Y1 ; i <= (int)Y2; i++)
                        {
                            v = MatInput.at<cv::Vec3b>(i, k);
                            sum += (int)v[Plan];
                            NbPix ++ ;
                        }
                    }
                }
                mean = sum / NbPix;
                ListeMoyenne.push_back((long)mean);
            //memset(Buf, 0, 50);
            //sprintf_s(Buf, 50, "%f\r",sum);
            //fwrite( Buf, sizeof( char ), 50, pFileDebug );
            }
        }
        if ( Plan == PLAN_LUM  ) // luminance
        {
            cv::Mat gs_rgb(MatInput.size(), CV_8UC1);
            cv::cvtColor(MatInput, gs_rgb, cv::COLOR_RGB2GRAY); // CV_RGB2GRAY
//			AfficherMat(gs_rgb,"Luminance");
            for ( j = 0 ; j < gs_rgb.cols; j++)
            {
                sum = 0 ;
                NbPix = 0 ;
                for ( k = j - LargeurLissage / 2 ; k < j + LargeurLissage/2 + 1; k++)
                {
                    if ( k >= 0 && k < MatInput.cols )
                    {
                        for ( i = (int)Y1 ; i <= (int)Y2; i++)
                        {
                            sum += gs_rgb.at<uint8_t>(i, k);
                            NbPix ++ ;
                        }
                    }
                }
                mean = sum / NbPix;
                ListeMoyenne.push_back((long)mean);
            //memset(Buf, 0, 50);
            //sprintf_s(Buf, 50, "%f\r",sum);
            //fwrite( Buf, sizeof( char ), 50, pFileDebug );
            }
        }
    //fclose(pFileDebug);

    }
    if ( MatInput.type() == CV_16UC1 )
    {
    //FILE * pFileDebug;
    //char Buf[50] ;
    //_tfopen_s(&pFileDebug,(LPCTSTR)_T("c:\\temp\\ProjectionX.csv"),_T("w"));
    //try
    //{
        for ( j = 0 ; j < MatInput.cols; j++)
        {
            sum = 0 ;
            for ( i = (int)Y1 ; i <= (int)Y2 && i < MatInput.rows; i++)
            {
                sum += (MatInput.at<uint16_t>(i, j))/(double)(Y2 - Y1 + 1);
            }
            mean = sum  ;
            ListeMoyenne.push_back((long)mean);
            //memset(Buf, 0, 50);
            //sprintf_s(Buf, 50, "%f\r",mean);
            //fwrite( Buf, sizeof( char ), 50, pFileDebug );
        }
    //}
    //catch (std::exception& e)
    //{
    //	::MessageBoxA(NULL, e.what(), __FUNCTION__, MB_OK | MB_ICONWARNING);
    //	return NULL;
    //}

    //fclose(pFileDebug);
    }
    if ( MatInput.type() == CV_8UC1 )
    {
        for ( j = 0 ; j < MatInput.cols; j++)
        {
            sum = 0 ;
            for ( i = (int)Y1 ; i <= (int)Y2 && i < MatInput.rows; i++)
            {
                sum += MatInput.at<uint8_t>(i, j)/ (double)(Y2 - Y1 + 1);
            }
            mean = sum ;
            ListeMoyenne.push_back((long)mean);
        }
    }


    return true ;
}

bool CImImage::GetProjectionY(cv::Mat MatInput, std::vector<long>&ListeMoyenne , long X1, long X2)
{
    if ( X1 < 0 ) return false ;
    if ( X1 > MatInput.cols ) return false ;
    if ( X2 < 0 ) return false ;
    if ( X2 > MatInput.cols ) return false ;
    if ( X2 - X1 <= 0) return false ;
    ////////////////////////////////////////////////////

    double mean ;
    double sum ;
    int i , j ;

    ListeMoyenne.clear();
    for ( i = 0 ; i < MatInput.rows; i++)
    {
        sum = 0 ;
        for ( j = (int)X1 ; j <= (int)X2; j++)
        {
            sum += MatInput.at<uint16_t>(i, j);
        }
        mean = sum / (X2 - X1 + 1);
        ListeMoyenne.push_back((long)mean);
    }


    return true ;
}

bool CImImage::DisplayCentroidEx(cv::Mat MatInput, long &Cx, long &Cy, short Circle)
{
    int i , j ;
    double SommeImage = 0 ;
    double TCX ;
    double TCY ;
    long Temp ;
    // dilatation erosion
    short dilation_size = 1;
    short Iteration = 1 ;
    cv::Mat element  ;
    cv::Mat MAtMask ;

    //int Circle = TYP_CIRCLE; //
    //Circle = TYP_ELLIPSE; //


    element = cv::getStructuringElement(cv::MORPH_RECT,
                            cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
                            cv::Point(dilation_size, dilation_size));


    cv::Mat MatTemp = MatInput.clone();
    cv::Mat colorMat = cv::Mat(MatInput.size(), CV_8UC3);
    cv::cvtColor(MatInput, colorMat, cv::COLOR_GRAY2BGR); // CV_GRAY2BGR
     cv::Scalar color2 = cv::Scalar( 255, 0, 0 );
   cv::Scalar color3 = cv::Scalar( 0, 255, 0 );
   cv::Scalar color;

    Cx = MatInput.cols / 2 ;
    Cy = MatInput.rows / 2 ;

    double *SommeParLigne = new double[MatTemp.rows];
    double *SommeParCol = new double[MatTemp.cols];

    for ( i = 0 ; i < MatTemp.cols ; i++)
    {
        SommeParCol[i] = 0;
    }
    for ( j = 0 ; j < MatTemp.rows ; j++)
    {
        SommeParLigne[j] = 0;
    }


    // calcul centroid
    SommeImage = 0 ;
    for ( i = 0 ; i < MatTemp.rows ; i++)
    {
        for ( j = 0 ; j < MatTemp.cols ; j++)
        {
            Temp = MatInput.at<uint8_t>(i,j);
            SommeImage += Temp;
            SommeParCol[j] += Temp;
            SommeParLigne[i] += Temp;
        }
    }

    //FILE * pFileDebug;
    //char Buf[50] ;
    //_tfopen_s(&pFileDebug,(LPCTSTR)_T("c:\\temp\\centroidCol.csv"),_T("w"));
    //memset(Buf, 0, 50);

    //	sprintf_s(Buf, 50, "%f\r",SommeImage);
    //	fwrite( Buf, sizeof( char ), 50, pFileDebug );
    //for ( i = 0 ; i < MatInput.cols ; i++)
    //{
    //	sprintf_s(Buf, 50, "%f\r",SommeParCol[i]);
    //	fwrite( Buf, sizeof( char ), 50, pFileDebug );
    //}
    //fclose(pFileDebug);

    //_tfopen_s(&pFileDebug,(LPCTSTR)_T("c:\\temp\\centroidRow.csv"),_T("w"));
    //memset(Buf, 0, 50);
    //	sprintf_s(Buf, 50, "%f\r",SommeImage);
    //	fwrite( Buf, sizeof( char ), 50, pFileDebug );
    //for ( j = 0 ; j < MatInput.rows ; j++)
    //{
    //	sprintf_s(Buf, 50, "%f\r",SommeParLigne[j]);
    //	fwrite( Buf, sizeof( char ), 50, pFileDebug );
    //}
    //fclose(pFileDebug);

    TCX = 0.0 ;
    TCY = 0.0 ;
    for ( j = 0 ; j < MatTemp.cols ; j++)
    {
        TCX += j * SommeParCol[j] ;
    }
    TCX /= SommeImage ;
    Cx = (long)TCX ;

    for ( i = 0 ; i < MatTemp.rows ; i++)
    {
        TCY += i * SommeParLigne[i] ;
    }
    TCY /= SommeImage ;
    Cy = (long)TCY ;

    //
    delete [] SommeParLigne;
    delete [] SommeParCol;

    cv::line(MatTemp, cv::Point((int)(Cx - 10), (int)(Cy - 10 )), cv::Point((int)(Cx + 10),(int)( Cy + 10)), cv::Scalar( 255, 0, 0 ), 1,8);
    cv::line(MatTemp, cv::Point((int)(Cx + 10), (int)(Cy - 10 )), cv::Point((int)(Cx - 10),(int)( Cy + 10)), cv::Scalar( 255, 0, 0 ), 1,8);

    AfficherMat(MatTemp, "test");

    // lire la valeur à cette endroit
    long ValCentroid = MatInput.at<uint8_t>((int)Cy,(int)Cx);

    long ValSeuil = ValCentroid * 1 / 2 ; // 2 / 3


    for ( int Tour = 0 ; Tour < 2 ; Tour ++)
    {
        if ( Tour == 0 ) ValSeuil = ValCentroid * 1 / 2 ; // 2 / 3
        if ( Tour == 1 ) ValSeuil = (long)(ValCentroid * 0.13); // 2 / 3

        MAtMask = MatInput > ValSeuil ;

        Iteration = 1 ;
        cv::erode(MAtMask, MAtMask, element, cv::Point(-1, -1), Iteration);
        Iteration = 2 ;
        cv::dilate(MAtMask, MAtMask, element, cv::Point(-1, -1), Iteration);

        AfficherMat(MAtMask, "Mask");

       std::vector<std::vector<cv::Point> > contours;
       std::vector<cv::Vec4i> hierarchy;

       cv::findContours( MAtMask, contours, hierarchy, cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

       std::vector<std::vector<cv::Point> >hull( contours.size() );

       for( int k = 0; k < (int)contours.size(); k++ )
       {
              convexHull( cv::Mat(contours[k]), hull[k], false );
       }

       // garder le plus grand
        double MaxArea = 0;
        double area = 0;
        int indexRect = 0;
        for( int jj = 0; jj < (int)contours.size(); jj++ )
        {
            area = cv::contourArea(cv::Mat(contours[jj]));
            //
            if ( area > MaxArea)
            {
                MaxArea = area ;
                indexRect = jj;
            }
        }


       //
       cv::Mat drawing = cv::Mat::zeros( MatInput.size(), MatInput.type() );
        std::vector<cv::RotatedRect> vMinRect( contours.size() );
       for( int jj = 0; jj< (int)contours.size(); jj++ )
          {
              vMinRect[jj] = cv::minAreaRect( cv::Mat(contours[jj]) );
              if ( jj == indexRect )
              {
                    cv::Scalar color1 = cv::Scalar( 255, 0, 0 );
                    drawContours( drawing, contours, jj, color1, 1, 8, std::vector<cv::Vec4i>(), 0, cv::Point() );

                cv::Point2f vertices[4];
                vMinRect[j].points(vertices);
    //			drawContours( colorMat, contours, i, color2, 1 , 8, cv::vector<cv::Vec4i>(), 0, cv::Point() );
                for ( long k = 0 ; k < 4 ; k++)	line(drawing, vertices[k], vertices[(k+1)%4], cv::Scalar(255,255,255),1,8);
                line(drawing, vertices[0], vertices[2], cv::Scalar(255,255,255),1,8);
                line(drawing, vertices[1], vertices[3], cv::Scalar(255,255,255),1,8);
              }
         }

        AfficherMat(drawing, "drawing");



          std::vector<cv::Point2f>center( contours.size() );
          std::vector<float>radius( contours.size() );
        ///////////////////////////////////////////////////////////////

        if ( Circle == TYP_CIRCLE )
        {
           cv::minEnclosingCircle(contours[indexRect],center[indexRect],radius[indexRect]);
            if ( Tour == 0) color = color2 ;
            if ( Tour == 1) color = color3 ;
            circle( colorMat, center[indexRect], (int)radius[indexRect], color, 1, 8, 0 );
            line(colorMat, cv::Point((int)(center[indexRect].x -radius[indexRect]), (int)(center[indexRect].y)  ),
                           cv::Point((int)(center[indexRect].x +radius[indexRect]), (int)(center[indexRect].y) ),
                           color,1,8);
            line(colorMat, cv::Point((int)(center[indexRect].x) , (int)(center[indexRect].y -radius[indexRect]) ),
                           cv::Point((int)(center[indexRect].x) , (int)(center[indexRect].y +radius[indexRect])),
                           color,1,8);
        }
        if ( Circle == TYP_ELLIPSE )
        {
            cv::RotatedRect RotRect = cv::fitEllipse(contours[indexRect]);
            if ( Tour == 0) color = color2 ;
            if ( Tour == 1) color = color3 ;
            cv::ellipse(colorMat, RotRect, color, 1,8);
            circle( colorMat, center[indexRect], (int)radius[indexRect], color, 1, 8, 0 );
            line(colorMat, cv::Point((int)(RotRect.center.x -100), (int)(RotRect.center.y)  ),
                           cv::Point((int)(RotRect.center.x +100), (int)(RotRect.center.y) ),
                           color,1,8);
            line(colorMat, cv::Point((int)(RotRect.center.x) , (int)(RotRect.center.y -100) ),
                           cv::Point((int)(RotRect.center.x) , (int)(RotRect.center.y +100)),
                           color,1,8);
        }

//centre,axes,angle = cv2.fitEllipse(c)
//MAJ = np.argmax(axes) # this is MAJor axis, 1 or 0
//MIN = 1-MAJ # 0 or 1, minor axis
//# Note: axes length is 2*radius in that dimension
//MajorAxisLength = axes[MAJ]
//MinorAxisLength = axes[MIN]
//Eccentricity    = np.sqrt(1-(axes[MIN]/axes[MAJ])**2)
//Orientation     = angle
//EllipseCentre   = centre # x,y


         //  for( i = 0; i < (int)contours.size(); i++ )
            //{
            //	area = cv::contourArea(cv::Mat(contours[i]));
            //	if(area > 100 )
            //	{
            //		cv::Point2f vertices[4];
            //		vMinRect[i].points(vertices);
            //		drawContours( colorMat, contours, i, color2, 1 , 8, cv::vector<cv::Vec4i>(), 0, cv::Point() );
            //		for ( long j = 0 ; j < 4 ; j++)	line(colorMat, vertices[j], vertices[(j+1)%4], cv::Scalar(0,255,255));
            //	}
            //}
        //	cv::rectangle(colorMat, vMinRect[indexRect], color3, 2,8);
            AfficherMat(colorMat,"rescolor_rot");
        ///////////////////////////////////////////////////////////////
    }



    return true ;
}



bool CImImage::DisplayCentroid(cv::Mat MatInput)
{

    // remplir les trou
   std::vector<std::vector<cv::Point> > contours;
   std::vector<cv::Vec4i> hierarchy;
    AfficherMat(MatInput, "Bin");
  cv::findContours( MatInput, contours, hierarchy,cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

  std::vector<std::vector<cv::Point> >hull( contours.size() );
   for( int i = 0; i < (int)contours.size(); i++ )
   {
          convexHull( cv::Mat(contours[i]), hull[i], false );
   }

   // garder le plus grand
    double MaxArea = 0;
    double area = 0;
    int indexRect = 0;
    for( int i = 0; i < (int)contours.size(); i++ )
    {
        area = cv::contourArea(cv::Mat(contours[i]));
        //
        if ( area > MaxArea)
        {
            MaxArea = area ;
            indexRect = i;
        }
    }


   //
   cv::Mat drawing = cv::Mat::zeros( MatInput.size(), MatInput.type() );
    std::vector<cv::RotatedRect> vMinRect( contours.size() );
   for( int i = 0; i< (int)contours.size(); i++ )
      {
          vMinRect[i] = cv::minAreaRect( cv::Mat(contours[i]) );
          if ( i == indexRect )
          {
                cv::Scalar color1 = cv::Scalar( 255, 0, 0 );
                drawContours( drawing, contours, i, color1, 1, 8, std::vector<cv::Vec4i>(), 0, cv::Point() );

            cv::Point2f vertices[4];
            vMinRect[i].points(vertices);
//			drawContours( colorMat, contours, i, color2, 1 , 8, cv::vector<cv::Vec4i>(), 0, cv::Point() );
            for ( long j = 0 ; j < 4 ; j++)	line(drawing, vertices[j], vertices[(j+1)%4], cv::Scalar(255,255,255),1,8);
            line(drawing, vertices[0], vertices[2], cv::Scalar(255,255,255),1,8);
            line(drawing, vertices[1], vertices[3], cv::Scalar(255,255,255),1,8);
          }
     }


    //// Get the moments
    std::vector<cv::Moments> mu(contours.size() );
    //for( int i = 0; i < contours.size(); i++ )
    //{
    //	mu[i] = cv::moments( contours[i], false );
    //}

    ////  Get the mass centers:
    std::vector<cv::Point2f> mc( contours.size() );
    //for( int i = 0; i < contours.size(); i++ )
    //{
    //	mc[i] = cv::Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
    //}

    mu[0] = cv::moments( contours[indexRect], false );
    mc[0] = cv::Point2f( (float)(mu[0].m10/mu[0].m00), (float)(mu[0].m01/mu[0].m00));

    MatInput = drawing.clone() ;
    cv::line(MatInput, cv::Point((int)(mc[0].x - 100), (int)(mc[0].y - 100) ), cv::Point((int)(mc[0].x + 100), (int)(mc[0].y + 100)), cv::Scalar( 255, 0, 0 ), 1,8);
    cv::line(MatInput, cv::Point((int)(mc[0].x + 100), (int)(mc[0].y - 100) ), cv::Point((int)(mc[0].x - 100), (int)(mc[0].y + 100)), cv::Scalar( 255, 0, 0 ), 1,8);

//	drawContours( drawing, mc, 0, cv::Scalar( 255, 0, 0 ), 1, 8, cv::vector<cv::Vec4i>(), 0, cv::Point() );
    AfficherMat(MatInput, "test");

    return true ;
}


bool CImImage::GetProjectionX(cv::Mat MatInput, cv::Mat &MatOut , long /*Y1*/, long /*Y2*/, long /*ValMin*/ , long /*ValMax*/ )
{
    cv::reduce(MatInput, MatOut, 1,cv::REDUCE_SUM);

    return true ;
}

bool CImImage::GetProjectionY(cv::Mat MatInput, cv::Mat &MatOut , long X1, long X2, long ValMin , long ValMax )
{
    //AfficherMat(MatInput,"init vv");
    //cv::reduce(MatInput, MatOut, 0, CV_REDUCE_AVG, -1);
    //AfficherMat(MatOut,"Projection moyenne Y brute vv");

    if ( X1 < 0 ) return false ;
    if ( X1 > MatInput.cols ) return false ;
    if ( X2 < 0 ) return false ;
    if ( X2 > MatInput.cols ) return false ;
    if ( X2 - X1 <= 0) return false ;
    ////////////////////////////////////////////////////

    double mean ;
    double sum ;
    long NBPixPerRow ;
    long NG ;
    int i , j ;

    for ( i = 0 ; i < MatInput.rows; i++)
    {
        sum = 0 ;
        NBPixPerRow = 0 ;
        for ( j = (int)X1 ; j <= (int)X2; j++)
        {
            NG = MatInput.at<uint16_t>(i, j);
            if ( NG > ValMin && NG < ValMax )
            {
                sum += NG;
                NBPixPerRow ++ ;
            }
        }
        if ( NBPixPerRow != 0)
            mean = sum / NBPixPerRow;
        else
            mean = 0 ;
        MatOut.at<uint16_t>(i, 0) = (uint16_t)mean;
    }


    return true ;
}


long CImImage::GetHisto(cv::Mat MatInput, float* HistoDest, cv::Mat Mask)
{
    // get histogram
    int i ;
    //long MaxIndice = 0;
    //long MaxWithoutZero = 0 ;
    //double Sum ;

    int histSize = 256;

    if ( MatInput.depth() == CV_8UC1) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

//    if ( MatInput.depth() == CV_8UC1) MaxIndice = 255 ;
//    if ( MatInput.depth() == CV_16U) MaxIndice = 65535 ;


     int hist_w = 1024; int hist_h = 800;

    cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

    if ( MatInput.type() == CV_8UC3 )
    {
        return -1 ;
    }

    if (MatInput.type() == CV_16UC1 || MatInput.type() == CV_8UC1)
    {
        float range[] = {0,(float)histSize};
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat nb_hist;

          /// Compute the histograms:
        calcHist( &MatInput, 1, 0, Mask, nb_hist, 1, &histSize, &histRange, uniform, accumulate );

        //Sum = 0.0 ;
        //long MMAX = histSize - 1 ;

        //double sumMoyenne = 0.0 ;
        //double A = 0.0;
        //double Variance = 0.0 ;
        //double n = 0;
        for( i = 0;  i < histSize; i++)
        {
            HistoDest[i] = (float)(nb_hist.at<float>(i) ) ;
        }

    //itoa(MaxWithoutZero, buf, 10);
    //::MessageBoxA(NULL, buf, "maxMaxWithoutZero", MB_OK | MB_ICONWARNING);

    }

    histImage.release();

    return histSize ;
}

long CImImage::CalculateFuzzyHisto(cv::Mat /*MatInput*/, float* /*HistoDest*/, float* /*FuzzyHistoDest*/, cv::Mat /*Mask*/)
{
    // transformation en mode fuzzy


    return 0 ;

}


long CImImage::GetFussyHisto(cv::Mat MatInput, std::vector<float> /*HistoFussy*/, cv::Mat /*Mask*/)
{
    int histSize = 256;

    if ( MatInput.depth() == CV_8UC1) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_8UC3) histSize = (int)pow(2.0, 8)  ;
    if ( MatInput.depth() == CV_16U) histSize = (int)pow(2.0, 16) ;

    float *HistoDest = new float[histSize];
    float *FuzzyHistoDest = new float[histSize];

    GetHisto(MatInput, HistoDest, cv::Mat());
    // fuzzy histogramme
    CalculateFuzzyHisto(MatInput, HistoDest, FuzzyHistoDest, cv::Mat());


    //
    delete []HistoDest ;
    delete []FuzzyHistoDest ;

    return 0 ;
}


