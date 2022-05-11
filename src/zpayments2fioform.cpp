#include <QSqlQuery>
#include <QSqlError>
#include <QCompleter>
#include "zpayments2fioform.h"
#include "zsettings.h"
#include "zmessager.h"
#include "zview.h"

ZPayments2FioForm::ZPayments2FioForm(QWidget* parent, int mode, Qt::WindowFlags flags) : ZEditAbstractForm(parent, flags)
{
	mMode = mode;
	ui.setupUi(this);
	connect(ui.cmdSave, SIGNAL(clicked()), this, SLOT(applyChanges()));
	connect(ui.cmdSaveNew, SIGNAL(clicked()), this, SLOT(addNewSlot()));
	
	ui.lblPayment->setText((mMode == 0) ? "Доплата:" : "Вычет:");

	ui.cboFIO->setEditable(true);
}

ZPayments2FioForm::~ZPayments2FioForm(){}

int ZPayments2FioForm::init(const QString &table, int id )
{
	ZEditAbstractForm::init(table, id);
	
	loadItemsToComboBox(ui.cboPeriod, "periods");

	loadItemsToComboBox(ui.cboPayment, "payments", QString("mode=%1").arg(mMode));

	loadFio();

	// new record
	if (curEditId == ADD_UNIC_CODE)
	{
		ui.cboPayment->setCurrentIndex(0);
		ui.cboFIO->setCurrentIndex(0);
		ui.dateEdit->setDate(QDate::currentDate());
		ui.spinVal->setValue(0);
		ui.txtComment->setText("");
		ui.cboPeriod->setCurrentIndex(ui.cboPeriod->findData(ZSettings::Instance().m_PeriodId));
		return true;
	}

	// execute request
	QString stringQuery = QString("SELECT payment,fio,dt,val,payments2fio.comment,period FROM payments2fio INNER JOIN payments ON(payments.id = payment) WHERE payments2fio.id = %1")
		.arg(curEditId);

	QSqlQuery query;
	bool result = query.exec(stringQuery);
	if (result)
	{
		if (query.next()) 
		{
			int indx = query.value(4).toInt();
/*
			QDate d = query.value(5).toDate();
			if (ZSettings::Instance().m_CloseDate.isValid() && d < ZSettings::Instance().m_CloseDate)
			{
				ZMessager::Instance().Message(_CriticalError, tr("Дата привязки меньше даты закрытия!"), tr("Редактирование запрещено"));
				return false;
			}
*/
			ui.cboPayment->setCurrentIndex(ui.cboPayment->findData(query.value(0).toInt()));
			ui.cboFIO->setCurrentIndex(ui.cboFIO->findData(query.value(1).toInt()));
			ui.dateEdit->setDate(query.value(2).toDate());
			ui.spinVal->setValue(query.value(3).toDouble());
			ui.txtComment->setText(query.value(4).toString());
			ui.cboPeriod->setCurrentIndex(ui.cboPeriod->findData(query.value(5).toInt()));
		}
	}	
	else 
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
	}

	return result;
}

void ZPayments2FioForm::loadFio()
{
	ui.cboFIO->clear();

	QSqlQuery query;
	if (query.exec("SELECT id,name FROM fio ORDER BY name"))
	{
		while (query.next())
		{
			ui.cboFIO->addItem(query.value(1).toString(), query.value(0).toInt());
		}
	}
	else
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
	}

	QCompleter* completer = new QCompleter(this);
	completer->setModel(ui.cboFIO->model());
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	ui.cboFIO->setCompleter(completer);
}

void ZPayments2FioForm::addNewSlot()
{
	curEditId = ADD_UNIC_CODE;
	applyChanges();
}

void ZPayments2FioForm::applyChanges()
{
	QString text, stringQuery;

	if (curEditId == ADD_UNIC_CODE)
		stringQuery = QString("INSERT INTO payments2fio (payment,fio,dt,val,period,comment) VALUES (?, ?, ?, ?, ?, ?)");
	else
		stringQuery = QString("UPDATE payments2fio SET payment=?, fio=?, dt=?, val=?, period=?, comment=? WHERE id=%1").arg(curEditId);

	QSqlQuery query;
	query.prepare(stringQuery);
	
//	QString t = ui.cboFIO->currentText();
//	int i = ui.cboFIO->findText(t);
	query.addBindValue(ui.cboPayment->itemData(ui.cboPayment->currentIndex(), Qt::UserRole));
	query.addBindValue(ui.cboFIO->itemData(ui.cboFIO->findText(ui.cboFIO->currentText()), Qt::UserRole));
	query.addBindValue(ui.dateEdit->date());
	query.addBindValue(ui.spinVal->value());
	query.addBindValue(ui.cboPeriod->itemData(ui.cboPeriod->findText(ui.cboPeriod->currentText()), Qt::UserRole));
	query.addBindValue(ui.txtComment->toPlainText());

	if(!query.exec())
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
		return;
	}
	
	accept();
}
