#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QDockWidget>
#include "InputEvent.h"
#include "ui_ghostDock.h"

class QTextCursor;
class QTime;
class InputEventGhost;
class InputLesson :
	public QWidget
{
	Q_OBJECT

public slots:
	void changeCursorPosition();
	void timeout();
public:
	InputLesson(const InputEvent &event, QWidget* parent = 0);
	void setManager(InputEventManager* manager);
protected:
//	void keyPressEvent(QKeyEvent *event);
//	void mouseReleaseEvent( QMouseEvent * e);

	~InputLesson(void);

private:
	void setSelectedTextColor(QColor color, QTextCursor* cursor);
	void InputLesson::addGhost();

	QTime* m_time;
	QTimer* m_timer;
	
	int m_timeout;
	int m_location;
	QTextCursor* m_cursor;
	InputEvent m_baseEvent;
	InputEvent m_inputEvent;
	QTextEdit* m_lesson;
	QTextEdit* m_input;

	InputEventGhost* m_ghost;
	InputEventManager* m_manager;
};

class GhostDock;
class InputEventGhost : public QTextEdit 
{

	Q_OBJECT

public:
	InputEventGhost(const InputEvent &event, QWidget* parent = 0);
	~InputEventGhost();

	void start();
	void pause();
	void stop();
	void rewind(int msec);

	bool isActive() const;
	bool isComplete() const;

	void setEvent(const InputEvent& event);
	const InputEvent* event() const;

private slots:
	void nextKey();

private:
	void setupDock();

	InputEvent m_event;
	QTextCursor* m_cursor;
	QTimer* m_timer;
	QVector<int> m_times;
	QString m_text;
	int m_location;
	bool m_running;
	GhostDock* m_dock;
};
class GhostDock : public QDockWidget , private Ui::GhostDock
{
	Q_OBJECT

friend InputEventGhost;

public slots:
	void populateComboBox(bool enabled);

public:
	GhostDock(InputEventGhost* ghost, QWidget* parent = 0);

private:

	InputEventGhost* m_ghost;
};