#ifndef DRAW_TREE_H
#define DRAW_TREE_H




#include <QLabel>
#include <QScrollArea>
#include <QGraphicsLineItem>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMainWindow>
#include <QSet>


#include <QThread>

class NODE; //Este es el nodo no gráfico declarado en training.h
class NODE_GRAPHIC;


//____________________ESTA CLASE SE USARA PARA MOSTRAR LA INFORMACIÓN COMPLETA DE CADA NODO______________________
class INFORMATION:public QGraphicsTextItem
{
NODE_GRAPHIC *node;
QRectF AreaParent;//Esta variable almacenara el área cuadrada visible del padre
QPointF mouseEnter; //Esta variable almacenara la coordenada de entrada del ratón
QColor backgroundColor;
QColor outlineColor;
QColor textColor;
QColor titleColor;
QStringList info;//Almacenara la información que hay que mostrar por cada nodo
public:
INFORMATION();
void setNode(NODE_GRAPHIC *node);
void show(QGraphicsSceneHoverEvent * event=0);
void push_back(QString str);
QPainterPath shape() const;
void paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget);
friend class NODE_GRAPHIC;
};
//________________________________________________________________________________________________________________







class Link : public QGraphicsItem
{
NODE_GRAPHIC *node;
public:
Link(NODE_GRAPHIC *toNode);
bool collisionState;
void setColor(const QColor &color=Qt::black,const int widthLine=2);
QRectF outlineRect() const;
void trackNodes();
QPainterPath shape() const;
QRectF boundingRect() const;
void paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget);
private:
QPen pen;
};




//Declaramos la clase NODE_GRAPHIC
class NODE_GRAPHIC:public QGraphicsItem
{

public:
enum NodeType{NodeAlone,NodeSplit,NodeTerminal};
enum Side{left,center,right};
NODE_GRAPHIC(NODE_GRAPHIC *parent=NULL,Side side=center,NODE *thisNode=NULL);
void setText(const QString &text);
void setTextColor(const QColor &color);
void setOutlineColor(const QColor &color);
void setBackgroundColor(const QColor &color);
void setType(const NodeType tp);
void setSide(Side side=center);
void setDataNode(NODE *thisNode);
QGraphicsScene *myScene();
void splitNode();
void extendBranch();
void correctTree();
void waringCollisionTotal();
NODE_GRAPHIC *correctTreeRight(NODE_GRAPHIC *nodeTemp);
NODE_GRAPHIC *correctTreeLeft(NODE_GRAPHIC *nodeTemp);

QRectF boundingRect() const;
QPainterPath shape() const;
void paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget);

bool collisionState;
QList<NODE_GRAPHIC *> *items;
QList<Link *> *links;
NODE_GRAPHIC *nodeParent;
NODE_GRAPHIC *nodeLeft;
NODE_GRAPHIC *nodeRight;
Link *branchLeft;
Link *branchRight;
Side mySide;
NODE *dataNode;
NodeType type;
INFORMATION myInfo;

QRectF outlineRect() const;
private:
QFont font;
QString myText;
QColor myTextColor;
QColor myBackgroundColor;
QColor myOutlineColor;
static const int separation_x=50;
static const int separation_y=70;


protected:
void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
void mousePressEvent(QGraphicsSceneMouseEvent * event);
void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
QVariant itemChange(GraphicsItemChange change,const QVariant &value);
void focusOutEvent(QFocusEvent * event);
void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
};


class TREE_ITEM:public QGraphicsRectItem
{

NODE_GRAPHIC *nodeRoot;


QColor myTextColor;
QColor myBackgroundColor;
QColor myOutlineColor;
QString myText;

public:
TREE_ITEM();

/*
void setText(const QString &text);
void setTextColor(const QColor &color);
void setOutlineColor(const QColor &color);
void setBackgroundColor(const QColor &color);

QRectF boundingRect() const;

QPainterPath shape() const;
*/
//void paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
//void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);

};



//Declaramos primero la hoja donde vamos a ver todas las escenas
class SHEET:public QGraphicsView
{
Q_OBJECT
public:
SHEET(QWidget *parent = 0);
protected:
void wheelEvent(QWheelEvent *event);
};

class SCENE:public QGraphicsScene
{


public:
SCENE(QObject * parent = 0);

};

//Declaramos la ventana principal donde va a ver nuestra hoja SHEET

class WINDOWS_PRINCIPAL_TREE: public QMainWindow
{
Q_OBJECT
SHEET *view;
SCENE *scene;
SCENE *scene2;
public:
WINDOWS_PRINCIPAL_TREE();
NODE_GRAPHIC *selectedNode() const;
//Miembros
NODE_GRAPHIC *myNode;
int widthScene;
int heightScene;
public slots:

//______FUNCIONES DE PRUEBA_______________


void addItem(QGraphicsItem *item);
//________________________________________

void createFirstNode(NODE_GRAPHIC *node);
void splitNode(NODE_GRAPHIC *node);
void removeFromScene(NODE_GRAPHIC *nodeRoot);
void setTerminalNode(NODE_GRAPHIC *node);
void seeGraphicNode(NODE_GRAPHIC *node);
void readCommands();
protected:
void keyPressEvent(QKeyEvent * event );
};



#endif
