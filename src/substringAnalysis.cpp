#include "substringAnalysis.h"
#include "ttTreeView.h"

SubstringAnalysis::SubstringAnalysis(QAbstractItemModel* model, QWidget* parent) : QWidget(parent)
  , m_view(0)
  , m_filter(0)
  , m_tree(0)
  , m_substr(0)
{
  for(int i = 0;i< model->rowCount();i++){
    m_rows << i;
  }
  m_filter = new InputEventFilterModel(model,m_rows,this);
  m_substr = new SubstringModel(m_filter,this);
  m_tree = new InputEventTreeModel(m_substr,this);
  QSortFilterProxyModel *sort = new QSortFilterProxyModel(this);
  sort->setSourceModel(m_tree);
  ttTreeView* treeView = new ttTreeView(this);
  treeView->setSortingEnabled(true);
  treeView->setModel(sort);

  QVBoxLayout* controlLayout = new QVBoxLayout;

  QCheckBox* check = new QCheckBox("Allow &Spaces",this);
  connect(check, SIGNAL(toggled(bool) ), m_substr, SLOT(setAllowSpaces(bool)));
  controlLayout->addWidget(check);

  QSlider* slider = new QSlider(Qt::Vertical,this);
  slider->setMaximum(10);
  slider->setMinimum(-1);
  slider->setSliderPosition(-1);
  slider->setTickPosition(QSlider::TicksLeft);
  slider->setTracking(false);
  controlLayout->addWidget(slider);
  controlLayout->addStretch();

  connect(slider,SIGNAL(valueChanged(int)),m_substr,SLOT(setSubstringLength(int)));

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(treeView);
  layout->addLayout(controlLayout);
  setLayout(layout);
}

void SubstringAnalysis::lengthChanged(int value)
{
  m_substr->setSubstringLength(value);
}
void SubstringAnalysis::spacesChanged(bool value)
{
  m_substr->setAllowSpaces(value);
}
SubstringAnalysis::~SubstringAnalysis(void)
{
}

SubstringModel::SubstringModel(QAbstractItemModel* sourceModel,QObject* parent) : QIdentityProxyModel(parent)
  , m_allowSpaces(false)
  , m_substrLength(-1)
{
  setSourceModel(sourceModel);
}
void SubstringModel::setAllowSpaces(bool value)
{
  m_allowSpaces = value;
  emit layoutChanged();
}
void SubstringModel::setSubstringLength(int length)
{
  m_substrLength = length;
  emit layoutChanged();
}
QVariant SubstringModel::data(const QModelIndex &index, int role) const
{
  if(role == InputEventModel::AggregateRole){
    const QList<InputEvent>& lst = InputEventManager::instance()->InputEvents();
    int row = index.data(InputEventModel::ItemOffsetRole).toInt();
    if(row < 0 || row >= lst.size())
      return(QVariant());
    if(m_substrLength == -1)
      return QVariant::fromValue<CountHash>(lst.at(row).words(!m_allowSpaces));
    return QVariant::fromValue<CountHash>(lst.at(row).substr(m_substrLength+1,m_allowSpaces));
  }
  return QIdentityProxyModel::data(index,role);
}