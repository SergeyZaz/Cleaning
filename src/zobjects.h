#pragma once
#include "zviewgroups.h"

class ZObjects : public ZViewGroups
{
public:
	ZObjects(QWidget* parent, Qt::WindowFlags flags = 0);
 
	void init(const QString &m_TblName);
};


