#ifndef GUIANDRESPOSE_H
#define GUIANDRESPOSE_H

#include <QDialog>


class QCustomPlot;
class QStackedWidget;
class QMenu;
class QAction;

class GUI_ROC_AND_RESPONSE:public QDialog
{
Q_OBJECT

QCustomPlot *customPlotRoc;
QCustomPlot *customPlotResponse;
QStackedWidget *stackedPlot;
QMenu *menuPlot;
QAction *actionSaveGraphic;

public:
GUI_ROC_AND_RESPONSE();

public slots:
void plotResponse(QVector<double> evalNegatives,QVector<double> evalPositives,bool plotThreshold=false,double threshold=0,bool seeError=false);
void plotROC(QVector<double> VPR,QVector<double> FPR,bool plotThreshold=false,QVector<double> threshold=QVector<double>());
void onCustomContextMenu(const QPoint & point);
void saveGraphic();

};

#endif
