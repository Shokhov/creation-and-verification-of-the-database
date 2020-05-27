#include "create_bd.h"
#include "ui_create_bd.h"

#include <QTreeWidgetItem>
#include <QMessageBox>

extern "C"
{
    #include <bd_bin.c>
}

Create_BD::Create_BD(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Create_BD)
{
    ui->setupUi(this);
    ui_struct_bd = new QTreeWidgetItem();

    ui->tree_bd->clear(); // Очиаем дерево
}

Create_BD::~Create_BD()
{
    delete ui;
    delete ui_struct_bd;
}
// Задаем имя БД
void Create_BD::on_push_name_bd_clicked()
{

    struct BD_BIN *bd=NULL;

    int a = bd_new(&bd,"test");

//    switch(0){
//        case 0: QMessageBox::about(this,"Заголовок", "Работает"); break;
//        case -1: QMessageBox::warning(this,"Заголовок", "НЕ работает");break;
//    }
}
