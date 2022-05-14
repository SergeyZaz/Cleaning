#include <QSqlQuery>
#include <QSqlError>

#include "zreports.h"
#include "zsettings.h"
#include "zmessager.h"

#define HOLIDAY_COLOR QColor(128, 255, 255)
int periodId = 0, estimateId = 0;

ZReports::ZReports(QWidget* parent, Qt::WindowFlags flags) : QWidget(parent, flags)
{
	curFindId = -1;

	ui.setupUi(this);

	ui.pnlFind->setVisible(false);
	
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
	if (ZSettings::Instance().m_UserType == 1)
		return;

	QSqlQuery query;
	if (!query.exec("DELETE FROM config WHERE key = 'memo'"))
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
		return;
	}

	if (!query.exec(QString("INSERT INTO config(key, value) VALUES('memo','%1')").arg(ui.txtMemo->toPlainText())))
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
		return;
	}
}

QSize ZReports::sizeHint() const
{
	return QSize(1600, 750);
}

void ZReports::changeEstimateSlot(int indx)
{
	estimateId = ui.cboEstimate->itemData(ui.cboEstimate->currentIndex(), Qt::UserRole).toInt();
	buildReport();
}

void ZReports::changePeriodSlot(int indx)
{
	periodId = ui.cboPeriod->itemData(ui.cboPeriod->currentIndex(), Qt::UserRole).toInt();
	buildReport();
}

void ZReports::buildReport()
{
	ui.lblReadOnly->setVisible(ZSettings::Instance().m_UserType == 1);

	QString stringQuery;
	QSqlQuery query;
	
	//зачитываю даты периода
	stringQuery = QString("SELECT d_open, d_close FROM periods WHERE id=%1").arg(periodId);
	if (!query.exec(stringQuery))
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), "Ошибка");
	if (query.next())
	{
		model.d_begin = query.value(0).toDate();
		model.d_end = query.value(1).toDate();
	}
		
	//зачитываю fio-организации
	stringQuery = QString("SELECT o.value, organisation.name FROM organisation2fio AS o \
INNER JOIN organisation ON(o.key = organisation.id) WHERE period=%1")
	.arg(periodId);

	if (!query.exec(stringQuery))
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), "Ошибка");
	
	QMap<int, QString> fio2org;
	while (query.next())
		fio2org.insert(query.value(0).toInt(), query.value(1).toString());


	model.m_data.clear();

	for (int i = 0; i < 2; i++)
	{
		stringQuery = QString("SELECT p.key, %4.name, p.fio, fio.name, p.id, p.comment FROM %1 AS p \
INNER JOIN fio ON(p.fio = fio.id) \
INNER JOIN %4 ON(p.key = %4.id) \
WHERE p.estimate_id=%2 AND p.period=%3 ORDER BY p.id")
		.arg(i == 0 ? "posts2fio" : "works2fio")
		.arg(estimateId)
		.arg(periodId)
		.arg(i == 0 ? "posts" : "works");

		if (!query.exec(stringQuery))
		{
			ZMessager::Instance().Message(_CriticalError, query.lastError().text(), "Ошибка");
			continue;
		}

		while (query.next())
		{
			ZReportsModel::elem v;
			
			v.post_id = query.value(0).toInt();
			v.post = query.value(1).toString();
			v.fio_id = query.value(2).toInt();
			v.fio = query.value(3).toString();
			v.id = query.value(4).toInt();
			if (i == 1)
				v.id *= -1;
			v.comment = query.value(5).toString();

			v.org = fio2org.value(v.fio_id);

			model.m_data << v;
		}
	}

	stringQuery = QString("SELECT fio, d, var FROM reports WHERE estimate_id=%1 AND period=%2")
		.arg(estimateId)
		.arg(periodId);

	if (!query.exec(stringQuery))
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), "Ошибка");
	}

	while (query.next())
	{
		ZReportsModel::elem *pv = model.getForFio(query.value(0).toInt());
		if (!pv)
			continue;
		pv->vars.insert(query.value(1).toInt(), query.value(2).toInt());
	}

	model.Update();

	int i, n = model.headers_before.size() + model.d_begin.daysTo(model.d_end);
	int end = model.columnCount(QModelIndex());
	for (i = model.headers_before.size(); i < end; i++)
	{
		ui.tbl->setColumnWidth(i, i <= n ? 30 : 80);
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

QStringList ZReportsModel::n_days;

ZReportsModel::ZReportsModel(QObject* parent)
{
	n_days << "" << QString("Пн") << QString("Вт") << QString("Ср") << QString("Чт") << QString("Пт") << QString("Сб") << QString("Вс");

	headers_before.append(QObject::tr("Орг."));
	headers_before.append(QObject::tr("ФИО"));
	headers_before.append(QObject::tr("Должность"));

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
		int nd = d_begin.daysTo(d_end);
		if (section <= nd)
		{
			QDate d = d_begin.addDays(section);
			return QString("%1\n%2").arg(d.day()).arg(n_days.at(d.dayOfWeek()));
		}

		section -= nd+1;
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
	int nd = d_begin.daysTo(d_end);
	if (c <= nd || c == nd + headers_after.size())
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
			if (role == Qt::DisplayRole)
				return pv->org;
			if (role == Qt::UserRole)
				return pv->id;// здесь возвращаю id записи в БД
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
	int nd = d_begin.daysTo(d_end);
	if (c <= nd)
	{
		if (role == Qt::BackgroundColorRole)
		{
			int dW = d_begin.addDays(c).dayOfWeek();
			if(dW == Qt::Saturday || dW == Qt::Sunday)
			return QColor(HOLIDAY_COLOR);
		}
			
		if (role == Qt::UserRole)
			return pv->vars.value(c); 
		if (role == Qt::DisplayRole)
			return ZReportsDelegate::map.value(pv->vars.value(c));
		return QVariant();
	}

	c -= nd+1;
	if (c < headers_after.size())
	{
		switch (c)
		{
		case 0:
			if (role == Qt::DisplayRole)
			{
				double s = 0;
				QMap<int, int>::const_iterator iT = pv->vars.constBegin();
				while (iT != pv->vars.constEnd()) {
					s += ZReportsDelegate::map.value(iT.value()).toInt();
					++iT;
				}
				return s;
			}
		case 1:
			if (role == Qt::DisplayRole)
				return pv->tariff;
		case 2:
			if (role == Qt::DisplayRole)
				return pv->zp;
		case 3:
			if (role == Qt::DisplayRole)
				return pv->doplata;
		case 4:
			if (role == Qt::DisplayRole)
				return pv->vichet;
		case 5:
			if (role == Qt::DisplayRole)
				return pv->zp + pv->doplata - pv->vichet;
		case 6:
			if (role == Qt::DisplayRole)
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

	int c = index.column();
	if (c >= columnCount(index))
		return false;

	int r = index.row();
	if (r >= rowCount(index))
		return false;

	if (c < headers_before.size())
		return false;

	elem* pv = &m_data[r];
	c -= headers_before.size();
	int nd = d_begin.daysTo(d_end);
	if (c <= nd)
	{
		if (role == Qt::UserRole)
		{
			int v = value.toInt();
			if (v == 0)
				pv->vars.remove(c);
			else
				pv->vars.insert(c, v);

			updateDataDB(v == 0 ? DEL_VALUE : UP_VALUE, pv, c);
		}
	}

	c -= nd + 1;
	if (c == headers_after.size() - 1)
	{
		if (role == Qt::EditRole)
		{
			pv->comment = value.toString();
			updateDataDB(UP_COMMENT, pv);
		}
	}
	return true;
}

bool ZReportsModel::updateDataDB(op_type t, elem* pData, int d)
{
	QString stringQuery;
	QSqlQuery query;

	switch (t)
	{
	case UP_COMMENT:
		stringQuery = QString("UPDATE %1 SET comment='%2' WHERE id=%3")
			.arg(pData->id > 0 ? "posts2fio" : "works2fio")
			.arg(pData->comment)
			.arg(pData->id > 0 ? pData->id : -1 * pData->id);
		break;
	case UP_VALUE:
		stringQuery = QString("DELETE FROM reports WHERE estimate_id=%1 AND period=%2 AND fio=%3 AND d=%4;\
INSERT INTO reports (estimate_id, period, fio, d, var) VALUES (%1, %2, %3, %4, %5);")
.arg(estimateId)
.arg(periodId)
.arg(pData->fio_id)
.arg(d)
.arg(pData->vars.value(d));
		break;
	case DEL_VALUE:
		stringQuery = QString("DELETE FROM reports WHERE estimate_id=%1 AND period=%2 AND fio=%3 AND d=%4")
			.arg(estimateId)
			.arg(periodId)
			.arg(pData->fio_id)
			.arg(d);
		break;
	default:
		return false;
	}

	if (!query.exec(stringQuery))
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), "Ошибка");
	return true;
}

ZReportsModel::elem* ZReportsModel::getForFio(int fio_id)
{
	int n = m_data.size();
	for (int i = 0; i < n; i++)
	{
		elem* p = &m_data[i];
		if (p->fio_id == fio_id)
			return p;
	}
	return NULL;
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

	int column = index.column();
	if (column == index.model()->columnCount(index) - 1)
		return QItemDelegate::createEditor(parent, option, index);

	QComboBox* w = new QComboBox(parent);
	w->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	return w;
}

void ZReportsDelegate::load()
{
	map.clear();
	QSqlQuery query;
	if (query.exec("SELECT id, name FROM variants ORDER BY name"))
		while (query.next())
			map.insert(query.value(0).toInt(), query.value(1).toString());
	map.insert(0, "");
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

//		model->setData(index, txt, Qt::EditRole);
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
