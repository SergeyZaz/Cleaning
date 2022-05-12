#include <QAction>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QCryptographicHash>
#include "zusers.h"
#include "zmessager.h"

ZUsers::ZUsers(QWidget* parent, Qt::WindowFlags flags)//: ZMdiChild(parent, flags)
{
	QAction* actSetCloseDate = new QAction("Установить выбранную дату закрытия всем пользователям");
	QAction* actSetPeriod = new QAction("Установить выбранный рабочий период всем пользователям");
	QList<QAction*> contextMnuActions;
	contextMnuActions.append(actSetCloseDate);
	contextMnuActions.append(actSetPeriod);
	ui.m_tbl->getTblView()->setContextMenuPolicy(Qt::ActionsContextMenu);
	ui.m_tbl->getTblView()->addActions(contextMnuActions);

	connect(actSetCloseDate, &QAction::triggered, [actSetCloseDate, this]()
		{
			QSqlQuery query;
			QString str_query = QString("UPDATE users SET d_close=(SELECT d_close FROM users WHERE id = %1)").arg(ui.m_tbl->getCurrentId());
			if (!query.exec(str_query))
			{
				ZMessager::Instance().Message(_CriticalError, query.lastError().text(), "Ошибка");
				return;
			}
			ui.m_tbl->reload();
		});
	connect(actSetPeriod, &QAction::triggered, [actSetPeriod, this]()
		{
			QSqlQuery query;
			QString str_query = QString("UPDATE users SET period=(SELECT period FROM users WHERE id = %1)").arg(ui.m_tbl->getCurrentId());
			if (!query.exec(str_query))
			{
				ZMessager::Instance().Message(_CriticalError, query.lastError().text(), "Ошибка");
				return;
			}
			ui.m_tbl->reload();
		});
}

void ZUsers::init(const QString &m_TblName)
{
	setup("objects", "Объекты");

	QList<int> hideColumns;
	QStringList headers;
	QList<int> cRem;
	
	hideColumns << 0;
	hideColumns << 2;
	headers << tr("id") << tr("Логин") << tr("Пароль") << tr("Тип") << tr("Рабочий период") << tr("Дата закрытия");

	ui.m_tbl->setTable(m_TblName, headers, cRem);
	ui.m_tbl->setCustomEditor(new ZUsersForm(this));
	
	QMap<int, QString>* pMap0 = new QMap<int, QString>;
	pMap0->insert(0, "Администратор");
	pMap0->insert(1, "Пользователь");
	ui.m_tbl->setRelation(3, pMap0);

	ui.m_tbl->setRelation(4, "periods", "id", "name");

	ui.m_tbl->init(hideColumns, 1, Qt::DescendingOrder);

	setLinkTableName("users2objects");
}

/////////////////////////////////////////////////////////////////////////////////////////

ZUsersForm::ZUsersForm(QWidget* parent, Qt::WindowFlags flags) : ZEditAbstractForm(parent, flags)
{
	ui.setupUi(this);
	connect(ui.cmdSave, SIGNAL(clicked()), this, SLOT(applyChanges()));
	connect(ui.cmdSaveNew, SIGNAL(clicked()), this, SLOT(addNewSlot()));
	
	loadItemsToComboBox(ui.cboPeriod, "periods");
}

ZUsersForm::~ZUsersForm() {}

int ZUsersForm::init(const QString& table, int id)
{
	ZEditAbstractForm::init(table, id);

	ui.txtLogin->setEnabled(id != 0);
	ui.cboMode->setEnabled(id != 0);
	ui.txtLogin->setText("");
	ui.txtPwd->setText("");
	ui.txtPwd2->setText("");
	ui.cboMode->setCurrentIndex(1);
	ui.cboPeriod->setCurrentIndex(0);
	ui.dateEdit->setDate(QDate::currentDate());

	QString stringQuery = QString("SELECT name, pwd, mode, period, d_close FROM users WHERE id = %1")
		.arg(curEditId);

	// new record
	if (curEditId == ADD_UNIC_CODE)
	{
		return true;
	}

	// execute request
	QSqlQuery query;
	bool result = query.exec(stringQuery);
	if (result)
	{
		if (query.next())
		{
			ui.txtLogin->setText(query.value(0).toString());
			ui.txtPwd->setText(query.value(1).toString());
			ui.cboMode->setCurrentIndex(query.value(2).toInt());
			ui.cboPeriod->setCurrentIndex(ui.cboPeriod->findData(query.value(3).toInt()));
			ui.dateEdit->setDate(query.value(4).toDate());
		}
	}
	else
	{
		QMessageBox::critical(this, tr("Ошибка"), query.lastError().text());
	}

	return result;
}

void ZUsersForm::addNewSlot()
{
	curEditId = ADD_UNIC_CODE;
	applyChanges();
}

void ZUsersForm::applyChanges()
{
	QString text, stringQuery;

	if (curEditId == ADD_UNIC_CODE)
		stringQuery = QString("INSERT INTO users (name, pwd, mode, period, d_close) VALUES (?, ?, ?, ?, ?)");
	else
		stringQuery = QString("UPDATE users SET name=?, pwd=?, mode=?, period=?, d_close=? WHERE id=%1").arg(curEditId);

	QSqlQuery query;
	query.prepare(stringQuery);

	text = ui.txtLogin->text();
	if (text.isEmpty())
	{
		QMessageBox::critical(this, tr("Ошибка"), tr("Введите логин!"));
		return;
	}
	query.addBindValue(text);

	text = ui.txtPwd->text();
	if (text.size() < 6)
	{
		QMessageBox::critical(this, tr("Ошибка"), tr("Пароль должен быть не менее 6 символов!"));
		return;
	}
	if (text != ui.txtPwd2->text())
	{
		QMessageBox::critical(this, tr("Ошибка"), tr("Введенные пароли не совпадают!"));
		return;
	}

	text = QCryptographicHash::hash(text.toLocal8Bit(), QCryptographicHash::Md5).toHex();

	query.addBindValue(text);
	query.addBindValue(ui.cboMode->currentIndex());
	
	int indx = ui.cboPeriod->currentIndex();
	if (indx == -1)
		query.addBindValue(0);
	else
		query.addBindValue(ui.cboPeriod->itemData(indx, Qt::UserRole));

	query.addBindValue(ui.dateEdit->date());

	if (!query.exec())
	{
		QMessageBox::critical(this, tr("Ошибка"), query.lastError().text());
		return;
	}

	accept();
}

bool CheckPwd(const QString& login, const QString& psw, int* pType, int* pCurPeriodId)
{
	QSqlQuery query;
	if (!query.exec(QString("SELECT pwd,mode,period,d_close FROM users WHERE name = '%1'").arg(login)))
	{
		QMessageBox::critical(NULL, QString("Ошибка"), query.lastError().text());
		return false;
	}
	
	if (!query.next())
	{
		QMessageBox::critical(NULL, QString("Ошибка"), QString("Данные отсутствуют!"));
		return false;
	}

	QString pwd = query.value(0).toString();

	if (pType)
		(*pType) = query.value(1).toInt();

	if (pCurPeriodId)
	{
		(*pCurPeriodId) = query.value(2).toInt();
		QDate td = query.value(3).toDate();

		if (td.isValid() && (*pCurPeriodId) > 0)
		{
			if (query.exec(QString("SELECT d_open,name FROM periods WHERE id = %1").arg(*pCurPeriodId)))
			{
				if (query.next() && td > query.value(0).toDate())
					QMessageBox::critical(NULL, QString("Внимание!"), QString("Текущий отчетный период (%1) закрыт, выберите другой!").arg(query.value(1).toString()));
			}
		}

	}

	QString txt = QCryptographicHash::hash(psw.toLocal8Bit(), QCryptographicHash::Md5).toHex();
	return (pwd == txt);
}
