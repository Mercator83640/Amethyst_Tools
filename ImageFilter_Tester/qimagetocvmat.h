#ifndef QIMAGETOCVMAT_H
#define QIMAGETOCVMAT_H

#include "opencv2/core/hal/interface.h"
#include <QDebug>

inline QImage  cvMatToQImage( const cv::Mat &inMat )
{
   switch ( inMat.type() )
   {
      // 8-bit, 4 channel
      case CV_8UC4:
      {
         QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_ARGB32 );
   //      qDebug() << "CV_8UC4 ==> QImage::Format_ARGB32" ;

         return image;
      }

      // 8-bit, 3 channel
      case CV_8UC3:
      {
         QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB888 );
  //       qDebug() << "CV_8UC3 ==> QImage::Format_RGB32" ;

         return image.rgbSwapped();
      }



      // 8-bit, 1 channel
      case CV_8UC1:
      {
         static QVector<QRgb>  sColorTable;

//         // only create our color table once
         if ( sColorTable.isEmpty() )
         {
            for ( int i = 0; i < 256; ++i )
               sColorTable.push_back( qRgb( i, i, i ) );
         }

         QImage image = QImage( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_Indexed8 ).copy();

         image.setColorTable( sColorTable );
  //       qDebug() << "CV_8UC1 ==> QImage::Format_Indexed8" ;

         return image;
//           return QImage(inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_Grayscale8).copy();
      }

       case CV_16UC1:
       {
//            qDebug() << " convert en 8 bits";
   //    qDebug() << "CV_16UC1 ==> QImage::Format_Grayscale8" ;
            cv::Mat Mat8b ;
   //         qDebug() << "CV_16UC1 ==> cv::Mat::zeros ...." ;
            Mat8b = cv::Mat::zeros( inMat.rows,inMat.cols,CV_8UC1) ;
   //         qDebug() << "CV_16UC1 ==> convert to ...." ;
            inMat.convertTo(Mat8b, CV_8UC1, 1.0/255.0);
   //         qDebug() << "CV_16UC1 ==> return QImage..." ;
            return QImage(Mat8b.data, Mat8b.cols, Mat8b.rows, Mat8b.step, QImage::Format_Grayscale8).copy();

       }
      default:
         qWarning() << "cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
         break;
   }

   return QImage();
}

inline QImage  cvUMatToQImage( const cv::UMat &inUMat )
{
    qDebug() << "cvUMattoQImage................." ;
   switch ( inUMat.type() )
   {
      // 8-bit, 4 channel
      case CV_8UC4:
      {
         QImage image( inUMat.u->data, inUMat.cols, inUMat.rows, inUMat.step, QImage::Format_ARGB32 );
         qDebug() << "CV_8UC4 ==> QImage::Format_ARGB32" ;

         return image;
      }

      // 8-bit, 3 channel
      case CV_8UC3:
      {
         QImage image( inUMat.u->data, inUMat.cols, inUMat.rows, inUMat.step, QImage::Format_RGB888 );

         return image.rgbSwapped();
      }



      // 8-bit, 1 channel
      case CV_8UC1:
      {
         static QVector<QRgb>  sColorTable;

//         // only create our color table once
         if ( sColorTable.isEmpty() )
         {
            for ( int i = 0; i < 256; ++i )
               sColorTable.push_back( qRgb( i, i, i ) );
         }

         QImage image = QImage( inUMat.u->data, inUMat.cols, inUMat.rows, inUMat.step, QImage::Format_Indexed8 ).copy();

         image.setColorTable( sColorTable );

         return image;
//           return QImage(inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_Grayscale8).copy();
      }

       case CV_16UC1:
       {
//            qDebug() << " convert en 8 bits";
            cv::Mat Mat8b ;
            Mat8b = cv::Mat::zeros( inUMat.rows,inUMat.cols,CV_8UC1) ;
            inUMat.convertTo(Mat8b, CV_8UC1, 1.0/255.0);
            return QImage(Mat8b.data, Mat8b.cols, Mat8b.rows, Mat8b.step, QImage::Format_Grayscale8).copy();

       }
      default:
         qWarning() << "cvMatToQImage() - cv::Mat image type not handled in switch:" << inUMat.type();
         break;
   }

   return QImage();
}

inline QPixmap cvMatToQPixmap( const cv::Mat &inMat )
{
   return QPixmap::fromImage( cvMatToQImage( inMat ) );
}

inline QPixmap cvUMatToQPixmap( const cv::UMat &inUMat )
{
   return QPixmap::fromImage( cvUMatToQImage( inUMat ) );
}



inline cv::Mat QImageToCvMat( const QImage &inImage, bool inCloneImageData = true )
{
   switch ( inImage.format() )
   {
      // 8-bit, 4 channel
      case QImage::Format_RGB32:
      {
         cv::Mat  mat( inImage.height(), inImage.width(), CV_8UC4, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine() );

         return (inCloneImageData ? mat.clone() : mat);
      }

      // 8-bit, 3 channel
      case QImage::Format_RGB888:
      {
         if ( !inCloneImageData )
            qWarning() << "QImageToCvMat() - Conversion requires cloning since we use a temporary QImage";

         QImage   swapped = inImage.rgbSwapped();

         return cv::Mat( swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine() ).clone();
      }
       // 8-bit, 4 channel
       case QImage::Format_ARGB32:
       {
          if ( !inCloneImageData )
             qWarning() << "QImageToCvMat() - Conversion requires cloning since we use a temporary QImage";

          QImage   swapped = inImage.rgbSwapped();

          qDebug() << "QImage::Format_ARGB32 == > CV_8UC4" ;
          return cv::Mat( swapped.height(), swapped.width(), CV_8UC4, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine() ).clone();
       }

      // 8-bit, 1 channel
      case QImage::Format_Indexed8:
      case QImage::Format_Grayscale8:
      {
         cv::Mat  mat( inImage.height(), inImage.width(), CV_8UC1, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine() );

         return (inCloneImageData ? mat.clone() : mat);
      }

      default:
         qWarning() << "QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
         break;
   }

   return cv::Mat();
}

inline cv::UMat QImageToCvUMat( const QImage &inImage, bool inCloneImageData = true )
{
    cv::UMat Umat ;
   switch ( inImage.format() )
   {
      // 8-bit, 4 channel
      case QImage::Format_RGB32:
      {
         cv::Mat  mat( inImage.height(), inImage.width(), CV_8UC4, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine() );

         mat.copyTo(Umat);
         return (Umat);
      }

      // 8-bit, 3 channel
      case QImage::Format_RGB888:
      {
         if ( !inCloneImageData )
            qWarning() << "QImageToCvMat() - Conversion requires cloning since we use a temporary QImage";

         QImage   swapped = inImage.rgbSwapped();

         cv::Mat( swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine() ).copyTo(Umat);
         return Umat;
      }
       // 8-bit, 4 channel
       case QImage::Format_ARGB32:
       {
          if ( !inCloneImageData )
             qWarning() << "QImageToCvMat() - Conversion requires cloning since we use a temporary QImage";

          QImage   swapped = inImage.rgbSwapped();

          qDebug() << "QImage::Format_ARGB32 == > CV_8UC4" ;
          cv::Mat( swapped.height(), swapped.width(), CV_8UC4, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine() ).copyTo(Umat);
          return Umat;
//          return cv::Mat( swapped.height(), swapped.width(), CV_8UC4, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine() ).clone();
       }

      // 8-bit, 1 channel
      case QImage::Format_Indexed8:
      case QImage::Format_Grayscale8:
      {
         cv::Mat  mat( inImage.height(), inImage.width(), CV_8UC1, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine() );

         mat.copyTo(Umat);
         return (Umat);
 //        return (inCloneImageData ? mat.clone() : mat);
      }

      default:
         qWarning() << "QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
         break;
   }

   return cv::UMat();
}

// If inPixmap exists for the lifetime of the resulting cv::Mat, pass false to inCloneImageData to share inPixmap's data
// with the cv::Mat directly
//    NOTE: Format_RGB888 is an exception since we need to use a local QImage and thus must clone the data regardless
inline cv::Mat QPixmapToCvMat( const QPixmap &inPixmap, bool inCloneImageData = true )
{
   return QImageToCvMat( inPixmap.toImage(), inCloneImageData );
}

#endif // QIMAGETOCVMAT_H
