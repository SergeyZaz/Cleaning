#pragma once

#include "zmdichild.h"

class ZContracts : public ZMdiChild
{

public:
	ZContracts(QWidget* parent, Qt::WindowFlags flags = 0);

	void init(const QString& m_TblName);
};