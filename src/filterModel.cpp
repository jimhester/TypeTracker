#include "FilterModel.h"
#include <QxtSpanSlider>

FilterModelWidget::FilterModelWidget(QAbstractItemModel* sourceModel, QWidget* parent)
  : QWidget(parent)
{
  m_filterModel = new FilterModel(sourceModel,this);

  for(int i =0;i<sourceModel->columnCount();i++){
    m_colnames.append(sourceModel->headerData(i,Qt::Horizontal).toString());
  }
  m_combo = new QComboBox(this);
  m_combo->insertItems(0,m_colnames);

  QPushButton* button = new QPushButton("Add &Filter");
  connect(button, SIGNAL(pressed()), this, SLOT(newFilter()));

  QHBoxLayout* layout = new QHBoxLayout();
  layout->addWidget(m_combo);
  layout->addWidget(button);
  layout->addStretch();
  layout->setMargin(0);

  QVBoxLayout* main = new QVBoxLayout();
  main->setDirection(QBoxLayout::BottomToTop);
  main->addLayout(layout);
  main->setMargin(0);
  setLayout(main);
}
void FilterModelWidget::newFilter()
{
  if(m_combo->currentIndex() >=0){ //< size?
    QAbstractItemModel* source = m_filterModel->sourceModel();
    QList<qreal> numbers;
    numbers.reserve(source->rowCount());
    qreal min,max;
    min=10e6;
    max=-1;
    for(int i =0;i<source->rowCount();i++){
      qreal number = source->data(source->index(i,m_combo->currentIndex())).toReal();
      if(min > number) min = number;
      if(max < number) max = number;
      numbers.append(number);
    }
    QLabel* colName = new QLabel(m_colnames.at(m_combo->currentIndex()));
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setMargin(0);
    if(min != max && !m_filtSpanSliders.contains(m_combo->currentIndex())){
      hlayout->addWidget(colName);
      SpanSlider* slider= new SpanSlider(Qt::Horizontal);
      slider->setHandleMovementMode(QxtSpanSlider::NoOverlapping);
      QDoubleSpinBox* minSpin = new QDoubleSpinBox();
      connect(slider,SIGNAL(lowerPositionChanged(double)),minSpin,SLOT(setValue(double)));   
      connect(minSpin,SIGNAL(valueChanged(double)),slider,SLOT(setLowerPosition(double)));

      QDoubleSpinBox* maxSpin = new QDoubleSpinBox(); 
      connect(slider,SIGNAL(upperPositionChanged(double)),maxSpin,SLOT(setValue(double)));
      connect(maxSpin,SIGNAL(valueChanged(double)),slider,SLOT(setUpperPosition(double)));
      
      connect(slider,SIGNAL(upperPositionChanged(double)),this,SLOT(rangeChanged()));
      connect(slider,SIGNAL(lowerPositionChanged(double)),this,SLOT(rangeChanged()));

      maxSpin->setRange(min,max);
      minSpin->setRange(min,max);

      slider->setMinimum(min);
      slider->setMaximum(max);
      slider->setLowerPosition(min);
      slider->setUpperPosition(max);

      hlayout->addWidget(minSpin);
      hlayout->addWidget(slider);
      hlayout->addWidget(maxSpin);

      m_filtSpanSliders[m_combo->currentIndex()]=slider;
    }
    static_cast<QVBoxLayout*>(layout())->addLayout(hlayout);
  }
}
void FilterModelWidget::rangeChanged()
{
  if(SpanSlider* slider = qobject_cast<SpanSlider*>(QObject::sender())){
    int col= m_filtSpanSliders.key(slider,-1);
    if(col != -1){
      range r;
      r.low = slider->lowerValue();
      r.high = slider->upperValue();
      m_filterModel->setRange(col,r);
    }
  }
}
QAbstractItemModel* FilterModelWidget::model() const
{
  return m_filterModel;
}

FilterModel::FilterModel(QAbstractItemModel* sourceModel, QWidget* parent)
  : QSortFilterProxyModel(parent)
{
  setSourceModel(sourceModel);
}
bool FilterModel::rowAccepted( int source_row, const QModelIndex & source_parent ) const
{
  bool keep=true;
  foreach(int col,m_ranges.keys()){
    QModelIndex index = sourceModel()->index(source_row,col,source_parent);
//    if(!sourceModel()->data(index).isNull()){
      qreal count = sourceModel()->data(index).toReal();
      range r = m_ranges.value(col);
      keep = count >= r.low && count <= r.high;
  //  }
    if(!keep) break;
  }
  return(keep);
}
bool FilterModel::hasAcceptedChildren(int source_row,const QModelIndex & source_parent ) const
{
  QModelIndex item = sourceModel()->index(source_row,0,source_parent);
    if (!item.isValid()) {
        //qDebug() << "item invalid" << source_parent << source_row;
        return false;
    }
     //check if there are children
    int childCount = item.model()->rowCount(item);
    if (childCount == 0)
        return false;
 
    for (int i = 0; i < childCount; ++i) {
        if (rowAccepted(i, item))
            return true;
        //recursive call -> NOTICE that this is depth-first searching, you're probably better off with breadth first search...
        if (hasAcceptedChildren(i, item))
            return true;
    }
     return false;
}
bool FilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  //return(rowAccepted(source_row,source_parent));
    if (rowAccepted(source_row, source_parent))
        return true;
     //accept if any of the parents is accepted on it's own merits
    QModelIndex parent = source_parent;
    while (parent.isValid()) {
        if (rowAccepted(parent.row(), parent.parent()))
            return true;
        parent = parent.parent();
    }
     //accept if any of the children is accepted on it's own merits
    if (hasAcceptedChildren(source_row, source_parent)) {
        return true;
    }
    return false;
}
 
void FilterModel::setRange(int col,range r)
{
  m_ranges[col] = r;
//  qDebug() << m_ranges.keys() << r.low << r.high;
  emit filterChanged();
}
QMap<int,range> FilterModel::ranges() const
{
  return m_ranges;
}

// SpanSlider
SpanSlider::SpanSlider(QWidget* parent) : QxtSpanSlider(parent)
  , m_minimum(0.0)
  , m_maximum(0.0)
{
  connect(this,SIGNAL(lowerPositionChanged(int)),this,SLOT(emitDoubleLowerPositionChanged(int)));
  connect(this,SIGNAL(upperPositionChanged(int)),this,SLOT(emitDoubleUpperPositionChanged(int)));
}
SpanSlider::SpanSlider(Qt::Orientation orientation, QWidget* parent) : QxtSpanSlider(orientation,parent)
  , m_minimum(0.0)
  , m_maximum(0.0)
{
  connect(this,SIGNAL(lowerPositionChanged(int)),this,SLOT(emitDoubleLowerPositionChanged(int)));
  connect(this,SIGNAL(upperPositionChanged(int)),this,SLOT(emitDoubleUpperPositionChanged(int)));
}
void SpanSlider::setMinimum(double value)
{
  m_minimum=value;
  emit lowerPositionChanged(value);
//  QxtSpanSlider::setMinimum(static_cast<int>(value));
}
void SpanSlider::setMaximum(double value)
{
  m_maximum=value;
  emit upperPositionChanged(value);
//  QxtSpanSlider::setMaximum(static_cast<int>(value));
}
void SpanSlider::setLowerPosition(double lower)
{
//  lower = qBound(m_minimum,lower,m_maximum);
  disconnect(this,SIGNAL(lowerPositionChanged(int)),this,SLOT(emitDoubleLowerPositionChanged(int)));
  QxtSpanSlider::setLowerPosition(static_cast<int>((lower - m_minimum)*99.0/(m_maximum-m_minimum)));
  emit lowerPositionChanged(lower);
  connect(this,SIGNAL(lowerPositionChanged(int)),this,SLOT(emitDoubleLowerPositionChanged(int)));
}
void SpanSlider::setUpperPosition(double upper)
{
//  upper = qBound(m_minimum,upper,m_maximum);
  disconnect(this,SIGNAL(upperPositionChanged(int)),this,SLOT(emitDoubleUpperPositionChanged(int)));
  QxtSpanSlider::setUpperPosition(static_cast<int>((upper - m_minimum)*99.0/(m_maximum-m_minimum)));
  emit upperPositionChanged(upper);
  connect(this,SIGNAL(upperPositionChanged(int)),this,SLOT(emitDoubleUpperPositionChanged(int)));
}
void SpanSlider::emitDoubleLowerPositionChanged(int value)
{
  double val = m_minimum+((static_cast<double>(value)/99.0)*(m_maximum-m_minimum));
  emit lowerPositionChanged(val);
}
void SpanSlider::emitDoubleUpperPositionChanged(int value)
{
  double val = m_minimum+((static_cast<double>(value)/99.0)*(m_maximum-m_minimum));
  emit upperPositionChanged(val);
}
double SpanSlider::lowerValue() const
{
  double val = m_minimum+((static_cast<double>(QxtSpanSlider::lowerPosition())/99.0)*(m_maximum-m_minimum));
  //qDebug() << QxtSpanSlider::lowerPosition() << QxtSpanSlider::upperPosition() << val;
  return val;
}
double SpanSlider::upperValue() const
{
  double val = m_minimum+((static_cast<double>(QxtSpanSlider::upperPosition())/99.0)*(m_maximum-m_minimum));
  //qDebug() << QxtSpanSlider::lowerPosition() << QxtSpanSlider::upperPosition() << val;
  return val;
}