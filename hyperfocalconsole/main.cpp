#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QStringList>

#include <opencv2/opencv.hpp>

#include "cimagefilter.h"

#include <QDir>

#include <windows.h>
#include <fstream>


void DumpLoadedDLL()
{
    HMODULE h = GetModuleHandleA("Lib_HyperFocalTreatment.dll");

    std::ofstream f("C:/Users/Public/HF_DLL_PATH.txt", std::ios::app);

    if (h)
    {
        char path[MAX_PATH] = {0};
        GetModuleFileNameA(h, path, MAX_PATH);

        f << "DLL LOADED = " << path << "\n";
    }
    else
    {
        f << "DLL NOT FOUND\n";
    }

    f.close();
}

static bool loadImage(const QString& path, cv::Mat& img)
{
    QFileInfo fi(path);

    qDebug() << "Load image:" << path
             << "exists=" << fi.exists()
             << "size=" << fi.size();

    if (!fi.exists())
        return false;

    img = cv::imread(path.toStdString(), cv::IMREAD_UNCHANGED);

    qDebug() << "Loaded:"
             << "empty=" << img.empty()
             << "rows=" << img.rows
             << "cols=" << img.cols
             << "type=" << img.type();

    return !img.empty();
}

int main(int argc, char *argv[])
{

    {
        std::ofstream f("C:/Users/Public/HF_MAIN_MARKER.txt", std::ios::app);
        f << "MAIN ENTER\n";
        f.flush();
    }

//    MessageBoxA(nullptr, "HF TEST EXE RUNNING", "HF TEST", MB_OK);

    char exePath[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);

    std::ofstream f("C:/Users/Public/HF_MAIN_MARKER.txt", std::ios::app);
    f << "EXE = " << exePath << "\n";
    f.flush();


    HMODULE h = GetModuleHandleA("Lib_HyperFocalTreatment.dll");
    if (h)
    {
        char dllPath[MAX_PATH] = {0};
        GetModuleFileNameA(h, dllPath, MAX_PATH);

        std::ofstream f("C:/Users/Public/HF_DLL_MARKER.txt", std::ios::app);
        f << "DLL = " << dllPath << "\n";
        f.flush();
    }
    else
    {
        std::ofstream f("C:/Users/Public/HF_DLL_MARKER.txt", std::ios::app);
        f << "DLL NOT LOADED\n";
        f.flush();
    }

    QCoreApplication app(argc, argv);

//    cv::ocl::setUseOpenCL(false);

    qDebug() << "OpenCL available =" << cv::ocl::haveOpenCL();
    qDebug() << "OpenCL use =" << cv::ocl::useOpenCL();


    QStringList args = QCoreApplication::arguments();

    if (args.size() != 7)
    {
        qDebug() << "Usage:";
        qDebug() << args[0] << "img0 img1 img2 img3 img4 outputHF.jpg";
        return 1;
    }

    std::vector<cv::Mat> stack;
    stack.reserve(5);

    for (int i = 1; i <= 5; ++i)
    {
        cv::Mat img;

        if (!loadImage(args[i], img))
        {
            qDebug() << "Failed loading image:" << args[i];
            return 2;
        }

        stack.push_back(img);
    }

    QString outputPath = args[6];

    qDebug() << "Output HF path =" << outputPath;

    DumpLoadedDLL();

    FOCUS_INFO methodFocus;
    methodFocus.m_method      = FOCUS_PROJ_WAVELET_LIB;
    methodFocus.m_NameOutput  = outputPath;
    methodFocus.m_tileWidth   = 32;
    methodFocus.m_tileHeight  = 32;
    methodFocus.m_bSharpen    = false;
    methodFocus.m_uniform     = 0;
    methodFocus.m_taille      = 0;
    methodFocus.m_resolution  = 0;
    methodFocus.m_threshold   = 1;
    methodFocus.m_diffMin     = 1;
    methodFocus.m_bGomme      = false;

    for (int i = 0; i <4; ++i)
    {
        qDebug() << "================ RUN" << i << "================";

        cv::Mat result = stack.front().clone();

        CImageFilter focus(methodFocus);
        focus.setMatInputStack(stack);
        focus.setMatOuput(&result);

        // qDebug() << "OpenCL available =" << cv::ocl::haveOpenCL();
        // qDebug() << "OpenCL use before________ =" << cv::ocl::useOpenCL();
        // cv::ocl::setUseOpenCL(false);
        // qDebug() << "OpenCL use after ______=" << cv::ocl::useOpenCL();

        // cv::setNumThreads(1);
        // qDebug() << "OpenCV threads =" << cv::getNumThreads();

        qDebug() << "Before focusProjectionXY";
        focus.focusProjectionXY();
        qDebug() << "After focusProjectionXY";

        qDebug() << "Current dir =" << QDir::currentPath();
        qDebug() << "Temp path =" << QDir::tempPath();
        qDebug() << "Output path =" << outputPath;
        qDebug() << "CPU arch =" << QSysInfo::currentCpuArchitecture();
        qDebug() << "OS =" << QSysInfo::prettyProductName();

        QFile f("c:/temp/hf_write_test.txt");
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            f.write("ok\n");
            f.close();
            qDebug() << "Write test OK";
        }
        else
        {
            qDebug() << "Write test FAILED";
        }

        QFileInfo fi(outputPath);
        qDebug() << "Output exists=" << fi.exists()
                 << "size=" << fi.size();

        if (!result.empty())
            cv::imwrite(outputPath.toStdString(), result);
    }

    return 0;
}
