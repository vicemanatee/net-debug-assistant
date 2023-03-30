#include "view.h"

view::view()
{
    showToUser = new controller;

    connect(showToUser->SCmodel, &model::SgetClientMessage,
            this, &view::outputClientMessage);
}

void view::outputClientMessage()
{
    //if(showToUser->ui)
}
