#include <training.h>

/*Llamadas a las funciones de openMP*/
#include <omp.h>

/* Librerias stl de C++ */
#include<iostream>
#include <string>
#include <vector>
#include <math.h>
#include <stack>//Borrar al final (usada en funciones de prueba del tiempo)
#include <time.h>//Borrar al final (usada en funciones de prueba del tiempo)
#include <ctime>//Borrar al final (usada en funciones de prueba del tiempo)
#include <cstdio>

//________________LIBRERIAS QT___________________
#include <QDir>
#include <QDebug>
#include <QMutex>//Importante para bandera de salida
#include <QMutexLocker>
QMutex G_STOP_TRAINING_MUTEX;
volatile bool   G_STOP_TRAINING_FLAG;// Se inicializa a true en la gard de imágenes positivas ();

void STOP_TRAINING()
{
QMutexLocker LOCKER(&G_STOP_TRAINING_MUTEX);
G_STOP_TRAINING_FLAG=false;
}

#if GRAPHICS_PART==1
std::vector<DRAW_TREE_COMMANDS> G_DRAW_TREE_COMMANDS;/*Esta lista almacenara los comandos mientras los nodos se cargan para posteriorme
mente dibujarlos en cuanto la interfaz principal responda*/
#endif

int G_ACTUAL_NUMBER_NEGATIVE;
int G_REQUIRED_QUANTITY;



//________________LIBRERIAS OPENCV___________________
#include "opencv2/opencv.hpp"

#include "opencv2/objdetect/objdetect.hpp" 
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


/*_________Variables globales útiles solo para este archivo________________*/
bool printOnlyFeatures=false;/*Si es true solo se imprimen las características*/
int rangeLowCharged=0;
int rangeHighCharged=0;
std::vector<cv::Point2i> range;
bool order=true;

static std::vector<NODE *> LIST_TERMINAL_NODES;//Lista temporal de nodos terminales (ver loadWeakLearn() y loadNode())
static std::vector<NODE *> LIST_NODES_TREE;//Lista temporal de todos los nodos de un arbol (ver loadWeakLearn() y loadNode())
/*_____________________________________________________________________________*/

//__________________________________FUNCIONES DE PRUEBA BORRAR AL FINAL______________________________________
void prueba()
{

#pragma omp parallel 
{
 #pragma omp critical 
 std::cout<<"Hilo="<<omp_get_thread_num()<<"\n";
}

}

time_t tstart,tend;

void tic() {
    tstart=time(0);
}

void toc() {
      
    tend=time(0);
    std::cout <<"EL TIEMPO EN SEGUNDOS ES DE="<<std::difftime(tend,tstart)<<"\n";

}


#include <cstdlib> // Para system()

std::string G_PATH_RANDOM_NAGATIVE_IMAGES = "";
void createFolderWithUTCName() {
    // Obtener la hora actual
    std::time_t now = std::time(NULL);  // Usar NULL en lugar de nullptr

    // Convertir a hora UTC
    std::tm* utcTime = std::gmtime(&now);

    // Formatear la hora UTC como "YYYY-MM-DD_HH-MM-SS"
    char folderName[100];
    std::strftime(folderName, sizeof(folderName), "%Y-%m-%d_%H-%M-%S", utcTime);

    // Crear la carpeta utilizando system() con el comando mkdir
    std::string command = "mkdir -p negative_images/" + std::string(folderName);
    if (system(command.c_str()) == 0) {
        std::cout << "Carpeta creada con éxito: " << folderName << std::endl;
		G_PATH_RANDOM_NAGATIVE_IMAGES = "./negative_images/" + std::string(folderName);
    } else {
        std::cerr << "Error al crear la carpeta: " << folderName << std::endl;
    }
}



//______________________________________________________________________________________________________________


//_______________GENERANDO LOCK-TABLE________________________________

std::vector<float> G_NPD_VALUES;//LOCK-TABLE
void generateLockTable()
{

if(G_NPD_VALUES.size()!=0) return; /*Significa que la LOOK-TABLE ya ha sido generada*/
float npd_temp;
int len=256;//hace referencia a la profundidad en píxeles de las imágenes
G_NPD_VALUES.reserve(len*len);
for(int x=0;x<len;x++)
{
for(int y=0;y<len;y++)
{
npd_temp=(float(x-y))/(x+y);
if(npd_temp!=npd_temp)npd_temp=0;//detectando valores NaN
G_NPD_VALUES.push_back(npd_temp);
}
}

}

//___________________________________________________________________________


//_______________GENERANDO LOCK-TABLE ADECUADA PARA LA EVALUACIÓN________________________________

float G_NPD_VALUES_FOR_EVALUATION[256][256];//LOCK-TABLE
void generateLockTableEvaluation()
{

float npd_temp;
for(int x=0;x<256;x++)
{
for(int y=0;y<256;y++)
{
npd_temp=(float(x-y))/(x+y);
if(npd_temp!=npd_temp)npd_temp=0;//detectando valores NaN
G_NPD_VALUES_FOR_EVALUATION[x][y]=npd_temp;
}
}

}

//___________________________________________________________________________



//____________________GENERANDO LAS CARACTERÍSTICAS NPD_______________________
bool *G_SET_FEATURE=NULL;
int G_WIDTH_IMAGE=0;
int G_HEIGHT_IMAGE=0;
int G_NUMBER_FEATURE=0;
Point2i *G_NPD=NULL;

void generateFeatures()
{

int p=G_WIDTH_IMAGE*G_HEIGHT_IMAGE;
G_NUMBER_FEATURE=p*(p-1)/2;//Esto es producto de sumar (p-1)+(p-2)....+1

if(G_NPD!=NULL)delete []G_NPD;/*Esto liberara la memoria si se produce una doble llamada, debido a que el alto y ancho base puede cambiar*/

G_NPD=new Point2i[G_NUMBER_FEATURE];//Pidiendo espacio para almacenar las características;
//____llenando con los valores de cada coordenada___
int count=0;
for(int i=0;i<p;i++){
for(int j=i+1;j<p;j++){
G_NPD[count]=Point2i(i,j);//para el calculo aplíquese (xi-xj)/(xi+xj), donde xi y xj son los píxeles en las coordenadas i y j respectivamente.
count++;
 }
}

}

std::string intToString(int num) {
    std::ostringstream oss;
    oss << num;
    return oss.str();
}


// Function to concatenate an image with its horizontally flipped version
cv::Mat concatenateWithFlipped(const cv::Mat& inputImage) {
    // Check if the input image is valid
    if (inputImage.empty()) {
        std::cerr << "Error: Input image is empty!" << std::endl;
        return cv::Mat(); // Return an empty matrix if the input is invalid
    }

    // Create a flipped version of the input image (horizontal flip)
    cv::Mat flippedImage;
    cv::flip(inputImage, flippedImage, 1);

    // Create the output image
    cv::Mat outputImage;

    // Decide whether to concatenate vertically or horizontally
    if (inputImage.rows < inputImage.cols) {
        // Concatenate vertically (stack one image below the other)
        cv::vconcat(inputImage, flippedImage, outputImage);
    } else {
        // Concatenate horizontally (stack one image beside the other)
        cv::hconcat(inputImage, flippedImage, outputImage);
    }

    return outputImage; // Return the resulting image
}

// Function to generate a random image
cv::Mat getRandomImageWithWget() {
    // Initialize random seed
    std::srand(std::time(NULL));

    // Generate random dimensions between 500 and 5000
    int width = std::rand() % 4501 + 500;  // Random size between 500 and 5000
    int height = std::rand() % 4501 + 500; // Random size between 500 and 5000

    // Construct the Picsum URL with random dimensions
    std::string url = "https://picsum.photos/" + intToString(width) + "/" + intToString(height);

    // Temporary file path for downloaded image
    std::string tempFilePath = "tmp_negative_image/random_image.jpg";

    // Ensure the temporary directory exists
    system("mkdir -p tmp_negative_image");

    // Use wget to download the image
    std::string command = "wget -q -O " + tempFilePath + " " + url;
    int downloadStatus = system(command.c_str());
    if (downloadStatus != 0) {
        std::cerr << "Error: Failed to download the image using wget." << std::endl;
        return cv::Mat();  // Return an empty Mat on failure
    }

    // Load the downloaded image into a cv::Mat
    cv::Mat image = cv::imread(tempFilePath, cv::IMREAD_COLOR);

    // Delete the temporary file immediately after loading
    std::remove(tempFilePath.c_str());

    // Check if the image was successfully loaded
    if (image.empty()) {
        std::cerr << "Error: Failed to load the downloaded image into a cv::Mat." << std::endl;
        return cv::Mat();  // Return an empty Mat if loading fails
    }

    // Flip the image vertically
    cv::Mat rotatedImage;
    cv::flip(image, rotatedImage, 0);

    // Return the rotated image
    return rotatedImage;
}



// Function to store images in a specified directory with sequential filenames
void saveImagesToDirectory(const std::vector<cv::Mat>& images, const std::string& directoryPath, int max_number_images) {
    // Ensure the directory path ends with a slash
    std::string basePath = directoryPath;
    if (!basePath.empty() && basePath[basePath.size() - 1] != '/' && basePath[basePath.size() - 1] != '\\') {
        basePath += "/";
    }

    // Iterate through the vector of images
	max_number_images = (max_number_images < images.size())? max_number_images: images.size();
    for (size_t i = 0; i < max_number_images; ++i) {
        const cv::Mat& image = images[i];

        // Check if the image is valid
        if (image.empty()) {
            std::cerr << "Error: Image at index " << i << " is empty and will not be saved." << std::endl;
            continue;
        }

        // Construct the filename
        std::string filename = basePath + "image_" + intToString(i + 1) + ".png";

        // Save the image
        if (cv::imwrite(filename, image)) {
            std::cout << "Image saved successfully: " << filename << std::endl;
        } else {
            std::cerr << "Error saving image to file: " << filename << std::endl;
        }
    }
}



//_______________________________________________________________________________


/*esta función cargara el label de imágenes positivas*/

std::vector< cv::Mat > G_LABEl_POSITIVE;
std::vector< cv::Mat > G_LABEl_NEGATIVE;

bool loadSetPositiveImages(std::string nameFile)
{

//cv::FileStorage setPositivesImages(G_NAME_DIRECTORY_SET_POSITIVE+nameFile,cv::FileStorage::READ);
cv::FileStorage setPositivesImages(nameFile,cv::FileStorage::READ);
if(!setPositivesImages.isOpened())return false; //Cuando el archivo en el directorio no existe


cv::FileNode Info=setPositivesImages["Info"];
G_WIDTH_IMAGE=Info["width_image"];//Aquí se asigna el valor global del ancho de la imagen
G_HEIGHT_IMAGE=Info["high_image"];//Aquí se asigna el valor global del alto de la imagen

cv::FileNode labels_number=Info["labels_number"];
int numberSetPositives=labels_number["set_positivos"];

if(G_LABEl_POSITIVE.size()==0)
G_LABEl_POSITIVE.reserve(numberSetPositives);
else{
G_LABEl_POSITIVE.clear();
G_LABEl_POSITIVE.reserve(numberSetPositives);
//return true;//Cuando el archivo ya ha sido cargado
}

cv::FileNode Data=setPositivesImages["Data"];
cv::FileNode Labels=Data["Labels"];
cv::FileNode set_positives=Labels["set_positivos"];

std::string str_base="Image_";//NOMBRE BASE OBLIGATORIO
cv::Mat temp;

for(int j=0;j<numberSetPositives;j++)
{
     
std::stringstream sstm;
sstm<<str_base<<(j+1);
set_positives[sstm.str()]>>temp;

/*
cv::imshow("V",temp);
cv::waitKey(0);
*/
G_LABEl_POSITIVE.push_back(temp.clone());
     
}

setPositivesImages.release();

/*IMPORTANTE::LLAMAR A LA FUNCIÓN GENERADORA DE LA LOOK TABLE Y LAS CARACTERÍSTICAS NPD*/
generateLockTable();
generateFeatures();

QMutexLocker LOCKER(&G_STOP_TRAINING_MUTEX);
G_STOP_TRAINING_FLAG=true;//Debe iniciarse a true para que las iteraciones corran

setPositivesImages.release();
return true;//Si se llega a este punto la operación termino correctamente y se devuelve un true
}





void windowSlidingTraining(cv::Mat &image,std::vector< cv::Mat > &vectorImageOut, double step_scale = 1.2, int step_x = 1, int step_y = 1)
{

if((G_WIDTH_IMAGE==0)||(G_HEIGHT_IMAGE==0)) return; //Para llamarse a esta función deben estar asignado estos valores

//Modificación introducida
if(image.channels()!=1)
cv::cvtColor(image,image,CV_BGR2GRAY);//Convirtiendo a escala de grises(la función también clona la imagen)


/*____________________________PARAMETROS DE ENVENTANADO______________________________*/
int sizeBase=std::min(G_WIDTH_IMAGE,G_HEIGHT_IMAGE);/*tamaño inicial de la ventana de búsqueda*/
//double step_scale=1.2;/*En este factor se incrementara el ancho y alto de la ventana de búsqueda*/ //por defecto siempre fue 2
//int step_x= 4 + 0*sizeBase/4;/*En este factor la ventana de búsqueda avanzara en filas*/  //por defecto de dividía entre 4
//int step_y= 4 + 0*sizeBase/4;/*En este factor la ventana de búsqueda avanzara en columnas*/ //por defecto de dividía entre 4

/*Nota:Para etapas después de la 8 el factor de avance de 5 se cree que es bueno pero eleve el factor escala pues 1.2 parece que fue muy pequeño*/

int topLeft_x=0;
int topLeft_y=0;
int bottomRight_x=sizeBase-1;
int bottomRight_y=sizeBase-1;
/*____________________________________________________________________________________*/


/*________________CALCULAMOS LA CANTIDAD DE ESCALAS REQUERIDA PARA LAS IMAGENES__________________________*/
int dim=std::min(image.rows,image.cols);
int numberScales=static_cast<int>((std::log(dim/sizeBase)/std::log(2))+1);
int N=0;
for(int i=0;i<=numberScales;i++)
N=N+(image.rows/(sizeBase*(int)std::pow(2,i)))*(image.cols/(sizeBase*(int)std::pow(2,i)));/*divisiones enteras*/
/*_______________________________________________________________________________________________________*/

cv::Mat windows;/*Variable local útil para llevar a acabo el enventanado*/


for(int k=0;k<numberScales;k++){
for(int i=0;i<=image.rows-sizeBase;i=i+step_x){
for(int j=0;j<=image.cols-sizeBase;j=j+step_y){

/*__________CALCULO DE LA POSICIÓN DE LA VENTANA______________*/
topLeft_x=i;
topLeft_y=j;
bottomRight_x=sizeBase+i-1;
bottomRight_y=sizeBase+j-1;
/*____________________________________________________________*/

/*_____________ENVENTANAMOS CADA SUBIMAGEN Y LA CONVERTIMOS A ESCALA DE GRISES____________*/

if(image.size()!=cv::Size(G_WIDTH_IMAGE,G_HEIGHT_IMAGE))
cv::resize(image(cv::Range(topLeft_x,bottomRight_x+1), cv::Range(topLeft_y,bottomRight_y+1)),windows,cv::Size(G_WIDTH_IMAGE,G_HEIGHT_IMAGE),0,0,cv::INTER_NEAREST);
else
windows=image(cv::Range(topLeft_x,bottomRight_x+1), cv::Range(topLeft_y,bottomRight_y+1));

/*Modificado ver unas lineas mas arriba:
cv::cvtColor(windows,windows,CV_BGR2GRAY);//Convirtiendo a escala de grises(la función también clona la imagen)
vectorImageOut.push_back(windows);
*/

//Modificación de las dos lineas anteriores,ver arriba
vectorImageOut.push_back(windows.clone());


/*________________________________________________________________________________________*/

}
}

/*_________ACTUALIZACIÓN DE LA ESCALA______*/
sizeBase=step_scale*sizeBase;
step_x=sizeBase/4;/*En este factor la ventana de búsqueda avanzara en filas*/  //por defecto dividía entre 4
step_y=sizeBase/4;/*En este factor la ventana de búsqueda avanzara en columnas*/  //por defecto dividía entre 4
/*_________________________________*/

}




}





/*__esta función debe ser llamada para  llenar por primera  vez el set de imágenes negativas__*/

QDir G_DIRECTORY_IMAGES_NEGATIVES=QDir(G_NAME_DIRECTORY_SET_NEGATIVE);/*Aquí cargamos el directorio estándar*/
std::vector<std::string> G_LIST_SCANNED_IMAGES;
bool loadSetNegativeImages(int numberImagesNegative,CASCADE_CLASSIFIERS *classifier,bool (*callback)())
{


if(G_PATH_RANDOM_NAGATIVE_IMAGES.empty())
   createFolderWithUTCName();

if(G_LABEl_NEGATIVE.size()>=numberImagesNegative)return true;

std::vector<cv::Mat> imageNegativeTemp;
imageNegativeTemp.reserve(numberImagesNegative);

for(int i=0;i<G_LABEl_NEGATIVE.size();i++)
imageNegativeTemp.push_back(G_LABEl_NEGATIVE[i].clone());

G_LABEl_NEGATIVE.clear();


if((G_WIDTH_IMAGE==0)||(G_HEIGHT_IMAGE==0)) return false; //Para llamarse a esta función deben estar asignado estos valores



/*
G_DIRECTORY_IMAGES_NEGATIVES.refresh();//refrescamos la información del directorio
QStringList namesImagesInDyrectory=G_DIRECTORY_IMAGES_NEGATIVES.entryList();//Aquí se extrae la lista de archivos en el directorio
namesImagesInDyrectory=namesImagesInDyrectory.filter(G_EXTENSIONS_IMAGES);//Aquí se filtran los archivos con extensión permitida
QStringList namesImagesInDyrectory_ant=namesImagesInDyrectory;//Para tener copia de los nombres de las imágenes

for(int i=0;i<G_LIST_SCANNED_IMAGES.size();i++)
namesImagesInDyrectory.removeOne(QString::fromStdString(G_LIST_SCANNED_IMAGES[i]));//Eliminando imágenes ya escaneadas

std::vector<std::string> tempListScannesImages;//Lista temporal para imágenes escaneadas

*/

/*_____________Esta parte del código aplicara para el set inicial,cuando no hay ningún árbol construido_____________________*/


while(imageNegativeTemp.size()<numberImagesNegative)
{


//____________PUNTO DE RUPTURA DE LA ITERACIÓN___________________
{

QMutexLocker LOCKER(&G_STOP_TRAINING_MUTEX);
if(G_STOP_TRAINING_FLAG==false){
std::cout<<"Detenido en loadSetNegativeImages(int numberImagesNegative,CASCADE_CLASSIFIERS *classifier,bool (*callback)())\n";
return false;
}

}
//_______________________________________________________________




/*

while(namesImagesInDyrectory.isEmpty()){

std::cout<<"Cantidad requerida="<<numberImagesNegative<<", cantidad actual="<<imageNegativeTemp.size()<<"\n";
G_ACTUAL_NUMBER_NEGATIVE=imageNegativeTemp.size();
G_REQUIRED_QUANTITY=numberImagesNegative;
if(false==callback())
{//Si la operación es abordada debe desocuparse la memoria G_LABEl_NEGATIVE 
return false;//Se devuelve false si se aborda la operación
}
 
G_DIRECTORY_IMAGES_NEGATIVES.refresh();//refrescamos la información del directorio
namesImagesInDyrectory=G_DIRECTORY_IMAGES_NEGATIVES.entryList();//Aquí leemos nuevamente la lista de archivos del directorio
namesImagesInDyrectory=namesImagesInDyrectory.filter(G_EXTENSIONS_IMAGES);//Aquí nuevamente se filtran los archivos con extensión permitida


for(int i=0;i<namesImagesInDyrectory_ant.size();i++){
namesImagesInDyrectory.removeOne(namesImagesInDyrectory_ant[i]);//Aquí removemos los valores de la lista antigua
}
namesImagesInDyrectory_ant=namesImagesInDyrectory_ant+namesImagesInDyrectory;//Adicionamos los nuevos nombres de imágenes

}

*/

/*

QString nameImage=namesImagesInDyrectory.takeFirst();
tempListScannesImages.push_back(nameImage.toStdString());//Acumulamos el nombre de cada una de las imágenes escaneadas
QString pathnameImage=G_DIRECTORY_IMAGES_NEGATIVES.absolutePath()+QString("/")+nameImage;

*/

//______________________________________________________________________________________//




//cv::Mat image=cv::imread(pathnameImage.toStdString());
// Crear una matriz de 1500x1500 en escala de grises
//cv::Mat image(5000, 5000, CV_8UC1);
//cv::Mat image(G_HEIGHT_IMAGE, G_WIDTH_IMAGE, CV_8UC1);
// Generar valores aleatorios con distribución uniforme en el rango [0, 255]
//cv::randu(image, cv::Scalar(0), cv::Scalar(256));
//cv::randn(image, cv::Scalar(128), cv::Scalar(50));

cv::Mat image = getRandomImageWithWget();


if(image.data!=NULL){/*Para evitar imágenes que se lean incorrectamente*/
if(classifier==NULL){
	
//windowSlidingTraining(image,imageNegativeTemp);
//Para los primeros clasificadores tomamos ventanas no traslapadas 
int sizeBase=std::min(G_WIDTH_IMAGE,G_HEIGHT_IMAGE);
windowSlidingTraining(image,imageNegativeTemp, 2, sizeBase, sizeBase);

}else{

 if(classifier->get_numberStrongLearns()==0){
 //imageNegativeTemp.push_back(image.clone());
 //windowSlidingTraining(image,imageNegativeTemp);
 
  //Para los primeros clasificadores tomamos ventanas no traslapadas 
  int sizeBase=std::min(G_WIDTH_IMAGE,G_HEIGHT_IMAGE);
  windowSlidingTraining(image,imageNegativeTemp, 2, sizeBase, sizeBase);
  
 }else{

      
      std::vector<cv::Mat> listImageTemp;	  
	  
	  if(classifier->get_numberStrongLearns() >= 3){
		 image = concatenateWithFlipped(image); //esto ampliara la imagen añadiendo su imagen rotada horizontalmente
         windowSlidingTraining(image, listImageTemp);
	  } else{
		 //Para los dos primeros clasificadores tomamos ventanas no traslapadas 
	     int sizeBase=std::min(G_WIDTH_IMAGE,G_HEIGHT_IMAGE);
         windowSlidingTraining(image,imageNegativeTemp, 2, sizeBase, sizeBase);
	  }
	  
      //Almacenamos las imágenes incorrectamente clasificadas
      for(int i=0;i<listImageTemp.size();i++){
      uchar *img=listImageTemp[i].ptr<uchar>(0);
      if(classifier->evaluateClassifier(img))imageNegativeTemp.push_back(listImageTemp[i].clone());
      }
	  
	  
	  /*
	  uchar *img=image.ptr<uchar>(0);
	  if(classifier->evaluateClassifier(img))imageNegativeTemp.push_back(image.clone());
	  */
	  
  }

}

}
//_________________________________________________________________________________________//

}

/*
//llegados a este punto acumulamos los nombres de las imágenes previamente escaneadas
for(int i=0;i<tempListScannesImages.size();i++)
G_LIST_SCANNED_IMAGES.push_back(tempListScannesImages[i]);
*/

/*Aquí borramos las imágenes de las posiciones superiores a numberImagesNegative*/
int sizeToErase=imageNegativeTemp.size()-numberImagesNegative;
imageNegativeTemp.erase(imageNegativeTemp.end()-sizeToErase,imageNegativeTemp.end());
G_LABEl_NEGATIVE.reserve(numberImagesNegative);

for(int i=0;i<numberImagesNegative;i++)
G_LABEl_NEGATIVE.push_back(imageNegativeTemp[i].clone());




//Guardando imagenes negativas de cada etapa
std::string tmp_path_random_negative_images = G_PATH_RANDOM_NAGATIVE_IMAGES + "/" + intToString(classifier->get_numberStrongLearns()) + "/";
std::string command = "mkdir -p " + tmp_path_random_negative_images;
if (system(command.c_str()) == 0) 
   saveImagesToDirectory(G_LABEl_NEGATIVE, tmp_path_random_negative_images, 10);

/*___________________________________________________________________________________________________________________________*/
return true;//Se devuelve true si la base de datos fue cargada correctamente
}





/*A continuación se declara la función que construirá las evaluaciones NPD y sus respectivas coordenadas*/

//____________________________________________________________________________________
struct MAP{
unsigned short int coordinate;//Almacena la referencia de coordenada de cada imagen del set de entrenamiento
                              //Restringe a bases de datos no mayores a 65536
unsigned short int val_s;//hace referencia un valor int short que representa un valor float
float val_f;//hace referencia a un valor float que se mapeo de un val_s
};

//Función de ordenacion especifica de menor a mayor con respecto a los valores val_f
bool sort_val_f(const MAP &map1, const MAP &map2) { return map1.val_f < map2.val_f; }
//_____________________________________________________________________________________



unsigned short int *G_COORDINATE_VECTOR=NULL;
int G_FEATURES_CHARGED_IN_MEMORY=0;
int G_NUMBER_TOTAL_IMAGES=0;

int generateEvaluations(std::vector<cv::Mat> &labelPositive,std::vector<cv::Mat> &labelNegative)
{

/*
Explicación sobre los valores de retorno

-return 0=La operación culmino satisfactoriamente
-return 1=Debe asignar una porción mayor de memoria
-return 2=No es posible abrir el archivo o significa punto de ruptura

*/


//____________PUNTO DE RUPTURA DE LA ITERACIÓN___________________
{

QMutexLocker LOCKER(&G_STOP_TRAINING_MUTEX);
if(G_STOP_TRAINING_FLAG==false){
std::cout<<"Detenido en generateEvaluations(std::vector<cv::Mat> &labelPositive,std::vector<cv::Mat> &labelNegative)\n";
return 2;
}

}
//_______________________________________________________________




//________________En este archivo se almacenara los resultados val_s de las evaluaciones_________________________
FILE *file_evaluationFeature;
if((file_evaluationFeature=fopen(G_FILE_NAME_EVALUATION_FEATURE, "wb"))==NULL)return 2;

    

//_______________________________________________________________________________________________________________

//______________En este archivo se almacenara la distribución de coordenadas de cada imagen______________________
FILE *file_coordinateImage;
if((file_coordinateImage=fopen(G_FILE_NAME_CORDINATE_IMAGE, "wb"))==NULL)return 2;

//_______________________________________________________________________________________________________________




std::cout<<"INICIANDO CALCULO DE CARACTERÍSTICAS NPD........\n";
omp_set_num_threads(G_NUMBER_CORES_FOR_PARALLEL_SECTIONS);//Estableciendo el número de núcleos 
#pragma omp parallel for
for(int i=0;i<G_NUMBER_FEATURE;i++){
int xi;//coordenada inicial de la característica NPD
int xj;//coordenada final de la característica NPD
int j_modify;
uchar *M;//variable puntero que apuntara al conjunto de imágenes del label l-esimo
MAP *r=new MAP[G_NUMBER_TOTAL_IMAGES];//Tipo propio que almacena un mapeo de la lock table
unsigned short int *evaluationNpd_s=new unsigned short int[G_NUMBER_TOTAL_IMAGES];
unsigned short int *coordinateEvaluation=new unsigned short int[G_NUMBER_TOTAL_IMAGES];//Restringe a bases de datos no mayores a 65536;

xi=G_NPD[i].x;//Extrayendo coordenada inicial
xj=G_NPD[i].y;//Extrayendo coordenada final


for(int j=0;j<labelPositive.size();j++){
M=(labelPositive.begin()+j)->ptr<uchar>(0);//Leyendo imagen j-esima del label l-esimo
r[j].coordinate=j;//Almacenando la coordenada de la imagen
r[j].val_s=256*M[xi]+M[xj];//Almacenando valor equivalente unsigned short
r[j].val_f=G_NPD_VALUES[r[j].val_s];//Almacenando valor real NPD de la evaluación.
}

for(int j=0;j<labelNegative.size();j++){
M=(labelNegative.begin()+j)->ptr<uchar>(0);//Leyendo imagen j-esima del label l-esimo
j_modify=labelPositive.size()+j;//coordenada jesima mas ofset;
r[j_modify].coordinate=j_modify;//Almacenando la coordenada de la imagen
r[j_modify].val_s=256*M[xi]+M[xj];//Almacenando valor equivalente unsigned short
r[j_modify].val_f=G_NPD_VALUES[r[j_modify].val_s];//Almacenando valor real NPD de la evaluación.
}

std::vector<MAP> myvector(r, r+G_NUMBER_TOTAL_IMAGES); //vector temporal que sera ordenado de menor a mayor
std::sort(myvector.begin(), myvector.end(),sort_val_f);//ordenando los valores con respecto al valor real val_f

//EXTRAYENDO LA INFORMACIÓN EN UN PUNTERO

#pragma omp parallel for 
for(int n=0;n<G_NUMBER_TOTAL_IMAGES;n++){
evaluationNpd_s[n]=myvector[n].val_s;//extrayendo los valores val_s de myvector
coordinateEvaluation[n]=myvector[n].coordinate;//extrayendo coordenadas de cada imagen
}


/*___________________________Aquí se almacenan los resultados_______________________________*/

/*_________________________AQUÍ SE PRECARGA EL PRIMER SEGMENTO DE COORDENADAS____________________________*/
if(i<G_FEATURES_CHARGED_IN_MEMORY)
{
int ofset=i*G_NUMBER_TOTAL_IMAGES;
std::copy(coordinateEvaluation,coordinateEvaluation+G_NUMBER_TOTAL_IMAGES,G_COORDINATE_VECTOR+ofset);
}
/*_______________________________________________________________________________________________________*/

#pragma omp critical
{
fseek(file_evaluationFeature,sizeof(unsigned short int)*(G_NUMBER_TOTAL_IMAGES)*i,SEEK_SET);
fwrite(evaluationNpd_s,sizeof(unsigned short int),G_NUMBER_TOTAL_IMAGES,file_evaluationFeature);//Almacenando la evaluación
}


#pragma omp critical
{
fseek(file_coordinateImage,sizeof(unsigned short int)*(G_NUMBER_TOTAL_IMAGES)*i,SEEK_SET);
fwrite(coordinateEvaluation,sizeof(unsigned short int),G_NUMBER_TOTAL_IMAGES,file_coordinateImage);///Almacenando la coordenada
}


/*_________________________________________________________________________________________*/


delete []r;
delete []evaluationNpd_s;
delete []coordinateEvaluation;


}    
    
/*__________Se establece el rango inicial para la función getNPD______________*/
rangeLowCharged=0;
rangeHighCharged=G_FEATURES_CHARGED_IN_MEMORY;

range.clear();
for(int n=0;n<G_NUMBER_FEATURE;n=n+G_FEATURES_CHARGED_IN_MEMORY){
int begin=n;
int end=(((n+G_FEATURES_CHARGED_IN_MEMORY)>G_NUMBER_FEATURE)?G_NUMBER_FEATURE:(n+G_FEATURES_CHARGED_IN_MEMORY));
range.push_back(cv::Point2i(begin,end));
}

order=true;
/*____________________________________________________________________________*/                  

fclose(file_evaluationFeature);//Cerrando el dispositivo
fclose(file_coordinateImage);//Se cierra solo si se crea



//____________PUNTO DE RUPTURA DE LA ITERACIÓN___________________
{

QMutexLocker LOCKER(&G_STOP_TRAINING_MUTEX);
if(G_STOP_TRAINING_FLAG==false){
std::cout<<"Detenido en generateEvaluations(std::vector<cv::Mat> &labelPositive,std::vector<cv::Mat> &labelNegative)\n";
return 2;
}

}
//_______________________________________________________________




std::cout<<"EL PROCESO DEL CALCULO DE LAS CARACTERÍSTICAS NPD A CULMINADO SATISFACTORIAMENTE\n";
return 0;
}



double *G_WEIGHTS_IMAGES=NULL;
NODE **getNPD(NODE *node,int numberThreads)
{
int numImagesNode=node->numImages;
const bool *const imagesInNode=node->imagesInNode;

/*____________________________PARTE DE PRUEBA______________________________*/
/*
std::cout<<"\n";
for(int k=0;k<G_NUMBER_TOTAL_IMAGES;k++)
std::cout<<imagesInNode[k]<<" ";
std::cout<<"\n";
getchar();
*/
/*__________________________________________________________________________*/


if(G_WEIGHTS_IMAGES==NULL) return NULL;

FILE *G_FILE_COORDINATE_FULL;
if((G_FILE_COORDINATE_FULL=fopen("file_coordinateImage", "rb"))==NULL) {
    printf("Cannot open file file_coordinateImage\n");
    return NULL;
}


double entropyByFeatures[G_NUMBER_FEATURE],thresholds[G_NUMBER_FEATURE];//Vector de entropía y umbrales óptimos por característica
int dis[G_NUMBER_FEATURE];//Almacena la distancia entre la dirección entropyByImages(ver mas adelante) y la entropía mínima en dicho vector


//omp_set_num_threads(numberThreads);//Estableciendo el número de núcleos 
for(int r=0;r<range.size();r++){

int n,begin,end;

if(order==true){
n=range[r].x;
begin=n;
end=range[r].y;
}
else
{
int sz=range.size();
n=range[sz-r-1].x;
begin=n;
end=range[sz-r-1].y;
}


if(!((rangeLowCharged==begin)&&(rangeHighCharged==end))){
fseek(G_FILE_COORDINATE_FULL,sizeof(unsigned short int)*G_NUMBER_TOTAL_IMAGES*begin,SEEK_SET);
fread(G_COORDINATE_VECTOR,sizeof(unsigned short int),G_NUMBER_TOTAL_IMAGES*G_FEATURES_CHARGED_IN_MEMORY,G_FILE_COORDINATE_FULL);
std::cout<<"SE LEYÓ ARCHIVO\n";
}

std::cout<<"ENCONTRANDO MEJOR CARACTERÍSTICA EN SECCIÓN PARALELA\n";

#pragma omp parallel 
{
  
  
//_________________________________________VARIABLES LOCALES A LA FUNCIÓN___________________________________________

//double entropyByFeatures[G_NUMBER_FEATURE],thresholds[G_NUMBER_FEATURE];//Vector de entropía y umbrales óptimos por característica
const unsigned short int *vec_coordinate;//Vector auxiliar para manejar las direcciones de vec_coordinate_full

double w,label;//w almacena el valor del peso temporal de una imagen y label almacena si es cara 1 y si es no cara -1
double tm[numImagesNode],y[numImagesNode];//tm=vector de sumatoria de pesos w, y=vector de ponderación (w*label)

int count_coordinateInitiation;//Almacena la coordenada desde donde se iniciara el calculo de entropía por característica.
int count;//Cuenta las veces que una imagen esta presente en el nodo

//int dis[G_NUMBER_FEATURE];//Almacena la distancia entre la dirección entropyByImages(ver mas adelante) y la entropía mínima en dicho vector
//int split,aux;//split almacena la coordenada de división óptima para la característica

double yl,yr,sl,sr,entropyByImages[numImagesNode-1];
//yl y sl almacenan el valor temporal de la media y la desviación estándar a la izquierda de la división split respectivamente 
//yr y sr almacenan el valor temporal de la media y la desviación estándar a la derecha de la división split respectivamente 
//entropyByImages almacena el valor de la entropía por cada división posible empezando desde la segunda imagen hasta numImagesNode-2
double *entropyByImages_min;//Almacena la dirección de la entropía mínima del vector entropyByImages



//___________________________________________________________________________________________________________________

omp_set_num_threads(numberThreads);//Estableciendo el número de núcleos 
#pragma omp for
for(int i=begin;i<end;i++) //Por cada característica el proceso debe calcularse el umbral, coordenada y entropia optima
{

if(G_SET_FEATURE[i]==true)//Se revisa que la característica en cuestión no haya sido previamente elegida
{

//Se lee el inicio de la dirección del segmento que almacena las coordenadas de cada imagen
//por característica                                                   
vec_coordinate=&(G_COORDINATE_VECTOR[(i-begin)*G_NUMBER_TOTAL_IMAGES]);

//____________Se averigua el indicie del vector vec_coordinate desde donde existe la primer imagen en el nodo_____________
count_coordinateInitiation=0;
while(imagesInNode[vec_coordinate[count_coordinateInitiation]]==false)
count_coordinateInitiation++;
//_________________________________________________________________________________________________________________________



/*____________________________PARTE DE PRUEBA______________________________*/
/*
std::cout.precision(15);
std::cout<<"feature="<<i<<"\n";
std::cout<<"Coordenada de inicio="<<count_coordinateInitiation<<"\n";
getchar();
*/
/*__________________________________________________________________________*/





//Se almacena temporalmente en label 1 si la coordenada en cuestión es una cara o -1 no es una cara
label=vec_coordinate[count_coordinateInitiation]<=(G_LABEl_POSITIVE.size()-1)? 1:-1;
w=G_WEIGHTS_IMAGES[vec_coordinate[count_coordinateInitiation]];//Se almacena el peso correspondiente la imagen

tm[0]=w;//Se almacena el primer peso
y[0]=w*label;//Se almacena la primera ponderación
count=1;//se inicia la cuenta del índice de tm y y

/*_________________________PARTE DE PRUEBA_________________________*/
//std::cout<<"J="<<count_coordinateInitiation<<"coor="<<vec_coordinate[count_coordinateInitiation]<<" w="<<w<<"\n";
/*_________________________________________________________________*/


for(int j=count_coordinateInitiation+1;j<G_NUMBER_TOTAL_IMAGES;j++)
{

if(imagesInNode[vec_coordinate[j]]==true)
{

label=vec_coordinate[j]<=(G_LABEl_POSITIVE.size()-1)? 1:-1;//label asociado a vec_coordinate[j]
w=G_WEIGHTS_IMAGES[vec_coordinate[j]];//Peso w asociado a vec_coordinate[j];

/*_________________________PARTE DE PRUEBA_________________________*/
//std::cout<<"J="<<j<<" coor="<<vec_coordinate[j]<<" w="<<w<<"\n";
/*_________________________________________________________________*/

tm[count]=tm[count-1]+w;//Acumulación de pesos
y[count]=y[count-1]+w*label;//Acumulación de ponderaciones
count++;//contador del índice para tm y y;

}

}

/*____________________________PARTE DE PRUEBA______________________________*/
/*
std::cout<<"\n y[k] y tm[k]:\n";
for(int k=0;k<numImagesNode;k++)
std::cout<<y[k]<<" ";
std::cout<<"\n";
for(int k=0;k<numImagesNode;k++)
std::cout<<tm[k]<<" ";

getchar();
*/
/*__________________________________________________________________________*/



for(count=0;count<numImagesNode-1;count++)
{

yl=y[count]/tm[count];
yr=(y[numImagesNode-1]-y[count])/(tm[numImagesNode-1]-tm[count]);

sl=1.0-yl*yl;
sr=1.0-yr*yr;

entropyByImages[count]=((count+1)*sl+sr*(numImagesNode-1-count))/numImagesNode;

}

entropyByImages_min=std::min_element(entropyByImages,entropyByImages+numImagesNode-1);
entropyByFeatures[i]=*entropyByImages_min;
dis[i]=std::distance(entropyByImages,entropyByImages_min)+1;//1 por el componente cero de la entropía y 1 por el y(0)

/*____________________________PARTE DE PRUEBA______________________________*/
/*
std::cout<<"entropyByImages[k]:\n";
for(int k=0;k<numImagesNode-1;k++)
std::cout<<entropyByImages[k]<<" ";
std::cout<<"\nentropyByImages_min="<<(*entropyByImages_min)<<" dis="<<dis[i]<<"\n";
getchar();
*/
/*__________________________________________________________________________*/



}else{
//________________________________________entropía no escogida________________________________
entropyByFeatures[i]=INFINITY;
dis[i]=0;
//____________________________________________________________________________________________
}

}

}//Cierra el for el primer for


}//Cierra el el pragma



double *entropyByNode=std::min_element(entropyByFeatures,entropyByFeatures+G_NUMBER_FEATURE);
int featureChosen=std::distance(entropyByFeatures,entropyByNode);
G_SET_FEATURE[featureChosen]=false;/*De esta forma siempre se eligirán características distintas para lograr un cierto grado de independencia*/

//_______________________________CALCULO DE LA COORDENADA DE DIVISIÓN___________________________

//const unsigned short int *vec_coordinate=&(G_COORDINATE_VECTOR[featureChosen*G_NUMBER_TOTAL_IMAGES]);
unsigned short int *vec_coordinate=new unsigned short int[G_NUMBER_TOTAL_IMAGES];
fseek(G_FILE_COORDINATE_FULL,sizeof(unsigned short int)*G_NUMBER_TOTAL_IMAGES*featureChosen,SEEK_SET);
fread(vec_coordinate,sizeof(unsigned short int),G_NUMBER_TOTAL_IMAGES,G_FILE_COORDINATE_FULL);


int split=-1,aux=0;/*OJO, EL SPLIT INICIA DESDE -1 PARA QUE vec_coordinate[0] HAGA PARTE*/
while(aux<dis[featureChosen])
{
split++;//debido a que la división se cuenta desde el número 1 hasta el ultimo-1 
aux=aux+(int)imagesInNode[vec_coordinate[split]];
}
//______________________________________________________________________________________________



FILE *G_FILE_EVALUATION_FEATURE_FULL;
if((G_FILE_EVALUATION_FEATURE_FULL=fopen(G_FILE_NAME_EVALUATION_FEATURE, "rb"))==NULL) {
    printf("Cannot open file file_evaluationFeature.\n");
    return NULL;
}
unsigned short int *vec_npd=new unsigned short int[G_NUMBER_TOTAL_IMAGES];
fseek(G_FILE_EVALUATION_FEATURE_FULL,sizeof(unsigned short int)*featureChosen*G_NUMBER_TOTAL_IMAGES,SEEK_SET);
fread(vec_npd,sizeof(unsigned short int),G_NUMBER_TOTAL_IMAGES,G_FILE_EVALUATION_FEATURE_FULL);
float thresholdFeatureChosen=G_NPD_VALUES[vec_npd[split]];
delete []vec_npd;
fclose(G_FILE_EVALUATION_FEATURE_FULL);


/*____________________________PARTE DE PRUEBA______________________________*/
/*
std::cout<<"entropyByFeatures\n";
for(int i=0;i<9;i++)
std::cout<<entropyByFeatures[i]<<" ";
std::cout<<"\n";

std::cout<<"Característica elegida="<<featureChosen<<"\n";
std::cout<<"split="<<split<<"\n";
std::cout<<"thresholdFeatureChosen="<<thresholdFeatureChosen<<"\n";
getchar();
*/
/*__________________________________________________________________________*/



//_______________________________________ORGANIZANDO LA SALIDA___________________________________
int cordinateSplitFeatureChosen=split;

fseek(G_FILE_COORDINATE_FULL,sizeof(unsigned short int)*featureChosen*G_NUMBER_TOTAL_IMAGES,SEEK_SET);
fread(vec_coordinate,sizeof(unsigned short int),G_NUMBER_TOTAL_IMAGES,G_FILE_COORDINATE_FULL);
fclose(G_FILE_COORDINATE_FULL);

//________________________ORGANIZANDO EL NODO DERECHO___________________________
int numImage_left=0,numImagesPositives_left=0,numImagesNegatives_left=0;
bool *imageInNode_left=new bool[G_NUMBER_TOTAL_IMAGES];//Nodo izquierdo de salida
std::fill(imageInNode_left,imageInNode_left+G_NUMBER_TOTAL_IMAGES,false);
for(int i=0;i<=cordinateSplitFeatureChosen;i++)
{
if(imagesInNode[vec_coordinate[i]]==true)
{
imageInNode_left[vec_coordinate[i]]=true;//Organizando coordenada de imágenes que pasan al nodo izquierdo
numImagesPositives_left=numImagesPositives_left+((vec_coordinate[i]<=(G_LABEl_POSITIVE.size()-1))? 1:0);//Número de caras nodo izquierdo
numImage_left++;
}
}
numImagesNegatives_left=numImage_left-numImagesPositives_left;//Número de NO caras nodo izquierdo
//______________________________________________________________________________

//________________________ORGANIZANDO EL NODO IZQUIERDO_________________________
int numImage_right=numImagesNode-numImage_left,numImagesPositives_right=0,numImagesNegatives_right=0;
bool *imageInNode_right=new bool[G_NUMBER_TOTAL_IMAGES];//Nodo derecho de salida
std::fill(imageInNode_right,imageInNode_right+G_NUMBER_TOTAL_IMAGES,false);
for(int i=cordinateSplitFeatureChosen+1;i<G_NUMBER_TOTAL_IMAGES;i++)
{
if(imagesInNode[vec_coordinate[i]]==true)
{
imageInNode_right[vec_coordinate[i]]=true;//Organizando coordenada de imágenes que pasan al nodo derecho
numImagesPositives_right=numImagesPositives_right+((vec_coordinate[i]<=(G_LABEl_POSITIVE.size()-1))? 1:0);//Número de caras nodo derecho
}
}
numImagesNegatives_right=numImage_right-numImagesPositives_right;//Número de NO caras nodo derecho
//_______________________________________________________________________________

//_________________________________________________________________________________________________
delete []vec_coordinate;



/*AQUÍ SE ORGANIZA LA INFORMACIÓN DE SALIDA PARA EL ACTUAL NODO*/
node->entropyExpected=*entropyByNode;
node->numFeature=featureChosen;

node->feature=new Point2i;
*(node->feature)=G_NPD[featureChosen];

node->threshold=thresholdFeatureChosen;

/*POR CONVENCIÓN SE TOMARA nodesReturn[0] COMO EL IZQUIERDO Y nodesReturn[1] COMO EL DERECHO*/
NODE **nodesReturn=new NODE*[2];

/*____________________________PARTE DE PRUEBA______________________________*/
/*
std::cout<<"Nodo izquierdo y nodo derecho\n";
for(int i=0;i<G_NUMBER_TOTAL_IMAGES;i++)
std::cout<<imageInNode_left[i]<<" ";
std::cout<<"\n";
for(int i=0;i<G_NUMBER_TOTAL_IMAGES;i++)
std::cout<<imageInNode_right[i]<<" ";
std::cout<<"\n";
*/
/*__________________________________________________________________________*/

/*_____se deja informando del próximo bloque precargado_______*/

cv::Point2i lastPoint;

if(order)
lastPoint=range.back();
else
lastPoint=range[0];

rangeLowCharged=lastPoint.x;
rangeHighCharged=lastPoint.y;

order=not(order);
/*____________________________________________________________*/

nodesReturn[0]=new NODE(node,imageInNode_left,numImagesPositives_left,numImagesNegatives_left);
nodesReturn[1]=new NODE(node,imageInNode_right,numImagesPositives_right,numImagesNegatives_right);

return nodesReturn;
}




/*______________________________A CONTINUACIÓN SE DEFINEN ALGUNAS CLASES IMPORTANTES____________________________*/

NODE::NODE(NODE *myParent,bool *initialSetImages,int num_images_positives,int num_images_negatives):parent(myParent),imagesInNode(initialSetImages),numImagesPositives(num_images_positives),numImagesNegatives(num_images_negatives)
{

#if GRAPHICS_PART==1
graphicNode=NULL;
#endif

nodeLeft=NULL;
nodeRight=NULL;

/*____________Aquí establecemos la regla para medir la profundidad (desde 0 hasta depth)_______________*/
if(myParent==NULL)
depth=0;
else{
depth=myParent->depth+1;
}
/*______________________________________________________________________________________________________*/



/*Por defecto el nodo no es terminal*/
pointerToFunctionEvaluation=&NODE::evaluateNodeNoTerminal;
nodeIsTerminal=false;
/*__________________________________*/

numImages=numImagesPositives+numImagesNegatives;
calculateEntropyInNode();/*Se calcula la entropía del nodo*/
setMean();/*Se calcula la media del el nodo*/

/*Valores no establecidos aun*/
/*Los valores NAN indicaran que la variable correspondiente aun no ha sido asignada*/
entropyExpected=NAN;
threshold=NAN;
numFeature=-1;/*El valor -1 indicara que aun no se ha asignado característica (0 hasta G_NUMBER_FEATURE)*/
feature=NULL;

}


NODE::~NODE()
{
/*NOTA: La eliminación de un nodo implica la eliminación de sus nodos hijos */
/*NOTA: Para el nodo padre no se debe eliminar la zona de memoria dinámica a la que apunta el puntero imagesInNode, pues esa zona puede estar siendo usada por un nodo raíz de otro árbol, razón por la cual se dejo esta responsabilidad a la clase STRONG_LEARN quien es a fin de cuantas quien crea dicha zona de memoria en su constructor*/

if(feature!=NULL)delete feature;/*Solo si el nodo no es terminal tendrá asignada memoria dinámica para feature*/

if(parent!=NULL)delete []imagesInNode;

if(nodeLeft!=NULL) delete nodeLeft;
if(nodeRight!=NULL) delete nodeRight;

}



//Calculara la entropía(para un solo nodo es la varianza) inicial con la cual se elaborara el criterio de parada
void NODE::calculateEntropyInNode()
{
//Calculo de la media
double y=0;
double sum=0,sum2=0;
for(int i=0;i<G_LABEl_POSITIVE.size();i++)
{
if(imagesInNode[i]){
sum2++;
sum=sum+G_WEIGHTS_IMAGES[i];
y=y+G_WEIGHTS_IMAGES[i];//MULTIPLICANDO POR LABEL=1
}
}

for(int i=G_LABEl_POSITIVE.size();i<G_NUMBER_TOTAL_IMAGES;i++)
{
if(imagesInNode[i]){
sum2++;
sum=sum+G_WEIGHTS_IMAGES[i];
y=y-G_WEIGHTS_IMAGES[i];//MULTIPLICANDO POR LABEL=-1 
}
}

if(sum!=0)y=y/sum;//Aplicamos el efecto de normalización de los pesos
entropyInNode=(1-y*y); //Se retorna la desviación estándar
//std::cout<<"Media="<<y<<" entropía="<<entropyInNode<<" sum2="<<sum2<<"\n";
//getchar();

}

double NODE::evaluateNodeNoTerminal(uchar *image)
{
/*
int xi=G_NPD[numFeature].x;//Extrayendo coordenada inicial
int xj=G_NPD[numFeature].y;//Extrayendo coordenada final
*/
int xi=feature->x;//Extrayendo coordenada inicial
int xj=feature->y;//Extrayendo coordenada final


/*
cv::namedWindow("ventana",CV_GUI_EXPANDED);
cv::Mat imShow;
cv::Vec3b pixelGray(0,255,0);
cv::Mat imFeature(G_WIDTH_IMAGE,G_HEIGHT_IMAGE,CV_8UC3,cv::Scalar(0,0,0));
cv::Point2i p=G_NPD[numFeature];

int x1=p.x%G_WIDTH_IMAGE;
int y1=p.x/G_WIDTH_IMAGE;
int x2=p.y%G_WIDTH_IMAGE;
int y2=p.y/G_WIDTH_IMAGE;
imFeature.at<cv::Vec3b>(y1,x1)=pixelGray;
imFeature.at<cv::Vec3b>(y2,x2)=pixelGray;

cv::cvtColor(cv::Mat(20,20,CV_8UC1,image),imShow,CV_GRAY2RGB);


cv::imshow("ventana",imShow+imFeature);
std::cout<<"Número de característica="<<numFeature<<" evaluación="<<(G_NPD_VALUES[256*image[xi]+image[xj]])<<"\n";
std::cout<<"xi="<<xi<<"  "<<" xj="<<xj<<"\n";
std::cout<<"image[xi]="<<(int)image[xi]<<"  "<<" image[xj]="<<(int)image[xj]<<"\n";
cv::waitKey();

*/







if(G_NPD_VALUES[256*image[xi]+image[xj]]>threshold) return nodeRight->evaluateNode(image); /*Se envía a el nodo derecho*/
return nodeLeft->evaluateNode(image);/*Se envía a el nodo izquierdo*/

}


double NODE::evaluationNodeTerminal(uchar *image)
{
return yt;
}


void NODE::setMean()
{
double yt_temp=0;
double sum_w=0;

/*NOTA: La predicción en cada Nodo debe ser la media yt o fm (función de regresión), esta se calcula teniendo en cuenta los pesos globales
normalizados, es decir la media en en el nodo*/

int cc=0;
for(int i=0;i<G_NUMBER_TOTAL_IMAGES;i++){
  if(imagesInNode[i]){
   cc++;
   yt_temp=yt_temp+((i<G_LABEl_POSITIVE.size())?1.0:-1.0)*G_WEIGHTS_IMAGES[i];
   sum_w=sum_w+G_WEIGHTS_IMAGES[i];
   }
}

   if(sum_w==0)
   yt=yt_temp;/*Posiblemente cero o un valor muy pequeño, esto evitara posibles valores inf, -inf o NaN*/
   else
   yt=yt_temp/sum_w;
}



void NODE::setNodeAsTerminal()
{
//Adicionamos la dirección del nodo a la lista temporal 
LIST_TERMINAL_NODES.push_back(this);

pointerToFunctionEvaluation=&NODE::evaluationNodeTerminal;
nodeIsTerminal=true;
}

double NODE::evaluateNode(uchar *image)
{
/*La función se modifico el día 8 de agosto a la 1:20 de la mañana, puesto que se encontró que la llamada no generaba un return cuando se compilaba en modo RELEASE, por esto se comenta la siguiente linea y se remplaza por la nueva linea:
 "return (this->*pointerToFunctionEvaluation)(image);
"*/

//(this->*pointerToFunctionEvaluation)(image);
return (this->*pointerToFunctionEvaluation)(image);
}


bool NODE::isNodeTerminal()const
{
return nodeIsTerminal;
}

void NODE::printNode(cv::FileStorage *fileNode,NODE *node)
{
 
if(node==NULL)
{
node=this;
*fileNode<<"nodeRoot"<<"{";


*fileNode<<"information"<<"{";

*fileNode<<"nodeIsTerminal"<<nodeIsTerminal;

if(printOnlyFeatures==false)
{

*fileNode<<"imagesInNode"<<"[";
for(int i=0;i<G_NUMBER_TOTAL_IMAGES;i++)
*fileNode<<imagesInNode[i];
*fileNode<<"]";


*fileNode<<"numImages"<<numImages;
*fileNode<<"numImagesPositives"<<numImagesPositives;
*fileNode<<"numImagesNegatives"<<numImagesNegatives;
*fileNode<<"entropyExpected"<<entropyExpected;
*fileNode<<"entropyInNode"<<entropyInNode;

}

*fileNode<<"threshold"<<threshold;
*fileNode<<"numFeature"<<numFeature;

if(printOnlyFeatures==false)
{

*fileNode<<"feature"<<"[";
if(node->feature==NULL){
*fileNode<<NAN;
*fileNode<<NAN;
}else{
*fileNode<<feature->x;
*fileNode<<feature->y;
}
*fileNode<<"]";


*fileNode<<"depth"<<depth;

}

*fileNode<<"yt"<<yt;

*fileNode<<"}";



}else{

*fileNode<<"information"<<"{";

*fileNode<<"nodeIsTerminal"<<node->nodeIsTerminal;

if(printOnlyFeatures==false)
{

*fileNode<<"imagesInNode"<<"[";
for(int i=0;i<G_NUMBER_TOTAL_IMAGES;i++)
*fileNode<<node->imagesInNode[i];
*fileNode<<"]";


*fileNode<<"numImages"<<node->numImages;
*fileNode<<"numImagesPositives"<<node->numImagesPositives;
*fileNode<<"numImagesNegatives"<<node->numImagesNegatives;
*fileNode<<"entropyExpected"<<node->entropyExpected;
*fileNode<<"entropyInNode"<<node->entropyInNode;

}

*fileNode<<"threshold"<<node->threshold;
*fileNode<<"numFeature"<<node->numFeature;

if(printOnlyFeatures==false)
{

*fileNode<<"feature"<<"[";
if(node->feature==NULL){
*fileNode<<NAN;
*fileNode<<NAN;
}else{
*fileNode<<(node->feature->x);
*fileNode<<(node->feature->y);
}
*fileNode<<"]";


*fileNode<<"depth"<<node->depth;

}

*fileNode<<"yt"<<node->yt;

*fileNode<<"}";

}



if(node->nodeLeft!=NULL){
*fileNode<<"nodeLeft"<<"{";
printNode(fileNode,node->nodeLeft);
}

if(node->nodeRight!=NULL){
*fileNode<<"nodeRight"<<"{";
printNode(fileNode,node->nodeRight);
}

*fileNode<<"}";

}


void NODE::loadNode(cv::FileNode nodeRootFile)
{

LIST_NODES_TREE.push_back(this);//Almacenamos cada nodo en la lista

cv::FileNode information=nodeRootFile["information"];
nodeIsTerminal=(bool)(int)information["nodeIsTerminal"];

if(parent!=NULL){/*Esto es debido a que el nodo raíz se enlaza con las imágenes en el árbol que son las mismas del clasificador fuerte*/
imagesInNode=new bool[G_NUMBER_TOTAL_IMAGES];
{
cv::FileNode node_temp=information["imagesInNode"];
cv::FileNodeIterator it=node_temp.begin(), it_end=node_temp.end();
int idx=0;
for( ; it!= it_end; ++it,idx++)
(*it)>>imagesInNode[idx];
}

}

numImages=(int)information["numImages"];
numImagesPositives=(int)information["numImagesPositives"];
numImagesNegatives=(int)information["numImagesNegatives"];
entropyExpected=(double)information["entropyExpected"];
entropyInNode=(double)information["entropyInNode"];
threshold=(double)information["threshold"];
numFeature=(int)information["numFeature"];



if(!nodeIsTerminal){

feature=new cv::Point2i;
int vecTemp[2];
{
cv::FileNode node_temp=information["feature"];
cv::FileNodeIterator it=node_temp.begin(), it_end=node_temp.end();
int idx=0;
for( ; it!= it_end; ++it,idx++)
(*it)>>vecTemp[idx];
}

feature->x=vecTemp[0];
feature->y=vecTemp[1];

}

if(nodeIsTerminal)
{
setNodeAsTerminal();/*Debe llamarse si el nodo es terminal*/ 
#if GRAPHICS_PART==1
setTerminalNode();
#endif
}

depth=(int)information["depth"];
yt=(double)information["yt"];

#if GRAPHICS_PART==1
if(parent==NULL)
{
createFirstNodeGraphic();
}
#endif

/*_________________APLICAMOS LA RECURSIVIDAD__________________________*/
if(!nodeIsTerminal){

#if GRAPHICS_PART==1
splitNodeGraphic();
#endif

cv::FileNode nodeLeftFile=nodeRootFile["nodeLeft"];
nodeLeft=new NODE(this);
nodeLeft->loadNode(nodeLeftFile);


cv::FileNode nodeRightFile=nodeRootFile["nodeRight"];
nodeRight=new NODE(this);
nodeRight->loadNode(nodeRightFile);

}
/*____________________________________________________________________*/


#if GRAPHICS_PART==1
if(parent==NULL)
{
removeFromScene();
}
#endif

}



#if GRAPHICS_PART==1


//___________Estas funciones sirven para almacenar los comandos que dibujaran posteriormente los nodos______________________

void NODE::createFirstNodeGraphic()
{
DRAW_TREE_COMMANDS tempCommands;
tempCommands.node=this;
tempCommands.command=1;
G_DRAW_TREE_COMMANDS.push_back(tempCommands);
}

void NODE::splitNodeGraphic()
{
DRAW_TREE_COMMANDS tempCommands;
tempCommands.node=this;
tempCommands.command=2;
G_DRAW_TREE_COMMANDS.push_back(tempCommands);
}


void NODE::removeFromScene()
{
DRAW_TREE_COMMANDS tempCommands;
tempCommands.node=this;
tempCommands.command=4;
G_DRAW_TREE_COMMANDS.push_back(tempCommands);
}


void NODE::setTerminalNode()
{
DRAW_TREE_COMMANDS tempCommands;
tempCommands.node=this;
tempCommands.command=3;
G_DRAW_TREE_COMMANDS.push_back(tempCommands);
}

//______________________________________________________________________________________________


#endif


void NODE::propieties()
{
std::cout<<"\n\n";
std::cout<<"Número de imágenes en el nodo="<<numImagesPositives+numImagesNegatives<<"\n";
std::cout<<"Número de imágenes positivas="<<numImagesPositives<<"\n";
std::cout<<"Número de imágenes negativas="<<numImagesNegatives<<"\n";
std::cout<<"Número de característica="<<numFeature<<"\n";
std::cout<<"Umbral de división="<<threshold<<"\n";
std::cout<<"Entropía en el nodo="<<entropyInNode<<"\n";
std::cout<<"Entropía esperada="<<entropyExpected<<"\n";
std::cout<<"Media en el nodo="<<yt<<"\n";
std::cout<<"Profundidad="<<depth<<"\n";
std::cout<<"Es el nodo terminal="<<nodeIsTerminal<<"\n";
std::cout<<"Yo soy="<<(long)this<<" dirección padre="<<(long)parent<<" left="<<(long)nodeLeft<<" right="<<(long)nodeRight<<"\n";
}


//__________________________________________________________________________________________________________________//


TREE_TRAINING::TREE_TRAINING(bool *initialSetImages,int num_images_positives,int num_images_negatives,int percentageQuantityStop,double percentageEntropyStop,int max_depth):imagesInTree(initialSetImages),maxDepth(max_depth)
{
/*NOTA IMPORTANTE:el puntero bool *initialSetImages corresponde al exterior del árbol (ámbito global) y no sera modificado ni por la clase TREE_TRAINING ni por la clase NODE, los nodos derivados del miembro nodeRoot crearan memoria dinámica adicional para almacenar sus vectores bool *initialSetImages puesto que lo necesitan para saber entre que conjunto de imágenes buscar su característica tal que se minimice la entropía, una vez finalizada la construcción del árbol se recomienda eliminar estos vectores ya que no serán de utilidad y solo ocuparan memoria dinámica*/



#if GRAPHICS_PART==1

connect(this,SIGNAL(createFirstNodeGraphic(NODE_GRAPHIC *)),G_WINDOW_TREE,SLOT(createFirstNode(NODE_GRAPHIC *)),Qt::BlockingQueuedConnection);
connect(this,SIGNAL(splitNodeGraphic(NODE_GRAPHIC *)),G_WINDOW_TREE,SLOT(splitNode(NODE_GRAPHIC *)),Qt::BlockingQueuedConnection);
connect(this,SIGNAL(removeFromScene(NODE_GRAPHIC *)),G_WINDOW_TREE,SLOT(removeFromScene(NODE_GRAPHIC *)),Qt::BlockingQueuedConnection);
connect(this,SIGNAL(setTerminalNode(NODE_GRAPHIC *)),G_WINDOW_TREE,SLOT(setTerminalNode(NODE_GRAPHIC *)),Qt::BlockingQueuedConnection);

#endif


#if GRAPHICS_PART==1
G_WINDOWS_CASCADE->pushTree();//Enviamos a la representación gráfica
#endif



/*Inicialmente se construye el nodo raíz*/
nodeRoot=new NODE(NULL,initialSetImages,num_images_positives,num_images_negatives);



/*Condiciones de parada*/
entropyStop=(double(percentageEntropyStop)/100.0)*nodeRoot->entropyInNode;

quantityStop=(double(percentageQuantityStop)/100.0)*nodeRoot->numImages;
if(quantityStop<1)quantityStop=1;

/*A continuación se divide el nodo raíz y se entra en recursividad*/

listNodes.push_back(nodeRoot);//Adicionamos el nodo a la lista
splitNode(nodeRoot);


std::cout<<"Antes de remover nodos\n";

#if GRAPHICS_PART==1
if(G_STOP_TRAINING_FLAG==true){
emit removeFromScene(nodeRoot->graphicNode);//Remueve el árbol de la escena
}
#endif


/*
//____________PUNTO DE RUPTURA DE LA ITERACIÓN___________________
{

QMutexLocker LOCKER(&G_STOP_TRAINING_MUTEX);
if(G_STOP_TRAINING_FLAG==false){
std::cout<<"Detenido en TREE_TRAINING::TREE_TRAINING()\n";
return;
}

}
//_______________________________________________________________
*/

calculateError();/*Por ultimo calculamos el error de el árbol*/


#if GRAPHICS_PART==1
disconnect(this,SIGNAL(createFirstNodeGraphic(NODE_GRAPHIC *)),G_WINDOW_TREE,SLOT(createFirstNode(NODE_GRAPHIC *)));
disconnect(this,SIGNAL(splitNodeGraphic(NODE_GRAPHIC *)),G_WINDOW_TREE,SLOT(splitNode(NODE_GRAPHIC *)));
disconnect(this,SIGNAL(removeFromScene(NODE_GRAPHIC *)),G_WINDOW_TREE,SLOT(removeFromScene(NODE_GRAPHIC *)));
disconnect(this,SIGNAL(setTerminalNode(NODE_GRAPHIC *)),G_WINDOW_TREE,SLOT(setTerminalNode(NODE_GRAPHIC *)));

#endif

}


TREE_TRAINING::~TREE_TRAINING()
{
std::cout<<"Destruyendo árbol\n";

if(nodeRoot!=NULL){


#if GRAPHICS_PART==1
if(nodeRoot->graphicNode!=NULL){

std::cout<<"Destruyendo árbol gráfico\n";
delete nodeRoot->graphicNode;

}
#endif


delete nodeRoot;
nodeRoot=NULL;
}

terminalNodes.clear();/*Para evitar doble liberación de memoria en los nodos terminales*/
}


void TREE_TRAINING::splitNode(NODE *node)
{



NODE **nodeRightAndLeft=getNPD(node,G_NUMBER_CORES_FOR_PARALLEL_SECTIONS);


#if GRAPHICS_PART==1
if(node->parent==NULL)
{

node->graphicNode=new NODE_GRAPHIC(NULL,NODE_GRAPHIC::center,node);
emit createFirstNodeGraphic(node->graphicNode);


}
#endif





node->nodeLeft=nodeRightAndLeft[0];
node->nodeRight=nodeRightAndLeft[1];

listNodes.push_back(node->nodeLeft);//Adicionamos el nodo a la lista
listNodes.push_back(node->nodeRight);//Adicionamos el nodo a la lista


#if GRAPHICS_PART==1
//if(G_STOP_TRAINING_FLAG==true){
emit splitNodeGraphic(node->graphicNode);
//}
std::cout<<"Nodo dividido\n";
#endif



node->propieties();//FUNCIÓN DE INFORMACIÓN (BORRAR AL FINAL)

/*A CONTINUACIÓN SE EXPECIFICA LAS CONDICIONES DE PARADA*/

/*___________________________________________________________________________*/
if((node->depth+1>=maxDepth)||(node->entropyExpected<=entropyStop)||(node->nodeLeft->entropyInNode<=entropyStop)||((node->nodeLeft->numImages)<=quantityStop))
{
node->nodeLeft->setNodeAsTerminal();
terminalNodes.push_back(node->nodeLeft);/*Si el nodo es terminal se lo inserta en la lista de nodos terminales*/
node->nodeLeft->propieties();//FUNCIÓN DE INFORMACIÓN (BORRAR AL FINAL)

#if GRAPHICS_PART==1
emit setTerminalNode(node->nodeLeft->graphicNode);
#endif
//return;
}
else{
splitNode(node->nodeLeft);/*Dividiendo el nodo izquierdo*/
}
/*___________________________________________________________________________*/


/*___________________________________________________________________________*/
if((node->depth+1>=maxDepth)||(node->entropyExpected<=entropyStop)||(node->nodeRight->entropyInNode<=entropyStop)||((node->nodeRight->numImages)<=quantityStop))
{
node->nodeRight->setNodeAsTerminal();
terminalNodes.push_back(node->nodeRight);/*Si el nodo es terminal se lo inserta en la lista de nodos terminales*/
node->nodeRight->propieties();

#if GRAPHICS_PART==1
emit setTerminalNode(node->nodeRight->graphicNode);
#endif

//return;
}
else{
splitNode(node->nodeRight);/*Dividiendo el nodo izquierdo*/
}
/*___________________________________________________________________________*/


}


void TREE_TRAINING::calculateError()
{
error=NAN;/* Se deja indeterminado para futuros cambios*/
}


double TREE_TRAINING::evaluateTree(uchar *image)
{
return nodeRoot->evaluateNode(image);
}


void TREE_TRAINING::printTreeTraining(cv::FileStorage *fileTreeTraining, int numTree)
{
std::string str_numTree;
std::stringstream sstm;
sstm<<"tree_"<<numTree;
str_numTree=sstm.str();

*fileTreeTraining<<str_numTree<<"{";

if(printOnlyFeatures==false)
{

*fileTreeTraining<<"information"<<"{";
*fileTreeTraining<<"error"<<error;
*fileTreeTraining<<"entropyStop"<<entropyStop;
*fileTreeTraining<<"quantityStop"<<quantityStop;
*fileTreeTraining<<"maxDepth"<<maxDepth;
*fileTreeTraining<<"}";

}

nodeRoot->printNode(fileTreeTraining);
*fileTreeTraining<<"}";

}


void TREE_TRAINING::loadWeakLearn(cv::FileNode weakLearnsTrees, int num)
{

#if GRAPHICS_PART==1
G_WINDOWS_CASCADE->pushTree();//Enviamos a la representación gráfica
#endif



std::string str_tree;
std::stringstream sstm;
sstm<<"tree_"<<num;
str_tree=sstm.str();


cv::FileNode tree=weakLearnsTrees[str_tree];
cv::FileNode information=tree["information"];

error=information["error"];
entropyStop=information["entropyStop"];
quantityStop=information["quantityStop"];
maxDepth=information["maxDepth"];


cv::FileNode nodeRootFile=tree["nodeRoot"];
nodeRoot=new NODE;
nodeRoot->imagesInNode=imagesInTree;/*Enlazando imágenes en nodo raíz, clasificador fuerte y árbol*/

LIST_NODES_TREE.clear();
LIST_TERMINAL_NODES.clear();//Limpiamos la lista de nodos terminales 
nodeRoot->loadNode(nodeRootFile);
terminalNodes=LIST_TERMINAL_NODES;//Almacenamos la lista
listNodes=LIST_NODES_TREE;

}


double TREE_TRAINING::getExpectedNumberFeaturestoEvaluate()
{

double numberFeaturesEvaluatedInTree=0;
for(int j=0;j<terminalNodes.size();j++)
numberFeaturesEvaluatedInTree=numberFeaturesEvaluatedInTree+terminalNodes[j]->depth;
numberFeaturesEvaluatedInTree=numberFeaturesEvaluatedInTree/terminalNodes.size();

return numberFeaturesEvaluatedInTree;
}

int TREE_TRAINING::getNumberFeatures()
{

return listNodes.size()-terminalNodes.size();
}

/*__________________________________________________FUNCIÓN GENTLE_ADA_BOOST__________________________________________________*/


void GENTLE_ADA_BOOST(const TREE_TRAINING *tree)
{
/*NOTA:ESTA FUNCIÓN ES FRIEND DE LA CLASE TREE_TRAINING*/
/*NOTA: ESTA FUNCIÓN MODIFICA LA VARIABLE GLOBAL G_WEIGHTS_IMAGES*/

std::vector<NODE *> terminalNodes=tree->terminalNodes;

for(int i=0;i<terminalNodes.size();i++)
{

/*Extraemos la salida de cada nodo terminal como la función de regresión en este caso dada por la media (regresor simple)*/
//_________________________________

double fm=terminalNodes[i]->yt;/*fm es la media en cada nodo*/

     
//_________________________________

double wf=std::exp(-fm);/*modifica los pesos pertenecientes a las CARAS(yi=1 remplazando en exp(-yi*fm) resulta exp(-fm))*/
double wnf=std::exp(fm);/*modifica los pesos pertenecientes a las NO CARAS(yi=-1 remplazando en exp(-yi*fm) resulta exp(fm))*/

/*Si se llega ha este punto los pesos pertenecientes a las CARAS deben disminuir y los pertenecientes a las NO CARAS aumentar*/
bool *imagesInNode=terminalNodes[i]->imagesInNode;


    for(int j=0;j<G_NUMBER_TOTAL_IMAGES;j++)/*Recuérdese que todo vector imagesInNode tiene longitud numTotalImage*/
    {
      if(imagesInNode[j]==true)
      {
      
         if(j<G_LABEl_POSITIVE.size())
         {
         /*Disminuimos los pesos para CARAS*/
         
         G_WEIGHTS_IMAGES[j]=G_WEIGHTS_IMAGES[j]*wf;   

         }else{/*los pesos j-esimos mayores a numLabel_0 pertenecen a las NO CARAS*/
         /*Aumentamos los pesos para NO CARAS*/
         G_WEIGHTS_IMAGES[j]=G_WEIGHTS_IMAGES[j]*wnf;    

         }


      }
      
    }



}


double D=0;
for(int i=0;i<G_NUMBER_TOTAL_IMAGES;i++)
D=D+G_WEIGHTS_IMAGES[i];

if(D==0) return;/*En esta situación la media seria cero, esto evitara valores inf y -inf o posibles NaN*/

for(int i=0;i<G_NUMBER_TOTAL_IMAGES;i++)
G_WEIGHTS_IMAGES[i]=G_WEIGHTS_IMAGES[i]/D;/*Aquí se renormalizan los pesos*/


}


//__________________________________________________________________________________________________________________________













/*_________________________________STRONG LEARN___________________________________________*/

STRONG_LEARN::STRONG_LEARN(double d_min,double f_max,int percentageQuantityStop,double percentageEntropyStop,int max_depth,std::vector<cv::Mat> &setValidationPositive,std::vector<cv::Mat> &setValidationNegative,bool renovateSetFeature):threshold(0.0),dMin(d_min),fMax(f_max)
{


#if GRAPHICS_PART==1
G_WINDOWS_CASCADE->pushStrongLearn(this);//Enviamos a la representación gráfica
#endif



/*PASOS:
1-Inicializar los pesos y elconjunto de imagenes disponibles (vector bool que indicara que la existencia o no de la imagen en el nodo)
2-Entrenar un clasificador fuerte (criterios )
3-Buscar el umbral de forma tal que el conjunto de imagenes positivas pase completamente
4-Evaluar el conjunto de imagenes negativas y pasar al dejar cargando el conjunto de imagenes negativas para el siguiente clasificador*/

/*_______________________________________________INICIALIZACIÓN DE VALORES IMPORTANTES______________________________________*/
dMinAchieved=0;
fMaxAchieved=0;
/*_____________En cada STRONG_LEARN debe inicializarse los pesos y el conjunto de imágenes_________________*/


/*___________Aquí se inicializa por primera vez el conjunto de características disponibles_________________*/
 
   
 if(renovateSetFeature==true)
 std::fill(G_SET_FEATURE,G_SET_FEATURE+G_NUMBER_FEATURE,true);/*Si se habilita la opción de renovar el conjunto de características*/
                                                             /*Nuevamente se restablecen las características previamente descartadas*/
                                                               /*por los clasificadores previos*/
/*________________________________________________________________________________________________________*/


if(G_WEIGHTS_IMAGES!=NULL)delete []G_WEIGHTS_IMAGES;/*Se reserva la posibilidad de entrenar cada clasificador con un número diferente*/
G_WEIGHTS_IMAGES=new double[G_NUMBER_TOTAL_IMAGES]; /*imágenes*/
double initialWeightsImages=1.0/(G_NUMBER_TOTAL_IMAGES);
std::fill (G_WEIGHTS_IMAGES,G_WEIGHTS_IMAGES+G_NUMBER_TOTAL_IMAGES,initialWeightsImages); 

imagesInStrongLearn=new bool[G_NUMBER_TOTAL_IMAGES];/*memoria dinámica perteneciente a los nodos raíces de cada árbol*/
std::fill(imagesInStrongLearn,imagesInStrongLearn+G_NUMBER_TOTAL_IMAGES,true);
 
/*__________________________________________________________________________________________________________*/

/*___________________________________________________________________________________________________________________________*/


while(true){


//____________PUNTO DE RUPTURA DE LA ITERACIÓN___________________
{

QMutexLocker LOCKER(&G_STOP_TRAINING_MUTEX);
if(G_STOP_TRAINING_FLAG==false){
std::cout<<"Detenido en STRONG_LEARN::STRONG_LEARN\n";
return;
}

}
//_______________________________________________________________

TREE_TRAINING *tree=new TREE_TRAINING(imagesInStrongLearn,G_LABEl_POSITIVE.size(),G_LABEl_NEGATIVE.size(),percentageQuantityStop,percentageEntropyStop,max_depth);

/*
//____________PUNTO DE RUPTURA DE LA ITERACIÓN___________________
{

QMutexLocker LOCKER(&G_STOP_TRAINING_MUTEX);
if(G_STOP_TRAINING_FLAG==false){
std::cout<<"Detenido en STRONG_LEARN::STRONG_LEARN\n";
return;
}

}
//_______________________________________________________________
*/


weakLearns.push_back(tree);

checkROC(setValidationPositive,setValidationNegative,true);

/*________________________________________________________________________________*/

if(VPR.size()>0){
std::cout<<"VPR=["<<VPR[0];
for(int i=1;i<VPR.size();i++)
std::cout<<","<<VPR[i];
std::cout<<"]";

std::cout<<"\n";
//getchar();
}

if(FPR.size()>0){
std::cout<<"FPR=["<<FPR[0];
for(int i=1;i<FPR.size();i++)
std::cout<<","<<FPR[i];
std::cout<<"]";

std::cout<<"\n";
//getchar();
}

if(thresholdsPossible.size()>0){
std::cout<<"thresholdsPossible=["<<thresholdsPossible[0];
for(int i=1;i<thresholdsPossible.size();i++)
std::cout<<","<<thresholdsPossible[i];
std::cout<<"]";
}

std::cout<<"\n";
//getchar();

if(searchBestThreshold())
{
std::cout<<"Mejor umbral="<<threshold<<"\n";
break;
}
else
std::cout<<"Mejor umbral="<<threshold<<"\n";


/*__________________________________________________________________________________*/





//std::cout<<"\nPresione tecla para ver número de aciertos de caras\n";
//getchar();
int cc=0;
for(int j=0;j<G_LABEl_POSITIVE.size();j++){
uchar *image=(G_LABEl_POSITIVE.begin()+j)->ptr<uchar>(0);
cc=cc+(int)((evaluateStrongLearnWithZeroThreshold(image)>=0)?1:0);
}
std::cout<<"Número de aciertos CARAS="<<cc<<" tasa de verdaderos positivos="<<(double(cc)/double(G_LABEl_POSITIVE.size()))<<"\n";
//std::cout<<"Presione tecla para ver número de aciertos de NO caras\n";
//getchar();
cc=0;
for(int j=0;j<G_LABEl_NEGATIVE.size();j++){
uchar *image=(G_LABEl_NEGATIVE.begin()+j)->ptr<uchar>(0);
cc=cc+(int)((evaluateStrongLearnWithZeroThreshold(image)<0)?1:0);
}
std::cout<<"Número de aciertos NO CARAS="<<cc<<" tasa de falsos negativos="<<(double(G_LABEl_NEGATIVE.size()-cc)/double(G_LABEl_NEGATIVE.size()))<<"\n";


//std::cout<<"\nPresione tecla para ver número de aciertos de caras CON UMBRAL\n";
//getchar();
cc=0;
for(int j=0;j<G_LABEl_POSITIVE.size();j++){
uchar *image=(G_LABEl_POSITIVE.begin()+j)->ptr<uchar>(0);
cc=cc+(int)evaluateStrongLearn(image);
}
std::cout<<"Número de aciertos CARAS="<<cc<<" verdaderos positivos="<<(double(cc)/double(G_LABEl_POSITIVE.size()))<<"\n";
//std::cout<<"Presione tecla para ver número de aciertos de NO caras CON UMBRAL\n";
//getchar();
cc=0;
for(int j=0;j<G_LABEl_NEGATIVE.size();j++){
uchar *image=(G_LABEl_NEGATIVE.begin()+j)->ptr<uchar>(0);
cc=cc+(int)not(evaluateStrongLearn(image));
}
std::cout<<"Número de aciertos NO CARAS="<<cc<<" falsos negativos="<<(double(G_LABEl_NEGATIVE.size()-cc)/double(G_LABEl_NEGATIVE.size()))<<"\n";
//getchar();


GENTLE_ADA_BOOST(tree);
}

removeNegativeCorrectlyClassified();/*Aquí removemos las imágenes del set negativo que sean correctamente clasificadas*/
std::cout<<"PRIMER CLASIFICADOR CONTRUIDO\n";

}



STRONG_LEARN::~STRONG_LEARN()
{

  std::cout<<"ELIMINANDO CLASIFICADOR FUERTE\n";

  for(int i=0;i<weakLearns.size();i++)
  delete weakLearns[i];

  weakLearns.clear();

  if(imagesInStrongLearn!=NULL)delete []imagesInStrongLearn;
  if(G_WEIGHTS_IMAGES!=NULL)delete []G_WEIGHTS_IMAGES;
  G_WEIGHTS_IMAGES=NULL;

}



double STRONG_LEARN::evaluateStrongLearnWithZeroThreshold(uchar *image)
{

double evaluation=0;

for(int i=0;i<weakLearns.size();i++)
evaluation=evaluation+weakLearns[i]->evaluateTree(image);

return evaluation;
}

bool STRONG_LEARN::evaluateStrongLearn(uchar *image)
{

if(evaluateStrongLearnWithZeroThreshold(image)>=threshold)
return true;/*Se clasifica como positivo*/
else 
return false;/*Se clasifica como negativo*/

}

bool STRONG_LEARN::evaluateStrongLearnEvaluation(uchar *image)
{

score=evaluateStrongLearnWithZeroThreshold(image);
if(score>=threshold)
return true;/*Se clasifica como positivo*/
else 
return false;/*Se clasifica como negativo*/

}


bool STRONG_LEARN::searchBestThreshold()
{
/*Este método busca y establece el mejor umbral de acuerdo a la tasa de falsos positivos y de detección preestablecidas*/
/*NOTA:Si los criterios no se cumplen la función regresa false y establece el como mejor umbral cero*/

if((VPR.size()==0)||(weakLearns.size()==0))return false; /*Debe haberse calculado una roc de antemano*/

for(int i=0;i<VPR.size();i++)
{
   if(VPR[i]>=dMin)
   {
      if(FPR[i]<=fMax)
       {
       threshold=thresholdsPossible[i];/*se allá el umbral tal que los falsos positivos sean mínimos y los verdaderos positivos máximos*/
       dMinAchieved=VPR[i];
       fMaxAchieved=FPR[i];
       }
  
   }
   
}

/*Si se llega a este punto quiere decir que no se encontró en la curva ROC los valores de dMin y fMax que cumplan el criterio*/
/*Se asigna umbral cero y se retorna false*/
if(threshold!=0)
return true;
else{
threshold=0.0;
return false;
}

}


/*____________________________ESTRUCTURA AUXILIAR PARA LA ORDENACIÓN DE LAS EVALUACIONES USADAS EN EL CALCULO DE LA ROC___________________*/
struct G_EVALUATIONS
{
double evaluation;
bool label;/*0 para imágenes del set negativo y 1 para imágenes del set positivo*/
};

//Función de ordenación especifica de menor a mayor con respecto a los valores evaluation
bool sort_evaluation(const G_EVALUATIONS &eva1, const G_EVALUATIONS &eva2) { return eva1.evaluation < eva2.evaluation; }
//_____________________________________________________________________________________

struct G_GROUP_EVALUATIONS
{
double evaluation;
int numberPositives;
int numberNegatives;
G_GROUP_EVALUATIONS():numberPositives(0),numberNegatives(0),evaluation(0){};
G_GROUP_EVALUATIONS(const G_EVALUATIONS &ev):numberPositives(0),numberNegatives(0),evaluation(0){evaluation=ev.evaluation;}

};

/*_______________________________________________________________________________________________________________________________________*/


void STRONG_LEARN::checkROC(std::vector<cv::Mat> &labelPositive,std::vector<cv::Mat> &labelNegative,bool saveROC)
{

int numberImages=labelPositive.size()+labelNegative.size();


/*A CONTINUACIÓN CALCULAMOS LAS EVALUACIONES PARA EL CONJUNTO DE ENTRENAMIENTO*/
std::vector<G_EVALUATIONS> evaluations;
evaluations.reserve(numberImages);
G_EVALUATIONS evaluationTemp;

for(int j=0;j<labelPositive.size();j++){
uchar *image=(labelPositive.begin()+j)->ptr<uchar>(0);

evaluationTemp.evaluation=evaluateStrongLearnWithZeroThreshold(image);
evaluationTemp.label=1;

evaluations.push_back(evaluationTemp);
}


for(int j=0;j<labelNegative.size();j++){
uchar *image=(labelNegative.begin()+j)->ptr<uchar>(0);

evaluationTemp.evaluation=evaluateStrongLearnWithZeroThreshold(image);
evaluationTemp.label=0;

evaluations.push_back(evaluationTemp);
}

/*Ordenamos el vector para poder revisar la ROC a travez de todos los umbrales*/
std::sort(evaluations.begin(),evaluations.end(),sort_evaluation);

/*Encontrando el número de umbrales diferentes*/
std::vector<G_GROUP_EVALUATIONS> groupEvaluations;

G_GROUP_EVALUATIONS groupEvaluations_temp=evaluations[0];
groupEvaluations.push_back(groupEvaluations_temp);

if(evaluations[0].label==true)
groupEvaluations.back().numberPositives++;
else
groupEvaluations.back().numberNegatives++;

for(int i=1;i<numberImages;i++)
{
  
  if(groupEvaluations_temp.evaluation!=evaluations[i].evaluation){
  groupEvaluations_temp=evaluations[i];
  groupEvaluations.push_back(groupEvaluations_temp);
  }

  if(evaluations[i].label==true)
  groupEvaluations.back().numberPositives++;
  else
  groupEvaluations.back().numberNegatives++;



}

/*A CONTINUACIÓN CALCULAMOS LA TABLA DE CONTINGENCIA O MATRIZ DE CONFUCIÓN*/


std::vector<double> VPR;
VPR.reserve(groupEvaluations.size()+2);
std::vector<double> FPR;
VPR.reserve(groupEvaluations.size()+2);
std::vector<double> thresholdsPossible;
thresholdsPossible.reserve(groupEvaluations.size()+2);

/*Prueba positiva*/
for(int i=0;i<groupEvaluations.size()+1;i++)
{
    int vp=0,fn=0,fp=0,vn=0;

    for(int j=0;j<i;j++)
    {
    fn=fn+groupEvaluations[j].numberPositives;
    vn=vn+groupEvaluations[j].numberNegatives;
    }

    for(int j=i;j<groupEvaluations.size();j++)
    {
     vp=vp+groupEvaluations[j].numberPositives; 
     fp=fp+groupEvaluations[j].numberNegatives;
    }

    VPR.push_back(double(vp)/double(vp+fn));
    FPR.push_back(double(fp)/double(fp+vn));
    if(i<groupEvaluations.size())
    thresholdsPossible.push_back((groupEvaluations[i].evaluation));
    
}

thresholdsPossible.push_back(thresholdsPossible.back()+1);/*umbral por defecto*/


/*______________________SI LA VARIABLE saveROC es true se almacenan los datos (por defecto no se almacenan)____________________*/
if(saveROC==true){
this->VPR=VPR;
this->FPR=FPR;
this->thresholdsPossible=thresholdsPossible;
}
/*______________________________________________________________________________________________________________________________*/


}


void STRONG_LEARN::removeNegativeCorrectlyClassified()
{

std::vector<cv::Mat> setNegativeTemp;

for(int j=0;j<G_LABEl_NEGATIVE.size();j++){

 uchar *image=(G_LABEl_NEGATIVE.begin()+j)->ptr<uchar>(0);
 if(evaluateStrongLearn(image))/*Las que retornen true no están bien clasificadas*/
 setNegativeTemp.push_back(G_LABEl_NEGATIVE[j].clone()); 

}

G_LABEl_NEGATIVE.clear();/*Eliminamos todas las imágenes*/
if(setNegativeTemp.size()!=0)G_LABEl_NEGATIVE.reserve(setNegativeTemp.size());

/*Ahora almacenamos nuevamente las imágenes que fueron mal clasificadas*/
for(int j=0;j<setNegativeTemp.size();j++)
{
G_LABEl_NEGATIVE.push_back(setNegativeTemp[j].clone());
}

}


double STRONG_LEARN::get_dMinAchieved()const
{
return dMinAchieved;
}

double STRONG_LEARN::get_fMaxAchieved()const
{
return fMaxAchieved;
}

void STRONG_LEARN::printStrongLearn(cv::FileStorage *fileStrongLearn,int stage)
{

std::string str_stage;
std::stringstream sstm;
sstm<<"stage_"<<stage;
str_stage=sstm.str();

*fileStrongLearn<<str_stage<<"{";

*fileStrongLearn<<"information"<<"{";

if(printOnlyFeatures==false)
{

*fileStrongLearn<<"imagesInStrongLearn"<<"[";
for(int i=0;i<G_NUMBER_TOTAL_IMAGES;i++)
*fileStrongLearn<<imagesInStrongLearn[i];
*fileStrongLearn<<"]";

}

*fileStrongLearn<<"threshold"<<threshold;

if(printOnlyFeatures==false)
{

*fileStrongLearn<<"dMin"<<dMin;
*fileStrongLearn<<"fMax"<<fMax;
*fileStrongLearn<<"dMinAchieved"<<dMinAchieved;
*fileStrongLearn<<"fMaxAchieved"<<fMaxAchieved;
*fileStrongLearn<<"VPR"<<VPR;
*fileStrongLearn<<"FPR"<<FPR;
*fileStrongLearn<<"thresholdsPossible"<<thresholdsPossible;

}

*fileStrongLearn<<"}";

*fileStrongLearn<<"weakLearns"<<"{";
for(int i=0;i<weakLearns.size();i++)
weakLearns[i]->printTreeTraining(fileStrongLearn,i);
*fileStrongLearn<<"}";

*fileStrongLearn<<"}";


}


void STRONG_LEARN::loadStrongLearn(cv::FileNode fileStrongLearn, int stage)
{

#if GRAPHICS_PART==1
G_WINDOWS_CASCADE->pushStrongLearn(this);//Enviamos a la representación gráfica
#endif

std::string str_stage;
std::stringstream sstm;
sstm<<"stage_"<<stage;
str_stage=sstm.str();

cv::FileNode strongLearn=fileStrongLearn[str_stage];
cv::FileNode information=strongLearn["information"];

if(imagesInStrongLearn!=NULL)delete []imagesInStrongLearn;
imagesInStrongLearn=new bool[G_NUMBER_TOTAL_IMAGES];
cv::FileNode node_temp=information["imagesInStrongLearn"];
cv::FileNodeIterator it=node_temp.begin(), it_end=node_temp.end();
int idx=0;
for( ; it!= it_end; ++it,idx++)
(*it)>>imagesInStrongLearn[idx];

threshold=(double)information["threshold"];
dMin=(double)information["dMin"];
fMax=(double)information["fMax"];
dMinAchieved=(double)information["dMinAchieved"];
fMaxAchieved=(double)information["fMaxAchieved"];
information["VPR"]>>VPR;
information["FPR"]>>FPR;

/*Interrumpir aquí para extraer vectores VPR y FPR*/
/*____________________________________________________________________*/
/*
if(VPR.size()>0){
std::cout<<"VPR=["<<VPR[0];
for(int i=1;i<VPR.size();i++)
std::cout<<","<<VPR[i];
std::cout<<"]";

std::cout<<"\n";
//getchar();
}
getchar();
if(FPR.size()>0){
std::cout<<"FPR=["<<FPR[0];
for(int i=1;i<FPR.size();i++)
std::cout<<","<<FPR[i];
std::cout<<"]";

std::cout<<"\n";
//getchar();
}
getchar();
*/
/*_______________________________________________________________________*/







information["thresholdsPossible"]>>thresholdsPossible;

/*A continuación se carga cada weakLearn o arbol*/
cv::FileNode weakLearnsTrees=strongLearn["weakLearns"];
weakLearns.reserve(weakLearnsTrees.size());

for(int i=0;i<weakLearnsTrees.size();i++)
{
TREE_TRAINING *treeAux=new TREE_TRAINING;
treeAux->imagesInTree=imagesInStrongLearn;/*Enlazando imágenes en nodo raíz, clasificador fuerte y árbol*/
treeAux->loadWeakLearn(weakLearnsTrees,i);/*Aquí se carga cada clasificador*/
weakLearns.push_back(treeAux);
}




std::cout<<"Clasificador fuerte número="<<stage<<"\n";

}


double STRONG_LEARN::getExpectedNumberFeaturestoEvaluate()
{

double numberFeaturesEvaluatedInStrongLearn=0;

for(int i=0;i<weakLearns.size();i++)
numberFeaturesEvaluatedInStrongLearn=numberFeaturesEvaluatedInStrongLearn+weakLearns[i]->getExpectedNumberFeaturestoEvaluate();

return numberFeaturesEvaluatedInStrongLearn;
}

int STRONG_LEARN::getNumberFeatures()
{

int numberFeaturesInStrongLearn=0;

for(int i=0;i<weakLearns.size();i++)
{
numberFeaturesInStrongLearn=numberFeaturesInStrongLearn+weakLearns[i]->getNumberFeatures();
}

return numberFeaturesInStrongLearn;

}


CASCADE_CLASSIFIERS::CASCADE_CLASSIFIERS(int negatives,double F,double d_min,double f_max,int percentageQuantityStop,double percentageEntropyStop,int max_depth,bool renovateSetFeature):numberSetNegative(negatives),F(F),dMin(d_min),fMax(f_max),percentageQuantityStop(percentageQuantityStop),percentageEntropyStop(percentageEntropyStop),maxDepth(max_depth),renovateSetFeature(renovateSetFeature),fileCascadeClassifier(NULL),isLoad(false),fGoal(F)
{




numberSetPositive=G_LABEl_POSITIVE.size();
/*Se establece por defecto el número de núcleos para zonas paralelas como el máximo número de hilos soportados*/
G_NUMBER_CORES_FOR_PARALLEL_SECTIONS=omp_get_max_threads();

int featuresChargedInMemory=G_NUMBER_FEATURE;
G_NUMBER_TOTAL_IMAGES=G_LABEl_POSITIVE.size()+negatives;

//Se quito esta llamada puesto que en compilación modo release no funciona
//getNumberFeaturesChargedInMemoryRecommended();/*Se establece cantidad de memoria en segmentos recomendada*/

}


CASCADE_CLASSIFIERS::~CASCADE_CLASSIFIERS()
{


#if GRAPHICS_PART==1
G_WINDOWS_CASCADE->deleteCascade();
#endif


std::cout<<"Destruyendo clasificador \n";

if(G_COORDINATE_VECTOR!=NULL)
{
delete []G_COORDINATE_VECTOR;
G_COORDINATE_VECTOR=NULL;
}


G_LABEl_NEGATIVE.clear();
G_NUMBER_TOTAL_IMAGES=0;

for(int i=0;i<strongLearns.size();i++)
delete strongLearns[i];

strongLearns.clear();



//_____________________PRUEBA___________________________________
G_NPD_VALUES.clear();;
G_NUMBER_FEATURE=0;
G_WIDTH_IMAGE=0;
G_HEIGHT_IMAGE=0;

if(G_NPD!=NULL){
delete []G_NPD;
G_NPD=NULL;
}

G_LABEl_POSITIVE.clear();
G_LABEl_NEGATIVE.clear();
G_LIST_SCANNED_IMAGES.clear();
G_FEATURES_CHARGED_IN_MEMORY=0;
G_NUMBER_TOTAL_IMAGES=0;

if(G_SET_FEATURE!=NULL){
delete []G_SET_FEATURE;
G_SET_FEATURE=NULL;
}

if(G_WEIGHTS_IMAGES!=NULL){
delete []G_WEIGHTS_IMAGES;
G_WEIGHTS_IMAGES=NULL;
}


printOnlyFeatures=false;/*Si es true solo se imprimen las características*/
rangeLowCharged=0;
rangeHighCharged=0;
range.clear();
order=true;

//________________________________________________________________

}


//_____________________________________FUNCIONES DE ESTABLECIMIENTO________________________________________________//
void CASCADE_CLASSIFIERS::set_negatives(int n)
{

numberSetNegative=n;
numberSetPositive=G_LABEl_POSITIVE.size();
G_NUMBER_TOTAL_IMAGES=G_LABEl_POSITIVE.size()+numberSetNegative;

}


void CASCADE_CLASSIFIERS::set_F(double F)
{
this->F=F;
fGoal=this->F;
}

void CASCADE_CLASSIFIERS::set_d_min(double d_min)
{
dMin=d_min;
}

void CASCADE_CLASSIFIERS::set_f_max(double f_max)
{

fMax=f_max;

}

void CASCADE_CLASSIFIERS::set_percentageQuantityStop(int percentageQuantityStop)
{

this->percentageQuantityStop=percentageQuantityStop;

}

void CASCADE_CLASSIFIERS::set_percentageEntropyStop(int percentageEntropyStop)
{

this->percentageEntropyStop=percentageEntropyStop;

}

void CASCADE_CLASSIFIERS::set_max_depth(int max_depth)
{

maxDepth=max_depth;

}

void CASCADE_CLASSIFIERS::set_renovateSetFeature(bool renovateSetFeature)
{

this->renovateSetFeature=renovateSetFeature;

}



void CASCADE_CLASSIFIERS::setNumberCore(int nc)
{

if(nc>omp_get_max_threads())
G_NUMBER_CORES_FOR_PARALLEL_SECTIONS=omp_get_max_threads();
else if(nc<1)
G_NUMBER_CORES_FOR_PARALLEL_SECTIONS=1;
else 
G_NUMBER_CORES_FOR_PARALLEL_SECTIONS=nc;

}

bool CASCADE_CLASSIFIERS::setNumberFeaturesChargedInMemory(int numberFeature)
{

try
{ 
    int tempNumberFeatures;

    if(numberFeature>G_NUMBER_FEATURE)
    tempNumberFeatures=G_NUMBER_FEATURE;
    else
    tempNumberFeatures=numberFeature;

    if(G_COORDINATE_VECTOR!=NULL)delete []G_COORDINATE_VECTOR;
    G_COORDINATE_VECTOR=new unsigned short int[G_NUMBER_TOTAL_IMAGES*tempNumberFeatures];
    G_FEATURES_CHARGED_IN_MEMORY=tempNumberFeatures;
    return true;
}
catch(std::bad_alloc&) 
{
    G_COORDINATE_VECTOR=NULL;
    return false;
}


}


//___________________________________FUNCIONES DE OBTENCIÓN_____________________________________


int CASCADE_CLASSIFIERS::get_negatives()
{
return numberSetNegative;
}

double CASCADE_CLASSIFIERS::get_F()
{
return F;
}

double CASCADE_CLASSIFIERS::get_fGoal()
{
return fGoal;
}

double CASCADE_CLASSIFIERS::get_dMin()
{
return dMin;
}

double CASCADE_CLASSIFIERS::get_fMax()
{
return fMax;
}

int CASCADE_CLASSIFIERS::get_percentageQuantityStop()
{
return percentageQuantityStop;
}


int CASCADE_CLASSIFIERS::get_percentageEntropyStop()
{
return percentageEntropyStop;
}

int CASCADE_CLASSIFIERS::get_max_depth()
{
return maxDepth;
}

bool CASCADE_CLASSIFIERS::get_renovateSetFeature()
{
return renovateSetFeature;
}

double CASCADE_CLASSIFIERS::getExpectedNumberFeaturestoEvaluate()
{

double numberFeaturesEvaluatedInCascade=strongLearns[0]->getExpectedNumberFeaturestoEvaluate();
for(int i=1;i<strongLearns.size();i++){
numberFeaturesEvaluatedInCascade=numberFeaturesEvaluatedInCascade+(strongLearns[i]->dMin)*strongLearns[i]->getExpectedNumberFeaturestoEvaluate();
}
return numberFeaturesEvaluatedInCascade;

}

int CASCADE_CLASSIFIERS::getNumberFeatures()
{

int numberFeatureInCascade=0;

for(int i=0;i<strongLearns.size();i++)
numberFeatureInCascade=numberFeatureInCascade+strongLearns[i]->getNumberFeatures();

return numberFeatureInCascade;

}

int CASCADE_CLASSIFIERS::getNumberFeaturesChargedInMemoryRecommended()
{
int N=MIN_NUMBER_FEATURES_CHARGED_IN_MEMORY;

std::cout<<"min "<<MIN_NUMBER_FEATURES_CHARGED_IN_MEMORY<<"\n";

while(N<=G_NUMBER_FEATURE){
if(setNumberFeaturesChargedInMemory(N))
{

if(N!=G_NUMBER_FEATURE){
delete []G_COORDINATE_VECTOR;
G_COORDINATE_VECTOR=NULL;
}

}
else
{
 G_COORDINATE_VECTOR=new unsigned short int[G_NUMBER_TOTAL_IMAGES*(N-MIN_NUMBER_FEATURES_CHARGED_IN_MEMORY)];
 G_FEATURES_CHARGED_IN_MEMORY=N-MIN_NUMBER_FEATURES_CHARGED_IN_MEMORY;
 break;
}

N=N+MIN_NUMBER_FEATURES_CHARGED_IN_MEMORY;

}

}

void CASCADE_CLASSIFIERS::startTraining()
{

double Fi=1,Di=1;
if(isLoad==false){/*Esto debe hacerse solo si el clasificador es creado, no cargado*/
/*_______________________Se inicializa el conjunto de características__________________________*/
if(G_SET_FEATURE==NULL)G_SET_FEATURE=new bool[G_NUMBER_FEATURE];
std::fill(G_SET_FEATURE,G_SET_FEATURE+G_NUMBER_FEATURE,true);
/*_____________________________________________________________________________________________*/
}else
{
Fi=F;
F=fGoal;
Di=D;



std::cout<<"G_LABEl_NEGATIVE.size()"<<G_LABEl_NEGATIVE.size()<<"\n";
/*Aquí se precarga el ultimo set negativo mal clasificado*/
for(int i=0;i<negativeMisclassified.size();i++)
G_LABEl_NEGATIVE.push_back(negativeMisclassified[i].clone());

}

std::cout<<"EN CLASIFICADOR fGoal="<<fGoal<<"\n";
std::cout<<"EN CLASIFICADOR F="<<F<<"\n";
while(Fi>F){

if(!loadSetNegativeImages(numberSetNegative,this))break;/*Carga las imágenes negativas de entrenamiento*/

 if(get_numberStrongLearns()!=0){
   for(int i=0;i<G_LABEl_NEGATIVE.size();i++){
     uchar *img=G_LABEl_NEGATIVE[i].ptr<uchar>(0);
     std::cout<<evaluateClassifier(img)<<" ";
     }
     std::cout<<"Tamaño="<<G_LABEl_NEGATIVE.size()<<"\n\n";
   }




if(generateEvaluations(G_LABEl_POSITIVE,G_LABEl_NEGATIVE)!=0)break;/*Genera las evaluaciones*/


/*____________________________CONSTRUIMOS UN CLASIFICADOR FUERTE CON LOS PARAMETROS DE ENTRADA___________________________________*/
STRONG_LEARN *strong=new STRONG_LEARN(dMin,fMax,percentageQuantityStop,percentageEntropyStop,maxDepth,G_LABEl_POSITIVE,G_LABEl_NEGATIVE,renovateSetFeature);
/*_______________________________________________________________________________________________________________________________*/

strongLearns.push_back(strong);/*adicionamos el clasificador al la cascada de clasificadores*/


/*___________SE ACTUALIZA EL CONJUNTO DE NEGATIVOS MAL CLASIFICADOS___________________*/
negativeMisclassified.clear();/*Se limpia el conjunto anterior*/
negativeMisclassified.reserve(G_LABEl_NEGATIVE.size());
for(int i=0;i<G_LABEl_NEGATIVE.size();i++)
negativeMisclassified.push_back(G_LABEl_NEGATIVE[i].clone());
/*____________________________________________________________________________________*/

Di=Di*strong->get_dMinAchieved();
D=Di;
Fi=Fi*strong->get_fMaxAchieved();

//_______________________AVISOS, BORRAR AL FINAL_______________________________________________//
std::cout<<"vpr="<<strong->get_dMinAchieved()<<" fpr="<<strong->get_fMaxAchieved()<<"  Fi="<<Fi<<"\n";
std::cout<<"Cantidad="<<G_LABEl_NEGATIVE.size()<<"\n";


   for(int i=0;i<G_LABEl_NEGATIVE.size();i++){
     uchar *img=G_LABEl_NEGATIVE[i].ptr<uchar>(0);
     std::cout<<evaluateClassifier(img)<<" ";
     }
std::cout<<"\n\n\n\n";
    for(int i=0;i<G_LABEl_NEGATIVE.size();i++){
     uchar *img=G_LABEl_NEGATIVE[i].ptr<uchar>(0);
     std::cout<<evaluateClassifier(img)<<" ";
     }
     std::cout<<"\n\n";


//_______________________________________________________________________________________________//




//____________PUNTO DE RUPTURA DE LA ITERACIÓN___________________
{

QMutexLocker LOCKER(&G_STOP_TRAINING_MUTEX);
if(G_STOP_TRAINING_FLAG==false){
std::cout<<"Detenido en CASCADE_CLASSIFIERS::startTraining()\n";
std::cout<<"Información acumulada en el ultimo temporal\n";
return;
}

}
//_______________________________________________________________





//________________________ACUMULACIÓN TEMPORAL______________________
std::cout<<"Guardando temporal\n";
double F_temp=F;
F=Fi;
printCascadeClasifier("temp.xml");
F=F_temp;
std::cout<<"Temporal guardado\n";
//____________________________________________________________________

}

D=Di;/*Actualizamos a el valor de detección alcanzado*/
F=Fi;/*Actualizamos a el valor de falsos positivos alcanzado*/

}



bool CASCADE_CLASSIFIERS::evaluateClassifier(uchar *image)
{

for(int i=0;i<strongLearns.size();i++)
if(!strongLearns[i]->evaluateStrongLearn(image)) return false;/*Se clasifica como label negativo*/

return true;/*Se clasifica como label positivo*/

}

bool CASCADE_CLASSIFIERS::evaluateClassifier(uchar *image,int number)
{

for(int i=0;i<number;i++)
if(!strongLearns[i]->evaluateStrongLearnEvaluation(image)) return false;/*Se clasifica como label negativo*/

score=(strongLearns[number-1])->score;

return true;/*Se clasifica como label positivo*/

}



int CASCADE_CLASSIFIERS::get_numberStrongLearns()const
{
return strongLearns.size();
}


void CASCADE_CLASSIFIERS::printCascadeClasifier(std::string nameFile)
{
fileCascadeClassifier=new cv::FileStorage(nameFile,cv::FileStorage::WRITE);

*fileCascadeClassifier<<"information"<<"{";
if(printOnlyFeatures==false)
{
*fileCascadeClassifier<<"numberSetPositive"<<numberSetPositive;
*fileCascadeClassifier<<"numberSetNegative"<<numberSetNegative;
*fileCascadeClassifier<<"negativeMisclassified"<<negativeMisclassified;
*fileCascadeClassifier<<"D"<<D;
*fileCascadeClassifier<<"F"<<F;
*fileCascadeClassifier<<"fGoal"<<fGoal;
*fileCascadeClassifier<<"dMin"<<dMin;
*fileCascadeClassifier<<"fMax"<<fMax;
*fileCascadeClassifier<<"percentageQuantityStop"<<percentageQuantityStop;
*fileCascadeClassifier<<"percentageEntropyStop"<<percentageEntropyStop;
*fileCascadeClassifier<<"maxDepth"<<maxDepth; 
*fileCascadeClassifier<<"renovateSetFeature"<<renovateSetFeature;
}/*OJO: Aquí se cierra if*/
/*______________________AQUÍ ALMACENAMOS LAS VARIABLES DE ÁMBITO GLOBAL_________________________*/
*fileCascadeClassifier<<"G_WIDTH_IMAGE"<<G_WIDTH_IMAGE;
*fileCascadeClassifier<<"G_HEIGHT_IMAGE"<<G_HEIGHT_IMAGE;

if(printOnlyFeatures==false)
{
*fileCascadeClassifier<<"G_LABEl_POSITIVE"<<G_LABEl_POSITIVE;
*fileCascadeClassifier<<"G_NUMBER_TOTAL_IMAGES"<<G_NUMBER_TOTAL_IMAGES;
*fileCascadeClassifier<<"G_LIST_SCANNED_IMAGES"<<G_LIST_SCANNED_IMAGES;
*fileCascadeClassifier<<"G_SET_FEATURE"<<"[";
for(int i=0;i<G_NUMBER_FEATURE;i++)
*fileCascadeClassifier<<G_SET_FEATURE[i];
*fileCascadeClassifier<<"]";
/*______________________________________________________________________________________________*/
}/*OJO: Aquí se cierra if*/

*fileCascadeClassifier<<"}";

*fileCascadeClassifier<<"cascade_classifiers"<<"{";

for(int i=0;i<strongLearns.size();i++)
strongLearns[i]->printStrongLearn(fileCascadeClassifier,i);


*fileCascadeClassifier<<"}";

fileCascadeClassifier->release();
if(fileCascadeClassifier!=NULL)delete fileCascadeClassifier;
fileCascadeClassifier=NULL;
}



void CASCADE_CLASSIFIERS::printCascadeClasifierEvaluation(std::string nameFile)
{

printOnlyFeatures=true;/*Para solo imprimir las características*/
printCascadeClasifier(nameFile);
printOnlyFeatures=false;/*Se deja nuevamente en su valor por defecto*/

}


void CASCADE_CLASSIFIERS::loadCascadeClasifier(std::string nameFile)
{
fileCascadeClassifier=new cv::FileStorage(nameFile,cv::FileStorage::READ);

cv::FileNode information=(*fileCascadeClassifier)["information"];
numberSetPositive=(int)information["numberSetPositive"];
numberSetNegative=(int)information["numberSetNegative"];
information["negativeMisclassified"]>>negativeMisclassified;
D=(double)information["D"];
F=(double)information["F"];
fGoal=(double)information["fGoal"];
dMin=(double)information["dMin"];
fMax=(double)information["fMax"];
percentageQuantityStop=(double)information["percentageQuantityStop"];
percentageEntropyStop=(double)information["percentageEntropyStop"];
maxDepth=(double)information["maxDepth"];
renovateSetFeature=(bool)(int)information["renovateSetFeature"];

/*__________AQUÍ SE ACTUALIZA LAS VARIABLES GLOBALES_____________*/
G_WIDTH_IMAGE=(int)information["G_WIDTH_IMAGE"];
G_HEIGHT_IMAGE=(int)information["G_HEIGHT_IMAGE"];
generateLockTable();//Restablece la  LOCK TABLE G_NPD_VALUES
generateFeatures();//Esto actualizara las variables G_NUMBER_FEATURE y G_NPD
information["G_LABEl_POSITIVE"]>>G_LABEl_POSITIVE;

/*Se establece por defecto el número de núcleos para zonas paralelas como el máximo número de hilos soportados*/
G_NUMBER_CORES_FOR_PARALLEL_SECTIONS=omp_get_max_threads();
G_NUMBER_TOTAL_IMAGES=(int)information["G_NUMBER_TOTAL_IMAGES"];
information["G_LIST_SCANNED_IMAGES"]>>G_LIST_SCANNED_IMAGES;

getNumberFeaturesChargedInMemoryRecommended();/*pidiendo memoria dinámica para G_NUMBER_TOTAL_IMAGES*/

if(G_SET_FEATURE!=NULL) delete []G_SET_FEATURE;
G_SET_FEATURE=new bool[G_NUMBER_FEATURE];

cv::FileNode node_temp=information["G_SET_FEATURE"];
cv::FileNodeIterator it=node_temp.begin(), it_end=node_temp.end();
int idx=0;
for( ; it!= it_end; ++it,idx++)
(*it)>>G_SET_FEATURE[idx];

/*_______________________________________________________________*/


/*A continuación se carga cada strong learn*/
cv::FileNode cascade_classifiers=(*fileCascadeClassifier)["cascade_classifiers"];
strongLearns.reserve(cascade_classifiers.size());

for(int i=0;i<cascade_classifiers.size();i++)
{
STRONG_LEARN *strongLearnAux=new STRONG_LEARN;
strongLearnAux->loadStrongLearn(cascade_classifiers,i);/*Aquí se carga cada clasificador*/
strongLearns.push_back(strongLearnAux);
}

fileCascadeClassifier->release();
if(fileCascadeClassifier!=NULL)
{
delete fileCascadeClassifier;
fileCascadeClassifier=NULL;
}

}










