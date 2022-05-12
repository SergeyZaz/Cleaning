#include "zposts.h"
#include "zmessager.h"

ZPosts::ZPosts(QWidget* parent, Qt::WindowFlags flags): ZMdiChild(parent, flags)
{
}

void ZPosts::init(const QString& m_TblName)
{
	QList<int> hideColumns;
	QStringList headers;
	QList<int> cRem;

	hideColumns << 0;
	headers << tr("id") << tr("Название") << tr("Комментарий") << tr("Привязка");

	m_tbl->setTable(m_TblName, headers, cRem);
	m_tbl->setCustomEditor(new ZPostsForm(this));
	m_tbl->setRelation(3, m_TblName, "id", "name");
	m_tbl->init(hideColumns);
}

//////////////////////////////////////////////////////////////////////////////////////////////

ZPostsForm::ZPostsForm(QWidget* parent, Qt::WindowFlags flags) : ZEditAbstractForm(parent, flags)
{
	ui.setupUi(this);
	connect(ui.cmdSave, SIGNAL(clicked()), this, SLOT(applyChanges()));
	connect(ui.cmdSaveNew, SIGNAL(clicked()), this, SLOT(addNewSlot()));
}

ZPostsForm::~ZPostsForm() {}

int ZPostsForm::init(const QString& table, int id)
{
	ZEditAbstractForm::init(table, id);

	loadItemsToComboBox(ui.cboMode, "posts");

	// new record
	if (curEditId == ADD_UNIC_CODE)
	{
		ui.cboMode->setCurrentIndex(0);
		ui.txtName->setText("");
		ui.txtComment->setText("");
		return true;
	}

	// execute request
	QString stringQuery = QString("SELECT name,comment,parent_id FROM posts WHERE id = %1")
		.arg(curEditId);

	QSqlQuery query;
	bool result = query.exec(stringQuery);
	if (result)
	{
		if (query.next())
		{
			ui.txtName->setText(query.value(0).toString());
			ui.txtComment->setText(query.value(1).toString());
			ui.cboMode->setCurrentIndex(ui.cboMode->findData(query.value(2).toInt()));
		}
	}
	else
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
	}

	return result;
}

void ZPostsForm::addNewSlot()
{
	curEditId = ADD_UNIC_CODE;
	applyChanges();
}

void ZPostsForm::applyChanges()
{
	QString text, stringQuery;

	if (curEditId == ADD_UNIC_CODE)
		stringQuery = QString("INSERT INTO posts (name,comment,parent_id) VALUES (?, ?, ?)");
	else
		stringQuery = QString("UPDATE posts SET name=?, comment=?, parent_id=? WHERE id=%1").arg(curEditId);

	QSqlQuery query;
	query.prepare(stringQuery);

	query.addBindValue(ui.txtName->text());
	query.addBindValue(ui.txtComment->toPlainText());
	query.addBindValue(ui.cboMode->itemData(ui.cboMode->currentIndex(), Qt::UserRole));

	if (!query.exec())
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
		return;
	}

	accept();
}
