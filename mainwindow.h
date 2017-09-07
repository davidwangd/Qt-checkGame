#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
#include "logic.h"
#include "const.h"
#include <QSignalMapper>
#include <QTcpServer>
#include <QTcpSocket>
#include <QEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class Logic;
    friend class BoardGame;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool eventFilter(QObject *, QEvent *);
    int isConnected();
public slots:
    void newGame();
    void newConnection();
    void connectToGame();
    void readSocket();
    void sendMessage();
    void disconnect();
    // process the package recieved by the network
    // return the value of package kind.
    int processPackage(QString &info);
private:
    Ui::MainWindow *ui;
    Logic *logic;
    QPushButton* grid[12][12];
    QTimer *timer;
    enum{
        None,
        Client,
        Server,
        ConnectedServer,
        ConnectedClient
    }networkStatus;
    QSignalMapper *mapper;
    QTcpSocket *socket;
    QTcpServer *server;
};

#endif // MAINWINDOW_H
