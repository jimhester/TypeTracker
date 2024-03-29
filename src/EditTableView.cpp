/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "EditTableView.h"
#include <QtGui/QKeyEvent>

EditTableView::EditTableView(QWidget *parent)
    : QTableView(parent)
{
}

void EditTableView::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Delete
        || event->key() == Qt::Key_Backspace)
        && model()) {
        removeSelected();
    } else {
        QAbstractItemView::keyPressEvent(event);
    }
}
void EditTableView::removeSelected()
{
	QItemSelection selection( selectionModel()->selection() );
	 
	QList<int> rows;
	foreach( const QModelIndex & index, selection.indexes() ) {
		rows.append( index.row() );
	}
	 
	qSort( rows );
	 
	int prev = -1;
	for( int i = rows.count() - 1; i >= 0; i -= 1 ) {
		int current = rows[i];
		if( current != prev ) {
			model()->removeRows( current, 1 );
			prev = current;
		}
	}
	QModelIndex idx = model()->index(rows.first(), 0, rootIndex());
	selectionModel()->setCurrentIndex(idx,QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}