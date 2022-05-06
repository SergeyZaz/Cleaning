#include <QAction>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QCryptographicHash>
#include "zusers.h"
#include "zmessager.h"

ZUsers::ZUsers(QWidget* parent, Qt::WindowFlags flags): ZMdiChild(parent, flags)
{
	QAction* actSetCloseDate = new QAction("Установить выбранную дату закрытия всем пользователям");
	QList<QAction*> contextMnuActions;
	contextMnuActions.append(actSetCloseDate);
	m_tbl->getTblView()->setContextMenuPolicy(Qt::ActionsContextMenu);
	m_tbl->getTblView()->addActions(contextMnuActions);
	connect(actSetCloseDate, SIGNAL(triggered()), this, SLOT(setDateSlot()));
}

void ZUsers::setDateSlot()
{
	QSqlQuery query;
	QString str_query = QString("UPDATE users SET d_close=(SELECT d_close FROM users WHERE id = %1)").arg(m_tbl->getCurrentId());
	if (!query.exec(str_query))
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), "Ошибка");
		return;
	}
	m_tbl->reload();
}

void ZUsers::init(const QString &m_TblName)
{
	QList<int> hideColumns;
	QStringList headers;
	QList<int> cRem;
	
	hideColumns << 0;
	hideColumns << 2;
	headers << tr("id") << tr("Логин") << tr("Пароль") << tr("Тип") << tr("Рабочий период") << tr("Дата закрытия");

	m_tbl->setTable(m_TblName, headers, cRem);	
	m_tbl->setCustomEditor(new ZUsersForm(this));
	
	QMap<int, QString>* pMap0 = new QMap<int, QString>;
	pMap0->insert(0, "Администратор");
	pMap0->insert(1, "Пользователь");
	m_tbl->setRelation(3, pMap0);

	m_tbl->setRelation(4, "periods", "id", "name");

	m_tbl->init(hideColumns, 1, Qt::DescendingOrder);
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
