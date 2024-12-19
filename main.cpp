#include <QApplication>
#include <QMetaType>
#include <QTime>
#include <QSplashScreen>

//____________________REGISTRANDO TIPOS NECESARIOS___________________________//
Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QVector<double>)

//___________________________________________________________________________//


//_________Archivos propios____________________
#include "GUI_trainingFaceDetector.h"
//___________________________________


void delay(int ms)
{
    QTime dieTime= QTime::currentTime().addMSecs(ms);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents,1);
}



int main(int argc, char *argv[])
{


QApplication app(argc, argv);
//cv::namedWindow("X");
//cv::namedWindow("Y");

//____________________REGISTRANDO TIPOS NECESARIOS___________________________//
qRegisterMetaType< QList<int> >("QList<int>");
qRegisterMetaType< QVector<double> >("QVector<double>");

//___________________________________________________________________________//


GUI_TRAININ_FACE_DETECTOR interfazEntrenamiento;
interfazEntrenamiento.hide();

Qt::Alignment topCenter = Qt::AlignHCenter | Qt::AlignTop; // Qt::AlignRight
QSplashScreen *splash = new QSplashScreen;
QString nameAnimation("o_a0b9b74a943a032b-");
int numFrames=400;//400
for(int i=0;i<numFrames;i++){
splash->setPixmap(QPixmap("./Imagenes de la aplicacion/start/o_a0b9b74a943a032b-"+QString::number(i)+".jpg"));
splash->show();

if(i==50)
splash->showMessage(QString("<h2><i><font color=blue>Bienvenido a <font color=red>UVtrainer <font color=blue>1.0</i>"),topCenter);

if(i==150)
splash->showMessage(QString("<h2><i>graficos por <font color=green>QT</i>"),Qt::AlignRight);

if(i==200)
splash->showMessage(QString("Programado en la Universidad del Valle, Cali, Colombia"),Qt::AlignRight);

if(i==250)
splash->showMessage(QString("Autor Roger Figueroa Quintero"),Qt::AlignRight);

if(i==300)
splash->showMessage(QString("Dirigido por Humberto Loaiza PhD"),Qt::AlignRight);

if(i==350)
splash->showMessage(QString::fromUtf8("Autor animación gráfica \"Growing Tree\" Tim Thomas"),Qt::AlignRight);


delay(40);
}


/*
Qt::Alignment topRight = Qt::AlignRight | Qt::AlignTop;
splash->showMessage(QObject::tr("Setting up the main window..."),
topRight, Qt::white);



splash->showMessage(QObject::tr("Loading modules..."),
topRight, Qt::white);

splash->showMessage(QObject::tr("Establishing connections..."),
topRight, Qt::white);
*/


interfazEntrenamiento.show();
splash->finish(&interfazEntrenamiento);
delete splash;

return app.exec();


}
