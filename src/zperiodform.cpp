#include <QSqlQuery>
#include <QSqlError>
#include "zperiodform.h"
#include "zsettings.h"
#include "zmessager.h"

ZPeriodForm::ZPeriodForm(QWidget* parent, Qt::WindowFlags flags) : ZEditAbstractForm(parent, flags)
{
	ui.setupUi(this);
	connect(ui.cmdSave, SIGNAL(clicked()), this, SLOT(applyChanges()));
	connect(ui.cmdSaveNew, SIGNAL(clicked()), this, SLOT(addNewSlot()));
}

ZPeriodForm::~ZPeriodForm(){}

int ZPeriodForm::init(const QString &table, int id )
{
	ZEditAbstractForm::init(table, id);

	QDate tmp_d = QDate::currentDate();
	bool f_day = tmp_d.day() < 16;
	ui.dateEditStart->setDate(QDate(tmp_d.year(), tmp_d.month(), f_day ? 1 : 16));
	ui.dateEditEnd->setDate(QDate(tmp_d.year(), tmp_d.month(), f_day ? 15 : tmp_d.daysInMonth()));
	ui.txtName->setText(QString("%1, %2 половина").arg(QDate::longMonthName(tmp_d.month())).arg(f_day ? "первая" : "вторая"));
	ui.txtComment->setText("");

	QString stringQuery = QString("SELECT name,comment,d_open,d_close FROM periods WHERE id = %1").arg(curEditId);

	// new record
	if (curEditId == ADD_UNIC_CODE)
		return true;

	// execute request
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

void ZPeriodForm::addNewSlot()
{
	curEditId = ADD_UNIC_CODE;
	applyChanges();
}

void ZPeriodForm::applyChanges()
{
	QString text, stringQuery;

	if (curEditId == ADD_UNIC_CODE)
		stringQuery = QString("INSERT INTO periods (name,comment,d_open,d_close) VALUES (?, ?, ?, ?)");
	else
		stringQuery = QString("UPDATE periods SET name=?, comment=?, d_open=?, d_close=? WHERE id=%1").arg(curEditId);

	QSqlQuery query;
	query.prepare(stringQuery);
	
	query.addBindValue(ui.txtName->text());
	query.addBindValue(ui.txtComment->toPlainText());
	query.addBindValue(ui.dateEditStart->date());
	query.addBindValue(ui.dateEditEnd->date());


	if(!query.exec())
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
		return;
	}
	
	accept();
}
