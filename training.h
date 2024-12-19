#ifndef INTERFAZENTRENAMIENTO_H
#define INTERFAZENTRENAMIENTO_H
#include <iostream>

#include <includes.h>


#define GRAPHICS_PART 1 //Descomente esta linea para activar la parte gráfica
//#define GRAPHICS_PART 0 //Descomente esta linea y comente la anterior para desactivar la parte gráfica

#if GRAPHICS_PART==1
//_________Para dibujar los arboles___________________
#include <drawTree.h>
extern WINDOWS_PRINCIPAL_TREE *G_WINDOW_TREE;
//____________________________________________________
//Para representar el clasificador
#include "viewCascade.h"
extern CASCADE_VIEW *G_WINDOWS_CASCADE;

class NODE;
struct DRAW_TREE_COMMANDS
{
DRAW_TREE_COMMANDS():node(NULL){}
NODE *node;//Nodo 
int command;//Comando
/*
1 significa crear primer nodo
2 significa dividir nodo
3 significa asignar como nodo terminal 
4 significa remover nodo de escena
*/
};

#endif

//_______________________________CLASES C++ STL__________________________________

#include<vector>

//_______________________________CLASES OCULTAS OPENCV__________________________________

namespace cv
{
class Mat;
class FileStorage;
class FileNode;
template<typename _Tp> class Point_;
};

typedef cv::Point_<int> Point2i;

/*DEFINICIONES ADELANTADAS PROPIAS*/
class NODE;
class TREE_TRAINING;
typedef double(NODE::*PointerToEvaluationNode)(uchar *) ;
class CASCADE_CLASSIFIERS;

//_______________________________________________________________________________________



//____________________________FUNCIONES DE PRUEBA BORRAR AL FINAL___________________________________
void prueba();//Borrar al final
void tic();
void toc();

/*LISTAS DE VARIABLES GLOBALES*/
extern std::vector<float> G_NPD_VALUES;
extern float G_NPD_VALUES_FOR_EVALUATION[256][256];
extern int G_NUMBER_FEATURE;
extern int G_WIDTH_IMAGE;
extern int G_HEIGHT_IMAGE;
extern Point2i *G_NPD;
extern std::vector< cv::Mat > G_LABEl_POSITIVE;
extern std::vector< cv::Mat > G_LABEl_NEGATIVE;
extern unsigned short int *G_COORDINATE_VECTOR;
extern int G_FEATURES_CHARGED_IN_MEMORY;
extern int G_NUMBER_TOTAL_IMAGES;
extern bool *G_SET_FEATURE;
extern double *G_WEIGHTS_IMAGES;
extern volatile bool G_STOP_TRAINING_FLAG;

/*LISTA DE FUNCIONES*/
bool loadSetPositiveImages(std::string nameFile);
bool loadSetNegativeImages(int numberImagesNegative,CASCADE_CLASSIFIERS *classifier=NULL,bool (*callback)()=&callback_addImagesMessage);/*Carga las imágenes negativas de entrenamiento*/
int generateEvaluations(std::vector<cv::Mat> &labelPositive,std::vector<cv::Mat> &labelNegative);
void generateLockTable();
void generateLockTableEvaluation();
void generateFeatures();
NODE **getNPD(NODE *node,int numberThreads=1);
void STOP_TRAINING();/*Esta función debe llamarse para detener el entrenamiento 

/*______________________________A CONTINUACIÓN SE DEFINEN ALGUNAS CLASES IMPORTANTES____________________________*/


class NODE:public QObject
{
Q_OBJECT
#if GRAPHICS_PART==1
NODE_GRAPHIC *graphicNode;
#endif



class NODE *parent;//Apunta al nodo padre, si es el nodo inicial su valor debe ser cero
class NODE *nodeLeft;//Apunta al nodo izquierdo, si no tiene un nodo que se derive de el debe apuntar a cero
class NODE *nodeRight;//Apunta al nodo derecho, si no tiene un nodo que se derive de el debe apuntar a cero


bool nodeIsTerminal;
bool *imagesInNode;/*En cada posición si la imagen esta en el nodo debe marcar true, si no false.(la eliminacion de la zona de memoria apuntada, es responsabilidad de la clase TREE_TRAINING)*/
int numImages;//Número total de imágenes
int numImagesPositives;//Número de caras en el nodo
int numImagesNegatives;//Número de no caras en el nodo
double entropyExpected;//Almacenara la entropía esperada de la división del nodo
double entropyInNode;//Almacenara la entropía del nodo, en este caso sera una varianza
double threshold;//Umbral de la característica NPD en el nodo
int numFeature;//Almacenara la posición de la característica referente a G_NPD
Point2i *feature;//Almacena las 2 coordenadas de la característica npd
int depth;
double yt;/*Almacenara la media del Nodo Y(t)*/


PointerToEvaluationNode pointerToFunctionEvaluation;/*Puntero a la función de evaluación*/


public:
NODE(NODE *myParent=NULL):parent(myParent),nodeLeft(NULL),nodeRight(NULL),feature(NULL),nodeIsTerminal(false){

pointerToFunctionEvaluation=&NODE::evaluateNodeNoTerminal;


#if GRAPHICS_PART==1
graphicNode=NULL;
#endif

}

NODE(NODE *myParent,bool *initialSetImages,int num_images_positives,int num_images_negatives);
~NODE();
void calculateEntropyInNode();
double evaluateNodeNoTerminal(uchar *image);/*Función de evaluación para nodo NODO terminal*/
double evaluationNodeTerminal(uchar *image);/*Función de evaluación para nodo terminal*/
void setMean();
void setNodeAsTerminal();/*Establece el nodo como terminal*/
bool isNodeTerminal()const;/*Devuelve true si el nodo es terminal, de lo contrario false*/
double evaluateNode(uchar *image);
void printNode(cv::FileStorage *fileNode,NODE *node=NULL);
void loadNode(cv::FileNode nodeRootFile);

#if GRAPHICS_PART==1
//___________Estas funciones sirven para llamar a la señales del nodo raíz______________________
void createFirstNodeGraphic();
void splitNodeGraphic();
void removeFromScene();
void setTerminalNode();
//______________________________________________________________________________________________

#endif

void propieties();/*Función de seguimiento a la clase*/

friend NODE **getNPD(NODE *node,int numberThreads);
friend class TREE_TRAINING;
friend void GENTLE_ADA_BOOST(const TREE_TRAINING *tree);

#if GRAPHICS_PART==1
friend class NODE_GRAPHIC;
friend class CASCADE_VIEW;
friend class WINDOWS_PRINCIPAL_TREE;
#endif

signals:



};


class TREE_TRAINING:public QObject
{

Q_OBJECT

bool *imagesInTree;/*No se acumula, solo sirve de enlace para el nodo raíz para cuando se cargue*/
NODE *nodeRoot;/*Nodo raíz del árbol*/
std::vector<NODE *> terminalNodes;/*Almacenara la lista de nodos terminales*/
std::vector<NODE *> listNodes;

double error;//Almacenara el error de clasificación del árbol
double entropyStop;//Esta variable almacenara la entropia de parada(es una fraccion de la entropia del nodo raiz)
int quantityStop;/*Esta variable almacenara la cantidad de imágenes para que un nodo sea declarado como nodo terminal*/
int maxDepth;
void calculateError();
public:
TREE_TRAINING():nodeRoot(NULL){}
TREE_TRAINING(bool *initialSetImages,int num_images_positives,int num_images_negatives,int percentageQuantityStop=10,double percentageEntropyStop=5,int max_depth=3);/*Los porcentajes están dados con respecto al nodo raíz*/
~TREE_TRAINING();
void splitNode(NODE *node);
double evaluateTree(uchar *image);
void printTreeTraining(cv::FileStorage *fileTreeTraining, int numTree);
void loadWeakLearn(cv::FileNode weakLearnsTrees, int num);
double getExpectedNumberFeaturestoEvaluate();
int getNumberFeatures();

friend void GENTLE_ADA_BOOST(const TREE_TRAINING *tree);
friend class STRONG_LEARN;

signals:
#if GRAPHICS_PART==1
void createFirstNodeGraphic(NODE_GRAPHIC *node);
void splitNodeGraphic(NODE_GRAPHIC *node);
void removeFromScene(NODE_GRAPHIC *node);
void setTerminalNode(NODE_GRAPHIC *node);
#endif

#if GRAPHICS_PART==1
friend class CASCADE_VIEW;
#endif


};


class STRONG_LEARN
{
std::vector<TREE_TRAINING *> weakLearns;
bool *imagesInStrongLearn;/*En cada posición si la imagen esta en el nodo debe marcar true, si no false, estas imágenes corresponden a los nodos raíces de cada árbol. NOTA: Esta zona de memoria debe de ser eliminada en el destructor de esta clase*/
double threshold;
double dMin;
double fMax;
double dMinAchieved;
double fMaxAchieved;
std::vector<double> VPR;
std::vector<double> FPR;
std::vector<double> thresholdsPossible;
public:
STRONG_LEARN():imagesInStrongLearn(NULL){}
STRONG_LEARN(double d_min,double f_max,int percentageQuantityStop=10,double percentageEntropyStop=5,int max_depth=3,std::vector<cv::Mat> &setValidationPositive=G_LABEl_POSITIVE,std::vector<cv::Mat> &setValidationNegative=G_LABEl_NEGATIVE,bool renovateSetFeature=false);
~STRONG_LEARN();
double evaluateStrongLearnWithZeroThreshold(uchar *image);/*Evaluación con umbral cero*/
bool evaluateStrongLearn(uchar *image);/*Evaluación usando threshold*/
bool searchBestThreshold();/*Este método se llamara para actualizar el umbral cada que se agregue un nuevo weakLearns*/
void checkROC(std::vector<cv::Mat> &labelPositive=G_LABEl_POSITIVE,std::vector<cv::Mat> &labelNegative=G_LABEl_NEGATIVE,bool saveROC=false);
void removeNegativeCorrectlyClassified();
double get_dMinAchieved()const;
double get_fMaxAchieved()const;
void printStrongLearn(cv::FileStorage *fileStrongLearn, int stage);
void loadStrongLearn(cv::FileNode fileStrongLearn, int stage);
double getExpectedNumberFeaturestoEvaluate();
int getNumberFeatures();

bool evaluateStrongLearnEvaluation(uchar *image);
double score;//Solo útil para la obtención de la ROC  de validación (ver evaluateClassifier(uchar *image,int number))

friend class CASCADE_CLASSIFIERS;
#if GRAPHICS_PART==1
friend class CASCADE_VIEW;
#endif


};


class CASCADE_CLASSIFIERS:public QObject
{

Q_OBJECT


std::vector< STRONG_LEARN *> strongLearns;/*Almacena cada uno de los clasificadores fuertes en la cascada*/
int numberSetPositive;
int numberSetNegative;
std::vector<cv::Mat> negativeMisclassified;
//std::vector<string> listScannedImages;
double D;/*Tasa global de detección*/
double F;/*Tasa global de falsos positivos*/
double dMin;
double fMax;
double fGoal;
double percentageQuantityStop;
double percentageEntropyStop;
double maxDepth; 
bool renovateSetFeature;
cv::FileStorage *fileCascadeClassifier;
bool isLoad;
public:

/*0.5 para 15 etapas 0.00003*/
/*0.2 para 15 etapas 0.000000015*/
                              
CASCADE_CLASSIFIERS():isLoad(true),fileCascadeClassifier(NULL){G_STOP_TRAINING_FLAG=true;}//Si se pretende cargar
CASCADE_CLASSIFIERS(bool flag):fileCascadeClassifier(NULL),isLoad(false){G_STOP_TRAINING_FLAG=true;};/*Si se pretende configurar (no importa el valor de flag),
el valor de G_STOP_TRAINING_FLAG se inicializa en la función que carga los positivos*/
CASCADE_CLASSIFIERS(int negatives,double F=0.00003,double d_min=0.999,double f_max=0.5,int percentageQuantityStop=6,double percentageEntropyStop=6,int max_depth=5,bool renovateSetFeature=false); //5  5
~CASCADE_CLASSIFIERS();

void set_negatives(int n);
void set_F(double F);
void set_d_min(double d_min);
void set_f_max(double f_max);
void set_percentageQuantityStop(int percentageQuantityStop);
void set_percentageEntropyStop(int percentageEntropyStop);
void set_max_depth(int max_depth);
void set_renovateSetFeature(bool renovateSetFeature);



int get_negatives();
double get_F();
double get_fGoal();
double get_dMin();
double get_fMax();
int get_percentageQuantityStop();
int get_percentageEntropyStop();
int get_max_depth();
bool get_renovateSetFeature();
double getExpectedNumberFeaturestoEvaluate();
int getNumberFeatures();


void setNumberCore(int nc=1);
bool setNumberFeaturesChargedInMemory(int numberFeature=MIN_NUMBER_FEATURES_CHARGED_IN_MEMORY);
int getNumberFeaturesChargedInMemoryRecommended();
void startTraining();
bool evaluateClassifier(uchar *image);
bool evaluateClassifier(uchar *image,int number);
int get_numberStrongLearns()const;
void printCascadeClasifier(std::string nameFile);
void printCascadeClasifierEvaluation(std::string nameFile);
void loadCascadeClasifier(std::string nameFile);

double score;//Solo útil para la obtención de la ROC  de validación (ver evaluateClassifier(uchar *image,int number))

#if GRAPHICS_PART==1
friend class CASCADE_VIEW;
#endif

signals:



};


 


#endif
