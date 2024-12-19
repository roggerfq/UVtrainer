#ifndef WIEVCASCADE_H
#define WIEVCASCADE_H


#include <QAbstractTableModel>
#include <QMainWindow>
#include <QMutex>


class QTreeView;
class QStandardItemModel;
class QStandardItem;
class QItemSelection;
class QMenu;
class QAction;


struct strongLearnItemModel;
class STRONG_LEARN;
class NODE_GRAPHIC;

class CASCADE_VIEW:public QMainWindow
{
Q_OBJECT

QTreeView *treeView; 
QStandardItemModel *standardModel;
QStandardItem  *rootNode;

QMutex mutex;


QMenu *contextMenuTree;
QAction *actionInformationTree;
QAction *actionSeeResponse;

QMenu *contextMenuStrongLearn;
QAction *actionInformationStrongLearn;
QAction *actionSeeResponseStrongLearn;

QMenu *contextMenuScene;
QAction *actionInformationCascade;
QAction *actionSeeResponseCascade; 
QAction *actionSeeRocCascade;
QAction *actionSeeRocCascadeAndThreshold;


QList <strongLearnItemModel *> strongLearnList;
QString nameBaseStrongLearn;
QString nameBaseTree;
int numberTree;

public:
CASCADE_VIEW(QWidget *parent);

public slots:
void selectionChangedSlot(const QItemSelection & /*newSelection*/, const QItemSelection & /*oldSelection*/);
void onCustomContextMenu(const QPoint &point);
void pushTree();
void pushStrongLearn(STRONG_LEARN *strongLearn);
void deleteCascade();

void seeResponse();
void seeResponseStrongLearn();
void seeResponseCascade();
void seeRocCascade(bool seeThreshold=false);
void seeRocCascadeAndThreshold();

void seeInformationTree();
void seeInformationStrongLearn();
void seeInformationCascade();

signals:
void seeTree(NODE_GRAPHIC *node);

};


#endif
