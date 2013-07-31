#include "checkwidget.h"
#include "ui_checkwidget.h"
#include <QDebug>
#include "sqlmanager.h"
#include <QSqlRecord>
CheckWidget::CheckWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CheckWidget), m_calendar(this)
{
    ui->setupUi(this);
    m_calendar.hide();
    connect(&m_calendar, SIGNAL(clicked(QDate)), this, SLOT(getDate(QDate)));
    ui->lineEdit->installEventFilter(this);
    loadMember();
    //设置表头
    ui->tableView->setModel(&m_model);
    setTableHeader();
    //设置默认日期
    ui->dateTimeEdit_Start->setDate(QDate::currentDate());
    ui->dateTimeEdit_End->setDate(QDate::currentDate().addDays(1));
    ui->lineEdit->setVisible(false);
    m_calendar.setVisible(false);
    ui->pushButton_2->setEnabled(false);
}

CheckWidget::~CheckWidget()
{
    delete ui;
}

bool CheckWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->lineEdit && event->type() == QEvent::FocusIn)
    {
        m_calendar.raise();
        m_calendar.show();
        m_calendar.move(ui->lineEdit->x(), ui->lineEdit->y()+ ui->lineEdit->height());
        m_calendar.resize(248,169);
        return true;
    }
    return false;
}

void CheckWidget::getDate(const QDate &date)
{
    m_calendar.hide();
    m_Date=date.toString("yyyyMMdd");
    m_Date.append(QTime::currentTime().toString("hhmm"));
    ui->lineEdit->setText(m_Date);
}

void CheckWidget::loadMember()
{
    ui->comboBox->insertItem(ui->comboBox->count(), "全部", 0);
    QString sql = tr("select userid, nickname from userinfo");
    QSqlQuery *query = getSqlManager()->ExecQuery(sql);
    if(query)
    {
        while(query->next())
        {
            ui->comboBox->insertItem(ui->comboBox->count(), query->value(1).toString(), query->value(0));
        }
    }
    ui->comboBox->setCurrentIndex(0);
}

void CheckWidget::setTableHeader()
{


}

void CheckWidget::calcTotal()
{
    QString text;
    if(ui->comboBox->currentText().compare("全部") != 0)
    {
        text.append(tr("服务员：<font size='6' color='red'><b>%1    </b></font>").arg(ui->comboBox->currentText()));
    }
    //text.append(tr("日期:<font size='6' color='red'><b>%1    </b></font>    ").arg(ui->lineEdit->text()));
    text.append(tr("日期:<font size='4' color='red'>%1<br>至%2</font>    ")
                .arg(ui->dateTimeEdit_Start->dateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(ui->dateTimeEdit_End->dateTime().toString("yyyy-MM-dd hh:mm:ss")));
    double paid = 0;
    for(int i = 0; i < m_model.rowCount(); i++)
    {
        paid += m_model.record(i).value(2).toDouble();
    }
    text.append(tr("营业额:<font size='6' color='red'><b>%1</b></font>元").arg(paid));

    ui->label_3->setText(text);
}

void CheckWidget::on_pushButton_clicked()
{
    ui->pushButton_2->setEnabled(true);
    QDate date = QDate::fromString(ui->lineEdit->text(), "yyyy-MM-dd");
    if(!date.isValid())
    {
        date = QDate::currentDate();
    }
    QDate lastDate = date.addDays(1);
    if(ui->dateTimeEdit_Start->dateTime().toTime_t() > ui->dateTimeEdit_End->dateTime().toTime_t())
    {
        return;
    }
    //    QString sql = tr("select orderid, accounts, paid, tableid,  begintime, userinfo.nickname from orderinfo "\
    //            "LEFT JOIN userinfo on userinfo.userid = orderinfo.userid "\
    //                     "where begintime > '%1' and begintime <= '%2' ").arg(date.toString("yyyy-MM-dd")).arg(lastDate.toString("yyyy-MM-dd"));
    QString sql = tr("select orderid, accounts, paid, tableid,  begintime, userinfo.nickname from orderinfo "\
                     "LEFT JOIN userinfo on userinfo.userid = orderinfo.userid "\
                     "where begintime > '%1' and begintime <= '%2' ").arg(ui->dateTimeEdit_Start->dateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(ui->dateTimeEdit_End->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    int userId = ui->comboBox->itemData(ui->comboBox->currentIndex()).toInt();
    if(userId > 0)
    {
        sql.append(tr(" and orderinfo.userid = %1").arg(userId));
    }
    QSqlQuery query(sql, *getSqlManager()->getdb());
    m_model.setQuery(query);
    m_model.setHeaderData(0, Qt::Horizontal, tr("订单号"));
    m_model.setHeaderData(1, Qt::Horizontal, tr("应收"));
    m_model.setHeaderData(2, Qt::Horizontal, tr("实收"));
    m_model.setHeaderData(3, Qt::Horizontal, tr("桌号"));
    m_model.setHeaderData(4, Qt::Horizontal, tr("下单时间"));
    m_model.setHeaderData(5, Qt::Horizontal, tr("服务员"));
    calcTotal();

}

void CheckWidget::on_tableView_doubleClicked(const QModelIndex &index)
{
    QString orderId = m_model.record(index.row()).value(0).toString();
    QString sql = tr("select dishes.dishesname, orderdetail.dishescount, dishes.price, orderdetail.dishestype from orderdetail " \
                     "LEFT JOIN dishes on orderdetail.dishesid = dishes.dishesid where orderid = '%1'").arg(orderId);
    m_detail.showDetail(sql);
}

void CheckWidget::on_pushButton_2_clicked()
{
    if(m_Date.isEmpty())
        m_Date=QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    m_excelInstance.Open(QApplication::applicationDirPath()+"/"+m_Date+".xls");
    m_excelInstance.SaveDataFrTable(ui->tableView);
    m_excelInstance.Close();
    ui->pushButton_2->setEnabled(false);
}
