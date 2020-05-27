#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "create_bd.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
// Создаем новую базу данных
void MainWindow::on_create_clicked()
{
    Create_BD *bd = new Create_BD;
    bd->setModal(true);
    bd->exec();
}
