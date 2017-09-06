#ifndef CONST_H
#define CONST_H
#include <QString>
#include <QMessageBox>
#include <QErrorMessage>

// This Message Type is 4 characters definitely.
enum MessageType{
    ChatInfo = 2492,
    StepInfo = 9932,
    DrawGameInfo = 2242,
    SurrenderInfo = 3552,
    TimeInfo = 9933,
    GameStartInfo = 3322,
    AcceptDraw = 2111,
    RejectDraw = 1977,
    LostInfo = 1772,
    TestInfo = 8765
};
const int SIZE = 70;
const QString availableStyle = "border-style:solid;border-width:2px;border-color:red;";
const QString operatedStyle = "border-style:solid;border-width:2px;border-color:black;";
const QString defaultStyle = "";
const int PORT = 8889;

inline int showMessageBox(QString str, QWidget *parent = nullptr){
    QMessageBox box(parent);
    box.setText(str);
    box.show();
    return box.exec();
}

inline int showErrorMessage(QString str, QWidget *parent = nullptr){
    QErrorMessage msg(parent);
    msg.showMessage(str);
    msg.show();
    return msg.exec();
}

#endif // CONST_H
