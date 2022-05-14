#include <QSqlQuery>
#include <QSqlError>

#include "zreports.h"
#include "zsettings.h"
#include "zmessager.h"


ZReports::ZReports(QWidget* parent, Qt::WindowFlags flags) : QWidget(parent, flags)
{
	curFindId = -1;

	ui.setupUi(this);
	
	loadItemsToComboBox(ui.cboEstimate, "estimates");
	loadItemsToComboBox(ui.cboPeriod, "periods");

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
	
	ui.cboPeriod->setCurrentIndex(ui.cboPeriod->findData(ZSettings::Instance().m_PeriodId));
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

	for (int i = 0; i < 2; i++)
	{
		stringQuery = QString("SELECT p.key, p.fio, fio.name FROM %1 AS p INNER JOIN fio ON(p.fio = fio.id) WHERE p.estimate_id=%2 AND p.period=%3 ORDER BY p.id")
			.arg(i == 0 ? "posts2fio" : "works2fio")
			.arg(estimateId)
			.arg(periodId);

		if (!query.exec(stringQuery))
		{
			ZMessager::Instance().Message(_CriticalError, query.lastError().text(), "Ошибка");
			continue;
		}

		model.m_data.clear();

		while (query.next())
		{
		}
	}
	model.Update();
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

QStringList ZReportsModel::n_days;

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

	ZReportsDelegate::load();
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
	
	int c = index.column();
	if (c < headers_before.size())
		return flags;

	c -= headers_before.size();
	if (c < d_begin.daysTo(d_end))
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
	
	const elem *pv = &m_data[r];

	if (c < headers_before.size())
	{
		switch (c)
		{
		case 0:
			return pv->org;
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
		if (role == Qt::UserRole)
			return pv->vars.value(c); 
		if (role == Qt::DisplayRole)
			return ZReportsDelegate::map.value(pv->vars.value(c));
		return QVariant();
	}

	c -= d_begin.daysTo(d_end);
	if (c < headers_after.size())
	{
		switch (c)
		{
		case 0:
		{
			double s = 0;
			QMap<int, int>::const_iterator iT = pv->vars.constBegin();
			while (iT != pv->vars.constEnd()) {
				s += iT.value();
				++iT;
			}
			return s;
		}
		case 1:
			return pv->tariff;
		case 2:
			return pv->zp;
		case 3:
			return pv->doplata;
		case 4:
			return pv->vichet;
		case 5:
			return pv->zp + pv->doplata - pv->vichet;
		case 6:
			return pv->comment;
		default:
			break;
		}
		return QVariant();
	}

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
	if (r >= rowCount(index))
		return false;


	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////
 QMap<int, QString> ZReportsDelegate::map;

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

void ZReportsDelegate::load()
{
	map.clear();
	QSqlQuery query;
	if (query.exec("SELECT id, name FROM variants"))
		while (query.next())
			map.insert(query.value(0).toInt(), query.value(1).toString());
}

void ZReportsDelegate::setEditorData(QWidget* editor,
	const QModelIndex& index) const
{
	QComboBox* cbo = dynamic_cast<QComboBox*>(editor);
	if (cbo)
	{
		load();
		loadItemsToComboBox(cbo, map);
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
