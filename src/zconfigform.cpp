#include "zconfigform.h"
#include "zmessager.h"
#include "zsettings.h"
#include "zview.h"

#include <QSqlQuery>
#include <QSqlError>

ZConfigForm::ZConfigForm(QWidget* parent, Qt::WindowFlags flags)
{
	ui.setupUi(this);
	connect(ui.cmdSave, SIGNAL(clicked()), this, SLOT(applyChanges()));
	
	loadItemsToComboBox(ui.cboPeriod, "periods");
	ui.cboPeriod->setCurrentIndex(ui.cboPeriod->findData(ZSettings::Instance().m_PeriodId));
}

ZConfigForm::~ZConfigForm() {}

void ZConfigForm::applyChanges()
{
	int id = ui.cboPeriod->itemData(ui.cboPeriod->currentIndex(), Qt::UserRole).toInt();

	QSqlQuery query;
	QString str_query = QString("UPDATE users SET period=%1 WHERE name = '%2'").arg(id).arg(ZSettings::Instance().m_UserName);
	if (!query.exec(str_query))
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), "Ошибка");
		return;
	}
	
	ZSettings::Instance().m_PeriodId = id;

	accept();
}

bool GetCloseDate()
{
	QSqlQuery query;
	QString str_query = QString("SELECT d_close FROM users WHERE name = '%1'").arg(ZSettings::Instance().m_UserName);
	if (!query.exec(str_query))
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), "Ошибка");
		return false;
	}

	if (!query.next())
		return false;

	ZSettings::Instance().m_CloseDate = query.value(0).toDate();
	return true;
}