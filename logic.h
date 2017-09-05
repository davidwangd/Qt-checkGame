#ifndef LOGIC_H
#define LOGIC_H
class MainWindow;
#include <QObject>
#include <QIcon>
#include "const.h"
class BoardGame;
class Logic : public QObject{
    Q_OBJECT
    friend class MainWindow;
    friend class BoardGame;
public:
    Logic(MainWindow *window);
    void updateFrame();
    void socketSend(MessageType type, QString info);
public slots:
    void surrender();
    void drawGame();
    void numberPressed(int num);
    void recieve(MessageType type, QString &str);
    void startGame(int player = 1);
private:
    QIcon __[10];
    int grid[12][12];
    QIcon *icons;
    MainWindow* window;
    BoardGame* game;
    int player;
    int myTime, enemyTime;
    int currentPlayer;
};

#endif // LOGIC_H
