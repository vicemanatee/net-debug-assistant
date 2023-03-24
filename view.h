#ifndef VIEW_H
#define VIEW_H

#include <control.h>
#include <controller.h>

class view : public QObject
{
   Q_OBJECT

public:
    view();
    controller* showToUser;

private:

private slots:
    void outputClientMessage();

};

#endif // VIEW_H
