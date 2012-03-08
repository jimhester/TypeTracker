#include <QString>
#include <QtSql>
#include <QHash>
#include <QMessageBox>
#include <QVariant>
#include <QStringList>
#include "InputEvent.h"

QRegExp InputEvent::m_regexp = QRegExp("(\\s*\\w+\\b){5}");

InputEvent::InputEvent(void)
{
	m_processed = false;
}
InputEvent::InputEvent(const QString & keys, int time)
	: m_keys(keys) ,
	m_processed(false)
{
	m_times.fill(time,keys.size());
	m_error.fill(false,keys.size());
	process();
}
InputEvent::InputEvent(const QString & keys, const QVector<int> & times, const QDateTime & date, const QVector<bool> & errors)
	: m_keys(keys) ,
	m_times(times) ,
	m_datetime(date) ,
	m_error(errors) ,
	m_processed(false)
{
	if(m_times.size() == 0)
		m_times.fill(0,keys.size());
	if(m_error.size() == 0)
		m_error.fill(false,keys.size());

	process();
}
InputEvent::InputEvent(const QString & keys, const QByteArray & times, const QDateTime & date)
	: m_keys(keys) ,
	m_datetime(date) ,
	m_processed(false)
{
	QByteArray a = times;
	{
		QDataStream s(&a, QIODevice::ReadOnly);
		s >> m_times;
	}
	m_datetime = date;
	if(m_times.size() == 0)
		m_times.fill(0,keys.size());
	if(m_error.size() == 0)
		m_error.fill(false,keys.size());

	process();
}
InputEvent::~InputEvent(void)
{
}

void InputEvent::process() const
{
	if(!m_processed){
		m_statCache.clear();
		m_statCache.resize(5);
		m_final.clear();
		m_finalErrors.clear();
		m_finalTimes.clear();
		int finalItr = 0;
		m_final.resize(m_keys.size());
		m_finalErrors.fill(false,m_keys.size());
		m_finalTimes.fill(0,m_keys.size());
		for(int i = 0; i < m_keys.size();i++){
			m_finalTimes[finalItr]+=m_times[i];
			if(m_keys.at(i) == 0x08){ //0x08 is backspace
				finalItr = (finalItr-1) % (finalItr + 1); //decrement until 0
				m_finalErrors[finalItr]=true;
			} else {
				if(m_error.at(i)){
					m_finalErrors[finalItr]=true;
				}
				m_final[finalItr] = m_keys.at(i);
				finalItr++;
			}
		}
		m_final = m_final.left(finalItr);
		m_finalTimes = m_finalTimes.mid(0,finalItr);
		m_finalErrors = m_finalErrors.mid(0,finalItr);
	}
	m_processed = true;
}

void InputEvent::clear()
{
	m_keys.clear();
	m_times.clear();
	m_final.clear();
	m_processed = false;
}

bool InputEvent::isValid() const
{
	process();
	if(m_regexp.isValid()){
		return(m_regexp.indexIn(m_final) != -1);
	}
	return false;
}
bool InputEvent::isEmpty() const
{
	return(m_keys.isEmpty());
}
void InputEvent::addKey(const QString & key, int msec,bool isCorrect)
{
//	qDebug() << (int)key.toAscii().at(0);
	m_keys += key;
	m_times += msec;
	m_error += !isCorrect;
}

const CountHash& InputEvent::substr(int size,bool allowSpace) const
{
	if(isValid() && !m_substr.contains(size)){
		CountHash ret;
		int loc = 0;
		while(loc < m_final.length()){
			if(loc+size < m_final.length()){
				bool error = false;
				bool space = false;
				int time=0;
				for(int i = loc;i<loc+size;i++){
					if(m_final[i].isSpace()){
						space = true;
					}
					if(m_finalErrors[i] == true){
						error = true;
					}
					time+=m_finalTimes.at(i);
				}
				if(allowSpace || !space){
					QString sub=m_final.mid(loc,size).toLower();
					if(error){
						ret[sub].error++;
					}
					ret[sub].totalTime+=time;
					ret[sub].number++;
				}
			}
			loc++;
		}
		m_substr[size]=ret;
	}
	return(m_substr[size]);
}
const CountHash& InputEvent::words() const
{
	if(isValid() && m_words.isEmpty()){
		int start = 0;
		while(start < m_final.length()-1){
			while(!m_final.at(start).isLetterOrNumber() && start < m_final.length()-1){
				start++;
			}
			int end = start+1;
			while(end < m_final.length() && m_final.at(end).isLetterOrNumber()){
				end++;
			}
			QString word = m_final.mid(start,end-start).toLower();
			bool error = false;
			int time=0;
			for(int i = start;i<end;i++){
				if(m_finalErrors[i] == true){
					error = true;
				}
				time+=m_times.at(i);
			}
			if(error){
				m_words[word].error++;
			}
			m_words[word].totalTime+=time;
			m_words[word].number++;
			start=end;
		}
	}
	return(m_words);
}

QDateTime InputEvent::date() const
{
	return QDateTime(m_datetime);
}
QString InputEvent::keys() const
{
	return m_keys;
}
QVector<int> InputEvent::times() const
{
	return m_times;
}
void InputEvent::setDate(const QDateTime &datetime)
{
	m_datetime = datetime;
}
QString InputEvent::str() const
{
	return QString(m_final);
}
int InputEvent::totalTime() const
{
	if(m_processed){
		if(m_statCache[TotalTime].isNull()){
			int sum=0;
			foreach(int time,m_finalTimes){
				sum+=time;
			}
			m_statCache[TotalTime] = sum;
		}
		return m_statCache[TotalTime].toInt();
	}
	return 0;
}
double InputEvent::trueWordsPerMinute() const
{
	if(!m_processed)
		return 0;
	if(m_statCache[TrueWordsPerMinute].isNull()){
		int time = totalTime();
		int wordsCnt =0;
		words();
		foreach(count cnt, m_words){
			wordsCnt += cnt.number;
		}
		m_statCache[TrueWordsPerMinute] = double(wordsCnt)/double(time)*60000;
	}
	return m_statCache[TrueWordsPerMinute].toDouble();
}
double InputEvent::normalizedWordsPerMinute(int size) const
{
	if(!m_processed)
		return 0;
	if(m_statCache[NormalizedWordsPerMinute].isNull()){
		int time = totalTime();
		m_statCache[NormalizedWordsPerMinute] = (double(m_final.length())/size)/double(time)*60000;
	}
	return m_statCache[NormalizedWordsPerMinute].toDouble();
}
double InputEvent::charactersPerMinute() const
{
	if(!m_processed)
		return 0;
	if(m_statCache[CharactersPerMinute].isNull()){
		int time = totalTime();
		m_statCache[CharactersPerMinute] = double(m_final.length())/double(time)*60000;
	}
	return m_statCache[CharactersPerMinute].toDouble();
}
double InputEvent::percentCorrect() const
{
	if(!m_processed)
		return 0;
	if(m_statCache[PercentCorrect].isNull()){
		int numErrors = 0;
		foreach(bool error,m_finalErrors){
			if(error)
				numErrors++;
		}
		m_statCache[PercentCorrect] = 100-(double(numErrors)*100/m_final.length());
	}
	return m_statCache[PercentCorrect].toDouble();
}
void InputEvent::setValidator(const QRegExp &regexp)
{
	m_regexp = regexp;
	m_regexp.setMinimal(true);
}
QRegExp InputEvent::validator()
{
	return m_regexp;
}
bool InputEvent::operator==(const InputEvent& event)
{
	return(m_datetime == event.m_datetime && m_keys == event.m_keys && m_times == event.m_times);
}
////////////////////////////////////////////////////////////////////////////////
// InputEventManager
////////////////////////////////////////////////////////////////////////////////

InputEventManager::InputEventManager(QObject *parent)
	: QObject(parent)
	, m_InputEventModel(0)
{
    load();

    m_InputEventModel = new InputEventModel(this, this);
}

InputEventManager::~InputEventManager()
{
	save();
}

QList<InputEvent> InputEventManager::InputEvents() const
{
    return m_InputEvents;
}

void InputEventManager::setInputEvents(const QList<InputEvent> &InputEvents)
{
	m_InputEvents.clear();
	m_similarity.clear();
    m_InputEvents = InputEvents;
	for(int i = 0;i<m_InputEvents.size();i++){
		m_similarity[hashInputEvent(m_InputEvents.at(i))] << i;
	}
	emit InputEventReset();
}

InputEventModel *InputEventManager::inputEventModel() const
{
    return m_InputEventModel;
}

//InputEventTreeModel *InputEventManager::inputEventTreeModel() const
//{
//    return m_InputEventTreeModel;
//}

void InputEventManager::addInputEvent(const InputEvent &item)
{
	m_similarity[hashInputEvent(item)] << m_InputEvents.size();
    m_InputEvents.append(item);
    emit entryAdded(item);
}
void InputEventManager::insertInputEvent(const InputEvent &item)
{
	QSqlQuery query;
	query.prepare("INSERT INTO events (eventTime, keys,times) VALUES (?,?,?)");
	query.addBindValue(item.date().toString(Qt::ISODate));
	query.addBindValue(item.keys());
	QByteArray a;
	{
		QDataStream s(&a,QIODevice::WriteOnly);
		s << item.times();
	}
	query.addBindValue(a);
	query.exec();
}

void InputEventManager::clear()
{
    m_InputEvents.clear();
    InputEventReset();
}

void InputEventManager::load()
{
//    loadSettings();

	QSqlDatabase* db = &QSqlDatabase::addDatabase("QSQLITE");
	db->setDatabaseName("test.db");
	if (!db->open()) {
         QMessageBox::critical(0, qApp->tr("Cannot open database"),
             qApp->tr("Unable to establish a database connection.\n"),QMessageBox::Cancel);
		 return;
	}
	QSqlQuery query;
	if(!query.exec("CREATE TABLE IF NOT EXISTS events (eventTime datetime NOT NULL DEFAULT CURRENT_TIMESTAMP, keys varchar, times varbinary)")){
		QMessageBox::critical(0, qApp->tr("Failed"),
             qApp->tr("Error %1 when performing %2.\n").arg(query.lastError().text()).arg(query.lastQuery()),QMessageBox::Cancel);
	//	return;
	}

    QList<InputEvent> list;

	if(!query.exec("SELECT * FROM events")){
		QMessageBox::critical(0, qApp->tr("Failed"),
             qApp->tr("Error %1 when performing %2.\n").arg(query.lastError().text()).arg(query.lastQuery()),QMessageBox::Cancel);
		return;
	}
	while(query.next()){
		InputEvent evnt = InputEvent(query.value(1).toString(),query.value(2).toByteArray(),query.value(0).toDateTime());
		list << evnt;
	}
    setInputEvents(list);
}

void InputEventManager::save()
{
	QSqlQuery query("DROP TABLE IF EXISTS events");
	query.exec("CREATE TABLE events (eventTime datetime NOT NULL DEFAULT CURRENT_TIMESTAMP, keys varchar, times varbinary)");
	foreach(const InputEvent &item,m_InputEvents){
		insertInputEvent(item);
	}
}
QList<int> InputEventManager::similarEvents(const InputEvent &evnt)
{
	return m_similarity.value(hashInputEvent(evnt));
}
QString InputEventManager::hashInputEvent(const InputEvent &item)
{
	QStringList words = item.words().keys();
	qSort(words);
	return(words.join(""));
}
InputEventManager* InputEventManager::m_inputEventManager = 0;
InputEventManager* InputEventManager::instance()
{
	if(!m_inputEventManager)
		m_inputEventManager = new InputEventManager();
	return m_inputEventManager;
}

////////////////////////////////////////////////////////////////////////////////
//End InputEventManager
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//Begin InputEventModel
////////////////////////////////////////////////////////////////////////////////
InputEventModel::InputEventModel(InputEventManager *InputEvent, QObject *parent)
    : QAbstractTableModel(parent)
    , m_InputEvent(InputEvent)
{
    Q_ASSERT(m_InputEvent);
    connect(m_InputEvent, SIGNAL(InputEventReset()),
            this, SLOT(InputEventReset()));
    connect(m_InputEvent, SIGNAL(entryRemoved(InputEvent)),
            this, SLOT(InputEventReset()));

    connect(m_InputEvent, SIGNAL(entryAdded(InputEvent)),
            this, SLOT(entryAdded()));
    connect(m_InputEvent, SIGNAL(entryUpdated(int)),
            this, SLOT(entryUpdated(int)));
}

void InputEventModel::InputEventReset()
{
    reset();
}

void InputEventModel::entryAdded()
{
    beginInsertRows(QModelIndex(), 0, 0);
    endInsertRows();
}

void InputEventModel::entryUpdated(int offset)
{
    QModelIndex idx = index(offset, 0);
    emit dataChanged(idx, idx);
}

QVariant InputEventModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal
        && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("Text");
            case 1: return tr("Time");
			case 2: return tr("TWPM");
			case 3: return tr("NWPM");
			case 4: return tr("CPM");
			case 5: return tr("Size");
			case 6: return tr("% Correct");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant InputEventModel::data(const QModelIndex &index, int role) const
{
    QList<InputEvent> lst = m_InputEvent->InputEvents();
    if (index.row() < 0 || index.row() >= lst.size())
        return QVariant();

    const InputEvent &item = lst[index.row()];
	if(role >= InputEventModel::SubstrRole)
		return QVariant::fromValue<CountHash>(item.substr(role - SubstrRole+1));
    switch (role) {
    case WordRole:
        return QVariant::fromValue<CountHash>(item.words());
	case ItemOffsetRole:
		return index.row();
    case Qt::DisplayRole:
    case Qt::EditRole: {
        switch (index.column()) {
            case 0:
				return item.str();
            case 1:
                return item.date();
			case 2:
				return item.trueWordsPerMinute();
			case 3:
				return item.normalizedWordsPerMinute();
			case 4:
				return item.charactersPerMinute();
			case 5:
				return item.str().length();
			case 6:
				return item.percentCorrect();
			}
		}
    }
    return QVariant();
}

int InputEventModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : 7;
}

int InputEventModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : m_InputEvent->InputEvents().count();
}

bool InputEventModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid())
        return false;
    int lastRow = row + count - 1;
    beginRemoveRows(parent, row, lastRow);
    QList<InputEvent> lst = m_InputEvent->InputEvents();
    for (int i = lastRow; i >= row; --i)
        lst.removeAt(i);
    disconnect(m_InputEvent, SIGNAL(InputEventReset()), this, SLOT(InputEventReset()));
    m_InputEvent->setInputEvents(lst);
    connect(m_InputEvent, SIGNAL(InputEventReset()), this, SLOT(InputEventReset()));
    endRemoveRows();
    return true;
}
////////////////////////////////////////////////////////////////////////////////
//End InputEventModel
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//Begin InputEventTreeModel
////////////////////////////////////////////////////////////////////////////////
InputEventTreeModel::InputEventTreeModel(QAbstractItemModel *sourceModel, QObject *parent, int substrLength)
    : QAbstractProxyModel(parent)
	, m_substrLength(substrLength)
{
    setSourceModel(sourceModel);
}

QVariant InputEventTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch(section){
	case 0:
		return "Substr";
	case 1:
		return "#";
	case 2:
		return "Speed";
	case 3:
		return "%Correct";
	}
	return sourceModel()->headerData(section-4, orientation, role);
}

QVariant InputEventTreeModel::data(const QModelIndex &index, int role) const
{
    if ((role == Qt::EditRole || role == Qt::DisplayRole)) {
        int start = index.internalId();
        if (start == 0) { //Substrings
			QString substr = m_substr[index.row()];
			switch(index.column()){
			case 0:
				return substr;
			case 1:
				return m_substrSums[substr].number;
			case 2:
				return (substr.length() * double(m_substrSums[substr].number)/5)/m_substrSums[substr].totalTime*60000;
			case 3:
				return 100-(m_substrSums[substr].error/double(m_substrSums[substr].number))*100;
            }
			return QVariant();
        }
    }
    return QAbstractProxyModel::data(index.sibling(index.row(),index.column()-4),role);
}

int InputEventTreeModel::columnCount(const QModelIndex &parent) const
{
	return sourceModel()->columnCount(mapToSource(parent)) + 4;
}

int InputEventTreeModel::rowCount(const QModelIndex &parent) const
{
    if ( parent.internalId() != 0
        || parent.column() > 0
        || !sourceModel())
        return 0;

    // row count OF substr
    if (!parent.isValid()) {
        if (!m_substr.isEmpty()){
			return m_substr.length();
		}
        int totalRows = sourceModel()->rowCount();
		QHash<QString,QList<int> > map;
        for (int i = 0; i < totalRows; ++i) {
            CountHash substrings = sourceModel()->index(i, 0).data(InputEventModel::SubstrRole + m_substrLength).value<CountHash>();
			foreach(QString substring, substrings.keys()){
				map[substring] << i;
//dd				qDebug() << substrings.value(substring).error;				
				m_substrSums[substring].error += substrings.value(substring).error;
				m_substrSums[substring].number += substrings.value(substring).number;
				m_substrSums[substring].totalTime += substrings.value(substring).totalTime;
			}
        }
		m_substr = map.keys();
		for (int i=0; i < m_substr.size();i++){
			m_sourceRowMap << map[m_substr[i]];
		}
		return m_substr.length();
    }
	if (m_sourceRowMap.isEmpty())
        rowCount(QModelIndex());
    // row count FOR a substr
	return (m_sourceRowMap[parent.row()].length());
}

QModelIndex InputEventTreeModel::mapToSource(const QModelIndex &proxyIndex) const
{
    int offset = proxyIndex.internalId();
    if (offset == 0)
        return QModelIndex();
    return sourceModel()->index(m_sourceRowMap[offset-1][proxyIndex.row()], proxyIndex.column());
}

QModelIndex InputEventTreeModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid())
        return QModelIndex();

    if (m_sourceRowMap.isEmpty())
        rowCount(QModelIndex());
	//QHash<QString, QVariant> substrings = sourceIndex.data(InputEventModel::SubstrRole + m_substrLength).toHash();
	CountHash substrings = sourceIndex.data(InputEventModel::SubstrRole + m_substrLength).value<CountHash>();
	int substrIdx = m_substr.indexOf(substrings.keys().at(0));
	int row = m_sourceRowMap[substrIdx].indexOf(sourceIndex.row());
	return createIndex(row, sourceIndex.column(), substrIdx+1);
}

QModelIndex InputEventTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0
        || column < 0 || column >= columnCount(parent)
        || parent.column() > 0)
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, 0);
    return createIndex(row, column, parent.row() + 1);
}

QModelIndex InputEventTreeModel::parent(const QModelIndex &index) const
{
    int offset = index.internalId();
    if (offset == 0 || !index.isValid())
        return QModelIndex();
    return createIndex(offset - 1, 0, 0);
}

bool InputEventTreeModel::hasChildren(const QModelIndex &parent) const
{
    QModelIndex grandparent = parent.parent();
    if (!grandparent.isValid())
        return true;
    return false;
}

Qt::ItemFlags InputEventTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
}

bool InputEventTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
	return(QAbstractProxyModel::removeRows(row,count,parent));
}

void InputEventTreeModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    if (sourceModel()) {
        disconnect(sourceModel(), SIGNAL(modelReset()), this, SLOT(sourceReset()));
        disconnect(sourceModel(), SIGNAL(layoutChanged()), this, SLOT(sourceReset()));
        disconnect(sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        disconnect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    }

    QAbstractProxyModel::setSourceModel(newSourceModel);

    if (newSourceModel) {
        connect(sourceModel(), SIGNAL(modelReset()), this, SLOT(sourceReset()));
        connect(sourceModel(), SIGNAL(layoutChanged()), this, SLOT(sourceReset()));
        connect(sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        connect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    }

    reset();
}

void InputEventTreeModel::sourceReset()
{
    m_sourceRowMap.clear();
	m_substr.clear();
	m_substrSums.clear();
    reset();
}

void InputEventTreeModel::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent); // Avoid warnings when compiling release
	Q_UNUSED(start);
	Q_UNUSED(end);
    Q_ASSERT(!parent.isValid());
	//This is a hack, but it is not worth dealing with, just reload the data
	sourceReset();
}

void InputEventTreeModel::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent); // Avoid warnings when compiling release
	Q_UNUSED(start);
	Q_UNUSED(end);
    Q_ASSERT(!parent.isValid());
	//This is a hack, but it is not worth dealing with, just reload the data
	sourceReset();
	return;
}

void InputEventTreeModel::setSubstrLength(int substrLength)
{
	m_substrLength = substrLength;
	sourceReset();
}
void InputEventTreeModel::setSubstrLength(double substrLength)
{
	m_substrLength = substrLength;
	sourceReset();
}

InputEventFilterModel::InputEventFilterModel(QAbstractItemModel *sourceModel,QList<int> rows,QObject *parent)
	: QSortFilterProxyModel(parent) ,
	m_rows(rows)
{
	setSourceModel(sourceModel);
}
bool InputEventFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex & source_parent) const
{
	return m_rows.contains(sourceRow);
}
bool InputEventFilterModel::removeRows ( int row, int count, const QModelIndex & parent)
{
	QMutableListIterator<int> i(m_rows);
	while(i.hasNext()) {
		if(i.next() >= row && i.next() <= row + (count - 1)){
			i.remove();
		} else if(i.next() > row) {
			i.next()-=count;
		}
	}
	return QSortFilterProxyModel::removeRows(row,count,parent);
}