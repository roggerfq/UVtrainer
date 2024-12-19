#ifndef GUICONFIGUREDATABASE_H
#define GUICONFIGUREDATABASE_H

#include <QWidget>

//________________LIBRERIAS OPENCV___________________
#include "opencv2/opencv.hpp"

/*___________CLASES OCULTAS QT__________*/
class QPushButton;
class QCheckBox;
class QSlider;
class QLineEdit;
class QLabel;


class CONFIGURE_DATABASE:public QWidget
{
Q_OBJECT

/*_______Estos objetos pertenecen al label de control___________*/
QPushButton *buttonCreateNewDataBase;
QPushButton *buttonLoadDataBase;
QPushButton *buttonAddImages;
QPushButton *buttonCancel;
QPushButton *numImagebottom;
QPushButton *numImageTop;


QSlider     *sliderGaussianBlur;
QSlider     *sliderImages;

QLineEdit   *editLineNameDataBase;
QLineEdit   *editLineGausianBlur;
QLineEdit   *editLineResize;
QLineEdit   *lineEdit_numImage;

QCheckBox   *actGaussianBlur;
QCheckBox   *actResize;

QLabel *show_maxNumImage;


std::vector<cv::Mat> imageDataBase;//Lista de imágenes
cv::Mat *img;//Imagen que contendrá la imagen sola
double sigma;//Desviación estándar filtro gausiano
int kx;// Tamaño dirección x filtro gausiano
int ky;// Tamaño dirección y filtro gausiano
int sizeImages;//Tamaño de las imágenes después de reescalar

bool isLoad;
bool flagAditionImages;
bool flagCutImages;
int flagCount;
std::string nameNewDataBase;

/*________Funciones privadas______*/
void setConfigLabelInfoGraphic();
void organizeObjects();
/*________________________________*/

public:
CONFIGURE_DATABASE(QWidget *parent = 0);
~CONFIGURE_DATABASE();
std::string pathDatabaseTraining;//Dirección que debe leer el entrenamiento
QLabel *labelInfoGraphic;//Label que contendrá la información gráfica

public slots:
bool saveDatabase();
bool deleteDataBase();
void addImages();
void activatedFilter();
void activatedResize();

void loadDataBase();
void createDataBase();
void cancelOperation();

void seeImageGray(const cv::Mat &image);
void actualizeSliderImages(int n);
void gausianBlur();
void resizeImage();
void nameDataBase();
void lineGausianBlurEdit();
void deleteBottom();
void deleteTop();
void editLabel_numImagen();

signals:
void seeImage(const QImage &qimg);

protected:
bool eventFilter(QObject *target, QEvent *event);

};




#endif
