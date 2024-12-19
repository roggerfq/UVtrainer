#ifndef GUITRAININGFACEDETECTOR_H
#define GUITRAININGFACEDETECTOR_H

#include "labelBase.h"




class CONFIGURE_DATABASE;
class CONFIGURE_TRAINING;
class WINDOWS_PRINCIPAL_TREE;
class CASCADE_VIEW;
class GUI_ROC_AND_RESPONSE;

class GUI_TRAININ_FACE_DETECTOR:public LABEL_BASE
{
Q_OBJECT

CONFIGURE_DATABASE *guiConfiguarateDatabase;
CONFIGURE_TRAINING *guiConfigureTraining;
WINDOWS_PRINCIPAL_TREE *windowTree;
CASCADE_VIEW *windowsCascade;
GUI_ROC_AND_RESPONSE *guiPlotRocAndResponse;


QPushButton *buttonBack;
QPushButton *buttonNext;

QPushButton *buttonLoadClassifier;
QPushButton *buttonBuildClassifier;

int state;
bool isLoad;
std::string nameFileClassifierLoad;

QLabel *labelGraphic;//Label que contendrá la información gráfica


void setConfigLabelGraphic();

public:
GUI_TRAININ_FACE_DETECTOR();
~GUI_TRAININ_FACE_DETECTOR();

public slots:
void clearLabelGraphic();
void seeImage(const QImage &image);
void next();//Llama a indagar el siguiente estado
void back();//Llama a indagar el siguiente estado
void loadClassifier();
void buildClassifier();
void plotResponse(QVector<double> evalNegatives,QVector<double> evalPositives,bool plotThreshold=false,double threshold=0,bool seeError=false);
void plotROC(QVector<double> VPR,QVector<double> FPR,bool plotThreshold=false,QVector<double> threshold=QVector<double>());

signals:
void nextState(int state);//Avanza al siguiente estado

protected:
void closeEvent(QCloseEvent *event);

};




#endif
