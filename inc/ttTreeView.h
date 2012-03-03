#ifndef TTTREEVIEW_H
#define TTTREEVIEW_H

#include <QTreeView>

class ttTreeView : public QTreeView
{
    Q_OBJECT

public:
    ttTreeView(QWidget *parent = 0);
	void setModel ( QAbstractItemModel * model );
public slots:
	void adaptColumns(const QModelIndex &topleft,const QModelIndex &bottomRight);
	void adaptColumns(const QModelIndex & index);

};

#endif // TTTREEVIEW_H

