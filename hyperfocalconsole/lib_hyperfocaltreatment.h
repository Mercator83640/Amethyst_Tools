#ifndef LIB_HYPERFOCALTREATMENT_H
#define LIB_HYPERFOCALTREATMENT_H

#include "Lib_HyperFocalTreatment_global.h"
#include <opencv2/core.hpp>
#include <QString>
#include "focusstack.hh"
#include "qsize.h"

class LIB_HYPERFOCALTREATMENT_EXPORT Lib_HyperFocalTreatment
{
public:
    Lib_HyperFocalTreatment();
    QString GetVersion();
    QSize m_inputSize;

    void InitFocusStackOption(QString Output);
    void PushImageRaw(const cv::Mat &RawImage);

    focusstack::FocusStack stack;
    cv::Mat HyperFocusRun();
    void PushImageRaw(QString PathImage);

    void PushListImageRaw(std::vector<QString> ListImage);
    void HyperFocusStart();
    void HyperFocusFinalMerge();
    bool HyperFocusWaitDone();
    cv::Mat HyperFocusGetResult();
    int ConsistencyFactor() const;
    void setConsistencyFactor(int newConsistencyFactor);

    float DenoisingFactor() const;
    void setDenoisingFactor(float newDenoisingFactor);

private :
    bool bImagebyName ;

    int m_ConsistencyFactor ;
    float m_DenoisingFactor ;
};

#endif // LIB_HYPERFOCALTREATMENT_H
