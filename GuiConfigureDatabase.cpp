#include "GuiConfigureDatabase.h"
#include "includes.h"

//_____ARCHIVOS DE CABECERA DE QT____________________
#include <QtGui>


extern QVector<QRgb>  sColorTable;


CONFIGURE_DATABASE::CONFIGURE_DATABASE(QWidget *parent):QWidget(parent)
{

nameNewDataBase="";
pathDatabaseTraining="";
sizeImages=0;
flagCount=0;


isLoad=false;
flagAditionImages=false;
flagCutImages=false;

img=new cv::Mat;

organizeObjects();
setConfigLabelInfoGraphic();

installEventFilter(this);
editLineNameDataBase->installEventFilter(this);

}


CONFIGURE_DATABASE::~CONFIGURE_DATABASE()
{

delete img;

}


void CONFIGURE_DATABASE::organizeObjects()
{

buttonLoadDataBase=new QPushButton(tr("Cargar base de datos"));
connect(buttonLoadDataBase,SIGNAL(clicked()),this,SLOT(loadDataBase()));
buttonLoadDataBase->setAutoDefault(false);


buttonCreateNewDataBase=new QPushButton(tr("Crear base de datos"));
connect(buttonCreateNewDataBase,SIGNAL(clicked()),this,SLOT(createDataBase()));
buttonCreateNewDataBase->setAutoDefault(false);

buttonAddImages=new QPushButton(QString::fromUtf8("Adicionar imágenes"));
buttonAddImages->setEnabled(false);
connect(buttonAddImages,SIGNAL(clicked()),this,SLOT(addImages()));
buttonAddImages->setAutoDefault(false);


buttonCancel=new QPushButton(tr("Cancelar"));
buttonCancel->setEnabled(false);
connect(buttonCancel,SIGNAL(clicked()),this,SLOT(cancelOperation()));
buttonCancel->setAutoDefault(false);


sliderGaussianBlur=new QSlider(Qt::Horizontal);
sliderGaussianBlur->setRange(0,100);
connect(sliderGaussianBlur,SIGNAL(valueChanged(int)),this,SLOT(gausianBlur()));
sliderGaussianBlur->setEnabled(false);

editLineNameDataBase=new QLineEdit;
connect(editLineNameDataBase,SIGNAL(editingFinished()),this,SLOT(nameDataBase()));
editLineNameDataBase->setEnabled(false);

editLineGausianBlur=new QLineEdit;
connect(editLineGausianBlur,SIGNAL(editingFinished()),this,SLOT(lineGausianBlurEdit()));
QDoubleValidator* sigmaPermitted=new QDoubleValidator(0.0,9,2,editLineGausianBlur); 
sigmaPermitted->setNotation(QDoubleValidator::StandardNotation); 
editLineGausianBlur->setValidator(sigmaPermitted);
editLineGausianBlur->setMaximumWidth(40);
editLineGausianBlur->setEnabled(false);



editLineResize=new QLineEdit;
QIntValidator *sizePermitted= new QIntValidator;
sizePermitted->setRange(0,1000);
editLineResize->setValidator(sizePermitted); 
connect(editLineResize,SIGNAL(editingFinished()),this,SLOT(gausianBlur()));//Primero debe hacerse el blur
editLineResize->setMaximumWidth(40);
editLineResize->setEnabled(false);

actGaussianBlur=new QCheckBox(tr("filtro gausiano"));
actGaussianBlur->setEnabled(false);
connect(actGaussianBlur,SIGNAL(stateChanged(int)),this,SLOT(activatedFilter()));

actResize=new QCheckBox(tr("Resize"));
actResize->setEnabled(false);
connect(actResize,SIGNAL(stateChanged(int)),this,SLOT(activatedResize()));



//En este layout reposaran todos los elementos
QGridLayout *layoutPrincipal = new QGridLayout;

QGridLayout *layout1 = new QGridLayout;
layout1->addWidget(buttonLoadDataBase,0,0,1,4,Qt::AlignCenter); 
layout1->addWidget(buttonCreateNewDataBase,1,0,1,4,Qt::AlignLeft);
layout1->addWidget(new QLabel("Nombre"),1,4,1,1,Qt::AlignLeft);
layout1->addWidget(editLineNameDataBase,1,5,1,4,Qt::AlignLeft);


QGridLayout *layout2 = new QGridLayout;
layout2->addWidget(actGaussianBlur,0,0,1,1,Qt::AlignLeft);
layout2->addWidget(sliderGaussianBlur,0,2,1,2,Qt::AlignCenter);
layout2->addWidget(editLineGausianBlur,0,5,1,1,Qt::AlignRight);
layout2->addWidget(actResize,1,0,1,1,Qt::AlignLeft);
layout2->addWidget(editLineResize,1,5,1,1,Qt::AlignRight);



layoutPrincipal->addLayout(layout1,0,0,1,4);
layoutPrincipal->addLayout(layout2,1,0,1,4);
layoutPrincipal->addWidget(buttonAddImages,2,1,1,2);
layoutPrincipal->addWidget(buttonCancel,3,3,1,1);


//Se estable el layout principal
setLayout(layoutPrincipal);


}



bool CONFIGURE_DATABASE::saveDatabase()
{


if(imageDataBase.size()==0){
QMessageBox::warning(this, tr("WARNING"),tr("Debe construir o cargar una base de datos.\n"),QMessageBox::Ok);
return false;
}



if(editLineResize->text()==""){//Si resize esta activado las imágenes tendrán igual tamaño y su alto y ancho sera igual
/*___________Se debe revitalizar que las imágenes tengan igual tamaño y su ancho y alto sean iguales_________*/
cv::Size sz=(imageDataBase[0]).size();
for(int i=0;i<imageDataBase.size();i++)
{

if(sz!=(imageDataBase[i]).size())
{
QMessageBox::warning(this, tr("WARNING"),QString::fromUtf8("Se encontraron imágenes cuyo ancho y alto son diferentes, establezca un tamaño común entre ellas.\n"),QMessageBox::Ok);
return false;
}else if((imageDataBase[i].rows!=imageDataBase[i].cols)){
QMessageBox::warning(this, tr("WARNING"),QString::fromUtf8("Se encontraron imágenes cuyo ancho y alto son diferentes, establezca un tamaño común entre ellas.\n"),QMessageBox::Ok);
return false;
}

}
/*________________________________________________________________________*/
}




if(isLoad==true){
if(((editLineResize->text()!="")||(editLineGausianBlur->text()!="")||(flagCount>1)||(flagAditionImages==true)||(flagCutImages==true))){//Indica que se debe guardar la copia de la nueva base de datos
QMessageBox::warning(this,QString::fromUtf8("INFORMACIÓN"),tr("Se guardara una copia de la base de datos resultante puesto que la inicial fue modificada.\n"),QMessageBox::Ok);
}else
{
return true;//No se aplican cambios
}
}



//Aplicando filtro gausiano
if(editLineGausianBlur->text()!=""){
for(int i=0;i<imageDataBase.size();i++)
GaussianBlur(imageDataBase[i],imageDataBase[i],cv::Size(kx,ky),sigma);
}

//Aplicando filtro resize
if(editLineResize->text()!=""){
for(int i=0;i<imageDataBase.size();i++)
cv::resize(imageDataBase[i],imageDataBase[i],cv::Size(sizeImages,sizeImages),0,0,CV_INTER_AREA);
}




std::string file_save;
QString file_temp;
QStringList nameList;


QDir directory(QString::fromStdString(G_NAME_DIRECTORY_SET_POSITIVE));
QStringList filters;
filters << "*.xml";
nameList=directory.entryList(filters);
if(nameNewDataBase==""){
QString nameNewDataBase_temp;

QString nameBase_NewDataBase("untilited_"); 
int i =0;
nameNewDataBase_temp=nameBase_NewDataBase+QString::number(i);
while(nameList.contains(nameNewDataBase_temp+".xml"))
{
i++;
nameNewDataBase_temp=nameBase_NewDataBase+QString::number(i); 
}

file_temp=QFileDialog::getSaveFileName(this, tr("Save Data Base"),directory.path()+"/"+nameNewDataBase_temp+".xml",tr("Data(*.xml)"));
if(file_temp=="")return false;

file_save=file_temp.toStdString();//Cuando el nombre de la base de datos lo propone el programa.
}else{
file_temp=QString::fromStdString(nameNewDataBase+".xml");
int i=1;
while(nameList.contains(file_temp))
{
file_temp=QString::fromStdString(nameNewDataBase)+"_"+QString::number(i)+".xml";
i++;
}

file_save=(directory.path()).toStdString()+"/"+file_temp.toStdString();//Cuando la base de datos es creada

}
//_________________________________________________________________________________________________


pathDatabaseTraining=file_save;
cv::FileStorage faceDataBase(file_save,cv::FileStorage::WRITE);

faceDataBase<<"Info"<<"{";
faceDataBase<<"width_image"<<sizeImages;
faceDataBase<<"high_image"<<sizeImages;
faceDataBase<<"labels_number"<<"{";

faceDataBase<<"set_positivos"<<int(imageDataBase.size());

faceDataBase<<"}";
faceDataBase<<"}";



faceDataBase<<"Data"<<"{";
faceDataBase<<"Labels"<<"{";


faceDataBase<<"set_positivos"<<"{";
//__________________
std::string nameLabelTemp;
for(int j=0;j<imageDataBase.size();j++)
{
std::stringstream sstm;
    sstm<<"Image_"<<j+1;//NOMBRE Image_ es obligatorio
    nameLabelTemp=sstm.str();
faceDataBase<<nameLabelTemp<<imageDataBase[j];
}
//__________________
faceDataBase<<"}";


faceDataBase<<"}";
faceDataBase<<"}";


faceDataBase.release();


isLoad=true;
flagAditionImages=false;
flagCutImages=false;
flagCount=1;

editLineResize->clear();
editLineGausianBlur->clear();

actGaussianBlur->setCheckState(Qt::Unchecked);
actResize->setCheckState(Qt::Unchecked);

sliderGaussianBlur->setValue(0);
sliderImages->setValue(0);


}


bool CONFIGURE_DATABASE::deleteDataBase()
{


if((isLoad==true)||(editLineNameDataBase->text()!="")||((editLineResize->text()!="")||(editLineGausianBlur->text()!="")||(flagCount>1)||(flagAditionImages==true)||(flagCutImages==true))){//Indica que se debe guardar la copia de la nueva base de datos

QMessageBox::StandardButton question;
question=QMessageBox::question(this, "Test", QString::fromUtf8("Desea terminar la edición de la base de datos?"),QMessageBox::Yes|QMessageBox::No);

if(question==QMessageBox::No)
return false;

}


cancelOperation();

return true;

}


void CONFIGURE_DATABASE::addImages()
{

 QStringList files = QFileDialog::getOpenFileNames(this,QString::fromUtf8("Seleccione las imágenes a adicionar"),QString::fromStdString(G_NAME_DIRECTORY_SET_POSITIVE),
                        "Images (*.png *.jpg *.pgm)");

 if(files.isEmpty()) return;//NO se cargaron imágenes

 flagAditionImages=true;
 
 //_____________Cargando imágenes__________________//

 for(int i=0;i<files.size();i++){
 std::string nameFile=(files[i]).toStdString();
 cv::Mat temp=cv::imread(nameFile);

 if(temp.channels()!=1)
 cv::cvtColor(temp,temp,cv::COLOR_BGR2GRAY);

 imageDataBase.push_back(temp.clone());
 }

actualizeSliderImages(imageDataBase.size());//Se informa al sistema que la base de datos cambio de numero

//Solo podrán editarse imágenes si existen imágenes
actGaussianBlur->setEnabled(true);
actResize->setEnabled(true);

}


void CONFIGURE_DATABASE::activatedFilter()
{

if(Qt::Checked==actGaussianBlur->checkState())
{
sliderGaussianBlur->setEnabled(true);
editLineGausianBlur->setEnabled(true);
}else{
editLineGausianBlur->clear();
sliderGaussianBlur->setValue(0);
sliderGaussianBlur->setEnabled(false);
editLineGausianBlur->setEnabled(false);
gausianBlur();
}

}

void CONFIGURE_DATABASE::activatedResize()
{


if(Qt::Checked==actResize->checkState())
{
editLineResize->setEnabled(true);
}else{
editLineResize->clear();
editLineResize->setEnabled(false);
gausianBlur();
}


}



void CONFIGURE_DATABASE::loadDataBase()
{


//____Abrimos el dialogo para cargar un archivo____
QString fileName = QFileDialog::getOpenFileName(this,tr("Abrir Base de Datos"),QString::fromStdString(G_NAME_DIRECTORY_SET_POSITIVE),tr("Archivos de datos xml (*.xml)\n"));
if(fileName=="") return;//Por si el usuario no abre nada.
//_________________________________________________


pathDatabaseTraining=fileName.toStdString();
cv::FileStorage faceDataBase(fileName.toStdString(),cv::FileStorage::READ);

//Extraemos los nodos y subnodos 
cv::FileNode Info=(faceDataBase)["Info"];
cv::FileNode labels_number=Info["labels_number"];
cv::FileNode Data=(faceDataBase)["Data"];
cv::FileNode labels_image=Data["Labels"];
cv::FileNode set_positivos=labels_image["set_positivos"];


//________Para proteger de archivos que no coincidan con el formato preestablecido__________
if(Info.empty()|| labels_number.empty()|| Data.empty() || labels_image.empty()){
QMessageBox::warning(this, tr("WARNING"),tr("El archivo no tiene el formato valido o ha sido corrompido.\n"),QMessageBox::Ok);
return;
}
//__________________________________________________________________________________________

isLoad=true;
flagCount++;


sizeImages=Info["width_image"];
int numberImages=labels_number["set_positivos"];


std::string str_base="Image_";//NOMBRE BASE OBLIGATORIO
for(int i=1;i<=numberImages;i++)
{
std::stringstream sstm;
sstm<<str_base<<i;

cv::Mat temp;
set_positivos[sstm.str()]>>(temp);

imageDataBase.push_back(temp.clone());

}



buttonCreateNewDataBase->setEnabled(false);
buttonCancel->setEnabled(true);
actGaussianBlur->setEnabled(true);
actResize->setEnabled(true);
buttonAddImages->setEnabled(true);

actualizeSliderImages(imageDataBase.size());
gausianBlur();

}




void CONFIGURE_DATABASE::createDataBase()
{

buttonAddImages->setEnabled(true);
buttonLoadDataBase->setEnabled(false);
editLineNameDataBase->setEnabled(true);


buttonCancel->setEnabled(true);

}

void CONFIGURE_DATABASE::cancelOperation()
{

buttonAddImages->setEnabled(false);

buttonLoadDataBase->setEnabled(true);
buttonCreateNewDataBase->setEnabled(true);

editLineNameDataBase->setEnabled(false);

actGaussianBlur->setEnabled(false);
actResize->setEnabled(false);
actGaussianBlur->setCheckState(Qt::Unchecked);
actResize->setCheckState(Qt::Unchecked);

editLineNameDataBase->clear();
editLineGausianBlur->clear();
editLineResize->clear();

sliderImages->setValue(0);
sliderImages->setEnabled(false);
numImagebottom->setEnabled(false);
numImageTop->setEnabled(false);
lineEdit_numImage->setEnabled(false);
show_maxNumImage->setText("Max:");


imageDataBase.clear();

isLoad=false;
flagAditionImages=false;
flagCutImages=false;
flagCount=0;

nameNewDataBase="";
pathDatabaseTraining="";

QImage black(2,2,QImage::Format_RGB888);
black.fill(QColor(0,0,0));
seeImage(black);

}





void CONFIGURE_DATABASE::setConfigLabelInfoGraphic()
{


//_________________BOTONES BOTTOM Y TOP______________________________________________________________________//
numImagebottom=new QPushButton;
numImagebottom->setIcon(QIcon(G_NAME_DIRECTORY_IMAGES_GUI+"/Botones/scissorsBottom.jpg"));
numImagebottom->setFixedSize(QSize(40,30));
numImagebottom->setIconSize(QSize(30,20));
connect(numImagebottom,SIGNAL(clicked()),this,SLOT(deleteBottom()));
numImagebottom->setAutoDefault(false);
numImagebottom->setEnabled(false);

numImageTop=new QPushButton;
numImageTop->setIcon(QIcon(G_NAME_DIRECTORY_IMAGES_GUI+"/Botones/scissorsTop.jpg"));
numImageTop->setFixedSize(QSize(40,30));
numImageTop->setIconSize(QSize(30,20));
connect(numImageTop,SIGNAL(clicked()),this,SLOT(deleteTop()));
numImageTop->setAutoDefault(false);
numImageTop->setEnabled(false);
//___________________________________________________________________________________________________________//


//__________________________________SLIDER DE IMAGENES______________________________________________________//
sliderImages=new QSlider(Qt::Horizontal);
connect(sliderImages,SIGNAL(valueChanged(int)),this,SLOT(gausianBlur()));
sliderImages->setFixedWidth(80);
sliderImages->setEnabled(false);
//__________________________________________________________________________________________________________//

QIntValidator *numberPermitted= new QIntValidator;
numberPermitted->setBottom(0);//Para solo permitir mayores e iguales a cero
lineEdit_numImage=new QLineEdit;  
lineEdit_numImage->setEnabled(false);
lineEdit_numImage->setFixedWidth(80);
connect(lineEdit_numImage,SIGNAL(returnPressed()),this,SLOT(editLabel_numImagen()));
lineEdit_numImage->setValidator(numberPermitted);


show_maxNumImage=new QLabel;
show_maxNumImage->setFixedWidth(108);
show_maxNumImage->setStyleSheet("color: rgb(255, 0, 0);");
show_maxNumImage->setText("Max:");



QHBoxLayout *infoGraphicLayout=new QHBoxLayout;

infoGraphicLayout->addWidget(numImagebottom);
infoGraphicLayout->addWidget(sliderImages);
infoGraphicLayout->addWidget(numImageTop);
infoGraphicLayout->addWidget(lineEdit_numImage);
infoGraphicLayout->addWidget(show_maxNumImage);



labelInfoGraphic=new QLabel;
labelInfoGraphic->setLayout(infoGraphicLayout);

}


void CONFIGURE_DATABASE::seeImageGray(const cv::Mat &image)
{


(*img)=image.clone();
QImage qimg=QImage((uchar*)img->data,img->cols,img->rows,img->step,QImage::Format_Indexed8);
qimg.setColorTable(sColorTable);
emit seeImage(qimg);
}

/*
void CONFIGURE_DATABASE::seeFeature(int index)
{

static cv::Point2i p;
static QImage im;
int x1,x2,y1,y2;

//__________Extrayendo coordenadas___________ 
p=G_NPD[index];

x1=p.x%G_WIDTH_IMAGE;
y1=p.x/G_WIDTH_IMAGE;

x2=p.y%G_WIDTH_IMAGE;
y2=p.y/G_WIDTH_IMAGE;
//___________________________________________

     QImage imgAndFeature=imageShow->convertToFormat(QImage::Format_RGB888);
     QPainter painter(&imgAndFeature);
   
     //Estableciendo estilo del lápiz virtual (pen)
     painter.setRenderHint(QPainter::Antialiasing);
     QPen penHLines(QColor(0,255,0),3,Qt::SolidLine,Qt::RoundCap,Qt::MiterJoin);
     painter.setPen(penHLines);
     
    //Calculando coordenadas relativas al tamaño de la nueva imagen
     x1=((width_labelInfoGraphic)/(G_WIDTH_IMAGE-1))*(x1+0.5);  
     y1=((height_labelControl)/(G_HEIGHT_IMAGE-1))*(y1+0.5);

     x2=((width_labelInfoGraphic)/(G_WIDTH_IMAGE-1))*(x2+0.5);  
     y2=((height_labelControl)/(G_HEIGHT_IMAGE-1))*(y2+0.5);
    //Dibujando el vector (Característica NPD) en la imagen
     painter.drawLine(x1,y1,x2,y2);
     painter.translate(x2,y2);
     double angle=(180/M_PI)*atan(double(x2-x1)/double(y2-y1));
     painter.rotate(-angle-90);
     painter.drawLine(0,0,10,10);
     painter.drawLine(0,0,10,-10);

     labelGraphic->setPixmap(QPixmap::fromImage((imgAndFeature)));

}
*/


void CONFIGURE_DATABASE::actualizeSliderImages(int n)
{


sliderImages->setMaximum(n-1);
sliderImages->setValue(0);
show_maxNumImage->setText(QString("Max:")+QString::number(n-1));

gausianBlur();
sliderImages->setEnabled(true);

numImagebottom->setEnabled(true);
numImageTop->setEnabled(true);
lineEdit_numImage->setEnabled(true);

}




void CONFIGURE_DATABASE::gausianBlur()
{

lineEdit_numImage->setText(QString::number(sliderImages->value()));

*img=imageDataBase[sliderImages->value()].clone(); 

if(Qt::Checked==actGaussianBlur->checkState()){

double sigma=9*double(sliderGaussianBlur->value())/100;//El sigma variara con una precisión de 1/100 entre 0  y 5;

kx=6*sigma;
ky=6*sigma;
/*____Convirtiendo siempre en impar los valores____*/
if(kx%2==0)
{
kx++;
ky++;
}
/*__________________________________________________*/

editLineGausianBlur->setText(QString::number(sigma));

qDebug()<<sigma<<" "<<kx<<" "<<ky<<"\n";
GaussianBlur(*img,*img,cv::Size(kx,ky),sigma);

}

resizeImage();


}


void CONFIGURE_DATABASE::resizeImage()
{


if(Qt::Checked==actResize->checkState()){
sizeImages=editLineResize->text().toInt();
if((sizeImages!=0)){

cv::resize(*img,*img,cv::Size(sizeImages,sizeImages),0,0,CV_INTER_AREA);

}
}

seeImageGray(*img);

}




void CONFIGURE_DATABASE::nameDataBase()
{

nameNewDataBase=editLineNameDataBase->text().toStdString();

}

void CONFIGURE_DATABASE::lineGausianBlurEdit()
{
int newValue=(100*editLineGausianBlur->text().toDouble())/9;
sliderGaussianBlur->setValue(newValue);
}

void CONFIGURE_DATABASE::deleteBottom()
{


int minimum=0, maximum=imageDataBase.size()-1;
bool ok;

int bottomImage = QInputDialog::getInt(
                 this,QString::fromUtf8("Edición"),
                 QString("Entre el bottom.")+"<font color=red>"+QString("MAX: ")+QString::number(maximum)+"</font></h2>", 0,  
                 minimum, maximum, 1, &ok);



if(bottomImage==maximum)
{
QMessageBox::warning(this, tr("WARNING"),QString::fromUtf8("No es permitido borrar todas las imágenes"),QMessageBox::Ok);
return;
}

if(ok){
imageDataBase.erase(imageDataBase.begin(),imageDataBase.begin()+bottomImage+1);
actualizeSliderImages(imageDataBase.size());
flagCutImages=true;
}

}


void CONFIGURE_DATABASE::deleteTop()
{

int minimum=0, maximum=imageDataBase.size()-1;
bool ok;
int topImage = QInputDialog::getInt(
                 this,QString::fromUtf8("Edición"),
                 QString("Entre el top.")+"<font color=red>"+QString("MIN: 0")+"</font></h2>", maximum,
                 minimum, maximum, 1, &ok);


if(topImage==minimum)
{
QMessageBox::warning(this, tr("WARNING"),QString::fromUtf8("No es permitido borrar todas las imágenes"),QMessageBox::Ok);
return;
}


if(ok){
imageDataBase.erase(imageDataBase.begin()+topImage,imageDataBase.end());
actualizeSliderImages(imageDataBase.size());
flagCutImages=true;
}


}


void CONFIGURE_DATABASE::editLabel_numImagen()
{

QString str=lineEdit_numImage->text();
int num=str.toInt();

if(str==QString(""))
lineEdit_numImage->setText(QString::number(sliderImages->value()));
else if(num<imageDataBase.size())
sliderImages->setValue(num);
else{
lineEdit_numImage->setText(QString::number(sliderImages->value()));
QMessageBox::warning(this, tr("WARNING"),QString("Debe digitar un numero entre 0 y ")+QString::number(imageDataBase.size()-1),QMessageBox::Ok);
}


lineEdit_numImage->clearFocus();    //Después de bloquear el lineEdit_widthImageNewDataBase se quita el focus del teclado
}


bool CONFIGURE_DATABASE::eventFilter(QObject *target, QEvent *event)
{


return QWidget::eventFilter(target, event);
}




