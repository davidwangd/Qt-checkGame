#include "logic.h"
#include "mainwindow.h"
#include <QPixmap>
#include <vector>
#include "board.h"
Logic::Logic(MainWindow *window) : QObject(window){
    player = 0;
    this -> window = window;
    icons = __ + 3;
    icons[0] = QIcon(QPixmap(":/5.png"));
    icons[1] = QIcon(QPixmap(":/1.png"));
    icons[2] = QIcon(QPixmap(":/6.png"));
    icons[-1] = QIcon(QPixmap(":/2.png"));
    icons[-2] = QIcon(QPixmap(":/4.png"));
    icons[3] = QIcon(QPixmap(":/3.png"));
    for (int i = 0;i < 10;i++){
        for (int j = 0;j < 10;j++){
            grid[i][j] = (i+j)&1?0:3;
        }
    }
    game = new BoardGame(this);
    game->build();
}

void Logic::updateFrame(){
    qDebug("Update Frame()!");
    for (int i = 0;i < 10;i++){
        for (int j = 0; j < 10;j++){
            window -> grid[i][j] -> setIconSize(QSize(SIZE-4, SIZE-4));
            window -> grid[i][j] -> setStyleSheet(defaultStyle);
            if (player == -1){
                window -> grid[i][j] -> setIcon(icons[grid[i][j]]);
            }else{
                window -> grid[i][j] -> setIcon(icons[grid[9-i][9-j]]);
            }
        }
    }
    std::vector<int> available = game->getAvailablePos();
    for (int num : available){
        int x = num / 10;
        int y = num % 10;
        if (player == -1){
            window->grid[x][y]->setStyleSheet(availableStyle);
        }else{
            window->grid[9-x][9-y]->setStyleSheet(availableStyle);
        }
    }
}

void Logic::recieve(MessageType type, QString &str){
    if (type == SurrenderInfo){
        showMessageBox("You Win! You Opponent Signed!", window);
        startGame(-player);
    }else if (type == DrawGameInfo){
        int type = showMessageBox("Your opponent Want a Draw Game!", window);
        if (type == QMessageBox::Accepted){
            socketSend(AcceptDraw,"Accepted!");
            showMessageBox("Draw Accepted! Draw Game!", window);
            startGame(-player);
        }else{
            socketSend(RejectDraw,"Rejected!");
        }
    }else if (type == AcceptDraw){
        showMessageBox("Draw Game", window);
        startGame(-player);
    }else if (type == RejectDraw){
        showMessageBox("Draw Game Request Rejected!", window);
    }else if (type == StepInfo){
        //收到对方的信号，此时正值对方星期，否则忽略信号。
        if (player == -currentPlayer){
            game->processOperation(Operation(str));
            game->build();
        }
    }
}

void Logic::drawGame(){
    socketSend(DrawGameInfo, "Draw");
}

void Logic::surrender(){
    socketSend(SurrenderInfo, "Surrender");
    showMessageBox("Surrendered, You lose", window);
    startGame(-player);
}

void Logic::socketSend(MessageType type, QString info)
{
    QByteArray *array = new QByteArray;
    array->clear();
    array->append(QString::number((int)type)+":"+info);
    window->socket->write(array->data());
}

void Logic::startGame(int player){
    this -> player = player;
    for (int i = 0;i <= 3;i++){
        for (int j = 0;j < 10;j++)
            if (i+j&1) grid[i][j] = 1;
    }
    for (int i = 6;i < 10;i++){
        for (int j = 0;j < 10;j++)
            if (i+j&1) grid[i][j] = -1;
    }
    currentPlayer = 1;
    game->build();
    updateFrame();
}

void Logic::numberPressed(int num){
    int x, y;
    if (currentPlayer == player){
        qDebug("%d,%d\n", num/10, num%10);
        if (player == 1) num = 99 - num;
        if (game->pushPos(num)){
            QString cur = game->getOperation().toString();
            socketSend(StepInfo, cur);
            game->processOperation(game->getOperation());
            game->build();
            updateFrame();
        }
    }
}
