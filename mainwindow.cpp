#include "mainwindow.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tcp = new TcpConnection(this, ui->plainTextEdit);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::doit);
    tcp->connectToHost("31.131.31.107");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::doit()
{
    tcp->registerAccount();
}
