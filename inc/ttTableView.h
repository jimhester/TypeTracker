#pragma once
#include <QTableView>
class ttTableView :
	public QTableView
{
	Q_OBJECT

public:
	ttTableView(QWidget * parent=0) :
	  QTableView(parent) {} ;
	~ttTableView(void);

	void mouseReleaseEvent ( QMouseEvent * e );
signals:
	void selectionRightClicked(const QModelIndexList& selection);
};

