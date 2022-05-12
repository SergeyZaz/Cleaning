#include "zcontracts.h"
#include "zmessager.h"

ZContracts::ZContracts(QWidget* parent, Qt::WindowFlags flags) : ZMdiChild(parent, flags)
{
}

void ZContracts::init(const QString &m_TblName)
{
	QList<int> hideColumns;
	QStringList headers;
	QList<int> cRem;

	hideColumns << 0;
	headers << tr("id") << tr("Название") << tr("Комментарий") << tr("Дата открытия") << tr("Дата закрытия");

	m_tbl->setTable(m_TblName, headers, cRem);
	m_tbl->setCustomEditor(new ZContractsForm(this));
	m_tbl->init(hideColumns);
}

//////////////////////////////////////////////////////////////////////////////////////


ZContractsForm::ZContractsForm(QWidget* parent, Qt::WindowFlags flags) : ZEditAbstractForm(parent, flags)
{
	ui.setupUi(this);
	connect(ui.cmdSave, SIGNAL(clicked()), this, SLOT(applyChanges()));
	connect(ui.cmdSaveNew, SIGNAL(clicked()), this, SLOT(addNewSlot()));
}

ZContractsForm::~ZContractsForm() {}

int ZContractsForm::init(const QString& table, int id)
{
	ZEditAbstractForm::init(table, id);
	
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
	QString stringQuery = QString("SELECT name,comment,d_open,d_close FROM contracts WHERE id = %1")
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
		}
	}
	else
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
	}

	return result;
}

void ZContractsForm::addNewSlot()
{
	curEditId = ADD_UNIC_CODE;
	applyChanges();
}

void ZContractsForm::applyChanges()
{
	QString text, stringQuery;

	if (curEditId == ADD_UNIC_CODE)
		stringQuery = QString("INSERT INTO contracts (name,comment,d_open,d_close) VALUES (?, ?, ?, ?)");
	else
		stringQuery = QString("UPDATE contracts SET name=?, comment=?, d_open=?, d_close=? WHERE id=%1").arg(curEditId);

	QSqlQuery query;
	query.prepare(stringQuery);

	query.addBindValue(ui.txtName->text());
	query.addBindValue(ui.txtComment->toPlainText());
	query.addBindValue(ui.dateEditStart->date());
	query.addBindValue(ui.dateEditEnd->date());

	if (!query.exec())
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
		return;
	}

	accept();
}
