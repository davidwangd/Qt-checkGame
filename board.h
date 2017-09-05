#ifndef BOARD_H
#define BOARD_H
#include "logic.h"
#include <vector>
#include <QString>
#include "ui_mainwindow.h"
class Operation{
public:
    std::vector<int> ops;
    Operation(){}
    Operation(QString &str){
        fromString(str);
    }
    QString toString() const{
        QString ret = QString();
        for (int i : ops){
            ret = ret + QString::number(i+100);
        }
        return ret;
    }
    void fromString(QString &str){
        ops.clear();
        for (int i = 0;i < str.length(); i += 3){
            ops.push_back(str.mid(i, 3).toInt() - 100);
        }
    }
};

struct TreeNode{
    int previous;
    int current, maxCount;
    int chessman, tic;
    std::vector<TreeNode*> children;
    TreeNode* parent;
    TreeNode(TreeNode* parent, int previous): parent(parent), previous(previous){
        if (parent != nullptr){
            current = parent -> current + 1;
            maxCount = current;
            chessman = parent -> chessman;
        }else{
            current = 0;
            maxCount = 0;
        }
        tic = -1;
        children.clear();
    }
    void clear(){
        for (TreeNode* node: children){
            delete node;
        }
        children.clear();
    }
    ~TreeNode(){
        clear();
        parent = nullptr;
    }
};

inline int getPos(int x, int y){
    return x * 10 + y;
}
inline void getXY(int pos, int &x, int &y){
    x = pos / 10;
    y = pos % 10;
}

class BoardGame{
public:
    BoardGame(Logic *logic);
    // return whether the game is ended.
    int pushPos(int pos);
    const Operation& getOperation(){
        return op;
    }
    // return winner;
    int processOperation(const Operation &op);
    void build();
    void showDebugInfo();
    std::vector<int> getAvailablePos();
private:
    int TryPush(TreeNode *node, int x, int y, int flag);
    int board[12][12];
    void dfs(TreeNode *node);
    Operation op;
    Logic *logic;
    TreeNode *root;
    TreeNode *current;
};

#endif // BOARD_H
