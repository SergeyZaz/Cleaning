#include "zposts.h"


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

	m_tbl->setRelation(3, m_TblName, "id", "name");

	m_tbl->init(hideColumns);
}