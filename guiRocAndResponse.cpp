#include "guiRocAndResponse.h"
#include <qcustomplot.h>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QMenu>
#include <QFileDialog>


#include <algorithm>//STL

GUI_ROC_AND_RESPONSE::GUI_ROC_AND_RESPONSE()
{

//_______________________________RESPONSE PLOT__________________________________________
//Configuraciones basicas
customPlotResponse=new QCustomPlot;


customPlotResponse->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); /*period as decimal separator and comma as thousand separator*/
customPlotResponse->legend->setVisible(true);
QFont legendFont1 =QFont();  // start out with MainWindow's font..
legendFont1.setPointSize(11); // and make a bit smaller for legend
customPlotResponse->legend->setFont(legendFont1);
customPlotResponse->legend->setBrush(QBrush(QColor(255,255,255,230)));
// by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
customPlotResponse->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignRight);


//Respuesta
customPlotResponse->addGraph(customPlotResponse->xAxis, customPlotResponse->yAxis);
customPlotResponse->graph(0)->setPen(QPen(Qt::blue));
customPlotResponse->graph(0)->setName("Respuesta");

 
//Umbral
customPlotResponse->addGraph(customPlotResponse->xAxis, customPlotResponse->yAxis);
customPlotResponse->graph(1)->setPen(QPen(Qt::red));
customPlotResponse->graph(1)->setName("Umbral");


//___________Titulo principal_______________________________//
customPlotResponse->plotLayout()->insertRow(0);
customPlotResponse->plotLayout()->addElement(0, 0, new QCPPlotTitle(customPlotResponse, "Respuesta del clasificador"));


//___________Titulo de los ejes_____________________________//
customPlotResponse->xAxis->setLabel(QString::fromUtf8("i-ésima muestra"));
customPlotResponse->yAxis->setLabel("Respuesta");



//customPlotResponse->setMinimumSize(700,400);//TAMAÑO MÍNIMO
customPlotResponse->setMinimumSize(100,100);//TAMAÑO MÍNIMO

//_____________________________________________________________________________________


//_______________________________ROC PLOT__________________________________________
//Configuraciones básicas
customPlotRoc=new QCustomPlot;



customPlotRoc->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); /*period as decimal separator and comma as thousand separator*/
customPlotRoc->legend->setVisible(true);
//QFont legendFont1 =QFont();  // start out with MainWindow's font..
//legendFont1.setPointSize(9); // and make a bit smaller for legend
customPlotRoc->legend->setFont(legendFont1);
customPlotRoc->legend->setBrush(QBrush(QColor(255,255,255,230)));
// by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
customPlotRoc->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignRight);


//ROC
customPlotRoc->addGraph(customPlotRoc->xAxis, customPlotRoc->yAxis);
customPlotRoc->graph(0)->setPen(QPen(Qt::blue));
customPlotRoc->graph(0)->setName("ROC");
 
//Umbral
customPlotRoc->addGraph(customPlotRoc->xAxis,customPlotRoc->yAxis);
customPlotRoc->graph(1)->setPen(QPen(Qt::red));
customPlotRoc->graph(1)->setName("Umbral");


//___________Titulo principal_______________________________//
customPlotRoc->plotLayout()->insertRow(0);
customPlotRoc->plotLayout()->addElement(0, 0, new QCPPlotTitle(customPlotRoc, "Curva ROC"));


//___________Titulo de los ejes_____________________________//
customPlotRoc->xAxis->setLabel("ith muestra");
customPlotRoc->yAxis->setLabel("Verdaderos Positivos");
customPlotRoc->yAxis2->setLabel("Valor Umbral");


//customPlotRoc->setMinimumSize(700,400);//TAMAÑO MÍNIMO
customPlotRoc->setMinimumSize(100,100);//TAMAÑO MÍNIMO

//_____________________________________________________________________________________


stackedPlot=new QStackedWidget;
stackedPlot->addWidget(customPlotResponse);
stackedPlot->addWidget(customPlotRoc);

QVBoxLayout *layoutPrincipal=new QVBoxLayout;
layoutPrincipal->addWidget(stackedPlot);
setLayout(layoutPrincipal);


//___________MENU CONTEXTUAL____________//
customPlotResponse->setContextMenuPolicy(Qt::CustomContextMenu);
customPlotRoc->setContextMenuPolicy(Qt::CustomContextMenu);

menuPlot=new QMenu(this);
actionSaveGraphic=new QAction("Guardar Grafica",this);
menuPlot->addAction(actionSaveGraphic);

connect(customPlotResponse,SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
connect(customPlotRoc,SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
connect(actionSaveGraphic,SIGNAL(triggered()),this,SLOT(saveGraphic()));
//______________________________________//

setWindowTitle("PLOT UNIVALLE TRAINING DETECTOR 1.0");

}



void GUI_ROC_AND_RESPONSE::plotResponse(QVector<double> evalNegatives,QVector<double> evalPositives,bool plotThreshold,double threshold,bool seeError)
{

customPlotResponse->clearGraphs();//Limpiamos el gráfico

QVector<double> negativesAndPositives=evalNegatives+evalPositives;

if(plotThreshold==true){
//Respuesta
customPlotResponse->addGraph(customPlotResponse->xAxis, customPlotResponse->yAxis);
customPlotResponse->graph(0)->setPen(QPen(Qt::blue));
//___________________________________________________________________
if(seeError==true){

int equivocations=0;
//_____________Creado el 20 de febrero de 2016____________________//
/*
Debido a que el error no es un dato informativo sobre la generalización del los clasificadores, se decidió tomar la medida de taza de detección y taza de falsos positivos.
*/
int NVP=0,NFP=0;
//________________________________________________________________//
for(int i=0;i<evalNegatives.size();i++)
{
if(evalNegatives[i]>=threshold)equivocations++;
if(evalNegatives[i]>=threshold)NFP++;
}

for(int i=0;i<evalPositives.size();i++)
{
if(evalPositives[i]<threshold)equivocations++;
if(evalPositives[i]>=threshold)NVP++;
}

double error=double(equivocations)/(evalNegatives.size()+evalPositives.size());
double TVP=double(NVP)/(evalPositives.size());
double TFP=double(NFP)/(evalNegatives.size());

//customPlotResponse->graph(0)->setName("Respuesta, error="+QString::number(error));
customPlotResponse->graph(0)->setName("Respuesta\nd="+QString::number(TVP)+"\nf="+QString::number(TFP)+"\nError="+QString::number(error));
}else
customPlotResponse->graph(0)->setName("Respuesta");
//___________________________________________________________________

customPlotResponse->graph(0)->setLineStyle(QCPGraph::lsNone);
customPlotResponse->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

//Umbral
customPlotResponse->addGraph(customPlotResponse->xAxis, customPlotResponse->yAxis);
customPlotResponse->graph(1)->setPen(QPen(Qt::red));
customPlotResponse->graph(1)->setName("Umbral="+QString::number(threshold));
customPlotResponse->graph(1)->setLineStyle(QCPGraph::lsLine);


QVector<double> x,valThreshold;
for(int i=0;i<negativesAndPositives.size();i++){
x.push_back(i);
valThreshold.push_back(threshold);
}

customPlotResponse->graph(0)->setData(x,negativesAndPositives);
customPlotResponse->graph(1)->setData(x,valThreshold);


}else
{

//Respuesta
customPlotResponse->addGraph(customPlotResponse->xAxis, customPlotResponse->yAxis);
customPlotResponse->graph(0)->setPen(QPen(Qt::blue));

//___________________________________________________________________
if(seeError==true){

int equivocations=0;
//_____________Creado el 20 de febrero de 2016____________________//
/*
Debido a que el error no es un dato informativo sobre la generalizacion del los clasificadores, se decidio tomar la medida de taza de deteccion y taza de falsos positivos.
*/
int NVP=0,NFP=0;
//________________________________________________________________//
for(int i=0;i<evalNegatives.size();i++)
{
if(evalNegatives[i]>=threshold)equivocations++;
if(evalNegatives[i]>=threshold)NFP++;
}


for(int i=0;i<evalPositives.size();i++)
{
if(evalPositives[i]<threshold)equivocations++;
if(evalPositives[i]>=threshold)NVP++;
}
qDebug()<<"pasando por aquí="<<threshold<<"\n";
double error=double(equivocations)/(evalNegatives.size()+evalPositives.size());
double TVP=double(NVP)/(evalPositives.size());
double TFP=double(NFP)/(evalNegatives.size());

//customPlotResponse->graph(0)->setName("Respuesta, error="+QString::number(error));
customPlotResponse->graph(0)->setName("Respuesta\nd="+QString::number(TVP)+"\nf="+QString::number(TFP)+"\nError="+QString::number(error));
}else
customPlotResponse->graph(0)->setName("Respuesta");
//___________________________________________________________________


customPlotResponse->graph(0)->setLineStyle(QCPGraph::lsNone);
customPlotResponse->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

QVector<double> x;
for(int i=0;i<negativesAndPositives.size();i++){
x.push_back(i);
}

customPlotResponse->graph(0)->setData(x,negativesAndPositives);

}

double minValue=*std::min_element(negativesAndPositives.begin(),negativesAndPositives.end());
double maxValue=*std::max_element(negativesAndPositives.begin(),negativesAndPositives.end());

customPlotResponse->xAxis->setRange(0,negativesAndPositives.size()-1);
customPlotResponse->yAxis->setRange(minValue-2,maxValue+1);


QVector< QString > listNameLabels;
listNameLabels.push_back("Conjunto Negativo="+QString::number(evalNegatives.size()));
listNameLabels.push_back(QString("|"));
listNameLabels.push_back("Conjunto Positivo="+QString::number(evalPositives.size()));
customPlotResponse->xAxis->setAutoTickLabels(false);
customPlotResponse->xAxis->setTickVectorLabels(listNameLabels);

customPlotResponse->yAxis->setAutoTickStep(false); 
customPlotResponse->yAxis->setTickStep(0.5);

QVector<double> listValues;
listValues.push_back((evalNegatives.size()/2));
listValues.push_back(evalNegatives.size());
listValues.push_back(evalNegatives.size()+(evalPositives.size()/2));
customPlotResponse->xAxis->setAutoTicks(false);
customPlotResponse->xAxis->setTickVector(listValues);


stackedPlot->setCurrentIndex(0);// Establecemos el índice de la pila
customPlotResponse->replot();

}




void GUI_ROC_AND_RESPONSE::plotROC(QVector<double> VPR,QVector<double> FPR,bool plotThreshold,QVector<double> threshold)
{


customPlotRoc->clearGraphs();//Limpiamos el gráfico


if(plotThreshold==true){

//ROC
customPlotRoc->addGraph(customPlotRoc->xAxis,customPlotRoc->yAxis);
customPlotRoc->graph(0)->setPen(QPen(Qt::blue));
customPlotRoc->graph(0)->setName("ROC");

//Umbral
customPlotRoc->addGraph(customPlotRoc->xAxis,customPlotRoc->yAxis2);
customPlotRoc->graph(1)->setPen(QPen(Qt::red));
customPlotRoc->graph(1)->setName("Umbral");



customPlotRoc->graph(0)->setData(FPR,VPR);
customPlotRoc->graph(1)->setData(FPR,threshold);

customPlotRoc->yAxis2->setVisible(true);

//Estableciendo rangos

double minValueVPR=*std::min_element(VPR.begin(),VPR.end());
double maxValueVPR=*std::max_element(VPR.begin(),VPR.end());

double minValueFPR=*std::min_element(FPR.begin(),FPR.end());
double maxValueFPR=*std::max_element(FPR.begin(),FPR.end());


customPlotRoc->xAxis->setRange(minValueFPR,maxValueFPR+0.2);
customPlotRoc->yAxis->setRange(minValueVPR,maxValueVPR+0.2);

double minValueThreshold=*std::min_element(threshold.begin(),threshold.end());
double maxValueThreshold=*std::max_element(threshold.begin(),threshold.end());
customPlotRoc->yAxis2->setRange(minValueThreshold-0.2,maxValueThreshold+0.2);

}
else
{

//ROC
customPlotRoc->addGraph(customPlotRoc->xAxis,customPlotRoc->yAxis);
customPlotRoc->graph(0)->setPen(QPen(Qt::blue));
customPlotRoc->graph(0)->setName("ROC");
 
customPlotRoc->graph(0)->setData(FPR,VPR);


//Estableciendo rangos
double minValueVPR=*std::min_element(VPR.begin(),VPR.end());
double maxValueVPR=*std::max_element(VPR.begin(),VPR.end());

double minValueFPR=*std::min_element(FPR.begin(),FPR.end());
double maxValueFPR=*std::max_element(FPR.begin(),FPR.end());

customPlotRoc->xAxis->setRange(minValueFPR,maxValueFPR+0.2);
customPlotRoc->yAxis->setRange(minValueVPR,maxValueVPR+0.2);

}

customPlotRoc->yAxis->setAutoTickStep(false); 
customPlotRoc->yAxis->setTickStep(0.1);

stackedPlot->setCurrentIndex(1);// Establecemos el índice de la pila

customPlotRoc->replot();

}




void GUI_ROC_AND_RESPONSE::onCustomContextMenu(const QPoint & point)
{

menuPlot->exec(stackedPlot->currentWidget()->mapToGlobal(point));

}


void GUI_ROC_AND_RESPONSE::saveGraphic()
{

QString file_temp=QFileDialog::getSaveFileName(this, tr("Save Data Base"),QDir::home().path()+QString("/imagen.png"),tr("Data(*.png)"));
if(file_temp=="")return;

if(stackedPlot->currentIndex()==0)
customPlotResponse->savePng(file_temp);
else
customPlotRoc->savePng(file_temp);


}





