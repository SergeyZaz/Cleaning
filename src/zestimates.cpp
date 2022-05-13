#include "zestimates.h"
#include "zmessager.h"
#include "zsettings.h"
#include <QMessageBox>

ZEstimates::ZEstimates(QWidget* parent, Qt::WindowFlags flags) : ZMdiChild(parent, flags)
{
}

void ZEstimates::init(const QString &m_TblName)
{
	QList<int> hideColumns;
	QStringList headers;
	QList<int> cRem;

	hideColumns << 0;
	headers << tr("id") << tr("Название") << tr("Комментарий") << tr("Дата открытия") << tr("Дата закрытия") << tr("Договор");

	m_tbl->setTable(m_TblName, headers, cRem);
	m_tbl->setCustomEditor(new ZEstimatesForm(this));
	m_tbl->setRelation(5, "contracts", "id", "name");
	m_tbl->init(hideColumns);
}

//////////////////////////////////////////////////////////////////////////////////////


ZEstimatesForm::ZEstimatesForm(QWidget* parent, Qt::WindowFlags flags) : ZEditAbstractForm(parent, flags)
{
	ui.setupUi(this);
	connect(ui.cmdSave, SIGNAL(clicked()), this, SLOT(applyChanges()));
	connect(ui.cmdSaveNew, SIGNAL(clicked()), this, SLOT(addNewSlot()));
	
	ui.treePosts->setItemDelegate(new ZEstimatesTreeDelegate(this, "posts", ui.treePosts));
	connect(ui.cmdAddPosts, SIGNAL(clicked()), this, SLOT(addRowSlot()));
	connect(ui.cmdDelPosts, SIGNAL(clicked()), this, SLOT(delRowSlot()));
	connect(ui.treePosts, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(itemChangedSlot(QTreeWidgetItem*, int)));
	ui.treePosts->setColumnWidth(0, 200);


	ui.treeWorks->setItemDelegate(new ZEstimatesTreeDelegate(this, "works", ui.treeWorks));
	connect(ui.cmdAddWorks, SIGNAL(clicked()), this, SLOT(addRowSlot()));
	connect(ui.cmdDelWorks, SIGNAL(clicked()), this, SLOT(delRowSlot()));
	connect(ui.treeWorks, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(itemChangedSlot(QTreeWidgetItem*, int)));
	ui.treeWorks->setColumnWidth(0, 200);

}

ZEstimatesForm::~ZEstimatesForm() {}

int ZEstimatesForm::init(const QString& table, int id)
{
	ZEditAbstractForm::init(table, id);

	QSqlQuery query;
	QString stringQuery;
		
	for (int i = 0; i < 2; i++)
	{
		stringQuery = QString("SELECT id, name, parent_id FROM %1").arg(i == 0 ? "posts" : "works");

		if (query.exec(stringQuery))
		{
			while (query.next())
			{
				linkInfo elem;
				elem.id = query.value(0).toInt();
				elem.name = query.value(1).toString();
				elem.parent_id = query.value(2).toInt();
				if(i == 0)
					mapPosts.insert(elem.id, elem);
				else
					mapWorks.insert(elem.id, elem);
			}
		}
	}


	loadTblInfo("estimates_posts", ui.treePosts);
	loadTblInfo("estimates_works", ui.treeWorks);

	updateSumm();

	loadItemsToComboBox(ui.cboContract, "contracts");

	QDate tmp_d = QDate::currentDate();
	ui.dateEditStart->setDate(tmp_d);
	ui.dateEditEnd->setDate(tmp_d.addMonths(1));

	// new record
	if (curEditId == ADD_UNIC_CODE)
	{
		ui.txtName->setText("");
		ui.txtComment->setText("");
		return true;
	}

	// execute request
	stringQuery = QString("SELECT name,comment,d_open,d_close,contract_id FROM estimates WHERE id = %1")
		.arg(curEditId);

	bool result = query.exec(stringQuery);
	if (result)
	{
		if (query.next())
		{
			ui.txtName->setText(query.value(0).toString());
			ui.txtComment->setText(query.value(1).toString());
			ui.dateEditStart->setDate(query.value(2).toDate());
			ui.dateEditEnd->setDate(query.value(3).toDate());
			ui.cboContract->setCurrentIndex(ui.cboContract->findData(query.value(4).toInt()));
		}
	}
	else
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
	}

	return result;
}

void ZEstimatesForm::addNewSlot()
{
	curEditId = ADD_UNIC_CODE;
	applyChanges();
}

void ZEstimatesForm::applyChanges()
{
	QString text, stringQuery;

	if (curEditId == ADD_UNIC_CODE)
		stringQuery = QString("INSERT INTO estimates (name,comment,d_open,d_close,contract_id) VALUES (?, ?, ?, ?, ?) RETURNING id");
	else
		stringQuery = QString("UPDATE estimates SET name=?, comment=?, d_open=?, d_close=?, contract_id=? WHERE id=%1").arg(curEditId);

	QSqlQuery query;
	query.prepare(stringQuery);

	query.addBindValue(ui.txtName->text());
	query.addBindValue(ui.txtComment->toPlainText());
	query.addBindValue(ui.dateEditStart->date());
	query.addBindValue(ui.dateEditEnd->date());
	query.addBindValue(ui.cboContract->itemData(ui.cboContract->findText(ui.cboContract->currentText()), Qt::UserRole));

	if (!query.exec())
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
		return;
	}

	if (curEditId == ADD_UNIC_CODE && oldId != ADD_UNIC_CODE && query.next())
	{
		curEditId = query.value(0).toInt();

		if (!copyData(oldId, curEditId))
			return;
	}

	//сохраняю информацию из таблиц
	QTreeWidget* tree;
	for (int i = 0; i < 2; i++)
	{
		stringQuery = QString("DELETE FROM %1 WHERE estimate_id=%2").arg(i==0 ? "estimates_posts" : "estimates_works").arg(curEditId);
		if (!query.exec(stringQuery))
			ZMessager::Instance().Message(_CriticalError, query.lastError().text());

		tree = (i == 0) ? ui.treePosts : ui.treeWorks;

		QTreeWidgetItemIterator it(tree);
		while (*it)
		{
			stringQuery = QString("INSERT INTO %1 (estimate_id,link_id,count,val) VALUES (%2, %3, %4, %5)")
				.arg(i == 0 ? "estimates_posts" : "estimates_works")
				.arg(curEditId)
				.arg((*it)->data(0, Qt::UserRole).toInt())
				.arg((*it)->data(1, Qt::DisplayRole).toInt())
				.arg((*it)->data(2, Qt::DisplayRole).toDouble());

			if (!query.exec(stringQuery))
				ZMessager::Instance().Message(_CriticalError, query.lastError().text());

			++it;
		}
	}

	accept();
}

int ZEstimatesForm::copyData(int curId, int newId)
{
	QSqlQuery query;
	QString stringQuery;

	return 1;
}

void ZEstimatesForm::updateSumm()
{
	int count = 0, summ = 0;
	QTreeWidgetItem* pItem;

	QTreeWidget* tree = ui.treeWorks;

	for (int i = 0; i < 2; i++)
	{
		if(i==1)
			tree = ui.treePosts;

		QTreeWidgetItemIterator it(tree);
		while (*it) 
		{
			if(!(*it)->parent())
				count += (*it)->text(1).toInt();
			summ += (*it)->text(3).toInt();
			++it;
		}
	}

	ui.lblTotal->setText(QString("Итого: человек: %1, сумма: %2").arg(count).arg(summ));
}

void ZEstimatesForm::addRowSlot()
{
	QTreeWidget* tree = ui.treeWorks;
	if (sender() == ui.cmdAddPosts)
		tree = ui.treePosts;

	QTreeWidgetItem* pItem = new QTreeWidgetItem(tree);
	pItem->setText(1, "1");
	pItem->setText(2, "1000");
	pItem->setText(3, "1000");
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
	pItem->setSizeHint(0, QSize(150, 25));
	tree->addTopLevelItem(pItem);
	updateSumm();
}

void ZEstimatesForm::delRowSlot()
{
	QTreeWidget* tree = ui.treeWorks;
	if (sender() == ui.cmdAddPosts)
		tree = ui.treePosts;
	
	QTreeWidgetItem *curItem = tree->currentItem();
	if (!curItem)
		return;
	if (curItem->parent())
		return;

	if (QMessageBox::question(this, "Внимание", "Вы действительно хотите удалить выделенную строку?", "Да", "Нет", QString::null, 0, 1) != 0)
		return;

	delete curItem;
	updateSumm();
}

int ZEstimatesForm::loadTblInfo(const QString& tbl, QTreeWidget* tree)
{
	tree->blockSignals(true);

	tree->clear();

	QMap<int, linkInfo>* pMap = (tbl == "estimates_posts") ? &mapPosts : &mapWorks;

	QSqlQuery query;
	QString stringQuery;
	QTreeWidgetItem* pItem;

	stringQuery = QString("SELECT link_id, count, val FROM %1 WHERE estimate_id=%2").arg(tbl).arg(curEditId);
	if (!query.exec(stringQuery))
	{
		ZMessager::Instance().Message(_CriticalError, query.lastError().text(), tr("Ошибка"));
	}
	else
	{
		while(query.next())
		{
			pItem = new QTreeWidgetItem(tree);
			int idx = query.value(0).toInt();
			pItem->setData(0, Qt::UserRole, idx);
			pItem->setText(0, pMap->value(idx).name);
			int v1 = query.value(1).toInt();
			pItem->setData(1, Qt::DisplayRole, v1);
			int v2 = query.value(2).toInt();
			pItem->setData(2, Qt::DisplayRole, v2);
			pItem->setData(3, Qt::DisplayRole, v1 * v2);
			pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
			pItem->setSizeHint(0, QSize(150, 25));
			tree->addTopLevelItem(pItem);
		}
	}

	QTreeWidgetItem* pParentItem;
	int p_id, id;
	for (int i = 0; i < tree->topLevelItemCount(); i++)
	{
		pItem = tree->topLevelItem(i);
		id = pItem->data(0, Qt::UserRole).toInt();
		p_id = pMap->value(id).parent_id;
		if (p_id == 0)
			continue;

		for (int j = 0; j < tree->topLevelItemCount(); j++)
		{
			if (i == j)
				continue;
			pParentItem = tree->topLevelItem(j);
			if (pParentItem->data(0, Qt::UserRole).toInt() == p_id)
			{
				pItem = tree->takeTopLevelItem(i);
				pItem->setText(1, "");
				pParentItem->insertChild(pParentItem->childCount(), pItem);
				i--;
				break;
			}
		}
	}
	tree->expandAll();
	tree->blockSignals(false);
	return 1;
}

void ZEstimatesForm::itemChangedSlot(QTreeWidgetItem* item, int column)
{
	QTreeWidget* tree = dynamic_cast<QTreeWidget*>(sender());
	if (!tree || column != 0)
	{
		updateSumm();
		return;
	}

	while(item->childCount())
		delete item->takeChild(0);

	int indx = item->data(0, Qt::UserRole).toInt();
	if (indx == 0)
		return;

	QMap<int, linkInfo>* pMap = (tree == ui.treePosts) ? &mapPosts : &mapWorks;
	QMap<int, linkInfo>::const_iterator iT = pMap->constBegin();
	while (iT != pMap->constEnd())
	{
		auto v = iT.value();
		if (v.parent_id == indx)//добавляю!
		{
			QTreeWidgetItem* pItem = new QTreeWidgetItem(item);
			pItem->setData(0, Qt::UserRole, v.id);
			pItem->setText(0, v.name);
			pItem->setData(2, Qt::DisplayRole, 0);
			pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
			pItem->setSizeHint(0, QSize(150, 25));
			item->addChild(pItem);
		}
		
		++iT;
	}

	tree->expandAll();
}
//////////////////////////////////////////////////////////////////////////////////////////////

ZEstimatesTreeDelegate::ZEstimatesTreeDelegate(ZEstimatesForm* p, const QString& tbl, QObject* parent)
	: QItemDelegate(parent), pParent(p), mTblName(tbl)
{
}

QWidget* ZEstimatesTreeDelegate::createEditor(QWidget* parent,
	const QStyleOptionViewItem& option,
	const QModelIndex& index) const
{
	if (ZSettings::Instance().f_ReadOnly || ZSettings::Instance().m_UserType == 1)
		return NULL;

	int column = index.column();

	QModelIndex p = index.parent();
	if(p.isValid() && column!=2)
		return NULL;

	if (column == 0)
	{
		QComboBox *w = new QComboBox(parent);
		w->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		return w;
	}
	if (column == 1 || column == 2)
	{
		QSpinBox *w = new QSpinBox(parent);
		w->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		w->setMinimum(0);
		w->setMaximum(1000000);
		return w;
	}

	return NULL;
}

void ZEstimatesTreeDelegate::setEditorData(QWidget* editor,
	const QModelIndex& index) const
{
	QSpinBox* spin = dynamic_cast<QSpinBox*>(editor);
	if (spin)
	{
		spin->setValue(index.model()->data(index, Qt::DisplayRole).toInt());
		return;
	}
	QComboBox* cbo = dynamic_cast<QComboBox*>(editor);
	if (cbo)
	{
		loadItemsToComboBox(cbo, mTblName, "parent_id=0");
		int idx = index.model()->data(index, Qt::UserRole).toInt();
		idx = cbo->findData(idx);
		cbo->setCurrentIndex(idx);
		return;
	}
	QItemDelegate::setEditorData(editor, index);
}

void ZEstimatesTreeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
	const QModelIndex& index) const
{
	QSpinBox* spin = dynamic_cast<QSpinBox*>(editor);
	if (spin)
	{
		int v = spin->value();
		model->setData(index, v, Qt::EditRole);

		int column = index.column() == 1 ? 2 : 1;
		int v2 = model->data(model->index(index.row(), column, index.parent()), Qt::DisplayRole).toInt();
		model->setData(model->index(index.row(), 3, index.parent()), v * v2, Qt::DisplayRole);
		return;
	}
	QComboBox* cbo = dynamic_cast<QComboBox*>(editor);
	if (cbo)
	{
		QString txt = cbo->currentText();
		int indx = cbo->itemData(cbo->findText(txt), Qt::UserRole).toInt();

		model->setData(index, txt, Qt::EditRole);
		model->setData(index, indx, Qt::UserRole);
		return;
	}
	QItemDelegate::setModelData(editor, model, index);
}

void ZEstimatesTreeDelegate::updateEditorGeometry(QWidget* editor,
	const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	editor->setGeometry(option.rect);
}
