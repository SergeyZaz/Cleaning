#include "zpayments2fio.h"
#include "zpayments2fioform.h"
#include "zparsexlsxfile.h"
#include "zsettings.h"
#include <QFileDialog>


ZPayments2fioBase::ZPayments2fioBase(QWidget* parent, int mode, Qt::WindowFlags flags) : QWidget(parent, flags)
{
	mMode = mode;
	filterOrganisationId = -1;
	filterPaymentId = -1;

	ui.setupUi(this);
	setWindowTitle((mMode == 0) ? "Доплаты" : "Вычеты");
	ui.lblPayment->setText((mMode == 0) ? "Доплата:" : "Вычет:");

	loadItemsToComboBox(ui.cboPeriod, "periods");
	filterPeriodId = ZSettings::Instance().m_PeriodId;
	ui.cboPeriod->setCurrentIndex(ui.cboPeriod->findData(filterPeriodId));

	loadItemsToComboBox(ui.cboFilter1, "organisation");
	ui.cboFilter1->setCurrentIndex(0);

	loadItemsToComboBox(ui.cboFilter2, "payments");
	ui.cboFilter2->setCurrentIndex(0);

	ui.cmdImport->setDisabled(true);// !!! убрать

	connect(ui.m_tbl, SIGNAL(needUpdateVal(int)), this, SLOT(UpdateSumma(int)));
	connect(ui.cmdImport, SIGNAL(clicked()), this, SLOT(ImportSlot()));
	connect(ui.m_tbl, SIGNAL(needUpdate()), this, SLOT(ChangeFilter()));

	connect(ui.cboFilter1, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index)
		{
			filterOrganisationId = ui.cboFilter1->currentData().toInt();
			ChangeFilter();
		});
	connect(ui.cboFilter2, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index)
		{
			filterPaymentId = ui.cboFilter2->currentData().toInt();
			ChangeFilter();
		});
	connect(ui.cboPeriod, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index)
		{
			filterPeriodId = ui.cboPeriod->currentData().toInt();
			ChangeFilter();
		});
}

void ZPayments2fioBase::ChangeFilter()
{
	QList<int> hideColumns;
	QStringList headers;

	hideColumns << 0 << 2 << 3 << 8;
	headers << tr("id") << tr("Дата") << tr("fio_id") << tr("payment_id") << tr("ФИО") << tr("Организация") << tr("Выплата") << tr("Значение") << tr("Период-id") << tr("Период") << tr("Заметка");

	QString query = QString("SELECT p.id, p.dt, p.fio, p.payment, fio.name, organisation.name, payments.name, p.val, p.period, periods.name, p.comment FROM payments2fio AS p \
INNER JOIN fio ON(p.fio = fio.id) \
INNER JOIN payments ON(p.payment = payments.id) \
LEFT JOIN organisation2fio ON(p.fio = value) \
LEFT JOIN periods ON(p.period = periods.id) \
LEFT JOIN organisation ON(organisation2fio.key = organisation.id) \
WHERE payments.mode=%1 ").arg(mMode);

	if (filterPeriodId > 0)
		query += QString("AND p.period = %1").arg(filterPeriodId);
	if (filterOrganisationId > 0)
		query += QString("AND organisation.id = %1").arg(filterOrganisationId);
	if (filterPaymentId > 0)
		query += QString("AND payments.id = %1").arg(filterPaymentId);

	ui.m_tbl->setQuery(query, headers);

	ui.m_tbl->init(hideColumns);

	ui.m_tbl->setReadOnly(false, false, false);

	ui.m_tbl->setColorHighligthIfColumnContain(8, 0, QColor(128, 255, 128));
	ui.m_tbl->setColorHighligthIfColumnContain(8, 1, QColor(255, 128, 128));
	
	connect(ui.m_tbl->getTblView()->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		this, SLOT(SelectionChanged(const QItemSelection&, const QItemSelection&)));


	UpdateSumma();
}

void ZPayments2fioBase::dateChangedSlot(const QDate &date)
{
	filterDate = date;
	ChangeFilter();
}

void ZPayments2fioBase::init(const QString& m_TblName)
{
	ui.m_tbl->setTable(m_TblName);
	ui.m_tbl->setCustomEditor(new ZPayments2FioForm(this, mMode));

	ChangeFilter();

	ui.m_tbl->changeFilter(1);
}

void ZPayments2fioBase::UpdateSumma(int)
{
	QString s;
	double summa = 0;
	int i, n = ui.m_tbl->getSortModel()->rowCount();
	for (i = 0; i < n; i++)
	{
		s = ui.m_tbl->getSortModel()->data(ui.m_tbl->getSortModel()->index(i, 7)).toString();
		summa += QString2Double(s);
	}
	ui.lblSumma->setText(QString("Сумма: %L1").arg(summa, 0, 'f', 2));
}

void ZPayments2fioBase::ImportSlot()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Выбор файла для импорта", "", "XLSX-файлы (*.xlsx);;Все файлы (*.*)");
	if (fileName.isEmpty())
		return;
	ZParseXLSXFile pFile;
	if (pFile.loadPayments(fileName))
	{
		ChangeFilter();
	}
}

void ZPayments2fioBase::SelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QModelIndexList indxs = ui.m_tbl->getTblView()->selectionModel()->selectedIndexes();
	if (indxs.size() == 0)
		return;

	bool fEdit = true;
	foreach(auto indx, indxs)
	{
		QDate d = indx.model()->data(indx.model()->index(indx.row(), 9)).toDate();
		if (ZSettings::Instance().m_CloseDate.isValid() && d < ZSettings::Instance().m_CloseDate)
			fEdit = false;
	}
	ui.m_tbl->setReadOnly(!fEdit, false, !fEdit);
}
