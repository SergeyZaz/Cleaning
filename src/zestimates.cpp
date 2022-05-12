#include "zestimates.h"
#include "zmessager.h"

ZEstimates::ZEstimates(QWidget* parent, Qt::WindowFlags flags) : ZMdiChild(parent, flags)
{
}

void ZEstimates::init(const QString &m_TblName)
{
	QList<int> hideColumns;
	QStringList headers;
	QList<int> cRem;

	hideColumns << 0;
	headers << tr("id") << tr("Название") << tr("Комментарий") << tr("Дата открытия") << tr("Дата закрытия") << tr("Договор");

	m_tbl->setTable(m_TblName, headers, cRem);
	m_tbl->setCustomEditor(new ZEstimatesForm(this));
	m_tbl->setRelation(5, "contracts", "id", "name");
	m_tbl->init(hideColumns);
}

//////////////////////////////////////////////////////////////////////////////////////


ZEstimatesForm::ZEstimatesForm(QWidget* parent, Qt::WindowFlags flags) : ZEditAbstractForm(parent, flags)
{
	ui.setupUi(this);
	connect(ui.cmdSave, SIGNAL(clicked()), this, SLOT(applyChanges()));
	connect(ui.cmdSaveNew, SIGNAL(clicked()), this, SLOT(addNewSlot()));
}

ZEstimatesForm::~ZEstimatesForm() {}

int ZEstimatesForm::init(const QString& table, int id)
{
	ZEditAbstractForm::init(table, id);
	
	loadItemsToComboBox(ui.cboContract, "contracts");

	QDate tmp_d = QDate::currentDate();
	ui.dateEditStart->setDate(tmp_d);
	ui.dateEditEnd->setDate(tmp_d.addMonths(1));

	// new record
	if (curEditId == ADD_UNIC_CODE)
	{
		ui.txtName->setText("");
		ui.txtComment->setText("");
		return true;
	}

	// execute request
	QString stringQuery = QString("SELECT name,comment,d_open,d_close,contract_id FROM estimates WHERE id = %1")
		.arg(curEditId);

	QSqlQuery query;
	bool result = query.exec(stringQuery);
	if (result)
	{
		if (query.next())
		{
			ui.txtName->setText(query.value(0).toString());
			ui.txtComment->setText(query.value(1).toString());
			ui.dateEditStart->setDate(query.value(2).toDate());
			ui.dateEditEnd->setDate(query.value(3).toDate());
			ui.cboContract->setCurrentIndex(ui.cboContract->findData(query.value(4).toInt()));
		}
	}
	else
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
	}

	return result;
}

void ZEstimatesForm::addNewSlot()
{
	curEditId = ADD_UNIC_CODE;
	applyChanges();
}

void ZEstimatesForm::applyChanges()
{
	QString text, stringQuery;

	if (curEditId == ADD_UNIC_CODE)
		stringQuery = QString("INSERT INTO estimates (name,comment,d_open,d_close,contract_id) VALUES (?, ?, ?, ?, ?) RETURNING id");
	else
		stringQuery = QString("UPDATE estimates SET name=?, comment=?, d_open=?, d_close=?, contract_id=? WHERE id=%1").arg(curEditId);

	QSqlQuery query;
	query.prepare(stringQuery);

	query.addBindValue(ui.txtName->text());
	query.addBindValue(ui.txtComment->toPlainText());
	query.addBindValue(ui.dateEditStart->date());
	query.addBindValue(ui.dateEditEnd->date());
	query.addBindValue(ui.cboContract->itemData(ui.cboContract->findText(ui.cboContract->currentText()), Qt::UserRole));

	if (!query.exec())
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
		return;
	}

	if (curEditId == ADD_UNIC_CODE && oldId != ADD_UNIC_CODE && query.next())
	{
		curEditId = query.value(0).toInt();

		if (!copyData(oldId, curEditId))
			return;
	}

	accept();
}

int ZEstimatesForm::copyData(int curId, int newId)
{
	QSqlQuery query;
	QString stringQuery;

	return 1;
}
