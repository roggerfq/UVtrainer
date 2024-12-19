#include "labelBase.h"
//_____ARCHIVOS DE CABECERA DE QT____________________
#include <QtGui>

LABEL_BASE::LABEL_BASE(QDialog *parent):QDialog(parent)
{

initializations();//Inicializando variables importantes
initializing_labelGraphic();
initializing_labelInfoGraphic();
initializing_labelControl();
initializing_labelTree();
initializing_labelSequence();
initializing_layoutPrincipal();

}



void LABEL_BASE::initializations()
{
WindowTrainingDetectionStyle=new QWindowsStyle;
//______________________________________________

//__ESTABLECIENDO LAS MEDIDAS MAS IMPORTANTES___
width_labelControl=370;
height_labelControl=240;

width_labelTree=900;
height_labelTree=500;

width_labelGraphic=width_labelControl;
height_labelGraphic=(width_labelGraphic)/(1.4);

width_labelInfoGraphic=width_labelGraphic;
height_labelInfoGraphic=65;

width_labelPresentation=width_labelGraphic;//width_labelControl+width_labelGraphic+5
height_labelPresentation=30;

height_labelSequence=50;

//______LABELES PRINCIPALES DE LA APLICACIÓN____
labelPresentation=new QLabel;
labelPresentation->setStyle(WindowTrainingDetectionStyle);
labelPresentation->setFrameShape(QFrame::StyledPanel);
labelPresentation->setMinimumSize(width_labelPresentation,height_labelPresentation);

}

//_______________________________________________________________


void LABEL_BASE::initializing_labelGraphic()
{

//_____________LABEL QUE CONTENDRA LA PILA DE OBJETOS EN LABEL INFOGRAFIC__________________//
labelGraphic=new QLabel;
labelGraphic->setStyle(WindowTrainingDetectionStyle);
labelGraphic->setFrameShape(QFrame::StyledPanel);
labelGraphic->setMinimumSize(width_labelGraphic,height_labelGraphic);
//________________________________________________________________________________________//

//____________Aquí ira la pila de objetos_____________________

stackedLabelGrafic=new QStackedWidget;
QHBoxLayout *layoutStakedGrafic=new QHBoxLayout;
layoutStakedGrafic->addWidget(stackedLabelGrafic);
labelGraphic->setLayout(layoutStakedGrafic);

//____________________________________________________________

}


void LABEL_BASE::initializing_labelInfoGraphic()
{

//_____________LABEL QUE CONTENDRA LA PILA DE OBJETOS EN LABEL INFOGRAFIC__________________//
labelInfoGraphic=new QLabel;
labelInfoGraphic->setStyle(WindowTrainingDetectionStyle);
labelInfoGraphic->setFrameShape(QFrame::StyledPanel);
labelInfoGraphic->setMinimumSize(width_labelInfoGraphic,height_labelInfoGraphic);
//________________________________________________________________________________________//


//____________Aquí ira la pila de objetos_____________________
stackedInfoLabelGrafic=new QStackedWidget;
QHBoxLayout *layoutStakedInfoGrafic=new QHBoxLayout;
layoutStakedInfoGrafic->addWidget(stackedInfoLabelGrafic);
labelInfoGraphic->setLayout(layoutStakedInfoGrafic);
//____________________________________________________________


}


void LABEL_BASE::initializing_labelControl()
{
//_____________LABEL QUE CONTENDRA LA PILA DE OBJETOS EN LABEL CONTROL__________________//
labelControl=new QLabel;
labelControl->setStyle(WindowTrainingDetectionStyle);
labelControl->setFrameShape(QFrame::StyledPanel);
labelControl->setMinimumSize(width_labelControl,height_labelControl);
labelControl->installEventFilter(this);
//______________________________________________________________________________________//

//____________Aquí ira la pila de objetos_____________________
stackedLabelControl=new QStackedWidget;
QHBoxLayout *layoutStakedControl=new QHBoxLayout;
layoutStakedControl->addWidget(stackedLabelControl);
labelControl->setLayout(layoutStakedControl);
//____________________________________________________________



}


void LABEL_BASE::initializing_labelTree()
{
labelShowTree=new QLabel;
labelShowTree->setStyle(WindowTrainingDetectionStyle);
labelShowTree->setFrameShape(QFrame::StyledPanel);
labelShowTree->setMinimumSize(width_labelTree,height_labelTree);
labelShowTree->installEventFilter(this);
}



void LABEL_BASE::initializing_labelSequence()
{

labelSequence=new QLabel;
labelSequence->setStyle(WindowTrainingDetectionStyle);
labelSequence->setFrameShape(QFrame::StyledPanel);
labelSequence->setMinimumSize(width_labelControl,height_labelSequence);
labelSequence->installEventFilter(this);


//____________Aquí ira la pila de objetos_____________________
stackedLabelSequence=new QStackedWidget;
QHBoxLayout *layoutStakedLabelSequence=new QHBoxLayout;
layoutStakedLabelSequence->addWidget(stackedLabelSequence);
labelSequence->setLayout(layoutStakedLabelSequence);
//____________________________________________________________



}


void LABEL_BASE::initializing_layoutPrincipal()
{


//____________ORGANIZACIÓN DE LAYOUT________________

layoutPrincipal = new QGridLayout;
layoutPrincipal->addWidget(labelPresentation,0,0,1,1,Qt::AlignLeft);
layoutPrincipal->addWidget(labelGraphic,2,0,5,1,Qt::AlignLeft);
layoutPrincipal->addWidget(labelInfoGraphic,8,0,2,6,Qt::AlignLeft);

layoutPrincipal->addWidget(labelControl,10,0,3,1,Qt::AlignLeft);
layoutPrincipal->addWidget(labelSequence,14,0,2,1,Qt::AlignLeft);

layoutPrincipal->addWidget(labelShowTree,2,1,14,4,Qt::AlignLeft);

/*
layoutPrincipal = new QGridLayout;

QGridLayout *layoutLeft= new QGridLayout;
layoutLeft->addWidget(labelPresentation,0,0,1,1,Qt::AlignLeft);
layoutLeft->addWidget(labelGraphic,2,0,5,1,Qt::AlignLeft);
layoutLeft->addWidget(labelInfoGraphic,8,0,2,6,Qt::AlignLeft);
layoutLeft->addWidget(labelControl,10,0,3,1,Qt::AlignLeft);
layoutLeft->addWidget(labelSequence,14,0,2,1,Qt::AlignLeft);



layoutPrincipal->addLayout(layoutLeft,0,0,1,20,Qt::AlignLeft);
layoutPrincipal->addWidget(labelShowTree,0,4,1,20,Qt::AlignLeft);
*/
//__________________________________________________

setLayout(layoutPrincipal);
setMinimumSize(sizeHint()); 
setWindowTitle(QString::fromUtf8("UVtrainer 1.0"));

}

//_____________________________________FUNCIONES DE ACCESO________________________________



void LABEL_BASE::setTextPresentation(const QString & text)
{
labelPresentation->setText(text);
}

int LABEL_BASE::getSizeStackLabelControl()const
{
return stackedLabelControl->count();
}


int LABEL_BASE::getSizeStackLabelInfoGrafic()const
{
return stackedInfoLabelGrafic->count();
}

int LABEL_BASE::getSizeStackLabelGrafic()const
{
return stackedLabelGrafic->count();
}

int LABEL_BASE::getSizeStackLabelSequence()const
{
return stackedLabelSequence->count();
}


//___________________________________FUNCIONES DE ESTABLECIMIENTO_________________________

void LABEL_BASE::addWidgetStackLabelControl(QWidget * widget)
{
stackedLabelControl->addWidget(widget);
}


void LABEL_BASE::setCurrentIndexLabelControl(int index)
{
stackedLabelControl->setCurrentIndex(index);
}

void LABEL_BASE::addWidgetStackLabelInfoGrafic(QWidget * widget)
{
stackedInfoLabelGrafic->addWidget(widget);
}


void LABEL_BASE::setCurrentIndexLabelInfoGrafic(int index)
{
stackedInfoLabelGrafic->setCurrentIndex(index);
}



void LABEL_BASE::addWidgetStackLabelGrafic(QWidget * widget)
{
stackedLabelGrafic->addWidget(widget);
}


void LABEL_BASE::setCurrentIndexLabelGrafic(int index)
{
stackedLabelGrafic->setCurrentIndex(index);
}



void LABEL_BASE::addWidgetStackLabelSequence(QWidget * widget)
{
stackedLabelSequence->addWidget(widget);
}


void LABEL_BASE::setCurrentIndexLabelSequence(int index)
{
stackedLabelSequence->setCurrentIndex(index);
}





//_____________________________________SLOTS_______________________________________


//_______________________________FUNCIONES PROTECTED REIMPLEMENTADAS______________________________

void LABEL_BASE::closeEvent(QCloseEvent *event)
 {

 //QMessageBox::warning(this, tr("WARNING"),tr("Ha presionado salir.\n"),QMessageBox::Ok); 
 delete WindowTrainingDetectionStyle;//Se elimina aquí ya que en la documentación no se encontró que el widget sea el encargado de borrarlo.
                                     //Así que por seguridad se elimina aquí.

  qDebug()<<"Aqui estamos\n";
  

  
 }

//_________________________________________________________________________________________________

