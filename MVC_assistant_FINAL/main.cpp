#include "controller.h"
#include "view.h"

#include <QApplication>

//PROBLEMS:
//SOLVED1. receive zone: ASC HEX translate? specify
//SOLVED: 2. sending big file larger than 42kb, imcomplete receive, method used: sending in segment extend buffer( which is larger than the file);
//SOLVED: 3. marks select
//4. when sending large file, add sth unkown, receive larger than the origin
//SOLVED5. receive socket get null message after receive file;
//ESS6. when click break, breakdown
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //controller w;
    //w.show();

    view *v = new view;
    v->showToUser->show();

    return a.exec();
}
