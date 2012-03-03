#pragma once

#include <QWidget>
#include "InputEvent.h"

class QTextCursor;
class QTextEdit;
class QTime;
class InputEventGhost;
class InputLesson :
	public QWidget
{
	Q_OBJECT

public slots:
	void changeCursorPosition();
public:
	InputLesson(const InputEvent &event, QWidget* parent = 0);
	void setManager(InputEventManager* manager);
protected:
	void keyPressEvent(QKeyEvent *event);
//	void mouseReleaseEvent( QMouseEvent * e);

	~InputLesson(void);

private:
	void setSelectedTextColor(QColor color, QTextCursor* cursor);
	void InputLesson::addGhost();

	QTime* m_time;
	int m_location;
	QTextCursor* m_cursor;
	InputEvent m_event;
	QTextEdit* m_lesson;
	QTextEdit* m_input;

	InputEventGhost* m_ghost;
	InputEventManager* m_manager;
};

class InputEventGhost : public QWidget
{

	Q_OBJECT

public:
	InputEventGhost(const InputEvent &event, QWidget* parent = 0);

	void start();
	void pause();
	void stop();

	bool isRunning() const;

private slots:
	void nextKey();

private:
	InputEvent m_event;
	QTextEdit* m_edit;
	QTextCursor* m_cursor;
	QTimer* m_timer;
	QVector<int> m_times;
	QString m_text;
	int m_location;
	bool m_running;
};