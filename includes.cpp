#include <includes.h>
#include <iostream>
#include <stdio.h>
#include <QRegExp>
#include "guiCongfigureTraining.h"


//________________LIBRERIAS OPENCV___________________
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "training.h"






/*_______________TABLA DE TRADUCCIÓN PARA IMÁGENES EN ESCALA DE GRISES OPENCV-QT_________________*/

static QVector<QRgb> sColorTableGenerate(){
//qDebug()<<"GENERANDO TABLA\n";
QVector<QRgb>  sColorTable;

     if(sColorTable.isEmpty() )
      {
       for ( int i = 0; i < 256; ++i )
        sColorTable.push_back( qRgb( i, i, i ) );
      }

return sColorTable;

}

QVector<QRgb>  sColorTable=sColorTableGenerate();

//_____________________________________________________________//






int G_NUMBER_CORES_FOR_PARALLEL_SECTIONS=1;
bool G_OPERATION_END_NO_IMAGES=false;
extern THREAD_TRAINING *G_THREAD_TRAINING;


bool callback_addImagesMessage()
{
G_OPERATION_END_NO_IMAGES=false;


#if GRAPHICS_PART==1
/*_____________________FUNCIONES CALLBACK PARA INTERFAZ GRÁFICA__________________*/
bool param;
QMetaObject::invokeMethod(G_THREAD_TRAINING, "messageNoImages",
                          Qt::BlockingQueuedConnection,Q_RETURN_ARG(bool,param));




if(param==true){ 
G_OPERATION_END_NO_IMAGES=false;//Útil en el hilo de entrenamiento(función run)
return true;//indicara que la operación debe continuar
}else{
G_OPERATION_END_NO_IMAGES=true;//Útil en el hilo de entrenamiento(función run)
return false;//Indicara que la operación se debe de abordar
}



#else

/*_____________________FUNCIONES CALLBACK PARA INTERFAZ CONSOLA__________________*/
/*____________Definición de interfaz consola_____________________________________*/
std::cout<<"No hay imágenes suficientes, escoja una de las siguientes opciones:\n";
std::cout<<"1 He adicionado nuevas imágenes\n";
std::cout<<"OTRA TECLA abordar la operación\n";
int option;

scanf("%d",&option);

if(option==1){ 
G_OPERATION_END_NO_IMAGES=false;//Útil en el hilo de entrenamiento(función run)
return true;//indicara que la operación debe continuar
}else{
G_OPERATION_END_NO_IMAGES=true;//Útil en el hilo de entrenamiento(función run)
return false;//Indicara que la operación se debe de abordar
}
/*________________________________________________________________________________*/


#endif

}


