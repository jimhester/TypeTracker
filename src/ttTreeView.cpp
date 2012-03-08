#include "ttTreeView.h"
#include <QDebug>

ttTreeView::ttTreeView(QWidget *parent)
    : QTreeView(parent)
{
}
void ttTreeView::setModel( QAbstractItemModel * model )
{
	connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex& ) ),
            this, SLOT(adaptColumns(const QModelIndex &, const QModelIndex&) ) );
	connect(this, SIGNAL(expanded(const QModelIndex &)), this, SLOT(adaptColumns(const QModelIndex &)));
	QTreeView::setModel(model);
	for(int i = 0;i<4;i++){
		resizeColumnToContents(i);
	}
}
void ttTreeView::adaptColumns (const QModelIndex & topleft, const QModelIndex& bottomRight)
{
    int firstColumn= topleft.column();
    int lastColumn = bottomRight.column();
    // Resize the column to the size of its contents
    do {
        resizeColumnToContents(firstColumn);
        firstColumn++;
    } while (firstColumn < lastColumn);
}
void ttTreeView::adaptColumns(const QModelIndex & index)
{
	for(int i = 5;i<model()->columnCount();i++){
		resizeColumnToContents(i);
	}
}