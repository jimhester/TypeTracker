#include "ttEdit.h"
#include <QKeyEvent>

void ttEdit::keyPressEvent(QKeyEvent *event){
	QTextEdit::keyPressEvent(event);
	event->ignore();
}
