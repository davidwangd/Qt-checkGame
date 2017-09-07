#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "const.h"
#include "logic.h"
#include <QErrorMessage>
#include <QByteArray>
#include <QInputDialog>
#include <QtNetwork>
#include <QKeyEvent>
#include <QMessageBox>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    logic = new Logic(this);
    timer = new QTimer(this);
    mapper = new QSignalMapper(this);
    for (int i = 0;i < 10;i++){
        for (int j = 0;j < 10;j++){
            grid[i][j] = new QPushButton(ui->frame);
            grid[i][j]->setGeometry(j*SIZE, i*SIZE, SIZE, SIZE);
            grid[i][j]->setIconSize(QSize(SIZE, SIZE));
            grid[i][j] -> show();
            connect(grid[i][j], SIGNAL(clicked()), mapper, SLOT(map()));
            mapper->setMapping(grid[i][j], i*10+j);
        }
    }
    connect(mapper, SIGNAL(mapped(int)), logic, SLOT(numberPressed(int)));
    connect(ui->actionNewGame, SIGNAL(triggered()), this, SLOT(newGame()));
    connect(ui->actionConnectToGame, SIGNAL(triggered()), this, SLOT(connectToGame()));
    connect(ui->btnSend, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->actionTest, SIGNAL(triggered()), logic, SLOT(test()));
    connect(ui->btnDrawGame, SIGNAL(clicked()), logic, SLOT(drawGame()));
    connect(ui->btnSurrender, SIGNAL(clicked()), logic, SLOT(surrender()));
    ui->lineEdit->installEventFilter(this);
    logic -> updateFrame();
    networkStatus = None;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newGame(){

    if (networkStatus != Server){
        QInputDialog dialog(this);
        dialog.setInputMode(QInputDialog::TextInput);
        dialog.setTextValue("8888");
        dialog.setLabelText("Please Select The Port You Want!");
        dialog.show();
        if (dialog.exec() != QInputDialog::Accepted){
            return;
        }
        QString str = dialog.textValue();
        networkStatus = Server;
        server = new QTcpServer();
        server->listen(QHostAddress::Any, str.toInt());
        int cur = server->isListening();
        if (cur){
            QString localHostName = QHostInfo::localHostName();
            QHostInfo info = QHostInfo::fromName(localHostName);
            QString address;
            foreach(QHostAddress add, info.addresses()){
                if (add.protocol() == QAbstractSocket::IPv4Protocol)
                    address = add.toString();
            }
            showErrorMessage("Successfully Build: Your Address :\n" + address + ":" + str, this);
            QObject::connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
            return;
        }
    }
    showMessageBox("Unsuccessfuly Connected!", this);
}

void MainWindow::newConnection(){
    qDebug("Connect!!1");
    if (networkStatus == Server){
        socket = server->nextPendingConnection();
        networkStatus = ConnectedServer;

        connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(disconnect()));
        logic -> startGame(-1);
        showMessageBox("Connected!, Game Start!", this);
    }
}

int MainWindow::processPackage(QString &info)
{
    QString start = info.mid(0, 4);
    QString mid = info.mid(4, 1);
    QString last = info.mid(5);
    int num = start.toInt();
    if (mid != QString(":")){
        num = -1;
    }
    info = last;
    ui->chatBrowser->append("Recieve: "+start+mid+info);
    return num;
}

int MainWindow::isConnected(){
    return networkStatus == ConnectedClient || networkStatus == ConnectedServer;
}

void MainWindow::readSocket(){
    QString info;
    info += socket->readAll();

    MessageType num = (MessageType)processPackage(info);
    if (num == ChatInfo){
        ui->chatBrowser->append("Ohter: " + info);
    }else{
        if (num == -1) return;
        logic -> recieve(num, info);
    }
}

void MainWindow::sendMessage(){
    if (networkStatus == ConnectedClient || networkStatus == ConnectedServer){
        QByteArray *array = new QByteArray;
        array->clear();
        array->append(QString::number(ChatInfo)+":"+ui->lineEdit->text());
        socket->write(array->data());
        ui->chatBrowser->append("I: "+ui->lineEdit->text());
        ui->lineEdit->clear();
    }else{
        showMessageBox("You have to Connect To The Server or Build a Game Wait for connect!", this);
        ui->lineEdit->clear();
    }
}

void MainWindow::connectToGame(){
    qDebug("Connect To Game");
    socket = new QTcpSocket;
    QInputDialog dialog;
    dialog.setLabelText("Please Input the String Of your friend's server message");
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.show();
    networkStatus = Client;
    if (dialog.exec()==QInputDialog::Accepted){
        QString cur = dialog.textValue();
        QStringList list = cur.split(':');

        socket->connectToHost(QHostAddress(list.at(0)), list.at(1).toInt());
        int status = socket->waitForConnected();
        if (status){
            connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
            showMessageBox("Successfully Connected: ", this);
            networkStatus = ConnectedClient;
            logic -> startGame(1);
            connect(socket, SIGNAL(disconnected()), this, SLOT(disconnect()));
        }else{
            networkStatus = None;
            showMessageBox("Unsuccessfully Connect, Please Check the IP you input or Build a game", this);
        }
    }
}

bool MainWindow::eventFilter(QObject *object, QEvent *event){
    if (object == ui->lineEdit && event -> type() == QEvent::KeyPress){
        QKeyEvent *key = (QKeyEvent *)event;
        if (key -> key() == Qt::Key_Return || key->key() == Qt::Key_Enter){
            sendMessage();
            return true;
        }
        return false;
    }
    return false;
}

void MainWindow::disconnect(){
    showMessageBox("Disconnect! Restart the game Please", this);
    networkStatus = None;
}
