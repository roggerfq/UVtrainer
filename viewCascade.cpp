#include <QTreeView>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QAction>
#include <QMenu>
#include <QHeaderView>
#include <QDir>
#include <QInputDialog>
#include <QSpacerItem>
#include <QGridLayout>


#include <QDebug>

#include "viewCascade.h"


#include <training.h>
#include "opencv2/opencv.hpp"//Se requiere para completar los tipos incompletos declarados en training.h

#include "GUI_trainingFaceDetector.h"
extern GUI_TRAININ_FACE_DETECTOR *G_GUI_TRAININ_FACE_DETECTOR;
#include "guiCongfigureTraining.h"
extern THREAD_TRAINING *G_THREAD_TRAINING;

//_________Para representara la estructura del clasificador_______________
struct strongLearnItemModel
{
QStandardItem *item;
STRONG_LEARN *strongLearn;
};
//________________________________________________________________________



CASCADE_VIEW::CASCADE_VIEW(QWidget *parent)
    : QMainWindow(parent)
{


    nameBaseStrongLearn="Etapa_";
    nameBaseTree=QString::fromUtf8("Árbol_");

    treeView = new QTreeView(this);
    
   
  
    setCentralWidget(treeView);
    standardModel = new QStandardItemModel;
    standardModel->setColumnCount (1);
    standardModel->setHeaderData(0,Qt::Horizontal,"classifier cascade");
    rootNode = standardModel->invisibleRootItem();
    
    



    //register the model
    treeView->setModel(standardModel);
    treeView->expandAll();

    //selection changes shall trigger a slot
    QItemSelectionModel *selectionModel= treeView->selectionModel();
    connect(selectionModel, SIGNAL(selectionChanged (const QItemSelection &, const QItemSelection &)),
            this, SLOT(selectionChangedSlot(const QItemSelection &, const QItemSelection &)));

 
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
   


//_______________MENU TREE________________________________________________
    contextMenuTree=new QMenu(treeView);
    actionInformationTree=new QAction(QString::fromUtf8("Información"), this);
    contextMenuTree->addAction(actionInformationTree);
    connect(actionInformationTree,SIGNAL(triggered()),this,SLOT(seeInformationTree()));
    actionSeeResponse=new QAction("Ver respuesta", this);
    contextMenuTree->addAction(actionSeeResponse);
    connect(actionSeeResponse,SIGNAL(triggered()),this,SLOT(seeResponse()));
   


//_______________MENUS STRONGLEARN________________________________________
    contextMenuStrongLearn=new QMenu(treeView);
    actionInformationStrongLearn=new QAction(QString::fromUtf8("Información"), this);
    contextMenuStrongLearn->addAction(actionInformationStrongLearn);
    connect(actionInformationStrongLearn,SIGNAL(triggered()),this,SLOT(seeInformationStrongLearn()));
 
    actionSeeResponseStrongLearn=new QAction("Ver respuesta", this);
    contextMenuStrongLearn->addAction(actionSeeResponseStrongLearn);
    connect(actionSeeResponseStrongLearn,SIGNAL(triggered()),this,SLOT(seeResponseStrongLearn()));
    

//_________________MENU ESCENA ITEMS_______________________________________ 
    contextMenuScene=new QMenu(treeView);  
    actionInformationCascade=new QAction(QString::fromUtf8("Información"),this);
    contextMenuScene->addAction(actionInformationCascade);
    connect(actionInformationCascade,SIGNAL(triggered()),this,SLOT(seeInformationCascade()));
    actionSeeResponseCascade=new QAction("Ver respuesta cascada",this);
    contextMenuScene->addAction(actionSeeResponseCascade);
    connect(actionSeeResponseCascade,SIGNAL(triggered()),this,SLOT(seeResponseCascade()));
    actionSeeRocCascade=new QAction("Ver curva ROC",this);
    contextMenuScene->addAction(actionSeeRocCascade);
    connect(actionSeeRocCascade,SIGNAL(triggered()),this,SLOT(seeRocCascade()));
    actionSeeRocCascadeAndThreshold=new QAction("Ver curva ROC y umbral",this);
    contextMenuScene->addAction(actionSeeRocCascadeAndThreshold);
    connect(actionSeeRocCascadeAndThreshold,SIGNAL(triggered()),this,SLOT(seeRocCascadeAndThreshold()));

    connect(treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));

    setFixedWidth(150);

}


void CASCADE_VIEW::selectionChangedSlot(const QItemSelection & /*newSelection*/, const QItemSelection & /*oldSelection*/)
{

/*NOTA:Este slot usa la lista strongLearnList, por lo cual debe protegerse con un mutex, debido a que se podría estar actualizando dicha lista desde otro hilo*/
     

  
     mutex.lock();

    //get the text of the selected item
    const QModelIndex index=treeView->selectionModel()->currentIndex();

    if(index.isValid()&&(index.parent()!=rootNode->index())&&(index.parent()!=QModelIndex()))//Se selecciono un árbol
    {
    //Averiguamos el índice de el item padre
    const QModelIndex indexParen=index.parent();
    

    STRONG_LEARN *strongLearn=strongLearnList[indexParen.row()]->strongLearn;


    if(index.row()<(strongLearn->weakLearns).size()){

    TREE_TRAINING *treeWeakLearn=strongLearn->weakLearns[index.row()];

    treeWeakLearn->nodeRoot;

    NODE_GRAPHIC *graphicNode=treeWeakLearn->nodeRoot->graphicNode;
   
    emit seeTree(graphicNode);
    
    }else{
 
    emit seeTree(NULL);
    
    }

    

    }
   
     mutex.unlock();




}



void CASCADE_VIEW::onCustomContextMenu(const QPoint &point)
{
    QModelIndex index = treeView->indexAt(point);

    if(index.isValid()&&(index.parent()==rootNode->index()))//Es strong learn
    {
    contextMenuStrongLearn->exec(treeView->mapToGlobal(point));
    }
    else if((index.parent()== QModelIndex())){
    contextMenuScene->exec(treeView->mapToGlobal(point));
    }else//Por defecto es árbol
    {
    contextMenuTree->exec(treeView->mapToGlobal(point));
    }
   
}

void CASCADE_VIEW::pushStrongLearn(STRONG_LEARN *strongLearn)
{

//NOTA: este slot se llama desde otro hilo, así que debe protegerse pues la lista strongLearnList es también accedida desde este mismo hilo*/

mutex.lock();

numberTree=0;//Siempre reiniciamos este número
//Estructura que almacena un puntero al item y al strong learn correspondiente
strongLearnItemModel *myStrongLearnItemModel=new strongLearnItemModel;

myStrongLearnItemModel->strongLearn=strongLearn;//Se enlaza con el strong learn correspondiente


QString qstr_numberStrongLearn=QString::number(strongLearnList.size());
myStrongLearnItemModel->item=new QStandardItem(nameBaseStrongLearn+qstr_numberStrongLearn);//Se crea el item

//Almacenamos la estructura en la lista
strongLearnList.push_back(myStrongLearnItemModel);

//Adherimos la información al modelo/vista
rootNode->appendRow(myStrongLearnItemModel->item);

mutex.unlock();

}


void CASCADE_VIEW::pushTree()
{

//NOTA: este slot se llama desde otro hilo, así que debe protegerse pues la lista strongLearnList es también accedida desde este mismo hilo*/

mutex.lock();

if(strongLearnList.isEmpty()) return;

numberTree++;

QString qstr_numberTree=QString::number(numberTree);
QStandardItem *treeItem=new QStandardItem(nameBaseTree+qstr_numberTree);

strongLearnList.last()->item->appendRow(treeItem);//Indexamos los arboles


mutex.unlock();

}

void CASCADE_VIEW::deleteCascade()
{

while(standardModel->rowCount()){
QList< QStandardItem* > items = standardModel->takeRow( 0 );
qDeleteAll( items );
}

strongLearnList.clear();//Desalojamos la lista de estructuras



}


void CASCADE_VIEW::seeResponse()
{

if(strongLearnList.size()==0){
QMessageBox::warning(this, tr("WARNING"),QString("Debe cargar o entrenar un clasificador previamente"),QMessageBox::Ok);
return;
}

if(G_THREAD_TRAINING->isRunning()==true)
{
QMessageBox::warning(this, tr("WARNING"),QString("Debe parar el entrenamiento previamente"),QMessageBox::Ok);
return;
}

QDir dirPositivesTest(G_NAME_DIRECTORY_SET_POSITIVE_TEST);
QDir dirNegativesTest(G_NAME_DIRECTORY_SET_NEGATIVE_TEST);
QStringList filters;
filters << "*.png" << "*.jpg" << "*.bmp"<<"*.pgm";

QStringList imageNameListPositivesTest= dirPositivesTest.entryList(filters, QDir::Files|QDir::NoDotAndDotDot);
QStringList imageNameListNegativesTest= dirNegativesTest.entryList(filters, QDir::Files|QDir::NoDotAndDotDot);





int indexStrongLearn=treeView->currentIndex().parent().row();
int indexTree=treeView->currentIndex().row();
STRONG_LEARN *strongLearn=(strongLearnList[indexStrongLearn])->strongLearn;//Extraemos el clasificador fuerte
TREE_TRAINING *treeTraining=strongLearn->weakLearns[indexTree];//Extraemos el árbol del clasificador fuerte correspondiente

QVector<double> evalNegatives,evalPositives;//Listas donde se almacenaran las evaluaciones

//_______________PARA LOS NEGATIVOS_________________________________________

for(int i=0;i<imageNameListNegativesTest.size();i++){
cv::Mat img=cv::imread(dirNegativesTest.path().toStdString()+"/"+imageNameListNegativesTest[i].toStdString());

if(img.data!=NULL){
//________Por si la imagen necesita convertirse a escala de grises_______________//
if(img.channels()!=1)
cv::cvtColor(img,img,CV_BGR2GRAY);

cv::resize(img,img,cv::Size(G_WIDTH_IMAGE,G_HEIGHT_IMAGE));//Se redimensiona al tamaño de las imágenes de entrenamiento

uchar *M=img.ptr<uchar>(0);//Leyendo la dirección de la imagen
double npd_c=treeTraining->evaluateTree(M);//Evaluando la imagen en el árbol

evalNegatives.push_back(npd_c);//Almacenamos el resultado en la lista
}

}




//_________________PARA LOS POSITIVOS_________________________________
for(int i=0;i<imageNameListPositivesTest.size();i++){
cv::Mat img=cv::imread(dirPositivesTest.path().toStdString()+"/"+imageNameListPositivesTest[i].toStdString());

if(img.data!=NULL){

//________Por si la imagen necesita convertirse a escala de grises_______________//
if(img.channels()!=1)
cv::cvtColor(img,img,CV_BGR2GRAY);

cv::resize(img,img,cv::Size(G_WIDTH_IMAGE,G_HEIGHT_IMAGE));//Se redimensiona al tamaño de las imágenes de entrenamiento

uchar *M=img.ptr<uchar>(0);//Leyendo la dirección de la imagen
double npd_c=treeTraining->evaluateTree(M);//Evaluando la imagen en el árbol

evalPositives.push_back(npd_c);//Almacenamos el resultado en la lista
}

}


QMetaObject::invokeMethod(G_GUI_TRAININ_FACE_DETECTOR,"plotResponse",
                          Qt::QueuedConnection,Q_ARG(QVector<double>,evalNegatives),
                          Q_ARG(QVector<double>,evalPositives),
                          Q_ARG(bool,false),
                          Q_ARG(double,0),
                          Q_ARG(bool,true));





}

void CASCADE_VIEW::seeResponseStrongLearn()
{


if(strongLearnList.size()==0){
QMessageBox::warning(this, tr("WARNING"),QString("Debe cargar o entrenar un clasificador previamente"),QMessageBox::Ok);
return;
}

if(G_THREAD_TRAINING->isRunning()==true)
{
QMessageBox::warning(this, tr("WARNING"),QString("Debe parar el entrenamiento previamente"),QMessageBox::Ok);
return;
}


QDir dirPositivesTest(G_NAME_DIRECTORY_SET_POSITIVE_TEST);
QDir dirNegativesTest(G_NAME_DIRECTORY_SET_NEGATIVE_TEST);
QStringList filters;
filters << "*.png" << "*.jpg" << "*.bmp"<<"*.pgm";

QStringList imageNameListPositivesTest= dirPositivesTest.entryList(filters, QDir::Files|QDir::NoDotAndDotDot);
QStringList imageNameListNegativesTest= dirNegativesTest.entryList(filters, QDir::Files|QDir::NoDotAndDotDot);





int indexStrongLearn=treeView->currentIndex().row();
STRONG_LEARN *strongLearn=(strongLearnList[indexStrongLearn])->strongLearn;//Extraemos el clasificador fuerte

QVector<double> evalNegatives,evalPositives;//Listas donde se almacenaran las evaluaciones

//_______________PARA LOS NEGATIVOS_________________________________________

for(int i=0;i<imageNameListNegativesTest.size();i++){
cv::Mat img=cv::imread(dirNegativesTest.path().toStdString()+"/"+imageNameListNegativesTest[i].toStdString());

if(img.data!=NULL){
//________Por si la imagen necesita convertirse a escala de grises_______________//
if(img.channels()!=1)
cv::cvtColor(img,img,CV_BGR2GRAY);

cv::resize(img,img,cv::Size(G_WIDTH_IMAGE,G_HEIGHT_IMAGE));//Se redimensiona al tamaño de las imagenes de entrenamiento

uchar *M=img.ptr<uchar>(0);//Leyendo la dirección de la imagen
double npd_c=strongLearn->evaluateStrongLearnWithZeroThreshold(M);//Evaluando la imagen en el clasificador fuerte

evalNegatives.push_back(npd_c);//Almacenamos el resultado en la lista
}

}




//_________________PARA LOS POSITIVOS_________________________________
for(int i=0;i<imageNameListPositivesTest.size();i++){
cv::Mat img=cv::imread(dirPositivesTest.path().toStdString()+"/"+imageNameListPositivesTest[i].toStdString());

if(img.data!=NULL){

//________Por si la imagen necesita convertirse a escala de grises_______________//
if(img.channels()!=1)
cv::cvtColor(img,img,CV_BGR2GRAY);

cv::resize(img,img,cv::Size(G_WIDTH_IMAGE,G_HEIGHT_IMAGE));//Se redimensiona al tamaño de las imágenes de entrenamiento

uchar *M=img.ptr<uchar>(0);//Leyendo la dirección de la imagen
double npd_c=strongLearn->evaluateStrongLearnWithZeroThreshold(M);//Evaluando la imagen en el clasificador fuerte

evalPositives.push_back(npd_c);//Almacenamos el resultado en la lista
}

}


QMetaObject::invokeMethod(G_GUI_TRAININ_FACE_DETECTOR,"plotResponse",
                          Qt::QueuedConnection,Q_ARG(QVector<double>,evalNegatives),
                          Q_ARG(QVector<double>,evalPositives),
                          Q_ARG(bool,true),
                          Q_ARG(double,strongLearn->threshold),
                          Q_ARG(bool,true));







}


void CASCADE_VIEW::seeResponseCascade()
{

if(strongLearnList.size()==0){
QMessageBox::warning(this, tr("WARNING"),QString("Debe cargar o entrenar un clasificador previamente"),QMessageBox::Ok);
return;
}

if(G_THREAD_TRAINING->isRunning()==true)
{
QMessageBox::warning(this, tr("WARNING"),QString("Debe parar el entrenamiento previamente"),QMessageBox::Ok);
return;
}

int minimum=1, maximum=strongLearnList.size();
bool ok;

int numberClassifiers = QInputDialog::getInt(
                 this,QString::fromUtf8("Edición"),
                 QString("Entre el numero de clasificadores\n que desea evaluar desde el inicial."),0,  
                 minimum, maximum, 1, &ok);

if(ok&&(numberClassifiers>0)){

QDir dirPositivesTest(G_NAME_DIRECTORY_SET_POSITIVE_TEST);
QDir dirNegativesTest(G_NAME_DIRECTORY_SET_NEGATIVE_TEST);
QStringList filters;
filters << "*.png" << "*.jpg" << "*.bmp"<<"*.pgm";

QStringList imageNameListPositivesTest= dirPositivesTest.entryList(filters, QDir::Files|QDir::NoDotAndDotDot);
QStringList imageNameListNegativesTest= dirNegativesTest.entryList(filters, QDir::Files|QDir::NoDotAndDotDot);





CASCADE_CLASSIFIERS *cascadeClassifier=G_THREAD_TRAINING->classifier;//Extraemos el clasificador fuerte


QVector<double> evalNegatives,evalPositives;//Listas donde se almacenaran las evaluaciones

//_______________PARA LOS NEGATIVOS_________________________________________

for(int i=0;i<imageNameListNegativesTest.size();i++){
cv::Mat img=cv::imread(dirNegativesTest.path().toStdString()+"/"+imageNameListNegativesTest[i].toStdString());

if(img.data!=NULL){
//________Por si la imagen necesita convertirse a escala de grises_______________//
if(img.channels()!=1)
cv::cvtColor(img,img,CV_BGR2GRAY);

cv::resize(img,img,cv::Size(G_WIDTH_IMAGE,G_HEIGHT_IMAGE));//Se redimensiona al tamaño de las imágenes de entrenamiento

uchar *M=img.ptr<uchar>(0);//Leyendo la dirección de la imagen
double npd_c=2*double(cascadeClassifier->evaluateClassifier(M,numberClassifiers))-1;//Evaluando la imagen en el clasificador fuerte
//Y=2x-1 transforma el rango entre 0 y 1 a -1 y 1

evalNegatives.push_back(npd_c);//Almacenamos el resultado en la lista
}

}




//_________________PARA LOS POSITIVOS_________________________________
for(int i=0;i<imageNameListPositivesTest.size();i++){
cv::Mat img=cv::imread(dirPositivesTest.path().toStdString()+"/"+imageNameListPositivesTest[i].toStdString());

if(img.data!=NULL){

//________Por si la imagen necesita convertirse a escala de grises_______________//
if(img.channels()!=1)
cv::cvtColor(img,img,CV_BGR2GRAY);

cv::resize(img,img,cv::Size(G_WIDTH_IMAGE,G_HEIGHT_IMAGE));//Se redimensiona al tamaño de las imagines de entrenamiento

uchar *M=img.ptr<uchar>(0);//Leyendo la dirección de la imagen
double npd_c=2*double(cascadeClassifier->evaluateClassifier(M,numberClassifiers))-1;//Evaluando la imagen en el clasificador fuerte

evalPositives.push_back(npd_c);//Almacenamos el resultado en la lista
}

}


QMetaObject::invokeMethod(G_GUI_TRAININ_FACE_DETECTOR,"plotResponse",
                          Qt::QueuedConnection,Q_ARG(QVector<double>,evalNegatives),
                          Q_ARG(QVector<double>,evalPositives),
                          Q_ARG(bool,false),
                          Q_ARG(double,0),
                          Q_ARG(bool,true));



}//Cierra el if Ok




}


//_____________________-ÚTIL PARA EL CALCULO DE LA ROC___________________________________________
struct EVAL_SCORES
{
double score;
bool label;//Para negativo 0 y para positivo 1
};
bool sortEvalScores(EVAL_SCORES eval1,EVAL_SCORES eval2) { return (eval1.score<eval2.score);}
//_______________________________________________________________________________________________


void CASCADE_VIEW::seeRocCascadeAndThreshold()
{
seeRocCascade(true);
}



void CASCADE_VIEW::seeRocCascade(bool seeThreshold)
{

if(strongLearnList.size()==0){
QMessageBox::warning(this, tr("WARNING"),QString("Debe cargar o entrenar un clasificador previamente"),QMessageBox::Ok);
return;
}

if(G_THREAD_TRAINING->isRunning()==true)
{
QMessageBox::warning(this, tr("WARNING"),QString("Debe parar el entrenamiento previamente"),QMessageBox::Ok);
return;
}


int minimum=1, maximum=strongLearnList.size();
bool ok;

int numberClassifiers = QInputDialog::getInt(
                 this,QString::fromUtf8("Edición"),
                 QString("Entre el numero de clasificadores\n que desea evaluar desde el inicial."),0,  
                 minimum, maximum, 1, &ok);

if(ok&&(numberClassifiers>0)){


QDir dirPositivesTest(G_NAME_DIRECTORY_SET_POSITIVE_TEST);
QDir dirNegativesTest(G_NAME_DIRECTORY_SET_NEGATIVE_TEST);
QStringList filters;
filters << "*.png" << "*.jpg" << "*.bmp"<<"*.pgm";

QStringList imageNameListPositivesTest= dirPositivesTest.entryList(filters, QDir::Files|QDir::NoDotAndDotDot);
QStringList imageNameListNegativesTest= dirNegativesTest.entryList(filters, QDir::Files|QDir::NoDotAndDotDot);



CASCADE_CLASSIFIERS *cascadeClassifier=G_THREAD_TRAINING->classifier;//Extraemos el clasificador fuerte
std::vector<EVAL_SCORES> evaluationsScores;
QVector<double> yVPR,xFPR;
//_______________PARA LOS NEGATIVOS_________________________________________

for(int i=0;i<imageNameListNegativesTest.size();i++){
cv::Mat img=cv::imread(dirNegativesTest.path().toStdString()+"/"+imageNameListNegativesTest[i].toStdString());

if(img.data!=NULL){
//________Por si la imagen necesita convertirse a escala de grises_______________//
if(img.channels()!=1)
cv::cvtColor(img,img,CV_BGR2GRAY);

cv::resize(img,img,cv::Size(G_WIDTH_IMAGE,G_HEIGHT_IMAGE));//Se redimensiona al tamaño de las imágenes de entrenamiento

uchar *M=img.ptr<uchar>(0);//Leyendo la dirección de la imagen


if(cascadeClassifier->evaluateClassifier(M,numberClassifiers)){//Evaluando la imagen en el clasificador fuerte
//Si la ventana pasa atraves de la cascada se le asigna su respectivo score
EVAL_SCORES tempEval;
tempEval.label=false;
tempEval.score=cascadeClassifier->score;
evaluationsScores.push_back(tempEval);
}

}

}


//_________________PARA LOS POSITIVOS_________________________________
for(int i=0;i<imageNameListPositivesTest.size();i++){
cv::Mat img=cv::imread(dirPositivesTest.path().toStdString()+"/"+imageNameListPositivesTest[i].toStdString());

if(img.data!=NULL){

//________Por si la imagen necesita convertirse a escala de grises_______________//
if(img.channels()!=1)
cv::cvtColor(img,img,CV_BGR2GRAY);

cv::resize(img,img,cv::Size(G_WIDTH_IMAGE,G_HEIGHT_IMAGE));//Se redimensiona al tamaño de las imágenes de entrenamiento

uchar *M=img.ptr<uchar>(0);//Leyendo la dirección de la imagen

if(cascadeClassifier->evaluateClassifier(M,numberClassifiers)){//Evaluando la imagen en el clasificador fuerte
//Si la ventana pasa a traves de la cascada se le asigna su respectivo score
EVAL_SCORES tempEval;
tempEval.label=true;
tempEval.score=cascadeClassifier->score;
evaluationsScores.push_back(tempEval);
}

}

}



//Extraemos los umbrale posibles
std::vector<double> possibleThresholds;
for(int i=0;i<evaluationsScores.size();i++)
possibleThresholds.push_back(evaluationsScores[i].score);

std::sort(possibleThresholds.begin(), possibleThresholds.end());
possibleThresholds.erase(unique(possibleThresholds.begin(),possibleThresholds.end()),possibleThresholds.end());  
possibleThresholds.push_back(possibleThresholds.back()+0.1);
possibleThresholds.insert(possibleThresholds.begin(),possibleThresholds[0]-0.1);

for(int i=0;i<possibleThresholds.size();i++)
{
int VP=0,FP=0,VN=0,FN=0;
for(int j=0;j<evaluationsScores.size();j++)
{

//________________________________________________________________
if(evaluationsScores[j].score>=possibleThresholds[i])
{
//_________________________________________
if(evaluationsScores[j].label==true)
VP++;
else
FP++;
//_________________________________________
}else
{

//_________________________________________
if(evaluationsScores[j].label==false)
VN++;
else
FN++;
//_________________________________________

}
//________________________________________________________________

}

//________CALCULO DE LOS PUNTOS DE LA ROC_________
double VPR=(double(VP)/(VP+FN));
double FPR=(double(FP)/(FP+VN));
yVPR.push_back(VPR);
xFPR.push_back(FPR);
//________________________________________________

}


//________________________________________________________________________//
if(seeThreshold==false){

QMetaObject::invokeMethod(G_GUI_TRAININ_FACE_DETECTOR,"plotROC",
                          Qt::QueuedConnection,
                          Q_ARG(QVector<double>,yVPR),
                          Q_ARG(QVector<double>,xFPR));


}else{

QVector<double> tresholds;
for(int i=0;i<possibleThresholds.size();i++)
tresholds.push_back(possibleThresholds[i]);

QMetaObject::invokeMethod(G_GUI_TRAININ_FACE_DETECTOR,"plotROC",
                          Qt::QueuedConnection,
                          Q_ARG(QVector<double>,yVPR),
                          Q_ARG(QVector<double>,xFPR),
                          Q_ARG(bool,true),
                          Q_ARG(QVector<double>,tresholds));

}
//________________________________________________________________________//



}



}



void CASCADE_VIEW::seeInformationTree()
{

int indexStrongLearn=treeView->currentIndex().parent().row();
int indexTree=treeView->currentIndex().row();
STRONG_LEARN *strongLearn=(strongLearnList[indexStrongLearn])->strongLearn;//Extraemos el clasificador fuerte
TREE_TRAINING *treeTraining=strongLearn->weakLearns[indexTree];//Extraemos el árbol del clasificador fuerte correspondiente

int expectedNumberFeaturestoEvaluate=treeTraining->getExpectedNumberFeaturestoEvaluate();
int numberFeatureInTree=treeTraining->getNumberFeatures();


QMessageBox information;
information.setWindowTitle(QString::fromUtf8("INFORMACIÓN DEL ARBOL DE REGRESION"));
QSpacerItem* horizontalSpacer = new QSpacerItem(550, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);


information.setText(
QString::fromUtf8("Cantidad de características totales=")+QString::number(numberFeatureInTree)+
QString::fromUtf8("\nNumero esperado de cracterísticas a evaluar por cada subventana=")+QString::number(expectedNumberFeaturestoEvaluate)
);


QGridLayout* layout = (QGridLayout*)information.layout();
layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

information.exec(); 


}

void CASCADE_VIEW::seeInformationStrongLearn()
{

int indexStrongLearn=treeView->currentIndex().row();
STRONG_LEARN *strongLearn=(strongLearnList[indexStrongLearn])->strongLearn;//Extraemos el clasificador fuerte

double threshold=strongLearn->threshold;
double dMin=strongLearn->dMin;
double fMax=strongLearn->fMax;
double dMinAchieved=strongLearn->dMinAchieved;
double fMaxAchieved=strongLearn->fMaxAchieved;
int numberTrees=strongLearn->weakLearns.size();
int expectedNumberFeaturestoEvaluate=strongLearn->getExpectedNumberFeaturestoEvaluate();
int numberFeatureInStrongLearn=strongLearn->getNumberFeatures();


QMessageBox information;
information.setWindowTitle(QString::fromUtf8("INFORMACIÓN DEL STRONG LEARN"));
QSpacerItem* horizontalSpacer = new QSpacerItem(550, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);


information.setText(
QString::fromUtf8("Umbral=")+QString::number(threshold)+
QString::fromUtf8("\nTasa mínima de detección aceptable establecida por etapa=")+QString::number(dMin)+
QString::fromUtf8("\nTasa máxima de falsos positivos aceptable establecida por etapa=")+QString::number(fMax)+
QString::fromUtf8("\nTasa de detección alcanzada=")+QString::number(dMinAchieved)+
QString::fromUtf8("\nTasa de falsos positivos alcanzada=")+QString::number(fMaxAchieved)+
QString::fromUtf8("\nCantidad de arboles totales=")+QString::number(numberTree)+
QString::fromUtf8("\nCantidad de características totales=")+QString::number(numberFeatureInStrongLearn)+
QString::fromUtf8("\nNumero esperado de cracterísticas a evaluar por cada subventana=")+QString::number(expectedNumberFeaturestoEvaluate)
);


QGridLayout* layout = (QGridLayout*)information.layout();
layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

information.exec(); 


}


void CASCADE_VIEW::seeInformationCascade()
{

if(strongLearnList.size()==0){
QMessageBox::warning(this, tr("WARNING"),QString("Debe cargar o entrenar un clasificador previamente"),QMessageBox::Ok);
return;
}

if(G_THREAD_TRAINING->isRunning()==true)
{
QMessageBox::warning(this, tr("WARNING"),QString("Debe parar el entrenamiento previamente"),QMessageBox::Ok);
return;
}

CASCADE_CLASSIFIERS *cascadeClassifier=G_THREAD_TRAINING->classifier;//Extraemos el clasificador fuerte


double D=cascadeClassifier->D;/*Tasa global de detección*/
double F=cascadeClassifier->F;/*Tasa global de falsos positivos*/
double dMin=cascadeClassifier->dMin;
double fMax=cascadeClassifier->fMax;
double fGoal=cascadeClassifier->fGoal;
double percentageQuantityStop=cascadeClassifier->percentageQuantityStop;
double percentageEntropyStop=cascadeClassifier->percentageEntropyStop;
double maxDepth=cascadeClassifier->maxDepth; 
int numberStrongLearn=cascadeClassifier->strongLearns.size();

int numberTree=0;
for(int i=0;i<numberStrongLearn;i++)
numberTree=numberTree+cascadeClassifier->strongLearns[i]->weakLearns.size();

int numberFeaturesInCascade=cascadeClassifier->getNumberFeatures();
int numberExpectedFeatureToEvaluate=cascadeClassifier->getExpectedNumberFeaturestoEvaluate();


QMessageBox information;
information.setWindowTitle(QString::fromUtf8("INFORMACIÓN DEL CLASIFICADOR EN CASCADA"));
QSpacerItem* horizontalSpacer = new QSpacerItem(550, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);


information.setText(
QString::fromUtf8("Tasa global actual de detección=")+QString::number(D)+
QString::fromUtf8("\nTasa global actual de falsos positivos=")+QString::number(F)+
QString::fromUtf8("\nTasa mínima de detección aceptable establecida por etapa=")+QString::number(dMin)+
QString::fromUtf8("\nTasa máxima de falsos positivos aceptable establecida por etapa=")+QString::number(fMax)+
QString::fromUtf8("\nTasa de falsos positivos máxima establecida para la cascada=")+QString::number(fGoal)+
QString::fromUtf8("\nPorcentaje de imágenes del nodo raíz mínima en cada nodo terminal=")+QString::number(percentageQuantityStop)+QString("%")+
QString::fromUtf8("\nPorcentaje de entropía del nodo raíz mínima esperada en cada división=")+QString::number(percentageEntropyStop)+QString("%")+
QString::fromUtf8("\nMáxima profundidad de los arboles=")+QString::number(maxDepth)+
QString::fromUtf8("\nCantidad de clasificadores fuertes=")+QString::number(numberStrongLearn)+
QString::fromUtf8("\nCantidad de arboles totales=")+QString::number(numberTree)+
QString::fromUtf8("\nCantidad de características totales=")+QString::number(numberFeaturesInCascade)+
QString::fromUtf8("\nNumero esperado de cracterísticas a evaluar por cada subventana=")+QString::number(numberExpectedFeatureToEvaluate)+
QString::fromUtf8("\nAncho de las imágenes de entrenamiento=")+QString::number(G_WIDTH_IMAGE)+
QString::fromUtf8("\nAlto de las imágenes de entrenamiento=")+QString::number(G_HEIGHT_IMAGE)+
QString::fromUtf8("\nNumero de caracterísitcas derivadas del tamaño de las imágenes=")+QString::number(G_NUMBER_FEATURE)

);


QGridLayout* layout = (QGridLayout*)information.layout();
layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

information.exec(); 




}

