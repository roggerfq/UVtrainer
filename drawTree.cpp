#include <drawTree.h>
#include "GUI_trainingFaceDetector.h"

#include "guiCongfigureTraining.h"
#include "training.h"
#include "opencv2/opencv.hpp"//Se requiere para completar los tipos incompletos declarados en training.h


//____________CLASES DE QT__________________
#include <QtGui>
//__________________________________________
#include<iostream>
#include<cmath>

extern CONFIGURE_TRAINING *G_GUI_CONFIG_TRAINING;
extern GUI_TRAININ_FACE_DETECTOR *G_GUI_TRAININ_FACE_DETECTOR;


//_____________________________ITEM DE INFORMACIÓN____________________________________________//


INFORMATION::INFORMATION()
{

QGraphicsTextItem::hide();//Esto hará que la ventana inicie oculta

//_______Inicialización de variables____________
mouseEnter=QPointF(0,0);//Por defecto
AreaParent=QRectF(-100,-100,200,200);

backgroundColor=QColor(Qt::yellow);
outlineColor=QColor(Qt::black);
textColor=QColor(Qt::blue);
titleColor=QColor(Qt::black);
//______________________________________________

setFlag(ItemIgnoresTransformations, true);

update();
}


void INFORMATION::setNode(NODE_GRAPHIC *node)
{
this->node=node;
}

void INFORMATION::show(QGraphicsSceneHoverEvent * event)
{

if(event)mouseEnter=event->pos();

//_____________Posiciona la ventana de información al lado contrario del ratón__________________________________
setPos(node->scenePos()+QPoint(-2*mouseEnter.x(),((mouseEnter.y()>0)?mouseEnter.y():0)+(AreaParent.width()/2)));
//______________________________________________________________________________________________________________

QGraphicsTextItem::show();
}


void INFORMATION::push_back(QString str)
{

if(info.size()<=6){
info.push_back(str);
update();//Para generar un repaint
}

}

QPainterPath INFORMATION::shape()const 
{
//_______________Esta función se construyo para que no existan colisiones con ningún item_________________________
QRectF rect(0,0,0,0);
QPainterPath path;
path.addRect(rect);
return path;
//_______________________________________________________________________________________________________________
}


void INFORMATION::paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget)
{


//painter->translate(70,70);
QPen pen;
pen.setColor(outlineColor);
pen.setStyle(Qt::DotLine);//Linea intermitente
pen.setWidth(4);//Ancho de linea 2
painter->setPen(pen);
painter->setBrush(QBrush(backgroundColor, Qt::Dense1Pattern));
painter->drawRoundedRect (AreaParent,40,40,Qt::RelativeSize);


pen.setWidth(1);
pen.setStyle(Qt::SolidLine);//Linea intermitente
pen.setColor(textColor);
painter->setPen(pen);
//painter->drawText(AreaParent, Qt::AlignCenter,QString("Informacion"));//Escribimos el texto centrado en el item
//_____________________Organizando información a mostrar_________________________________________________________
painter->setFont(QFont("Times", 12, QFont::Bold));
painter->drawText(AreaParent, Qt::AlignTop,QString::fromUtf8(" \n Información:\n"));//Escribimos el texto centrado en el item
painter->setFont(QFont("Times", 11, QFont::Normal));


if(info.isEmpty())return;
if(info.size()<=4)
{
painter->drawText(AreaParent, Qt::AlignTop,QString::fromUtf8(" \n\n\n No imágenes=")+info[0]+QString("\n No caras=")+info[1]+QString("\n No no caras=")+info[2]+QString::fromUtf8("\n Entropía en el nodo=")+info[3]);
}else{
painter->drawText(AreaParent, Qt::AlignTop,QString::fromUtf8(" \n\n\n No imágenes=")+info[0]+QString("\n No caras=")+info[1]+QString("\n No no caras=")+info[2]+QString::fromUtf8("\n Entropía en el nodo=")+info[3]+QString::fromUtf8("\n Entropía esperada=")+info[4]+QString::fromUtf8("\n característica=")+info[5]);
}
//_______________________________________________________________________________________________________________

}


//____________________________________________________________________________________________//



Link::Link(NODE_GRAPHIC *toNode):node(toNode)
{
collisionState=false;
setFlags(ItemNegativeZStacksBehindParent);
setColor();
trackNodes();
}

void Link::setColor(const QColor &color,const int widthLine)
{
pen.setColor(color);
pen.setWidth(widthLine);
update();
}


QRectF Link::outlineRect() const
{

QRectF rect=QRectF(node->pos().x(),0,-node->pos().x(),node->pos().y()).normalized();
const int Padding = 8;
rect.adjust(-Padding, -Padding, +Padding, +Padding);
return rect;

}

void Link::trackNodes()
{
prepareGeometryChange();
update();
}


QPainterPath Link::shape() const
{

QPainterPath path;

double radiusParent=(node->nodeParent->outlineRect().width())/2;
double myRadius=(node->outlineRect().width())/2;
double c=std::sqrt(std::pow(node->pos().x(),2)+std::pow(node->pos().y(),2));
QPointF vector=(node->pos())/c;
QPointF vectorNormal=QPointF(vector.y(),-vector.x());

QPointF p1=(radiusParent+(c/8))*vector;
QPointF p2=p1+5*vectorNormal;
QPointF p3=(c-myRadius-(c/8))*vector;
QPointF p4=p3+5*vectorNormal;
QPointF p5=p3-5*vectorNormal;
QPointF p6=p1-5*vectorNormal;

QPolygonF polygon;
polygon<<p1<<p2<<p4<<p3<<p5<<p6;
path.addPolygon(polygon);

return path;

}


QRectF Link::boundingRect() const
{
return outlineRect();
}

void Link::paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget)
{

painter->setPen(pen);
QRectF rect = outlineRect();
painter->drawLine(QPointF (0,0),node->pos());

}





NODE_GRAPHIC::NODE_GRAPHIC(NODE_GRAPHIC *parent,Side side,NODE *thisNode):nodeParent(parent),mySide(side),dataNode(thisNode)
{


/*Esto construirá una lista lineal con todos los items (NODE_GRAPHIC Y link) asociados a un árbol*/
if(parent==NULL){
items=new QList<NODE_GRAPHIC *>;
links=new QList<Link *>;
items->push_back(this);
}else{
items=parent->items;
links=parent->links;
items->push_back(this);
}
//_______________________________________________________________________________________________

collisionState=false;


//___Inicializacion de variables___________

nodeLeft=NULL;
nodeRight=NULL;
branchLeft=NULL;
branchRight=NULL;
//_________________________________________

font = qApp->font();
font.setBold(true);//Establecemos la fuente con negrilla
setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges|ItemNegativeZStacksBehindParent|ItemIsFocusable);
setAcceptHoverEvents(true);



myInfo.setNode(this);

//_______________________POR DEFECTO____________________________//

if(dataNode!=NULL){
setText(QString::number(dataNode->numImages));

//_____INFORMACIÓN_____________________//
myInfo.push_back(QString::number(dataNode->numImages));
myInfo.push_back(QString::number(dataNode->numImagesPositives));
myInfo.push_back(QString::number(dataNode->numImagesNegatives));
myInfo.push_back(QString::number(dataNode->entropyInNode));
myInfo.push_back(QString::number(dataNode->entropyExpected));
myInfo.push_back(QString::number(dataNode->numFeature));
//_____________________________________//

}


setTextColor(Qt::black);
setOutlineColor(Qt::red);
setBackgroundColor(Qt::green);
setType(NODE_GRAPHIC::NodeAlone);
//______________________________________________________________//


}


void NODE_GRAPHIC::setText(const QString &text)
{
prepareGeometryChange();
myText = text;
update();
}


void NODE_GRAPHIC::setTextColor(const QColor &color)
{
myTextColor=color;
update();
}

void NODE_GRAPHIC::setOutlineColor(const QColor &color)
{
myOutlineColor=color;
update();
}

void NODE_GRAPHIC::setBackgroundColor(const QColor &color)
{
myBackgroundColor=color;
update();
}


void NODE_GRAPHIC::setType(const NodeType tp)
{
prepareGeometryChange();
type=tp;
update();
}


void NODE_GRAPHIC::setSide(Side side)
{
prepareGeometryChange();
mySide=side;
update();
}

void NODE_GRAPHIC::setDataNode(NODE *thisNode)
{
dataNode=thisNode;
}


QGraphicsScene *NODE_GRAPHIC::myScene()
{
//std::cout<<topLevelItem()<<"\n";
return topLevelItem()->scene();
}

void NODE_GRAPHIC::splitNode()
{

//________Información posterior____________//
/*NOTA: debido a que la función push_back del objeto myInfo restringe a 6 la longitud su lista interna, esto no tendra efectos para los nodos padres*/
myInfo.push_back(QString::number(dataNode->entropyExpected));
myInfo.push_back(QString::number(dataNode->numFeature));
//_________________________________________//



nodeLeft=new NODE_GRAPHIC(this,NODE_GRAPHIC::left);
dataNode->nodeLeft->graphicNode=nodeLeft;
nodeLeft->setDataNode(dataNode->nodeLeft);
//nodeLeft->setText(QString::number(1000));
nodeLeft->setTextColor(Qt::black);
nodeLeft->setOutlineColor(Qt::red);
nodeLeft->setBackgroundColor(Qt::green);
nodeLeft->setType(NODE_GRAPHIC::NodeAlone);
//____________Información________________________//
nodeLeft->setText(QString::number(dataNode->nodeLeft->numImages));

nodeLeft->myInfo.push_back(QString::number(dataNode->nodeLeft->numImages));
nodeLeft->myInfo.push_back(QString::number(dataNode->nodeLeft->numImagesPositives));
nodeLeft->myInfo.push_back(QString::number(dataNode->nodeLeft->numImagesNegatives));
nodeLeft->myInfo.push_back(QString::number(dataNode->nodeLeft->entropyInNode));

//_______________________________________________//



nodeRight=new NODE_GRAPHIC(this,NODE_GRAPHIC::right);
dataNode->nodeRight->graphicNode=nodeRight;
nodeRight->setDataNode(dataNode->nodeRight);
//nodeRight->setText(QString::number(1000));
nodeRight->setTextColor(Qt::black);
nodeRight->setOutlineColor(Qt::red);
nodeRight->setBackgroundColor(Qt::green);
nodeRight->setType(NODE_GRAPHIC::NodeAlone);
//____________Información________________________//
nodeRight->setText(QString::number(dataNode->nodeRight->numImages));

nodeRight->myInfo.push_back(QString::number(dataNode->nodeRight->numImages));
nodeRight->myInfo.push_back(QString::number(dataNode->nodeRight->numImagesPositives));
nodeRight->myInfo.push_back(QString::number(dataNode->nodeRight->numImagesNegatives));
nodeRight->myInfo.push_back(QString::number(dataNode->nodeRight->entropyInNode));


//_______________________________________________//






nodeLeft->setParentItem(this);
nodeRight->setParentItem(this);

nodeLeft->moveBy(-separation_x,separation_y);
nodeRight->moveBy(separation_x,separation_y);


branchLeft=new Link(nodeLeft);
branchRight=new Link(nodeRight);
branchLeft->setParentItem(this);
branchRight->setParentItem(this);
branchLeft->trackNodes();
branchRight->trackNodes();

branchLeft->setZValue(-1);
branchRight->setZValue(-1);

links->push_back(branchLeft);
links->push_back(branchRight);


correctTree();

}


void NODE_GRAPHIC::extendBranch()
{
nodeLeft->moveBy(-separation_x/1.5,0);
nodeRight->moveBy(separation_x/1.5,0);
branchLeft->trackNodes();
branchRight->trackNodes();
}


NODE_GRAPHIC *NODE_GRAPHIC::correctTreeLeft(NODE_GRAPHIC *nodeTemp)
{
if((nodeTemp==NULL)||(nodeTemp->nodeParent==NULL))return 0;
//____________CORRIGIENDO POR IZQUIERDA___________________
QList<QGraphicsItem *> collidingLeft=nodeTemp->nodeLeft->collidingItems();
if(collidingLeft.count()>0)
{
//Encontrar nodo padre de la rama mas próxima derecha(si no existe se llega al nodo padre)
while(nodeTemp->mySide==left)
nodeTemp=nodeTemp->nodeParent;
//Extender dicho nodo
if(nodeTemp->nodeParent==NULL)
{
nodeTemp->extendBranch();
}
else
{
nodeTemp->nodeParent->extendBranch();
//Se revisa nuevamente el mismo nodo para asegurar que ya no existe choque
collidingLeft=nodeLeft->collidingItems();
}

}
//________________________________________________________
return nodeTemp;
}

NODE_GRAPHIC *NODE_GRAPHIC::correctTreeRight(NODE_GRAPHIC *nodeTemp)
{
if((nodeTemp==NULL)||(nodeTemp->nodeParent==NULL))return 0;
//____________CORRIGIENDO POR DERECHA______________________
QList<QGraphicsItem *> collidingRight=nodeTemp->nodeRight->collidingItems();
if(collidingRight.count()>0)
{
//Encontrar nodo padre de la rama mas próxima derecha(si no existe se llega al nodo padre)
while(nodeTemp->mySide==right)
nodeTemp=nodeTemp->nodeParent;
//Extender dicho nodo
if(nodeTemp->nodeParent==NULL)
{
nodeTemp->extendBranch();
}
else
{
nodeTemp->nodeParent->extendBranch();
//Se revisa nuevamente el mismo nodo para asegurar que ya no existe choque
collidingRight=nodeRight->collidingItems();
}

}
//___________________________________________________________

return nodeTemp;
}


void NODE_GRAPHIC::correctTree()
{

//_____________________________________________________________
NODE_GRAPHIC *node;
NODE_GRAPHIC *nodeAux;
Link *link;

for(int j=0;j<1;j++){ //Se revisa un determinado numero de veces si las colisiones ya se solucionaron (usar un while seria bloqueante)
for (int i = 0; i <items->size(); ++i) {
QList<QGraphicsItem *> it=(*items)[i]->collidingItems(); //Extrayendo la lista de items que colisionan con el actual item
if(it.count()>0)
{
node=dynamic_cast<NODE_GRAPHIC *>(it[0]);//La mayoría de veces siempre sera un un solo item(itme[0])
if(node==NULL){ //Si el item con el que colisiona es un enlace
link=dynamic_cast<Link *>(it[0]); //Se convierte nuevamente a objeto Link
QList<QGraphicsItem *> it_link=link->collidingItems(); //Se miran las colisiones de este enlace
int aux=0;//Variable usada para evitar un loop infinito a continuación
while((it_link.count()>0)&&(aux<50)) //Mientras colisione se extienden las ramas del nodo abuelo(nodeAux->nodeParent)
{
aux++;
nodeAux=dynamic_cast<NODE_GRAPHIC *>(it_link[0]);
if((nodeAux==NULL)||(nodeAux->nodeParent==NULL)) break;//Colisión entre enlaces o nodo raíz
nodeAux->nodeParent->extendBranch();
it_link=link->collidingItems();
}

}else{
correctTreeLeft(node->nodeParent);
correctTreeRight(node->nodeParent);
}

}

}

}

//____________________________________________________________

waringCollisionTotal();

}



void NODE_GRAPHIC::waringCollisionTotal()
{
//_____________________
for(int i=0;i<items->size();i++)
{
int fl=((*items)[i]->collidingItems()).count();
if((fl==0)&&((*items)[i]->collisionState==true)){
(*items)[i]->collisionState=false;
(*items)[i]->setBackgroundColor(Qt::green);
}else if((fl!=0)&&((*items)[i]->collisionState==false)){
(*items)[i]->collisionState=true;
(*items)[i]->setBackgroundColor(Qt::red);
}

}


for(int i=0;i<links->size();i++)
{
int fl=((*links)[i]->collidingItems()).count();
if((fl==0)&&((*links)[i]->collisionState==true)){
(*links)[i]->collisionState=false;
(*links)[i]->setColor(Qt::black,2);
}else if((fl!=0)&&((*links)[i]->collisionState==false)){
(*links)[i]->collisionState=true;
(*links)[i]->setColor(Qt::red,5);
}

}
//_____________________

}



QRectF NODE_GRAPHIC::boundingRect() const
{
return outlineRect().adjusted(-1000,-1000,1000,1000);
}

QPainterPath NODE_GRAPHIC::shape() const
{
QRectF rect = outlineRect();
QPainterPath path;
path.addEllipse(rect);
return path;
}

void NODE_GRAPHIC::paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget)
{

QPen pen(myOutlineColor);
//________________Vista para cuando el item es seleccionado____________________
if (option->state & QStyle::State_Selected) {
pen.setStyle(Qt::DotLine);//Linea intermitente
pen.setWidth(2);//Ancho de linea 2
}
//_____________________________________________________________________________


painter->setPen(pen);//Lápiz normal de color myOutlineColor
painter->setBrush(myBackgroundColor);//Fondo de color myBackgroundColor
QRectF rect = outlineRect();//Necesario por que la geometría puede cambiar

//____________________TIPOS DE NODO SEGUN EL CASO______________________________
switch(type)
{
case NodeAlone:painter->drawEllipse(rect);break;//Si es un nodo solo (ejemplo un nodo padre aun no dividido)
case NodeTerminal:painter->drawRect(rect);break;//Para cuando el nodo es terminal
}
//______________________________________________________________________________

painter->setFont(font);//Se establece la fuente como font(Se le establecio negrilla en el constructor)
painter->setPen(myTextColor);//Cambiamos el color del lápiz a myTextColor
painter->drawText(rect, Qt::AlignCenter, myText);//Escribimos el texto centrado en el item

}




QRectF NODE_GRAPHIC::outlineRect() const
{
const int Padding =8;
QFontMetricsF metrics = qApp->font();
QRectF rect = metrics.boundingRect(myText);
int dim=std::max(rect.width(),rect.height());
rect=QRectF(0,0,dim,dim);
rect.adjust(-Padding, -Padding, +Padding, +Padding);
rect.translate(-rect.center());
return rect;
}


void NODE_GRAPHIC::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
if(branchRight||branchLeft)return;
//splitNode2();
};

void NODE_GRAPHIC::mousePressEvent(QGraphicsSceneMouseEvent * event)
{

static bool flagAllOr=true;//Para que pueda verse la característica del nodo raíz

QList<int> listFeatures;

if(nodeParent!=NULL)//No es un nodo raíz
{
int numFeature=dataNode->numFeature;
if(numFeature!=-1)//Si no es un nodo terminal
{

listFeatures.push_back(numFeature);
QMetaObject::invokeMethod(G_GUI_CONFIG_TRAINING, "seeTree",
                          Qt::QueuedConnection,Q_ARG(QList<int>,listFeatures));

}

}else 
{//Es un nodo raíz
//______________________________________________________________//



if(flagAllOr==true){
QList<QGraphicsItem *> itemList=scene()->items();//Extraemos todos los graphicItems hijos
NODE_GRAPHIC *nodeTemp;

for(int i=0;i<itemList.size();i++)
{
//_________________________________________________//
nodeTemp=dynamic_cast<NODE_GRAPHIC *>(itemList[i]);
if(nodeTemp!=NULL)//Si es un nodo
{
int numFeature=nodeTemp->dataNode->numFeature;
if(numFeature!=-1)//Si no es un nodo terminal
{
listFeatures.push_back(numFeature);
}

}
//________________________________________________//
}


QMetaObject::invokeMethod(G_GUI_CONFIG_TRAINING, "seeTree",
                          Qt::QueuedConnection,Q_ARG(QList<int>,listFeatures));

//_______________________________________________________________//
}else
{

int numFeature=dataNode->numFeature;
if(numFeature!=-1)//Si no es un nodo terminal
{

listFeatures.push_back(numFeature);
QMetaObject::invokeMethod(G_GUI_CONFIG_TRAINING, "seeTree",
                          Qt::QueuedConnection,Q_ARG(QList<int>,listFeatures));

}

}

flagAllOr=not(flagAllOr);


}




}

void NODE_GRAPHIC::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{

    if(dataNode->numFeature!=-1)
    {//Opción valida solo para nodos no terminales
    QMenu menu;
    //QAction *removeAction = menu.addAction("Ver ROC");
    QAction *markAction = menu.addAction("Ver respuesta");
    QAction *selectedAction = menu.exec(event->screenPos());

    if(selectedAction!= 0)
    qDebug()<<selectedAction->text()<<"\n";
    }


QDir dirPositivesTest(G_NAME_DIRECTORY_SET_POSITIVE_TEST);
QDir dirNegativesTest(G_NAME_DIRECTORY_SET_NEGATIVE_TEST);
QStringList filters;
filters << "*.png" << "*.jpg" << "*.bmp"<<"*.pgm";

QStringList imageNameListPositivesTest= dirPositivesTest.entryList(filters, QDir::Files|QDir::NoDotAndDotDot);
QStringList imageNameListNegativesTest= dirNegativesTest.entryList(filters, QDir::Files|QDir::NoDotAndDotDot);

cv::Point2i p=G_NPD[dataNode->numFeature];//Se extrae la característica
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
float npd_c=float(M[p.x]-M[p.y])/(M[p.x]+M[p.y]);//Evaluando característica NPD

evalNegatives.push_back(double(npd_c));//Almacenamos el resultado en la lista
}

}




//_________________PARA LOS POSITIVOS_________________________________
for(int i=0;i<imageNameListPositivesTest.size();i++){
cv::Mat img=cv::imread(dirPositivesTest.path().toStdString()+"/"+imageNameListPositivesTest[i].toStdString());

if(img.data!=NULL){

//________Por si la imagen necesítame convertirse a escala de grises_______________//
if(img.channels()!=1)
cv::cvtColor(img,img,CV_BGR2GRAY);

cv::resize(img,img,cv::Size(G_WIDTH_IMAGE,G_HEIGHT_IMAGE));//Se redimensiona al tamaño de las imágenes de entrenamiento

uchar *M=img.ptr<uchar>(0);//Leyendo la dirección de la imagen
float npd_c=float(M[p.x]-M[p.y])/(M[p.x]+M[p.y]);//Evaluando característica NPD

evalPositives.push_back(double(npd_c));//Almacenamos el resultado en la lista
}

}


QMetaObject::invokeMethod(G_GUI_TRAININ_FACE_DETECTOR,"plotResponse",
                          Qt::QueuedConnection,Q_ARG(QVector<double>,evalNegatives),
                          Q_ARG(QVector<double>,evalPositives),
                          Q_ARG(bool,true),
                          Q_ARG(double,dataNode->threshold));



}




QVariant NODE_GRAPHIC::itemChange(GraphicsItemChange change,const QVariant &value)
{

if ((change == ItemPositionHasChanged)&&(isSelected())) 
{

//___________PARA QUE LA VENTANA DE INFORMACIÓN SIGA AL NODO______________

//Descomente para que el recuadro de información siga a los nodos
/*
myScene()->addItem(&myInfo);
myInfo.show();
update();
*/
//________________________________________________________________________

if(nodeParent!=NULL) {
waringCollisionTotal();//Revisa cada vez que hay un movimiento si existen colisiones
//___________Aquí se sigue cada Link_______________
if(mySide==left)
nodeParent->branchLeft->trackNodes();
else
nodeParent->branchRight->trackNodes();
//_________________________________________________
}



}

return QGraphicsItem::itemChange(change, value);
}

void NODE_GRAPHIC::focusOutEvent(QFocusEvent * event)
{
//qDebug()<<"Deselecciono\n"<<"\n";

}


void NODE_GRAPHIC::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{

myScene()->addItem(&myInfo);
myInfo.show(event);
update();
if(nodeParent!=NULL)nodeParent->myInfo.hide();//Este código oculta recursivamente la venta de información para nodos anteriores

}

void NODE_GRAPHIC::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{

myScene()->removeItem(&myInfo);
update();

}



//_________________________________IMPLEMENTACIÓN DE EL ÁRBOL_______________________________________________



TREE_ITEM::TREE_ITEM()
{


myTextColor = Qt::darkGreen;
myOutlineColor = Qt::darkBlue;
myBackgroundColor = Qt::white;
setFlags(ItemIsMovable|ItemIsSelectable);

setBrush(myBackgroundColor);
QPen pen(myOutlineColor);
setPen(pen);
setRect(0,0,100,100);

nodeRoot=new NODE_GRAPHIC; 
nodeRoot->setText(QString::number(1000));
nodeRoot->setTextColor(Qt::black);
nodeRoot->setOutlineColor(Qt::red);
nodeRoot->setBackgroundColor(Qt::green);
nodeRoot->setType(NODE_GRAPHIC::NodeAlone);

nodeRoot->setParentItem(this);
QPointF pos=nodeRoot->pos();
nodeRoot->moveBy(50,50);
nodeRoot->setVisible(false);

}

/*
void TREE_ITEM::setText(const QString &text)
{


}

void TREE_ITEM::setTextColor(const QColor &color)
{



}


void TREE_ITEM::setOutlineColor(const QColor &color)
{



}


void TREE_ITEM::setBackgroundColor(const QColor &color)
{



}




QPainterPath TREE_ITEM::shape() const
{

//_______________Esta función se construyo para que no existan colisiones con ningún item_________________________
QRectF rect(0,0,0,0);
QPainterPath path;
path.addRect(rect);
return path;
//_______________________________________________________________________________________________________________


}




void TREE_ITEM::paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget)
{




}

*/

/*
void TREE_ITEM::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{

QList<QGraphicsView *> view=scene()->views();

view[0]->setScene(scene2);

qDebug()<<"Doble click\n";

}
*/

//____________________________________________________________________________________________________________



//_____________________Definimos la hoja que mostrara las diferentes escenas__________________________
SHEET::SHEET(QWidget *parent):QGraphicsView(parent)
{
setDragMode(ScrollHandDrag);
}


void SHEET::wheelEvent(QWheelEvent *event)
{
double numDegrees = -event->delta() / 8.0;
double numSteps = numDegrees / 15.0;
double factor = std::pow(1.125, numSteps);
scale(factor, factor);
}


//_____________________________________________________________________________________________________


SCENE::SCENE(QObject * parent)
{


}



//__________________Definimos la ventana principal donde va a ver nuestra hoja SHEET__________________

WINDOWS_PRINCIPAL_TREE::WINDOWS_PRINCIPAL_TREE()
{

widthScene=1000;
heightScene=3000;
scene= new SCENE;
scene2= new SCENE;

scene->setSceneRect(0,0,widthScene,heightScene);
scene2->setSceneRect(0,0,widthScene,heightScene);


view = new SHEET;
view->setScene(scene);
setCentralWidget(view);
setWindowTitle(tr("ESTRUCTURA DEL CLASIFICADOR"));


}





//______FUNCIONES DE PRUEBA_______________


void WINDOWS_PRINCIPAL_TREE::addItem(QGraphicsItem *item)
{

scene->addItem(item);
item->moveBy((scene->width())/2,(scene->height())/2);
}




//________________________________________









NODE_GRAPHIC *WINDOWS_PRINCIPAL_TREE::selectedNode() const
{

QList<QGraphicsItem *> items = scene->selectedItems();
if (items.count() == 1) {
return dynamic_cast<NODE_GRAPHIC  *>(items.first());
} else {
return 0;
}

}


void WINDOWS_PRINCIPAL_TREE::createFirstNode(NODE_GRAPHIC *node)
{

node->moveBy((scene->width())/2,(scene->height())/2);
scene->addItem(node);

}

void WINDOWS_PRINCIPAL_TREE::splitNode(NODE_GRAPHIC *node)
{

node->splitNode();

}


void WINDOWS_PRINCIPAL_TREE::removeFromScene(NODE_GRAPHIC *nodeRoot)
{


QList<QGraphicsItem *> itemsList=scene->items(); 
for(int i=0;i<itemsList.size();i++)
{
INFORMATION *infoTemp=dynamic_cast<INFORMATION *>(itemsList[i]);
if(infoTemp!=NULL)scene->removeItem(infoTemp);
}


scene->removeItem(nodeRoot);/*Se procede a remover todos los nodos y links de la escena (puesto que cada nodo y link subsiguiente es un hijo 
de nodeRoot solo es necesario remover el item padre en este caso nodeRoot es padre de todos los nodos y links)*/
}



void WINDOWS_PRINCIPAL_TREE::setTerminalNode(NODE_GRAPHIC *node)
{

//________Información posterior____________//
//NOTA:Esto se hace debido a que los nodos terminales ya no visitan su función de división*/ 
node->myInfo.push_back(QString::number(node->dataNode->entropyExpected));
node->myInfo.push_back(QString::number(node->dataNode->numFeature));
//_________________________________________//


node->setType(NODE_GRAPHIC::NodeTerminal);
}


void WINDOWS_PRINCIPAL_TREE::keyPressEvent(QKeyEvent * event )
{

 static int numScena=0;
  
 //qDebug() << "Ate key press" << event->key();
  
 qDebug()<<"Escena numero="<<numScena<<"\n";
 myNode=selectedNode();
 if(!myNode) return;
 myNode->extendBranch();
 myNode->waringCollisionTotal();
 numScena++;

}

//_____________________________________________________________________________________________________





void WINDOWS_PRINCIPAL_TREE::seeGraphicNode(NODE_GRAPHIC *node)
{

qDebug()<<"Se envió un nodo a dibujar\n";

/*
if(node->scene()==scene)//Recuerde que scene es la escena que muestra el árbol en construcción
{
//Cuando se selecciona el ultimo ultimo árbol

qDebug()<<"Árbol en construcción\n";

//_______________________________________________________________
if(view->scene()==scene)
{
//Cuando el árbol en construcción es visible
return;//No se hace nada

}else
{
view->setScene(scene);//Cuando aun no es visible
return;//No se hace mas nada
}
//________________________________________________________________

}
*/

if(node==NULL)
{

//_______________________________________________________________
if(view->scene()==scene)
{
//Cuando el árbol en construcción es visible
return;//No se hace nada

}else
{
view->setScene(scene);//Cuando aun no es visible
return;//No se hace mas nada
}
//________________________________________________________________


}else
{
//De lo contrario se habrá seleccionado un árbol ya construido

//___________________________________________________________________________________________________________
if(node->scene()==scene2)//Averiguamos si el árbol esta instalado en la escena 2
{
//_______________________________________________________________
if(view->scene()==scene2){
return;//El árbol ya esta en la escena 2 y esta es visible
}else{
view->setScene(scene2);//El árbol esta instalado en la escena 2 pero esta no es visible, así que se hace visible
return;
}
//_______________________________________________________________

}
else
{

qDebug()<<QString::fromUtf8("Árbol previamente construido")<<"\n";

//___________________Removemos el árbol actualmente instalado____________________//

QList<QGraphicsItem *> ListItemsInScene2=scene2->items();

if(!ListItemsInScene2.empty())//Si la lista no es vacía, existe algún árbol y se debe remover
{

int countTemp=0;
while((countTemp<ListItemsInScene2.size()))
{

if(ListItemsInScene2[countTemp]->parentItem()==0)//Buscamos el nodo raíz de el árbol
{
scene2->removeItem(ListItemsInScene2[countTemp]);
break;
}

countTemp++;
}


}

//________________________________________________________________________________//

if(view->scene()==scene2){
scene2->addItem(node);
}
else{
view->setScene(scene2);
scene2->addItem(node);
}

}



//_____________________________________________________________________________________________________________

}


}

extern std::vector<DRAW_TREE_COMMANDS> G_DRAW_TREE_COMMANDS;
void WINDOWS_PRINCIPAL_TREE::readCommands()
{

int sz=G_DRAW_TREE_COMMANDS.size();

for(int i=0;i<sz;i++)
{

switch (G_DRAW_TREE_COMMANDS[i].command)
{
case 1:
G_DRAW_TREE_COMMANDS[i].node->graphicNode=new NODE_GRAPHIC(NULL,NODE_GRAPHIC::center,G_DRAW_TREE_COMMANDS[i].node);
createFirstNode(G_DRAW_TREE_COMMANDS[i].node->graphicNode);
break;

case 2:
splitNode(G_DRAW_TREE_COMMANDS[i].node->graphicNode);
break;

case 3:
setTerminalNode(G_DRAW_TREE_COMMANDS[i].node->graphicNode);
break;

case 4:
removeFromScene(G_DRAW_TREE_COMMANDS[i].node->graphicNode);
break;
}

}



//__________________AQUÍ SE LIMPIA LA VARIABLE GLOBAL___________________
G_DRAW_TREE_COMMANDS.clear();
//______________________________________________________________________

}






