#include "ttEdit.h"
#include <qkeyevent>

void ttEdit::keyPressEvent(QKeyEvent *event){
	QTextEdit::keyPressEvent(event);
	event->ignore();
}