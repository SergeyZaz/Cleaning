#include <QSqlQuery>
#include <QSqlError>

#include "zreports.h"
#include "zsettings.h"
#include "zmessager.h"


ZReports::ZReports(QWidget* parent, Qt::WindowFlags flags) : QWidget(parent, flags)
{
	curFindId = -1;

	ui.setupUi(this);

	connect(ui.cboEstimate, SIGNAL(currentIndexChanged(int)), this, SLOT(changeEstimateSlot(int)));
	connect(ui.cboPeriod, SIGNAL(currentIndexChanged(int)), this, SLOT(changePeriodSlot(int)));
	connect(ui.cmdFind, SIGNAL(clicked()), this, SLOT(findNextSlot()));
	connect(ui.txtFind, SIGNAL(textChanged(const QString&)), this, SLOT(findFirstSlot(const QString&)));

	QSqlQuery query;
	if (!query.exec("SELECT value FROM config WHERE key = 'memo'"))
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), "Ошибка");
	}

	if (query.next())
		ui.txtMemo->setPlainText(query.value(0).toString());

	ui.txtMemo->setReadOnly(ZSettings::Instance().m_UserType == 1);
	
	
	ui.tbl->setModel(model.sortModel);
	model.sortModel->setDynamicSortFilter(true);
	model.sortModel->setFilterKeyColumn(1);
	ui.tbl->horizontalHeader()->setSortIndicator(1, Qt::DescendingOrder);
	ui.tbl->sortByColumn(1, Qt::DescendingOrder);
	ui.tbl->setColumnWidth(0, 50);
	ui.tbl->setColumnWidth(1, 200);
	ui.tbl->verticalHeader()->setDefaultSectionSize(20);
	ui.tbl->setItemDelegate(new ZReportsDelegate(ui.tbl));
}

ZReports::~ZReports()
{

}

void ZReports::changeEstimateSlot(int indx)
{
	buildReport();
}

void ZReports::changePeriodSlot(int indx)
{
	buildReport();
}

void ZReports::buildReport()
{
	QString stringQuery;
	QSqlQuery query;

	int periodId = ui.cboPeriod->itemData(ui.cboPeriod->currentIndex(), Qt::UserRole).toInt();
	int estimateId = ui.cboEstimate->itemData(ui.cboEstimate->currentIndex(), Qt::UserRole).toInt();

	stringQuery = QString("SELECT link_id, count, val FROM %1 WHERE estimate_id=%2").arg(periodId).arg(estimateId);
	if (!query.exec(stringQuery))
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
	}
	else
	{
		model.m_data.clear();

		while (query.next())
		{
		}
	}
}

void ZReports::findFirstSlot(const QString& text)
{
	curFindId = -1;
	findText(text);
}

void ZReports::findNextSlot()
{
	QString text = ui.txtFind->text();
	findText(text);
}

int ZReports::findText(const QString& text)
{
	ui.tbl->clearSelection();

	bool fExist = false;

	//...

	if (!fExist && curFindId != -1)
	{
		curFindId = -1;
		findText(text);
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ZReportsModel::ZReportsModel(QObject* parent)
{
	n_days << "" << QString("Пн") << QString("Вт") << QString("Ср") << QString("Чт") << QString("Пт") << QString("Сб") << QString("Вс");

	headers_before.append(QObject::tr("Орг."));
	headers_before.append(QObject::tr("ФИО"));
	headers_before.append(QObject::tr("Должность/Вид работ"));

	headers_after.append(QObject::tr("Часов"));
	headers_after.append(QObject::tr("Ставка"));
	headers_after.append(QObject::tr("ЗП"));
	headers_after.append(QObject::tr("Доплата"));
	headers_after.append(QObject::tr("Вычет"));
	headers_after.append(QObject::tr("Остаток"));
	headers_after.append(QObject::tr("Примечание"));

	sortModel = new ZSortFilterProxyModel;
	sortModel->setSourceModel(this);
}
	
ZReportsModel::~ZReportsModel() 
{
	delete sortModel;
}

QVariant ZReportsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section < headers_before.size())
			return headers_before.at(section);

		section -= headers_before.size();
		if (section < d_begin.daysTo(d_end))
		{
			QDate d = d_begin.addDays(section);
			return QString("%1\n%2").arg(d.day()).arg(n_days.at(d.dayOfWeek()));
		}

		section -= d_begin.daysTo(d_end);
		if (section < headers_after.size())
			return headers_after.at(section);
	}

	return QVariant();
}

Qt::ItemFlags ZReportsModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	if (index.column() == 1)
		flags |= Qt::ItemIsEditable;
	return flags;
}

QVariant ZReportsModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	int c = index.column();
	if (c >= columnCount(index))
		return QVariant();

	int r = index.row();
	if (r >= rowCount(index))
		return QVariant();
	
	const elem *pv = &m_data.at(r);

	if (c < headers_before.size())
	{
		switch (c)
		{
		case 0:
			break;
		case 1:
			if (role == Qt::UserRole)
				return pv->fio_id;
			if (role == Qt::DisplayRole)
				return pv->fio;
			break;
		case 2:
			if (role == Qt::UserRole)
				return pv->post_id;
			if (role == Qt::DisplayRole)
				return pv->post;
		default:
			break;
		}
		return QVariant();
	}
	
	c -= headers_before.size();
	if (c < d_begin.daysTo(d_end))
	{
		//int n = d_begin.daysTo(c);
		return QVariant();
	}

//	section -= d_begin.daysTo(d_end);
//	if (section < headers_after.size())
//		return headers_after.at(section);

	return QVariant();
}
	
void ZReportsModel::Update()
{
	beginResetModel();
	endResetModel();
}
	
bool ZReportsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid())
		return false;

	int r = index.row();
	if (r >= rows)
		return false;


	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////

ZReportsDelegate::ZReportsDelegate(QObject* parent)
	: QItemDelegate(parent)
{
}

QWidget* ZReportsDelegate::createEditor(QWidget* parent,
	const QStyleOptionViewItem& option,
	const QModelIndex& index) const
{
//	if (ZSettings::Instance().f_ReadOnly || ZSettings::Instance().m_UserType == 1)
//		return NULL;

//	int column = index.column();

//	if (column != 1)
//		return NULL;

	QComboBox* w = new QComboBox(parent);
	w->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	return w;
}

void ZReportsDelegate::setEditorData(QWidget* editor,
	const QModelIndex& index) const
{
	QComboBox* cbo = dynamic_cast<QComboBox*>(editor);
	if (cbo)
	{
		loadItemsToComboBox(cbo, "variants");
		int idx = index.model()->data(index, Qt::UserRole).toInt();
		idx = cbo->findData(idx);
		cbo->setCurrentIndex(idx);
		return;
	}
	QItemDelegate::setEditorData(editor, index);
}

void ZReportsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
	const QModelIndex& index) const
{
	QComboBox* cbo = dynamic_cast<QComboBox*>(editor);
	if (cbo)
	{
		QString txt = cbo->currentText();
		int indx = cbo->itemData(cbo->findText(txt), Qt::UserRole).toInt();

		model->setData(index, txt, Qt::EditRole);
		model->setData(index, indx, Qt::UserRole);
		return;
	}
	QItemDelegate::setModelData(editor, model, index);
}

void ZReportsDelegate::updateEditorGeometry(QWidget* editor,
	const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	editor->setGeometry(option.rect);
}
