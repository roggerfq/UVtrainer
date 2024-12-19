#ifndef INCLUDES_H
#define INCLUDES_H

#include <QRegExp>


/*_________________________________Variables globales de la aplicación__________________________________*/
const QRegExp G_EXTENSIONS_IMAGES("(.jpg|.jpeg|.png|.JPG|.pgm)");/*Formatos de lectura aceptados*/
const QString G_NAME_DIRECTORY_SET_NEGATIVE="./DataBase/Deteccion/SetNegatives";/*nombre de la dirección estándar del conjunto de imágenes negativas*/
const QString G_NAME_DIRECTORY_SET_POSITIVE_TEST="./DataBase/Deteccion/SetPositiveTest";/*nombre de la dirección estándar del conjunto de imágenes positivas de prueba*/
const QString G_NAME_DIRECTORY_SET_NEGATIVE_TEST="./DataBase/Deteccion/SetNegativesTest";/*nombre de la dirección estándar del conjunto de imágenes negativas de prueba*/

const std::string G_NAME_DIRECTORY_IMAGE_DISCARTED="discardedImages";/*nombre de la dirección estándar del conjunto de imágenes negativas descartadas*/
const std::string G_NAME_DIRECTORY_SET_POSITIVE="./DataBase/Deteccion/setPositives/";/*nombre de la dirección estándar del conjunto de imágenes positivas*/
const char *const G_FILE_NAME_CORDINATE_IMAGE="file_coordinateImage";/*Nombre del fichero que almacenara las coordenadas de la imagen*/
const char *const G_FILE_NAME_EVALUATION_FEATURE="file_evaluationFeature";/*Nombre del archivo que almacenara las evaluaciones*/

const QString G_NAME_DIRECTORY_IMAGES_GUI="./Imagenes de la aplicacion";
//static int G_NUMBER_CORES_FOR_PARALLEL_SECTIONS=1;
extern int G_NUMBER_CORES_FOR_PARALLEL_SECTIONS;
const int MIN_NUMBER_FEATURES_CHARGED_IN_MEMORY=100;

/*_____________________FUNCIONES CALLBACK PARA INTERFAZ CONSOLA__________________*/
bool callback_addImagesMessage();


#endif


