#include "zperiods.h"

ZPeriods::ZPeriods(QWidget* parent, Qt::WindowFlags flags): ZMdiChild(parent, flags)
{
}

void ZPeriods::init(const QString &m_TblName)
{
	QList<int> hideColumns;
	QStringList headers;
	QList<int> cRem;
	
	hideColumns << 0;
	headers <<  tr("id") << tr("Название") << tr("Комментарий");

	m_tbl->setTable(m_TblName, headers, cRem);	
	m_tbl->init(hideColumns);
}

