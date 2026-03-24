#ifndef CIMAGEFILTER_H
#define CIMAGEFILTER_H

#include <QObject>
#include "opencv2/opencv.hpp"

#define FOCUS_PROJ_MAX 0
#define FOCUS_PROJ_TILE 1
#define FOCUS_PROJ_GRADIENT 2
#define FOCUS_PROJ_LAP 3
#define FOCUS_PROJ_MERTENS 4
#define FOCUS_PROJ_WAVELET 5
#define FOCUS_PROJ_WAVELET_LIB 6

#define AWB_GWT   0
#define AWB_RM    1
#define AWB_GWR   2
#define AWB_SDWGW 3

#define AUTOGAMMA_PICHIST 0
#define AUTOGAMMA_MEANHIST 1

#define IMAGE_JPEG2000 0
#define IMAGE_JPEG 1
#define IMAGE_TIFF 2

#define CUPULETEST_NONTESTE 0
#define CUPULETEST_EXISTE 1
#define CUPULETEST_NEXISTEPAS 2

#define TYP_CIRCLE 1
#define TYP_ELLIPSE 2

#define FLIP_HORIZONTAL 0
#define FLIP_VERTICAL 1


#define PI 3.14159265359

#define PLAN_BLUE 0
#define PLAN_GREEN 1
#define PLAN_RED 2
#define PLAN_LUM 3
#define PLAN_RATIO_R_B 4

struct FOCUS_INFO
{
short			m_method;
long			m_tileWidth;
long			m_tileHeight;
bool			m_bSharpen;
bool			m_uniform;		//wavelet & gradient
unsigned short	m_taille;		//laplacian - wavelet1
unsigned short	m_resolution;	//wavelet & gradient
double			m_threshold;	//laplacian
long			m_diffMin ; //wavelet1
bool			m_bGomme ; // wavelet1
QString         m_NameOutput ;
};

struct ImageStatistique
{
    long Mean;
    long Median;
    long MinPicWithoutZero;
    long MaxWithoutMaxRange;
    long MinWithoutZero;
    long MaxPicWithoutZero ;
    long EcartType ;
    float Entropy ;
    long Mode ;
    long MoyenneD;
    long MoyenneQ1;
    long MoyenneQ2;
    long MoyenneQ3;
    long PourcentQ1;
    long PourcentQ2;
    long PourcentQ3;
    long PourcentPicQ1;
    long PourcentPicQ2;
    long PourcentPicQ3;
    long indicePicQ1;
    long indicePicQ2;
    long indicePicQ3;
    long PourcentageMaxCalcul ; // par défault 0
    long PourcentageMinCalcul ; // par défault 0
    long MaxCalcul ; // calcul à partir du cumul des valeurs > PourcentageMaxCalcul
    long MinCalcul ; // calcul à partir du cumul des valeurs > PourcentageMinCalcul
    long PicMaxInRange;
    long Range_Min;
    long Range_Max;
};



class CImageFilter : public QObject
{
    Q_OBJECT
public:
    CImageFilter();
    CImageFilter(FOCUS_INFO pMethod);
    ~CImageFilter(void);


    const FOCUS_INFO &pMethod() const;
    void setPMethod(const FOCUS_INFO &newPMethod);

    void setMatInputStack(std::vector<cv::Mat> newMatInputStack);

    void setMatOuput(cv::Mat *newMatOuput);


    cv::Mat *getMatOuput() const;

private :
    FOCUS_INFO m_pMethod ;
    void focusProjectionXY_Max();
    void focusProjectionXY_Tile();
    void focusProjectionXY_Gradient();
    void focusProjectionXY_Laplacien();

    cv::Mat GetKernelGradient(int direction);

    std::vector<cv::Mat> MatInputStack ;
    cv::Mat *MatOuput ;

    void StopProcessUser();
    void GetCaracteristiqueImage(cv::Mat imMat);
    void focusProjectionXY_Mertens();
    void focusProjectionXY_Wavelet();



    // Wavelet part
    int m_refidx ;
    cv::Mat m_refcolor;
    cv::Mat m_refgray;
    cv::Mat m_guo ;
    cv::Mat m_gauss_mean ;
    cv::Mat m_gauss_dev ;
    cv::Mat  m_gauss_amp;

    cv::Mat m_result ;
    std::vector<cv::Mat> ResultStack ;
    std::vector<cv::Mat> GrayMatInputStack ;
    std::vector<cv::Mat> AlignedMatInputStack ;
    std::vector<cv::Mat> AlignedGrayMatInputStack ;

    std::vector<cv::Mat> WMerge_batchStack;
    std::vector<cv::Mat> WFocusMeasureStack;
    cv::Mat m_depthmap ;
    int m_MaxDetph ;
    int m_consistency ;
    std::unordered_map<int, cv::Mat* > m_index_map;
    std::vector<cv::Rect> m_valid_area_Stack;

    std::vector<cv::Size> m_orig_size_Stack;



public slots :
    void focusProjectionXY();

signals :
    void focusFinished();
protected:
    void focusProjectionXY_Wavelet_Lib();
};

class CImImage
{
public:
    CImImage(void);
    ~CImImage(void);

    void CalcHistoMeanMedianBC(cv::Mat MatInput, ImageStatistique *pStat, cv::Mat Mask);
    void CalcHistoMeanMedian(cv::Mat MatInput, ImageStatistique *pStat, cv::Mat Mask);

    long GetNGLowestHisto(cv::Mat MatInput, cv::Mat Mask, long BorneMin = 0, long BorneMax = 65535 );
    long GetPicHisto256(cv::Mat MatInput, cv::Mat Mask, long BorneMin = 0, long BorneMax = 65535, long *PicPourcent = NULL );
    long GetFirstPicHisto256FromBottom(cv::Mat MatInput, cv::Mat Mask, long BorneMin = 0, long BorneMax = 65535 );
    long GetPicHisto(cv::Mat MatInput, cv::Mat Mask, long BorneMin = 0, long BorneMax = 65535, long *PicPourcent = NULL );
    long GetMinHisto(cv::Mat MatInput, long PourDixMil, cv::Mat Mask, bool bHorzZero);
    long GetMaxHisto(cv::Mat MatInput, long PourDixMil, cv::Mat Mask, bool bHorzMax,bool bIncludeMax = true, long Depth = -1 );
    long GetMeanHisto(cv::Mat MatInput, cv::Mat Mask );
    bool GetMeanSDHisto(cv::Mat MatInput, cv::Mat Mask, long *Mean, double *SD ) ;
    long GetEnergyMaxHisto(cv::Mat MatInput, long EcartementAutourPic, cv::Mat Mask, bool bWithoutZero = true , bool bWithoutMax = false, short Depth = 16) ;
    long GetPourCentOverHisto256(cv::Mat MatInput, cv::Mat Mask, long Borne);
    float GetEntropyHisto(cv::Mat MatInput, cv::Mat Mask) ;
    float GetContrasteHisto(cv::Mat MatInput, cv::Mat Mask) ;
    float GetContrasteEdgeHisto(cv::Mat MatInput, cv::Mat Mask) ;

    bool Conversion16to8Auto(cv::Mat MatInput, cv::Mat &MatOut ) ;
    void AfficherMat(cv::Mat MatInput, std::string NameWindows);
    void CloseAfficherMat(std::string NameWindows);
    void ExporterMat(cv::Mat MatInput, std::string NameWindows);
    bool GetBinaryImage(cv::Mat MatInput, cv::Mat &MatBin, long Seuil) ;
    bool GetNegativeImage(cv::Mat MatInput, cv::Mat &MatOut);

    bool GetProjectionX(cv::Mat MatInput, std::vector<long>&ListeMoyenne , long Y1, long Y2, short Plan, short LargeurLissage = 0);
    bool GetProjectionY(cv::Mat MatInput, std::vector<long>&ListeMoyenne , long X1, long X2);


    bool GetProjectionX(cv::Mat MatInput, cv::Mat &MatOut , long Y1, long Y2, long ValMin = 0, long ValMax = 65535);
    bool GetProjectionY(cv::Mat MatInput, cv::Mat &MatOut , long X1, long X2, long ValMin = 0, long ValMax = 65535);



    bool DisplayCentroid(cv::Mat MatInput);
    bool DisplayCentroidEx(cv::Mat MatInput, long &Cx, long &Cy, short mForm);
    void Flip(cv::Mat &MatInput, int Mode = 0) ;

    long CalculateFuzzyHisto(cv::Mat MatInput, float* HistoDest, float* FuzzyHistoDest, cv::Mat Mask);
    long GetHisto(cv::Mat MatInput, float* HistoDest, cv::Mat Mask);
    long GetFussyHisto(cv::Mat MatInput, std::vector<float> HistoFussy, cv::Mat Mask);


    void Release()
    {
        delete this;
    } ;
    bool ConversionRGBtoGrey(cv::Mat MatInput, cv::Mat &MatOut);
};


class CContrast
{
public:
    CContrast(void);
    ~CContrast(void);

    unsigned int  m_BBHE_nLowerTotalNumber;
    unsigned int  m_BBHE_nUpperTotalNumber;
    unsigned int  m_BBHE_nImageWidth;
    unsigned int  m_BBHE_nImageHeight;
    unsigned long m_BBHE_lImageSize;
    unsigned long m_BBHE_byBrightnessMean;
    unsigned long m_BBHE_byDecomposeTh;
    unsigned long m_BBHE_byBrightnessMin;
    unsigned long m_BBHE_byBrightnessMax;


    void AutoGamma(cv::Mat MatInput, cv::Mat &MatOut, bool boucle );
    void AutoGamma2(cv::Mat MatInput, cv::Mat &MatOut, bool boucle );

    double Get_AutoGamma4(cv::Mat MatInput, cv::Mat &MatOut, float FacteurGamma , float MaxGamma, float MinGamma );
    double Get_AutoGamma5(cv::Mat MatInput, cv::Mat &MatOut, long Moyenne, cv::Mat Mask );
    double Get_AutoGamma6(cv::Mat MatInput, cv::Mat &MatOut, long PourCentQ1, cv::Mat Mask ) ;
    double Get_AutoGammaQ2(cv::Mat MatInput, cv::Mat &MatOut, long PourCentQ2, cv::Mat Mask ) ;
    double Get_AutoGammaQ2Centre(cv::Mat MatInput, cv::Mat &MatOut, long CentreQ2, cv::Mat Mask ) ;

    double Get_AutoGamma16(cv::Mat MatInput, cv::Mat &MatOut, long Moyenne, cv::Mat Mask );
    double Get_AutoGamma256(cv::Mat MatInput, cv::Mat &MatOut, long Moyenne, cv::Mat Mask, short Method );


    double AutoGammaHRD(cv::Mat MatInput, cv::Mat &MatOut , short NbPas, double dPas );

    void Equalization(cv::Mat MatInput, cv::Mat &MatOut, long Min = 0, long Max = 65535, short Methode = 0);
    bool KernelGaussian5x5(cv::Mat MatInput, cv::Mat &MatOut);
    bool Sharpen5x5b(cv::Mat MatInput, cv::Mat &MatOut);
    bool Sharpen5x5(cv::Mat MatInput, cv::Mat &MatOut);
    bool Sharpen3x3(cv::Mat MatInput, cv::Mat &MatOut);
    bool UnsharpenMask(cv::Mat MatInput, cv::Mat &MatOut, float sigma, float FFact)  ;
    bool UnsharpenMaskEx(cv::Mat MatInput, cv::Mat &MatOut, float sigma, float FFact)  ;

    bool SIF_ApplyGamma(cv::Mat MatInput, cv::Mat &MatOuput, double Gamma);
    bool SIF_ApplyGammaImageJ(cv::Mat MatInput, cv::Mat &MatOuput, double Gamma);

    bool Autohisto4(cv::Mat MatInput, cv::Mat &MatOut, long Min, long Max);
    bool Autohisto2(cv::Mat MatInput, cv::Mat &MatOut, long Min, long Max, float PourcentageHaut );
    bool Autohisto1_Plaque(cv::Mat MatInput, cv::Mat &MatOut, float PourcentageHaut) ;
    bool Autohisto3_Plaque(cv::Mat MatInput, cv::Mat &MatOut, float PourcentageHaut , float CoeffMax ) ;

    void MelangeImagesMasque( cv::Mat Mat1, cv::Mat Mat2, cv::Mat &MatOut , cv::Mat Mask);
    void MelangeImagesMasqueMax( cv::Mat Mat1, cv::Mat Mat2, cv::Mat &MatOut , cv::Mat Mask) ;

    bool AdjustContrast(cv::Mat MatInput, cv::Mat &MatOut , short Gain, double Cutoff) ;
    bool AdjustContrast(cv::Mat MatInput, cv::Mat &MatOut , double Gain, double Cutoff) ;
    bool AdjustContrastEx(cv::Mat MatInput, cv::Mat &MatOut , double Gain, double CutoffPlus) ;
    bool Normalize(cv::Mat MatInput, cv::Mat &MatOut , cv::Mat Mask, short Method = cv::NORM_MINMAX, double Facteur = 0.0) ;

    long Unsharpen(cv::Mat MatInput, cv::Mat &MatOut, long ValMask, float CoeffGaussien, float CoeffMultG, float CoeffMultFinal, float Decalage);
    long Unsharpen_1(cv::Mat MatInput, cv::Mat &MatOut);
    bool RehaussementContour(cv::Mat MatInput, cv::Mat &MatOut, float SigmaGB, short ShiftPixel, float FactMul, short Etape);

    bool LutLineaire(cv::Mat MatInput, cv::Mat &MatOut, long Min, long Max, bool bAuto, long MinOut = 0, long MaxOut = 65535);

    bool UnderWaterRehaussement(cv::Mat MatInput, cv::Mat &MatOut);

    bool AddRandomNoise(cv::Mat &MatInput);

    void GetEnhanceResult(cv::Mat MatInput, cv::Mat &MatOut, double lfCdf[]);
    void ComputeCDF(double lfCdf[],float histogram[], long HistoSize);
    void DecomposeImage(cv::Mat MatInput, long  byDecomposTh);
    void GetImageBrightnessPara( cv::Mat MatInput);
    bool BBHE(cv::Mat MatInput, cv::Mat &MatOut);



    void Release()
    {
        delete this;
    } ;

private:
    uint16_t GetPixelValue(cv::Mat MatInput, int Row, int Col) ;

};



#endif // CIMAGEFILTER_H


