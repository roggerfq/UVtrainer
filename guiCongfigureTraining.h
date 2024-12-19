#ifndef GUICONFIGURETRAINING_H
#define GUICONFIGURETRAINING_H


#include <QWidget>
#include <QMessageBox>
class QGraphicsItem;



namespace cv
{
class Mat;
}
class CASCADE_CLASSIFIERS;



//________________LIBRERIAS QT_______________________
#include <QThread>
#include <QMutex>


//_________________________________________________________________________________________________________

class THREAD_TRAINING:public QThread
{

Q_OBJECT

//volatile bool stopped;
QMutex mutex;

bool flagStop;

public:
THREAD_TRAINING(std::string pathFile,bool isLoad=false);//isLoad=false para construir, true para cargar
~THREAD_TRAINING();

//void createClassifier();
//void loadClassifier();

bool isLoad;


CASCADE_CLASSIFIERS *classifier;

public slots:
void stop();
bool messageNoImages();
signals:
void endTraining();
void endStop();


protected:
void run();

};


//__________________________________________________________________________________________________________

















/*___________CLASES OCULTAS QT__________*/
class QPushButton;
class QCheckBox;
class QSlider;
class QLineEdit;
class QLabel;



class CONFIGURE_TRAINING:public QWidget
{

Q_OBJECT

THREAD_TRAINING *threadTraining;
std::string path;
bool isLoad;
bool fileIsSave;
bool trainingStoped;

QMessageBox messagePressStop;
QMessageBox messageDestroyingThread;


cv::Mat *imgOPencv;
QImage img;
QImage imgAndFeature;
QList<int> listFeatures;

public:
//________OBJETOS DE EL LABEL GRÁFICO__________//
QLabel *labelInfoGraphic;
QLabel *labelShowEvalFeatures;

QLineEdit *lineEditNumFeature;


QSlider *sliderNumImages;
QSlider *sliderFeatures;
//____________________________________________//
private:

/*_______LUGARES DE EDICIÓN__________*/
QLineEdit *editLineNumberNegatives;
QLineEdit *editLineNumberStages;    
QLineEdit *editLineD_min;
QLineEdit *editLineF_max;
QLineEdit *editLinePercentageQuantityStop;
QLineEdit *editLinePercentageEntropyStop;
QLineEdit *editLineMax_depth;
QLineEdit *editLineFeaturesChargedInMemory;
QLineEdit *editLineNumberCores;

/*_________________________________*/

QCheckBox   *actRenovateSetFeature;

QPushButton *buttonStartTraining;
QPushButton *buttonStopTraining;

//____________ACTUALES________________
int numberNegatives;
double dMin;
double fMax;
double F;
int stages;
int percentageQuantityStop;
int percentageEntropyStop;
int max_depth;
bool renovateSetFeature;

int numberFeature;
int nc;
//____________________________________

//_____________PREVIOS_______________

int p_numberNegatives;
double p_dMin;
double p_fMax;
double p_F;
int p_stages;
int p_percentageQuantityStop;
int p_percentageEntropyStop;
int p_max_depth;
bool p_renovateSetFeature;

int p_numberFeature;
int p_nc;
//___________________________________

public:
CONFIGURE_TRAINING(QWidget *parent = 0);
~CONFIGURE_TRAINING();

public slots:
void starTraining();
void stopTraining();
void createClasifier(std::string pathFile);
void loadClasifier(std::string pathFile);
bool deleteClassifier(bool question=false);
void endTraining();
void saveCopyTraining();
void saveConfig();
void previousConfig();
bool isEdited();
void endStop();

void actualizeSliders();
void actualizeLineEdit();
void setImage();
void evaluateFeature();
void seeTree(const QList<int> myListFeatures);
signals:
void stop();
void seeFeature(const QImage &image);


};
















#endif
