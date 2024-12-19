#include "guiCongfigureTraining.h"
//_____ARCHIVOS DE CABECERA DE QT____________________
#include <QtGui>

//________________LIBRERIAS OPENCV___________________
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "training.h"

#include <omp.h>

extern std::string G_NAME_SET_POSITIVE_DATA_BASE;
extern WINDOWS_PRINCIPAL_TREE *G_WINDOW_TREE;
extern bool G_OPERATION_END_NO_IMAGES;//Indica si la operación termino por falta de imágenes (true)
THREAD_TRAINING *G_THREAD_TRAINING;

extern int G_ACTUAL_NUMBER_NEGATIVE;
extern int G_REQUIRED_QUANTITY;

extern QVector<QRgb>  sColorTable;//Declarado en includes.cpp


THREAD_TRAINING::THREAD_TRAINING(std::string pathFile,bool isLoad):isLoad(isLoad)
{

G_THREAD_TRAINING=this;

classifier=NULL;

std::cout<<"Creando, dirección al entrar="<<classifier<<"\n";

if(isLoad==false){
std::cout<<"Clasificador creado\n";
loadSetPositiveImages(pathFile);//Se cargan inicialmente las el set de imágenes positivas
classifier=new CASCADE_CLASSIFIERS(false);
}else
{
std::cout<<"Clasificador cargado\n";
classifier=new CASCADE_CLASSIFIERS;
classifier->loadCascadeClasifier(pathFile);
}


std::cout<<"Creando, dirección al salir="<<classifier<<" dir="<<this<<"\n";

}


THREAD_TRAINING::~THREAD_TRAINING()
{
qDebug()<<"Destruyendo datos de el hilo\n";


stop();//Por si el algoritmo esta corriendo
mutex.lock();
if(classifier!=NULL){
delete classifier;
classifier=NULL;
}
mutex.unlock();

wait();

}


void THREAD_TRAINING::stop()
{

std::cout<<"Entre  a stop\n";


if(isRunning()){//Si el hilo esta corriendo el hilo enviara al otro hilo la señal de endStop
flagStop=true; //Indica que el entrenamiento finalizo sin que el algoritmo converja
STOP_TRAINING();
}else
{//Si el hilo esta parado la señal de endStop se envía inmediatamente
emit endStop();
}


}

bool THREAD_TRAINING::messageNoImages()
{


int question = QMessageBox::warning(NULL,QString::fromUtf8("INFORMACIÓN"),
                               QString::fromUtf8("Faltan imágenes para completar el conjunto de imágenes negativas, la cantidad actual es ")+QString::number(G_ACTUAL_NUMBER_NEGATIVE)+" y se requiere completar "+QString::number(G_REQUIRED_QUANTITY)+QString::fromUtf8(", por favor agregue mas imágenes a la carpeta ")+G_NAME_DIRECTORY_SET_NEGATIVE+" y pulse OK, de lo contrario detenga el entrenamiento pulsando CANCEL.\n",
                               QMessageBox::Ok|QMessageBox::Cancel);



if(question==QMessageBox::Ok)
return true;
else
return false;//Por defecto

}



void THREAD_TRAINING::run()
{

mutex.lock();

flagStop=false;//Inicia el entrenamiento, aun stop no se ha presionado

std::cout<<"El algoritmo inicia:\n";
//________INTERNAMENTE LA CLASE CONTIENE LAS ITERACIONES________________//
classifier->startTraining();

std::cout<<"El algoritmo finaliza:\n";

std::cout<<"G_STOP_TRAINING_FLAG="<<G_STOP_TRAINING_FLAG<<"  flagStop="<<flagStop<<"\n";
if((G_STOP_TRAINING_FLAG==true)&&(flagStop==false)&&(G_OPERATION_END_NO_IMAGES==false)){
//Significa que el algoritmo termino sin que se haya presionado stop
emit endTraining();//Se envía la señal que el entrenamiento termino
}

if((!G_STOP_TRAINING_FLAG)||(G_OPERATION_END_NO_IMAGES==true))
emit endStop();

mutex.unlock();





}






CONFIGURE_TRAINING::CONFIGURE_TRAINING(QWidget *parent):QWidget(parent)
{



imgOPencv=new cv::Mat;

threadTraining=NULL;
trainingStoped=false;

//________Aquí configuro algunos mensajes previos___________________
messagePressStop.setText("Por favor pare primero el entrenamiento");
messagePressStop.setWindowTitle(QString::fromUtf8("INFORMACIÓN"));

messageDestroyingThread.setText("Espere mientras se destruye el hilo de entrenamiento");
messageDestroyingThread.setWindowTitle(QString::fromUtf8("INFORMACIÓN"));
messageDestroyingThread.setStandardButtons(QMessageBox::NoButton);



//__________________________________________________________________




//________OBJETOS DE EL LABEL INFO GRÁFICO__________//


labelInfoGraphic=new QLabel;



sliderFeatures=new QSlider(Qt::Horizontal);
sliderFeatures->setFixedWidth(50);
connect(sliderFeatures,SIGNAL(valueChanged(int)),this,SLOT(evaluateFeature()));


sliderNumImages=new QSlider(Qt::Horizontal);
sliderNumImages->setFixedWidth(50);
connect(sliderNumImages,SIGNAL(valueChanged(int)),this,SLOT(setImage()));


QIntValidator *numberPermitted= new QIntValidator;
numberPermitted->setBottom(0);//Para solo permitir mayores e iguales a cero
lineEditNumFeature=new QLineEdit;  
lineEditNumFeature->setFixedWidth(60);
connect(lineEditNumFeature,SIGNAL(returnPressed()),this,SLOT(actualizeLineEdit()));
lineEditNumFeature->setValidator(numberPermitted);


labelShowEvalFeatures=new QLabel;
labelShowEvalFeatures->setFixedWidth(108);
labelShowEvalFeatures->setStyleSheet("color: rgb(255, 0, 0);");
labelShowEvalFeatures->setText("eval:");


QHBoxLayout *infoGraphicLayout1=new QHBoxLayout;
QFormLayout *formLayoutNpd = new QFormLayout;
formLayoutNpd->addRow("NPD:",sliderFeatures);
infoGraphicLayout1->addLayout(formLayoutNpd);
infoGraphicLayout1->addWidget(lineEditNumFeature);
infoGraphicLayout1->addWidget(labelShowEvalFeatures);
infoGraphicLayout1->setSpacing(0);


QFormLayout *formLayoutImg = new QFormLayout;
formLayoutImg->addRow("IMG:",sliderNumImages);




QHBoxLayout *infoGraphicLayoutPrincipal=new QHBoxLayout;
infoGraphicLayoutPrincipal->addLayout(formLayoutImg);
infoGraphicLayoutPrincipal->addLayout(infoGraphicLayout1);

infoGraphicLayoutPrincipal->setSpacing(5);

labelInfoGraphic->setLayout(infoGraphicLayoutPrincipal);

//____________________________________________//





/*_______LUGARESDE EDICIÓN__________*/

editLineNumberNegatives=new QLineEdit;
editLineNumberStages=new QLineEdit;
editLineD_min=new QLineEdit;
editLineF_max=new QLineEdit;
editLinePercentageQuantityStop=new QLineEdit;
editLinePercentageEntropyStop=new QLineEdit;
editLineMax_depth=new QLineEdit;
editLineFeaturesChargedInMemory=new QLineEdit;
editLineNumberCores=new QLineEdit;


editLineNumberNegatives->setMaximumWidth(60);
editLineNumberStages->setMaximumWidth(50);
editLineD_min->setMaximumWidth(50);
editLineF_max->setMaximumWidth(50);
editLinePercentageQuantityStop->setMaximumWidth(50);
editLinePercentageEntropyStop->setMaximumWidth(50);
editLineMax_depth->setMaximumWidth(40);
editLineFeaturesChargedInMemory->setMaximumWidth(60);
editLineNumberCores->setMaximumWidth(40);




editLineNumberNegatives->setValidator(new QIntValidator(1,65536/2)); //65536 es el limite de imágenes de entrenamiento
editLineNumberStages->setValidator(new QIntValidator(1,79800));
QDoubleValidator* Dpermitted=new QDoubleValidator(0,1,3,editLineD_min); 
Dpermitted->setNotation(QDoubleValidator::StandardNotation); 
editLineD_min->setValidator(Dpermitted);
editLineF_max->setValidator(Dpermitted);
editLinePercentageQuantityStop->setValidator(new QIntValidator(1,100));
editLinePercentageEntropyStop->setValidator(new QIntValidator(1,100));
editLineMax_depth->setValidator(new QIntValidator(1,100));
editLineNumberCores->setValidator(new QIntValidator(1,16));







/*_________________________________*/


actRenovateSetFeature=new QCheckBox(tr("renovar NPD"));

buttonStartTraining=new QPushButton(tr("Entrenar"));
buttonStartTraining->setAutoDefault(false);
connect(buttonStartTraining,SIGNAL(clicked()),this,SLOT(starTraining()));


buttonStopTraining=new QPushButton(tr("Cancelar"));
buttonStopTraining->setAutoDefault(false);
buttonStopTraining->setEnabled(false);
connect(buttonStopTraining,SIGNAL(clicked()),this,SLOT(stopTraining()));


//_______________________ORGANIZACIÓN____________________________________________//

QFormLayout *formLayout1 = new QFormLayout;
formLayout1->addRow(QString::fromUtf8("# imágenes negativas:"),editLineNumberNegatives);
formLayout1->addRow(QString::fromUtf8("% cantidad de parada:"),editLinePercentageQuantityStop);
formLayout1->addRow(QString::fromUtf8("% entropía de parada:"),editLinePercentageEntropyStop);


QFormLayout *formLayout3 = new QFormLayout;
formLayout3->addRow(tr("profundidad arboles:"),editLineMax_depth);
formLayout3->addRow(tr("bloques de memoria:"),editLineFeaturesChargedInMemory);


QFormLayout *formLayout2= new QFormLayout;
formLayout2->addRow(tr("Etapas:"),editLineNumberStages);
formLayout2->addRow(tr("D:"),editLineD_min);
formLayout2->addRow(tr("fmax:"),editLineF_max);

QGridLayout *layoutPrincipal=new QGridLayout;


layoutPrincipal->addLayout(formLayout2,0,0,3,4,Qt::AlignLeft);
layoutPrincipal->addLayout(formLayout1,0,5,3,4,Qt::AlignRight);
layoutPrincipal->addLayout(formLayout3,3,0,2,7,Qt::AlignLeft);
layoutPrincipal->addWidget(new QLabel("# Cores"),3,7,1,1,Qt::AlignRight);
layoutPrincipal->addWidget(editLineNumberCores,3,8,1,1,Qt::AlignRight);
layoutPrincipal->addWidget(actRenovateSetFeature,4,7,1,2,Qt::AlignRight);
layoutPrincipal->addWidget(buttonStartTraining,5,8,1,1,Qt::AlignRight);
layoutPrincipal->addWidget(buttonStopTraining,5,0,1,1,Qt::AlignLeft);


setLayout(layoutPrincipal);
//____________________________________________________________________________________//


}


CONFIGURE_TRAINING::~CONFIGURE_TRAINING()
{

delete imgOPencv;

//if(!deleteClassifier(true))return;

qDebug()<<"Destruyendo  en ~CONFIGURE_TRAINING()\n";
if(threadTraining!=NULL){
threadTraining->stop();
threadTraining->wait();
delete threadTraining;
}

}


void CONFIGURE_TRAINING::starTraining()
{


if(editLineNumberStages->text().toInt()<=0){
QMessageBox::information(this, QString::fromUtf8("INFORMACIÓN"),tr("El numero de etapas debe ser mayor o igual a 1.\n"),QMessageBox::Ok);
return;
}



if((editLineNumberNegatives->text()=="")||(editLineNumberStages->text()=="")||(editLineD_min->text()=="")||(editLineF_max->text()=="")||(editLinePercentageQuantityStop->text()=="")||(editLinePercentageEntropyStop->text()=="")||(editLineMax_depth->text()=="")||(editLineFeaturesChargedInMemory->text()=="")||(editLineNumberCores->text()==""))
{
QMessageBox::warning(this, tr("WARNING"),tr("Faltan campos por llenar.\n"),QMessageBox::Ok);
}





//____________________________________________________________________________________________________________
if(trainingStoped==true){

//std::cout<<"Reentrenamiento\n";

//Eliminacion de la memoria
//Recuerde que previamente se pulso stop
if(threadTraining!=NULL){
disconnect(threadTraining,SIGNAL(endTraining()),this,SLOT(endTraining()));
delete threadTraining;
threadTraining=NULL;
}


//____________________Cuando el clasificador a sido cargado______________________

if(isLoad)
{
//std::cout<<"Es cargado\n";
QString tempNumberFeatures=editLineFeaturesChargedInMemory->text();
QString tempNumberCores=editLineNumberCores->text();

cv::FileStorage fileTemp("temp.xml",cv::FileStorage::READ);
if(fileTemp.isOpened())
{
//std::cout<<"Leyendo temporal\n";
fileTemp.release();
std::string pathTemp=path;
loadClasifier("temp.xml");//Continua el entrenamiento
G_WINDOW_TREE->readCommands();//IMPORTANTE: CARGA LA PARTE GRÁFICA DE LOS NODOS;
path=pathTemp;
}else
{
//std::cout<<"Cargando nuevamente\n";
//std::string pathTemp=path;
loadClasifier(path);
G_WINDOW_TREE->readCommands();//IMPORTANTE: CARGA LA PARTE GRÁFICA DE LOS NODOS;
//path=pathTemp;
}

editLineFeaturesChargedInMemory->setText(tempNumberFeatures);
editLineNumberCores->setText(tempNumberCores);

//isLoad=true;
trainingStoped=true;

}
//_______________________________________________________________________________
else
{
//std::cout<<"Es creado\n";
//______________Cuando el clasificador ha sido creado____________________________
//Si ha habido una edición en la configuración debe empezar nuevamente el entrenamiento, si no se continua con el archivo temporal
if(isEdited())//Ha habido una edición??
{
//std::cout<<"Ha sido editada la configuración\n";
saveConfig();//Guarda la configuración previa
createClasifier(path);
previousConfig();//Establece la configuración previa
//isLoad=false;
//trainingStoped=false;
}
else
{


cv::FileStorage fileTemp("temp.xml",cv::FileStorage::READ);
if(fileTemp.isOpened())
{
//std::cout<<"Leyendo temporal\n";
fileTemp.release();
QString tempNumberFeatures=editLineFeaturesChargedInMemory->text();
QString tempNumberCores=editLineNumberCores->text();

std::string pathTemp=path;
loadClasifier("temp.xml");//Continua el entrenamiento
G_WINDOW_TREE->readCommands();//IMPORTANTE: CARGA LA PARTE GRÁFICA DE LOS NODOS;
path=pathTemp;

editLineFeaturesChargedInMemory->setText(tempNumberFeatures);
editLineNumberCores->setText(tempNumberCores);
isLoad=false;
trainingStoped=true;
}else
{
//std::cout<<"Creando nuevamente\n";
saveConfig();
createClasifier(path);
previousConfig();
//isLoad=false;
//trainingStoped=false;
}

}
//_______________________________________________________________________________

//isLoad=false;//Ya que realmente es creado

}







//trainingStoped=true;//Para que la secuencia funcione
}
//____________________________________________________________________________________________________________







if((!isLoad)&&(trainingStoped==false)){

//std::cout<<"Establecimiento clasificador cargado\n";

numberNegatives=editLineNumberNegatives->text().toInt();

dMin=editLineD_min->text().toDouble();
if(dMin>=1)dMin=1;
fMax=editLineF_max->text().toDouble();
if(fMax>=1)fMax=1;

F=std::pow(fMax,editLineNumberStages->text().toInt());
stages=editLineNumberStages->text().toInt();

percentageQuantityStop=editLinePercentageQuantityStop->text().toInt();
percentageEntropyStop=editLinePercentageEntropyStop->text().toInt();
max_depth=editLineMax_depth->text().toInt();

renovateSetFeature=false;
if(actRenovateSetFeature->checkState()==Qt::Checked)
renovateSetFeature=true;


threadTraining->classifier->set_negatives(numberNegatives);

//if(!trainingStoped)//La primera debe asignar F, las subsiguientes etapas F cambiara y su valor inicial es fGoal
threadTraining->classifier->set_F(F);


threadTraining->classifier->set_d_min(dMin);
threadTraining->classifier->set_f_max(fMax);
threadTraining->classifier->set_percentageQuantityStop(percentageQuantityStop);
threadTraining->classifier->set_percentageEntropyStop(percentageEntropyStop);
threadTraining->classifier->set_max_depth(max_depth);
threadTraining->classifier->set_renovateSetFeature(renovateSetFeature);

}else{

if(threadTraining->classifier->get_F()<=threadTraining->classifier->get_fGoal())
{

QMessageBox::information(this, QString::fromUtf8("INFORMACIÓN"),tr("El clasificador ya ha alcanzado el criterio de falsos positivos.\n"),QMessageBox::Ok);

return;
}


}



nc=editLineNumberCores->text().toInt();
if(nc<1)nc=1;

numberFeature=editLineFeaturesChargedInMemory->text().toInt();
if(numberFeature<100)numberFeature=100;

threadTraining->classifier->setNumberCore(nc);
threadTraining->classifier->setNumberFeaturesChargedInMemory(numberFeature);

buttonStartTraining->setEnabled(false);


saveConfig();//Almacena el ultimo estado de las variables


threadTraining->start();
buttonStopTraining->setEnabled(true);
}







void CONFIGURE_TRAINING::stopTraining()
{

QMessageBox::StandardButton question;
question=QMessageBox::question(this, "PREGUNTA", "Realmente desea detener el entrenamiento?",QMessageBox::Yes|QMessageBox::No);
if(question==QMessageBox::No){
return;
}

messageDestroyingThread.show();


std::cout<<"Presiono stop\n";
buttonStopTraining->setEnabled(false);
emit stop();
std::cout<<"Salí de  stop\n";

/*
//buttonStartTraining->setEnabled(false);
std::cout<<"Stop ejecutado\n";

trainingStoped=true;



if(isLoad==false)
{

//________________HABILITANDO OPCIONES_________________________
editLineNumberNegatives->setEnabled(true);
editLineNumberStages->setEnabled(true);
editLineD_min->setEnabled(true);
editLineF_max->setEnabled(true);
editLinePercentageQuantityStop->setEnabled(true);
editLinePercentageEntropyStop->setEnabled(true);
editLineMax_depth->setEnabled(true);
actRenovateSetFeature->setEnabled(true);
//________________________________________________________________


}


buttonStartTraining->setEnabled(true);

*/

}


void CONFIGURE_TRAINING::createClasifier(std::string pathFile)
{

std::cout<<"CREANDO DE="<<pathFile<<"\n";


QFile::remove("temp.xml");
trainingStoped=false;
fileIsSave=false;
path=pathFile;
isLoad=false;


if(threadTraining!=NULL){
deleteClassifier();
}

threadTraining=new THREAD_TRAINING(pathFile,false);
connect(threadTraining,SIGNAL(endTraining()),this,SLOT(endTraining()));
connect(this,SIGNAL(stop()),threadTraining,SLOT(stop()));
connect(threadTraining,SIGNAL(endStop()),this,SLOT(endStop()));


editLineNumberNegatives->setText(QString::number(G_LABEl_POSITIVE.size()));
threadTraining->classifier->set_negatives(G_LABEl_POSITIVE.size());


editLineNumberStages->setText("15");
editLineD_min->setText("0.999");
editLineF_max->setText("0.2");
editLinePercentageQuantityStop->setText("7");
editLinePercentageEntropyStop->setText("7");
editLineMax_depth->setText("5");
editLineFeaturesChargedInMemory->setText(QString::number(G_NUMBER_FEATURE));
editLineFeaturesChargedInMemory->setValidator(new QIntValidator(1,G_NUMBER_FEATURE));
numberFeature=G_NUMBER_FEATURE;

actRenovateSetFeature->setCheckState(Qt::Unchecked);
bool renovateSetFeature=false;

if(omp_get_max_threads()==1)
editLineNumberCores->setText(QString::number(omp_get_max_threads()));
else
editLineNumberCores->setText(QString::number(omp_get_max_threads()-1));



//_____________HABILITAMOS LAS OPCIONES__________________
buttonStopTraining->setEnabled(true);
editLineNumberNegatives->setEnabled(true);
editLineNumberStages->setEnabled(true);  
editLineD_min->setEnabled(true);
editLineF_max->setEnabled(true);
editLinePercentageQuantityStop->setEnabled(true);
editLinePercentageEntropyStop->setEnabled(true);
editLineMax_depth->setEnabled(true);
editLineFeaturesChargedInMemory->setEnabled(true);
editLineNumberCores->setEnabled(true);

actualizeSliders();//ACTUALIZAMOS LOS SLIDERS

}



void CONFIGURE_TRAINING::loadClasifier(std::string pathFile)
{


trainingStoped=false;
fileIsSave=false;
path=pathFile;
isLoad=true;


if(threadTraining!=NULL){
deleteClassifier();
}

threadTraining=new THREAD_TRAINING(pathFile,true);
connect(threadTraining,SIGNAL(endTraining()),this,SLOT(endTraining()));
connect(this,SIGNAL(stop()),threadTraining,SLOT(stop()));
connect(threadTraining,SIGNAL(endStop()),this,SLOT(endStop()));

numberNegatives=threadTraining->classifier->get_negatives();
dMin=threadTraining->classifier->get_dMin();
fMax=threadTraining->classifier->get_fMax();
stages=std::log(threadTraining->classifier->get_fGoal())/log(fMax);
percentageQuantityStop=threadTraining->classifier->get_percentageQuantityStop();
percentageEntropyStop=threadTraining->classifier->get_percentageEntropyStop();
max_depth=threadTraining->classifier->get_max_depth();
renovateSetFeature=threadTraining->classifier->get_renovateSetFeature();

editLineNumberNegatives->setText(QString::number(numberNegatives));
editLineNumberStages->setText(QString::number(stages));
editLineD_min->setText(QString::number(dMin));
editLineF_max->setText(QString::number(fMax));
editLinePercentageQuantityStop->setText(QString::number(percentageQuantityStop));
editLinePercentageEntropyStop->setText(QString::number(percentageEntropyStop));
editLineMax_depth->setText(QString::number(max_depth));


if(renovateSetFeature==true)
actRenovateSetFeature->setCheckState(Qt::Checked);


//________________DESHABILITANDO OPCIONES_________________________
editLineNumberNegatives->setEnabled(false);
editLineNumberStages->setEnabled(false);
editLineD_min->setEnabled(false);
editLineF_max->setEnabled(false);
editLinePercentageQuantityStop->setEnabled(false);
editLinePercentageEntropyStop->setEnabled(false);
editLineMax_depth->setEnabled(false);
actRenovateSetFeature->setEnabled(false);
//________________________________________________________________




editLineFeaturesChargedInMemory->setText(QString::number(G_NUMBER_FEATURE));
editLineFeaturesChargedInMemory->setValidator(new QIntValidator(1,G_NUMBER_FEATURE));
numberFeature=G_NUMBER_FEATURE;


if(omp_get_max_threads()==1)
editLineNumberCores->setText(QString::number(omp_get_max_threads()));
else
editLineNumberCores->setText(QString::number(omp_get_max_threads()-1));


editLineFeaturesChargedInMemory->setEnabled(true);
editLineNumberCores->setEnabled(true);


actualizeSliders();//ACTUALIZAMOS LOS SLIDERS

}



bool CONFIGURE_TRAINING::deleteClassifier(bool question)
{


if(threadTraining->isRunning()==true){//Para curarnos en salud
//Llegados a este punto significa que el algoritmo esta aun corriendo y debe presionarse stop primeramente

messagePressStop.show();

return false;
}




if(question==true){

QMessageBox::StandardButton question;
question=QMessageBox::question(this, "Test", "Desea detener el entrenamiento?",QMessageBox::Yes|QMessageBox::No);

if(question==QMessageBox::No){
return false;
}

}



if(threadTraining!=NULL){
buttonStartTraining->setEnabled(true);
buttonStopTraining->setEnabled(false);
threadTraining->stop();
threadTraining->wait();
disconnect(threadTraining,SIGNAL(endTraining()),this,SLOT(endTraining()));
disconnect(this,SIGNAL(stop()),threadTraining,SLOT(stop()));
disconnect(threadTraining,SIGNAL(endStop()),this,SLOT(endStop()));
delete threadTraining;
threadTraining=NULL;
}

trainingStoped=false;

saveCopyTraining();


return true;
}



void CONFIGURE_TRAINING::endTraining()
{



buttonStartTraining->setEnabled(false);
buttonStopTraining->setEnabled(false);
editLineNumberNegatives->setEnabled(false);
editLineNumberStages->setEnabled(false);  
editLineD_min->setEnabled(false);
editLineF_max->setEnabled(false);
editLinePercentageQuantityStop->setEnabled(false);
editLinePercentageEntropyStop->setEnabled(false);
editLineMax_depth->setEnabled(false);
editLineFeaturesChargedInMemory->setEnabled(false);
editLineNumberCores->setEnabled(false);
actRenovateSetFeature->setEnabled(false);

QMessageBox::information(this,tr(" CONGRATULATIONS"),tr("Su clasificador esta entrenado\n"),QMessageBox::Ok);

trainingStoped=true;//Indica que el entrenamiento esta parado

saveCopyTraining();

}

void CONFIGURE_TRAINING::saveCopyTraining()
{


if(fileIsSave==false){//Si no ha sido guardada


cv::FileStorage fileTemp("temp.xml",cv::FileStorage::READ);
if(fileTemp.isOpened())//SI el archivo existe
{

QMessageBox::StandardButton question;
question=QMessageBox::question(this, "Test", "Desea guardar una copia?",QMessageBox::Yes|QMessageBox::No);

if(question==QMessageBox::Yes){

QString file_temp=QFileDialog::getSaveFileName(this, tr("Save Data Base"),QDir::homePath()+QString("/classifier.xml"),tr("Data(*.xml)"));

//_________Aquí llamamos al proceso que imprime_______________
QProcess processPrintf;
QString program="./printfClassifier";
QStringList arguments;
arguments<<file_temp;
processPrintf.start(program,arguments);
processPrintf.waitForFinished(-1);
//_____________________________________________________________

fileIsSave=true;

}

fileTemp.release();
}


}



}


void CONFIGURE_TRAINING::saveConfig()
{

p_numberNegatives=editLineNumberNegatives->text().toInt();
p_dMin=editLineD_min->text().toDouble();
p_fMax=editLineF_max->text().toDouble();
//p_F=F;
p_stages=editLineNumberStages->text().toInt();
p_percentageQuantityStop=editLinePercentageQuantityStop->text().toInt();
p_percentageEntropyStop=editLinePercentageEntropyStop->text().toInt();
p_max_depth=editLineMax_depth->text().toInt();

if(actRenovateSetFeature->checkState()==Qt::Checked)
p_renovateSetFeature=true;
else
p_renovateSetFeature=false;

p_numberFeature=editLineFeaturesChargedInMemory->text().toInt();
p_nc=editLineNumberCores->text().toInt();

}


void CONFIGURE_TRAINING::previousConfig()
{
editLineNumberNegatives->setText(QString::number(p_numberNegatives));
editLineNumberStages->setText(QString::number(p_stages));
editLineD_min->setText(QString::number(p_dMin));
editLineF_max->setText(QString::number(p_fMax));
editLinePercentageQuantityStop->setText(QString::number(p_percentageQuantityStop));
editLinePercentageEntropyStop->setText(QString::number(p_percentageEntropyStop));
editLineMax_depth->setText(QString::number(p_max_depth));
editLineFeaturesChargedInMemory->setText(QString::number(p_numberFeature));
editLineNumberCores->setText(QString::number(p_nc));

if(p_renovateSetFeature)
actRenovateSetFeature->setCheckState(Qt::Checked);
else
actRenovateSetFeature->setCheckState(Qt::Unchecked);

}

bool CONFIGURE_TRAINING::isEdited()
{


bool tempRenovateSetFeature=false;
if(actRenovateSetFeature->checkState()==Qt::Checked)
tempRenovateSetFeature=true;

if(

(p_numberNegatives!=editLineNumberNegatives->text().toInt())||
(p_dMin!=editLineD_min->text().toDouble())||
(p_fMax!=editLineF_max->text().toDouble())||
(p_stages!=editLineNumberStages->text().toInt())||
(p_percentageQuantityStop!=editLinePercentageQuantityStop->text().toInt())||
(p_percentageEntropyStop!=editLinePercentageEntropyStop->text().toInt())||
(p_max_depth!=editLineMax_depth->text().toInt())||
(p_renovateSetFeature!=tempRenovateSetFeature)

)
return true;
else 
return false;

}



void CONFIGURE_TRAINING::endStop()
{


std::cout<<"Stop ejecutado\n";
buttonStopTraining->setEnabled(false);//Se repite debido a que puede llamarse o desde este hilo o independiente mente desde el otro hilo 




if(isLoad==false)
{

//________________HABILITANDO OPCIONES_________________________
editLineNumberNegatives->setEnabled(true);
editLineNumberStages->setEnabled(true);
editLineD_min->setEnabled(true);
editLineF_max->setEnabled(true);
editLinePercentageQuantityStop->setEnabled(true);
editLinePercentageEntropyStop->setEnabled(true);
editLineMax_depth->setEnabled(true);
actRenovateSetFeature->setEnabled(true);
//________________________________________________________________


}


buttonStartTraining->setEnabled(true);

trainingStoped=true;//Indica que el entrenamiento esta parado
messageDestroyingThread.hide();

}



void CONFIGURE_TRAINING::actualizeSliders()
{

sliderNumImages->setMaximum(G_LABEl_POSITIVE.size()-1);
sliderFeatures->setMaximum(G_NUMBER_FEATURE-1);

sliderNumImages->setValue(0);
sliderFeatures->setValue(0);

setImage();

}


void CONFIGURE_TRAINING::actualizeLineEdit()
{

QString str=lineEditNumFeature->text();
int num=str.toInt();

if(str==QString(""))
lineEditNumFeature->setText(QString::number(sliderFeatures->value()));
else if(num<G_NUMBER_FEATURE)
sliderFeatures->setValue(num);
else{
lineEditNumFeature->setText(QString::number(sliderFeatures->value()));
QMessageBox::warning(this, tr("WARNING"),QString::fromUtf8("El rango de características esta entre 0 y ")+QString::number(G_NUMBER_FEATURE-1),QMessageBox::Ok);
}


lineEditNumFeature->clearFocus(); 
}


void CONFIGURE_TRAINING::setImage()
{

int numImg=sliderNumImages->value();//El numero de la imagen

(*imgOPencv)=(G_LABEl_POSITIVE[numImg]).clone();

img=QImage(imgOPencv->ptr<uchar>(0),imgOPencv->cols,imgOPencv->rows,imgOPencv->step,QImage::Format_Indexed8);
img.setColorTable(sColorTable);


if(listFeatures.isEmpty())
evaluateFeature();
else{
QList<int> temp;
seeTree(temp);
}

}


void CONFIGURE_TRAINING::evaluateFeature()
{

static cv::Point2i p;//variable que almacenara la característica NPD
static QImage imgAndFeature;//Imagen que almacenara la imagen y la característica
static int scaleFactor=16;//Factor de escala por que se engrandara la imagen para poder dibujar la característica en ella


int x1,x2,y1,y2;


if(!listFeatures.isEmpty())listFeatures.clear();

//__________extrayendo coordenadas___________ 
p=G_NPD[sliderFeatures->value()];//Se extrae la correspondiente característica NPD
lineEditNumFeature->setText(QString::number(sliderFeatures->value()));

x1=p.x%G_WIDTH_IMAGE;
y1=p.x/G_WIDTH_IMAGE;

x2=p.y%G_WIDTH_IMAGE;
y2=p.y/G_WIDTH_IMAGE;
//___________________________________________

//____________Aquí evaluamos la característica en la imagen____________
//_____________________Prueba___________________________________________________
uchar *M=imgOPencv->ptr<uchar>(0);//Leyendo imagen j-esima del label l-esimo
float npd_c=float(M[p.x]-M[p.y])/(M[p.x]+M[p.y]);//Evaluando característica NPD
//float npd_m=G_NPD_VALUES[256*M[p.x]+M[p.y]];
//float npd_t2=G_NPD_VALUES_FOR_EVALUATION[M[p.x]][M[p.y]];
//std::cout<<"Característica="<<sliderFeatures->value()<<" NPD_C="<<npd_c<<"  y "<<npd_m<<" val tab2="<<npd_t2<<"\n";

labelShowEvalFeatures->setText(QString("eval: ")+QString::number(npd_c));
//_______________________________________________________________________________


//_____________________________________________________________________




     imgAndFeature=img.convertToFormat(QImage::Format_RGB888).scaled(scaleFactor*img.width(),scaleFactor*img.height());
     QPainter painter(&imgAndFeature);
     //Estableciendo estilo del lápiz virtual (pen)
     QPen penHLines(QColor(0,255,0),3,Qt::SolidLine,Qt::RoundCap,Qt::MiterJoin);
     painter.setPen(penHLines);
     painter.setRenderHint(QPainter::Antialiasing);
     
     x1=scaleFactor*(x1+0.5);
     x2=scaleFactor*(x2+0.5);
     y1=scaleFactor*(y1+0.5);
     y2=scaleFactor*(y2+0.5);


     painter.drawLine(x1,y1,x2,y2);
  
   
//____________LA FLECHA DEBE APUTAR HACIA EL PÍXEL DE MAYOR VALOR_______________________
     if(npd_c>=0){
     painter.translate(x1,y1);
     double angle=(180/M_PI)*atan(double(x2-x1)/double(y2-y1));
     painter.rotate(-angle+90);
     painter.drawLine(0,0,10,10);
     painter.drawLine(0,0,10,-10);
     }
     else{
     painter.translate(x2,y2);
     double angle=(180/M_PI)*atan(double(x2-x1)/double(y2-y1));
     painter.rotate(-angle-90);
     painter.drawLine(0,0,10,10);
     painter.drawLine(0,0,10,-10);
     }
//_______________________________________________________________________________________

     emit seeFeature(imgAndFeature);


}



//_____________________AQUÍ SE DIBUJA EL ÁRBOL_______________________________//
void CONFIGURE_TRAINING::seeTree(const QList<int> myListFeatures)
{

if(!myListFeatures.isEmpty())
listFeatures=myListFeatures;

qDebug()<<"listFeatures="<<listFeatures<<"\n";



static cv::Point2i p;//Variable que almacenara la característica NPD
static QImage imgAndFeature;//Imagen que almacenara la imagen y la característica
static int scaleFactor=16;//Factor de escala por que se engrandara la imagen para poder dibujar la característica en ella


int x1,x2,y1,y2;


imgAndFeature=img.convertToFormat(QImage::Format_RGB888).scaled(scaleFactor*img.width(),scaleFactor*img.height());
for(int i=0;i<listFeatures.size();i++){

//__________Extrayendo coordenadas___________ 
p=G_NPD[listFeatures[i]];//Se extrae la correspondiente característica NPD

x1=p.x%G_WIDTH_IMAGE;
y1=p.x/G_WIDTH_IMAGE;

x2=p.y%G_WIDTH_IMAGE;
y2=p.y/G_WIDTH_IMAGE;
//___________________________________________

//____________Aquí evaluamos la característica en la imagen____________
//_____________________Prueba___________________________________________________
uchar *M=imgOPencv->ptr<uchar>(0);//Leyendo imagen j-esima del label l-esimo
float npd_c=float(M[p.x]-M[p.y])/(M[p.x]+M[p.y]);//Evaluando característica NPD
//float npd_m=G_NPD_VALUES[256*M[p.x]+M[p.y]];
//float npd_t2=G_NPD_VALUES_FOR_EVALUATION[M[p.x]][M[p.y]];
//std::cout<<"Característica="<<sliderFeatures->value()<<" NPD_C="<<npd_c<<"  y "<<npd_m<<" val tab2="<<npd_t2<<"\n";

//_______________________________________________________________________________


//_____________________________________________________________________




     
     QPainter painter(&imgAndFeature);
     //Estableciendo estilo del lápiz virtual (pen)
     QPen penHLines(QColor(0,255,0),3,Qt::SolidLine,Qt::RoundCap,Qt::MiterJoin);
     painter.setPen(penHLines);
     painter.setRenderHint(QPainter::Antialiasing);
     
     x1=scaleFactor*(x1+0.5);
     x2=scaleFactor*(x2+0.5);
     y1=scaleFactor*(y1+0.5);
     y2=scaleFactor*(y2+0.5);


     painter.drawLine(x1,y1,x2,y2);
   
//____________LA FLECHA DEBE APUTAR HACIA EL PÍXEL DE MAYOR VALOR_______________________
     if(npd_c>=0){
     painter.translate(x1,y1);
     double angle=(180/M_PI)*atan(double(x2-x1)/double(y2-y1));
     painter.rotate(-angle+90);
     painter.drawLine(0,0,10,10);
     painter.drawLine(0,0,10,-10);
     }
     else{
     painter.translate(x2,y2);
     double angle=(180/M_PI)*atan(double(x2-x1)/double(y2-y1));
     painter.rotate(-angle-90);
     painter.drawLine(0,0,10,10);
     painter.drawLine(0,0,10,-10);
     }
//_______________________________________________________________________________________

    
}

 emit seeFeature(imgAndFeature);

}
//___________________________________________________________________________//



