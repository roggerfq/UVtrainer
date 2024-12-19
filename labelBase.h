#ifndef LABELBASE_H
#define LABELBASE_H

#include <QDialog>

class QImage;
class QLabel;
class QWindowsStyle;
class QComboBox;
class QSlider;
class QLineEdit;
class QFormLayout;
class QStackedWidget;
class QGridLayout;

class LABEL_BASE:public QDialog
{
Q_OBJECT

//_____OBJETOS GRAFICOS_________

QLabel *labelPresentation;
QLabel *labelGraphic;
QStackedWidget *stackedLabelGrafic;
QLabel *labelInfoGraphic;
QStackedWidget *stackedInfoLabelGrafic;
QLabel *labelControl;
QStackedWidget *stackedLabelControl;
public:
QLabel *labelShowTree;
private:
QGridLayout *layoutPrincipal;
QLabel *labelSequence;
QStackedWidget *stackedLabelSequence;
//_________MEDIDAS PARA LA ORGANIZACIÓN DE LOS OBJETOS GRÁFICOS________
public:
int width_labelControl;
int height_labelControl;
int width_labelTree;
int height_labelTree;
int width_labelGraphic;
int height_labelGraphic;
int width_labelInfoGraphic;
int height_labelInfoGraphic;
int width_labelPresentation;
int height_labelPresentation;
int height_labelSequence;
private:
QWindowsStyle *WindowTrainingDetectionStyle;
//_________FUNCIONES______________
void initializations();
void initializing_labelGraphic();
void initializing_labelInfoGraphic();
void initializing_labelControl();
void initializing_labelTree();
void initializing_labelSequence();
void initializing_layoutPrincipal();
public:
LABEL_BASE(QDialog *parent = 0);
//_____FUNCIONES DE ACCESO____________
void setTextPresentation(const QString & text);
int getSizeStackLabelControl()const;
int getSizeStackLabelInfoGrafic()const;
int getSizeStackLabelGrafic()const;
int getSizeStackLabelSequence()const;


//____FUNCIONES DE ESTABLECIMIENTO____
void addWidgetStackLabelControl(QWidget * widget);//Inserta los widget en la pila de label control
void setCurrentIndexLabelControl(int index);//index indica el widget de la pila que se vera en label control

void addWidgetStackLabelInfoGrafic(QWidget * widget);
void setCurrentIndexLabelInfoGrafic(int index);

void addWidgetStackLabelGrafic(QWidget * widget);
void setCurrentIndexLabelGrafic(int index);

void addWidgetStackLabelSequence(QWidget * widget);
void setCurrentIndexLabelSequence(int index);

public slots:


protected:
void closeEvent(QCloseEvent *event);

};


#endif

