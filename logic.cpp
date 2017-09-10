#include "logic.h"
#include "mainwindow.h"
#include <QPixmap>
#include <vector>
#include <cstring>
#include "board.h"
Logic::Logic(MainWindow *window) : QObject(window){
    player = 0;
    this -> window = window;
    tested = 0;
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
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
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
    int myP = 0, myK = 0, eP = 0,eK = 0;
    for (int i = 0;i < 10;i++)
        for (int j = 0;j < 10;j++){
            if (grid[i][j] == player) myP++;
            else if (grid[i][j] == player * 2) myK++;
            else if (grid[i][j] == player * (-1)) eP++;
            else if (grid[i][j] == player * (-2)) eK++;
        }
    window->ui->lcdEnemySec->display(eP);
    window->ui->lcdEnemyMin->display(eK);
    window->ui->lcdMyMin->display(myK);
    window->ui->lcdMySec->display(myP);
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
    Operation op = game->getOperation();
    for (int pos:op.ops){
        int x= pos / 10, y = pos % 10;
        if (player == 1) x = 9 - x, y = 9 - y;
        window->grid[x][y]->setStyleSheet(operatedStyle);
    }
}

void Logic::recieve(MessageType type, QString &str){
    if (type == SurrenderInfo){
        startGame(-player);
        showMessageBox("You Win! You Opponent Signed!", window);
    }else if (type == DrawGameInfo){

        QMessageBox cur(QMessageBox::Question, "Draw Game", "Your Opponent Want A Draw Game. Do You Agree?",QMessageBox::Yes | QMessageBox::No);
        cur.setButtonText(QMessageBox::Yes, "Yes");
        cur.setButtonText(QMessageBox::No, "No");
        cur.show();
        int type = cur.exec();
//        showMessageBox(QString::number(type), window);
        if (type == 16384){
            startGame(-player);
            socketSend(AcceptDraw,"Accepted!");
            showMessageBox("Draw Accepted! Draw Game!", window);
        }else{
            socketSend(RejectDraw,"Rejected!");
        }
    }else if (type == AcceptDraw){
        startGame(-player);
        showMessageBox("Draw Game", window);
    }else if (type == RejectDraw){
        showMessageBox("Draw Game Request Rejected!", window);
    }else if (type == StepInfo){
        //收到对方的信号，此时正值对方星期，否则忽略信号。
        // 忽略这个信号
        // 这个信号目前只用于检测输赢。。
        if (player == -currentPlayer){
            if (game->getAvailablePos().size() == 0){
                startGame(-player);
                socketSend(LostInfo, "Lost : Win");
                showMessageBox("Sorry,You Lost The Game");
            }
        }
    }else if (type == LostInfo){
        startGame(-player);
        showMessageBox("You Win ! Good Game!", window);
    }else if (type == TimeInfo){
        enemyTime = str.mid(0, 6).toInt() - 100000;
        myTime = str.mid(6).toInt() - 100000;
    }else if (type == TestInfo){
        test();
    }else if (type == PressInfo){
        numberPressed(str.toInt());
    }
}

void Logic::drawGame(){
    socketSend(DrawGameInfo, "Draw");
}

void Logic::surrender(){
    if (window -> isConnected()){
        socketSend(SurrenderInfo, "Surrender");
        showMessageBox("Surrendered, You lose", window);
        startGame(-player);
    }
}

void Logic::socketSend(MessageType type, QString info)
{
    // 忽略掉所有的时间信息
    if (type == TimeInfo) return;
    QByteArray *array = new QByteArray;
    array->clear();
    array->append(QString::number((int)type)+":"+info);
    window->socket->write(array->data());
    window->ui->chatBrowser->append("Send  "+QString::number(type)+":"+info);
}

void Logic::startGame(int player){
    memset(grid, 0, sizeof(grid));
    //timer -> start(1000);
    myTime = enemyTime = 0;
    this -> player = player;
    for (int i = 0;i <= 3;i++){
        for (int j = 0;j < 10;j++)
            if (i+j&1) grid[i][j] = 1;
    }

    //timer -> start();
    for (int i = 6;i < 10;i++){
        for (int j = 0;j < 10;j++)
            if (i+j&1) grid[i][j] = -1;
    }
    for (int i = 0;i < 10;i++){
        for (int j = 0;j < 10;j++)
            if ((i+j)%2==0) grid[i][j] = 3;
    }
    currentPlayer = 1;
    game->build();
    updateFrame();
}

void Logic::timeout(){
    // This Function Is Banned From now on
    return;

    if (currentPlayer == player){
        myTime++;
        window->ui->lcdMyMin->display(myTime / 60);
        window->ui->lcdMySec->display(myTime % 60);
    }else{
        enemyTime++;
        window->ui->lcdEnemyMin->display(enemyTime / 60);
        window->ui->lcdEnemySec->display(enemyTime % 60);
    }
    if (window->networkStatus==MainWindow::ConnectedServer){
        socketSend(TimeInfo, QString::number(100000 + myTime) + QString::number(100000 + enemyTime));
    }
}

void Logic::numberPressed(int num){
    if (!window->isConnected()) return;
    qDebug("NumberPress: %d , sender : %d",num, this->sender());
    if (this->sender() && currentPlayer == player){
        socketSend(PressInfo, QString::number(99-num));
    }
    if (currentPlayer == player || this->sender()==0){
        qDebug("%d,%d\n", num/10, num%10);
        if (player == 1) num = 99 - num;
        if (game->pushPos(num)){
            QString cur = game->getOperation().toString();
            game->processOperation(game->getOperation());
            game->build();
            updateFrame();
            QApplication::beep();
            if (game->getAvailablePos().size() == 0){
                if (player == currentPlayer){
                    startGame(-player);
                    showMessageBox("Sorry, You Lost", window);
                }else{
                    startGame(-player);
                    showMessageBox("Congratulations!, You Won!", window);
                }
            }
        }
    }
}

void Logic::test(){
    if (tested != 1){
        tested = 1;
        socketSend(TestInfo, "Test");
    }else{
        return;
    }
    showMessageBox("Test Mode On!", window);
    currentPlayer = -1;
    for (int i = 0;i < 10;i++)
        for (int j = 0;j < 10;j++)
            grid[i][j] = i+j&1?0:3;
    grid[1][2] = 1; grid[1][6] = -1;
    grid[1][8] = -1; grid[2][9] = 1;
    grid[3][2] = 1; grid[4][3] = -1;
    grid[5][2] = 1; grid[6][5] = 1;
    grid[7][2] = 1; grid[8][5] = -1;
    game->build();
    updateFrame();
}
