#include "board.h"
#include "const.h"
#include "logic.h"
#include "mainwindow.h"
#include <vector>
#include <algorithm>
#include <QThread>
using namespace std;

BoardGame::BoardGame(Logic *logic){
    this -> logic = logic;
    this -> root = this -> current = new TreeNode(nullptr, -1);
}

void BoardGame::build(){
    root -> clear();
    for (int i = 0;i < 10;i++){
        for (int j = 0;j < 10;j++){
            board[i][j] = logic -> grid[i][j] * logic -> currentPlayer;
        }
    }

    root->maxCount = 0;
    current = root;
    dfs(root);
    op.ops.clear();
    showDebugInfo();
}

inline int inBoard(int x, int y){
    return 0 <= x && x <= 9 && 0 <= y && y <= 9 && (x+y)%2;
}

inline int abs(int x){
    return x < 0 ? -x : x;
}

inline int sgn(int x){
    if (x < 0) return -1;
    if (x > 0) return 1;
    return 0;
}

void BoardGame::showDebugInfo(){/*
    logic->window->ui->chatBrowser->setText("");
    for (TreeNode* x:current->children){
        logic->window->ui->chatBrowser->append(QString::number(x->previous));
    }
*/
}

int BoardGame::pushPos(int pos)
{
    for (TreeNode *node : current->children){
        if (node -> previous == pos){
            current = node;
            op.ops.push_back(node->previous);
            qDebug("Before Update Frame %d..",node->previous);
            logic->updateFrame();
            qDebug("After Update Frame %d..", node->previous);
            showDebugInfo();
            return current->children.size() == 0;
        }
    }
    if (current != root){
        op.ops.clear();
        current = root;
        pushPos(pos);
    }
    showDebugInfo();
    logic -> updateFrame();
    return 0;
}

vector<int> BoardGame::getAvailablePos(){
    vector<int> ret;
    for (TreeNode *node : current->children){
        ret.push_back(node->previous);
    }
    return ret;
}

int BoardGame::processOperation(const Operation &op)
{
    int curX, curY, tarX, tarY;
    for (int i = 1;i < op.ops.size();i++){
        getXY(op.ops[i-1],curX,curY);
        getXY(op.ops[i], tarX, tarY);
        qDebug("Begin Process((%d,%d)-(%d,%d))", curX, curY, tarX, tarY);
        for (int i = 1;i < abs(curX-tarX);i++){
            int x = curX - i * sgn(curX - tarX);
            int y = curY - i * sgn(curY - tarY);
            logic -> grid[x][y] = 0;
        }
        logic -> grid[tarX][tarY] = logic -> grid[curX][curY];
        logic -> grid[curX][curY] = 0;
        logic -> updateFrame();
    }
    logic -> currentPlayer = -logic -> currentPlayer;
    if (logic->grid[tarX][tarY] && tarX == 9) logic->grid[tarX][tarY] = 2;
    else if (logic->grid[tarX][tarY]==-1 && tarX == 0) logic->grid[tarX][tarY] = -2;
    build();
    logic->updateFrame();
    return 0;
}

int BoardGame::TryPush(TreeNode *node, int x, int y, int flag){
    if ((x+y)%2==0){
        qDebug("Error With x = %d, y = %d", x, y);
        return 0;
    }
    if (flag == 0){
        if (!inBoard(x, y) || board[x][y] != 0) return 0;
        TreeNode *cur = new TreeNode(node, getPos(x, y));

        cur -> chessman = 0;
        cur -> current = cur -> maxCount = node -> current;
        node->children.push_back(cur);
        return 1;
    }
    else{
        int pre_x, pre_y;
        getXY(node->previous, pre_x, pre_y);
        if (!inBoard(x, y) || board[x][y] != 0) return 0;
        TreeNode *cur = new TreeNode(node, getPos(x, y));
        int tic = -1;
        for (int i = 1;i < abs(x - pre_x);i++){
            int _x = x - i * sgn(x - pre_x);
            int _y = y - i * sgn(y - pre_y);
            if (board[_x][_y] < 0){
                if (tic == -1) tic = getPos(_x, _y);
                else return 0;
            }
            if (board[_x][_y] > 0) return 0;
        }
        if (tic == -1){
            return 1;
        }else{
            TreeNode *cur = new TreeNode(node, getPos(x, y));
            cur -> tic = tic;
            node->children.push_back(cur);
            return 1;
        }
    }
}

void BoardGame::dfs(TreeNode *node){
    // 从根开始
    if (node->parent == nullptr){
        for (int i = 0;i < 10;i++){
            for (int j = 1 - (i&1);j < 10;j += 2){
                if (board[i][j] > 0){
                    TreeNode *cur = new TreeNode(node, getPos(i, j));
                    cur->chessman = board[i][j];
                    dfs(cur);
                    if (cur->children.size() == 0){
                        delete cur;
                        continue;
                    }
                    if (cur->maxCount > node -> maxCount){
                        node->clear();
                        node->maxCount = cur->maxCount;
                        node->children.push_back(cur);

                 //       showMessageBox(QString::number(cur->previous)+":"+QString::number(node->previous), logic->window);
                    }
                    else if (cur -> maxCount == node -> maxCount){
                        node -> children.push_back(cur);
                    }else{
                        delete cur;
                    }
                }
            }
        }
    }else{
        //之前只经历了一步搜索，这里可以选择没有吃棋子的情况。
        int _x, _y, dir = logic -> currentPlayer;
        getXY(node->previous, _x, _y);
        if ((_x + _y)%2 == 0){
            qDebug("Fuck You x = %d, y = %d", _x, _y);
        }
        if (node -> chessman != 0){

            if (node->parent->parent == nullptr){
                if (node->chessman == 1){
                    TryPush(node, _x + dir, _y + 1, 0);
                    TryPush(node, _x + dir, _y - 1, 0);
                }else if (node -> chessman == 2){
                    for (int i = 1;i < 10;i++)
                        if (!TryPush(node, _x + i, _y + i, 0)) break;
                    for (int i = 1;i < 10;i++)
                        if (!TryPush(node, _x + i, _y - i, 0)) break;
                    for (int i = 1;i < 10;i++)
                        if (!TryPush(node, _x - i, _y + i, 0)) break;
                    for (int i = 1;i < 10;i++)
                        if (!TryPush(node, _x - i, _y - i, 0)) break;
                }
            }
            // 这里选择可以需要吃子的情况。
            if (node -> chessman == 1){
                TryPush(node, _x + 2, _y + 2, 1);
                TryPush(node, _x + 2, _y - 2, 1);
                TryPush(node, _x - 2, _y + 2, 1);
                TryPush(node, _x - 2, _y - 2, 1);
            }else{
                for (int i = 2;i < 10;i++)
                    if (!TryPush(node, _x + i, _y + i, 1));
                for (int i = 2;i < 10;i++)
                    if (!TryPush(node, _x + i, _y - i, 1));
                for (int i = 2;i < 10;i++)
                    if (!TryPush(node, _x - i, _y + i, 1));
                for (int i = 2;i < 10;i++)
                    if (!TryPush(node, _x - i, _y - i, 1));
            }

            // 统一的进行DFS处理。
            for (TreeNode *child : node -> children){
                if (child -> tic != -1){
                    int x, y;
                    int x1, x2, y1, y2;

                    getXY(child->tic, x, y);
                    getXY(child->previous, x2, y2);
                    getXY(node->previous, x1, y1);
                    // 行走过程中
                    int prev = board[x][y];
                    board[x][y] = 10;
                    board[x2][y2] = board[x1][y1];
                    board[x1][y1] = 0;
                    dfs(child);
                    board[x][y] = prev;
                    board[x1][y1] = board[x2][y2];
                    board[x2][y2] = 0;
                    node -> maxCount = max(node->maxCount, child -> maxCount);
                }
            }
            // 统一DFS之后.. 然后修复所有的问题。
            for (int i = 0;i < node -> children.size();i++){
                if (node->children[i]->maxCount < node->maxCount){
                    delete node->children[i];
                    node->children.erase(node->children.begin() + i);
                    i--;
                }
            }
        }
    }
    if (node != root){
    //   qDebug("After DFS Finished : %d Previous Position %d", node->parent->previous,node->previous);
    }
}
