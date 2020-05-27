#ifndef CREATE_BD_H
#define CREATE_BD_H

#include <QDialog>
#include <QTreeWidgetItem>

namespace Ui {
class Create_BD;
}

class Create_BD : public QDialog
{
    Q_OBJECT

public:
    explicit Create_BD(QWidget *parent = 0);
    ~Create_BD();

private slots:
    void on_push_name_bd_clicked();

private:
    Ui::Create_BD *ui;
    QTreeWidgetItem *ui_struct_bd;
};

#endif // CREATE_BD_H
