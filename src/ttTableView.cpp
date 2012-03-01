#include "ttTableView.h"
#include <QMouseEvent>


void ttTableView::mouseReleaseEvent ( QMouseEvent * e )
{
	if(e->button() == Qt::RightButton){
		QModelIndexList sel = selectedIndexes();
		if(sel.size() > 0){
			emit(selectionRightClicked(sel));
		}
	}
	QTableView::mouseReleaseEvent(e);
}
ttTableView::~ttTableView(void)
{
}
