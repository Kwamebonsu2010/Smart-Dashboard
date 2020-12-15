/*
 * @brief The main file will be used to initialize the application, then creates and opens the main window.
 *
 * @file main.cpp
 * @author Nelaven Subaskaran, Ajith Rahugnanam, Jaspreet Sandhu, Filip Durca, & Kwame Bonsu
 * @version 1.1 12/02/2020
 */

#include "smartmirror.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SmartMirror w;
    w.show();
    w.setFixedSize(w.size());
    return a.exec();


}
