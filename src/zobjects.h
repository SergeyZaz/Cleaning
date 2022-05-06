#pragma once

#include "zmdichild.h"


class ZObjects : public ZMdiChild
{

public:
	ZObjects(QWidget* parent, Qt::WindowFlags flags = 0);

	void init(const QString& m_TblName);
};
