#ifndef TTEDIT_H
#define TTEDIT_H

#include <QTextEdit>
#include <qdebug>

class QPlainTextEdit;

class ttEdit : public QTextEdit
{
	Q_OBJECT
	
public:
	ttEdit() {};
protected:
	void keyPressEvent(QKeyEvent *event);
};


#endif