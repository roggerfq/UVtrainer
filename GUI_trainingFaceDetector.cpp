#include "GUI_trainingFaceDetector.h"
#include "GuiConfigureDatabase.h"
#include "guiCongfigureTraining.h"
#include <drawTree.h>
#include "viewCascade.h"
#include "guiRocAndResponse.h"

WINDOWS_PRINCIPAL_TREE *G_WINDOW_TREE=NULL;
CASCADE_VIEW *G_WINDOWS_CASCADE=NULL;
CONFIGURE_TRAINING *G_GUI_CONFIG_TRAINING=NULL;
GUI_TRAININ_FACE_DETECTOR *G_GUI_TRAININ_FACE_DETECTOR=NULL;

//________________LIBRERIAS QT_______________________
#include <QtGui>

std::string G_NAME_SET_POSITIVE_DATA_BASE="";


GUI_TRAININ_FACE_DETECTOR::GUI_TRAININ_FACE_DETECTOR():state(0),isLoad(false),nameFileClassifierLoad("")
{

//_____________TITULO DE PRESENTACIÓN__________________//

setTextPresentation("<font color=red>"+QString("UVtrainer")+"<font color=blue>"+QString(" 1.0"));

//_____________________________________________________//

G_GUI_TRAININ_FACE_DETECTOR=this;

//_____________________INSALAMOS LA VENTANA DONDE SE DIBUJARAN LOS ARBOLES________________________________
windowTree=new WINDOWS_PRINCIPAL_TREE;
G_WINDOW_TREE=windowTree;


windowsCascade=new CASCADE_VIEW(NULL);
G_WINDOWS_CASCADE=windowsCascade;

connect(windowsCascade,SIGNAL(seeTree(NODE_GRAPHIC *)),windowTree,SLOT(seeGraphicNode(NODE_GRAPHIC *)));



QHBoxLayout *layoutTreeAndCascade=new QHBoxLayout;
layoutTreeAndCascade->addWidget(windowTree);
layoutTreeAndCascade->addWidget(windowsCascade);


labelShowTree->setLayout(layoutTreeAndCascade);


//_________________________________________________________________________________________________________


//___________________________SOBRE EL PLOT DE LA RESPUESTA Y LA ROC______________________________

guiPlotRocAndResponse=new GUI_ROC_AND_RESPONSE;

//________________________________________________________________________________________________


setConfigLabelGraphic();
addWidgetStackLabelGrafic(labelGraphic);

//_____________________________Primer label________________________________________________________//
QFont font;
font.setPointSize(16);
QLabel *labelQuestion=new QLabel;
buttonLoadClassifier=new QPushButton("Cargar un clasificador");
buttonLoadClassifier->setFont(font);

buttonLoadClassifier->setFixedHeight(80);
buttonLoadClassifier->setAutoDefault(false);
buttonBuildClassifier=new QPushButton("Construir un clasificador");
buttonBuildClassifier->setFont(font);
buttonBuildClassifier->setFixedHeight(80);
buttonBuildClassifier->setAutoDefault(false);


connect(buttonLoadClassifier,SIGNAL(clicked()),this,SLOT(loadClassifier()));
connect(buttonBuildClassifier,SIGNAL(clicked()),this,SLOT(buildClassifier()));



QVBoxLayout *layoutQuestion=new QVBoxLayout;
layoutQuestion->addWidget(buttonLoadClassifier);
layoutQuestion->addWidget(buttonBuildClassifier);

labelQuestion->setLayout(layoutQuestion);

addWidgetStackLabelControl(labelQuestion);

//_______________________SOBRE EL LABEL DE BIENVENIDA__________________________________//
QLabel *labelWelcome=new QLabel("<font color=blue>"+QString::fromUtf8("¡BIENVENIDO!"));
labelWelcome->setFont(font);
labelWelcome->setFixedWidth(150);
QHBoxLayout *layoutWelcome=new QHBoxLayout;
layoutWelcome->addWidget(labelWelcome);
QLabel *labelWelcomePrincipal=new QLabel;
labelWelcomePrincipal->setLayout(layoutWelcome);
addWidgetStackLabelInfoGrafic(labelWelcomePrincipal);
//_____________________________________________________________________________________//

//_________________________________________________________________________________________________//

//_____________________________CONFIGURACIÓN BASE DE DATOS__________________________________________//
guiConfiguarateDatabase=new CONFIGURE_DATABASE;
connect(guiConfiguarateDatabase,SIGNAL(seeImage(const QImage &)),this,SLOT(seeImage(const QImage &)));
//connect(guiConfiguarateDatabase,SIGNAL(nextEco(int)),this,SLOT(selectState(int)));
//connect(this,SIGNAL(nextState(int)),guiConfiguarateDatabase,SLOT(next(int)));

addWidgetStackLabelInfoGrafic(guiConfiguarateDatabase->labelInfoGraphic);
addWidgetStackLabelControl(guiConfiguarateDatabase);

//_________________________________________________________________________________________________//

//_____________________________CONFIGURACIÓN ENTRENAMIENTO__________________________________________//
guiConfigureTraining=new CONFIGURE_TRAINING;
G_GUI_CONFIG_TRAINING=guiConfigureTraining;
addWidgetStackLabelControl(guiConfigureTraining);
addWidgetStackLabelInfoGrafic(guiConfigureTraining->labelInfoGraphic);

connect(guiConfigureTraining,SIGNAL(seeFeature(const QImage &)),this,SLOT(seeImage(const QImage &)));

//___________________________________________________________________________________________________//

//___________________BOTONES DE SECUENCIA_______________________________//
buttonBack=new QPushButton(tr("Back"));
connect(buttonBack,SIGNAL(clicked()),this,SLOT(back()));
buttonBack->setAutoDefault(false);
buttonBack->setEnabled(false);

buttonNext=new QPushButton(tr("Next"));
connect(buttonNext,SIGNAL(clicked()),this,SLOT(next()));
buttonNext->setEnabled(false);
buttonNext->setAutoDefault(false);

buttonBack->setFixedSize(QSize(50,20));
buttonNext->setFixedSize(QSize(50,20));

QHBoxLayout *layoutSecuence=new QHBoxLayout;
QLabel *labelSecuence=new QLabel;
layoutSecuence->addWidget(buttonBack);
layoutSecuence->addWidget(buttonNext);

labelSecuence->setLayout(layoutSecuence);
addWidgetStackLabelSequence(labelSecuence);
//______________________________________________________________________//

show();

}

GUI_TRAININ_FACE_DETECTOR::~GUI_TRAININ_FACE_DETECTOR()
{


}



void GUI_TRAININ_FACE_DETECTOR::setConfigLabelGraphic()
{

labelGraphic=new QLabel;
/*_________________MUY IMPORTANTE PARA QUE LA IMAGEN SE VEA COMPLETA_______________________*/
labelGraphic->setBackgroundRole(QPalette::Base);
labelGraphic->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
labelGraphic->setScaledContents(true);
/*_________________________________________________________________________________________*/


clearLabelGraphic();

}


void GUI_TRAININ_FACE_DETECTOR::clearLabelGraphic()
{
QImage black(width_labelGraphic,height_labelGraphic,QImage::Format_RGB888);
black.fill(QColor(0,0,0));
labelGraphic->setPixmap(QPixmap::fromImage((black)));

}

void GUI_TRAININ_FACE_DETECTOR::next()
{

if(state==1){
if(isLoad==false){
if(!guiConfiguarateDatabase->saveDatabase())return;
}
}

state++;



if(state==2){

if(isLoad==false)
guiConfigureTraining->createClasifier(guiConfiguarateDatabase->pathDatabaseTraining);
else
guiConfigureTraining->loadClasifier(nameFileClassifierLoad);
buttonNext->setEnabled(false);

QFile::remove("temp.xml");
}


setCurrentIndexLabelControl(state);
setCurrentIndexLabelInfoGrafic(state);
}



void GUI_TRAININ_FACE_DETECTOR::back()
{

if(state==2)
{
if(!guiConfigureTraining->deleteClassifier(true))return;
buttonBack->setEnabled(true);
buttonNext->setEnabled(true);
if(isLoad==true)isLoad=false;
buttonNext->setEnabled(true);

QFile::remove("temp.xml");
}

if(state==1)
{
if(!guiConfiguarateDatabase->deleteDataBase())return;

buttonNext->setEnabled(false);
buttonBack->setEnabled(false);
isLoad=false;
}

if(state>=1)
state--;

setCurrentIndexLabelControl(state);
setCurrentIndexLabelInfoGrafic(state);

}



void GUI_TRAININ_FACE_DETECTOR::seeImage(const QImage &image)
{
labelGraphic->setPixmap(QPixmap::fromImage((image.scaled(width_labelGraphic,height_labelGraphic,Qt::IgnoreAspectRatio))));
}



void GUI_TRAININ_FACE_DETECTOR::closeEvent(QCloseEvent *event)
{

qDebug()<<"destruyendo en interfaz principal"<<"\n";


if(state==2){
if(!guiConfigureTraining->deleteClassifier(true))
{
event->ignore();
return;
}
}


event->accept();

}


void GUI_TRAININ_FACE_DETECTOR::loadClassifier()
{



QString fileName = QFileDialog::getOpenFileName(this,tr("Cargar un clasificador"),QString::fromStdString("./"),tr("Archivos de datos xml (*.xml)\n"));


if(fileName==QString("")) return;

nameFileClassifierLoad=fileName.toStdString();
state=1;
isLoad=true;
buttonBack->setEnabled(true);

qDebug()<<fileName<<"\n";
QFile::remove("temp.xml");
qDebug()<<fileName<<"\n";
next();
windowTree->readCommands();//IMPORTANTE:AQUÍ SE LEE LOS NODOS CUANDO SON CARGADOS
}


void GUI_TRAININ_FACE_DETECTOR::buildClassifier()
{

buttonNext->setEnabled(true);
buttonBack->setEnabled(true);
isLoad=false;
next();

}


void GUI_TRAININ_FACE_DETECTOR::plotResponse(QVector<double> evalNegatives,QVector<double> evalPositives,bool plotThreshold,double threshold,bool seeError)
{

guiPlotRocAndResponse->plotResponse(evalNegatives,evalPositives,plotThreshold,threshold,seeError);

guiPlotRocAndResponse->show();


}

void GUI_TRAININ_FACE_DETECTOR::plotROC(QVector<double> VPR,QVector<double> FPR,bool plotThreshold,QVector<double> threshold)
{

guiPlotRocAndResponse->plotROC(VPR,FPR,plotThreshold,threshold);

guiPlotRocAndResponse->show();

}


