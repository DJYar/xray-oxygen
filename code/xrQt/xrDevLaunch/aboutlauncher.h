/*************************************************
* VERTVER, 2018 (C)
* X-RAY OXYGEN 1.7 PROJECT
*
* Edited: 28 March, 2018
* aboutlauncher.h - header for "About" dialog 
*************************************************/

#pragma once
/////////////////////////////////////////
#include "xrMain.h"
/////////////////////////////////////////
namespace Ui {
class AboutLauncher;
}

class AboutLauncher : public QDialog
{
    Q_OBJECT

public:
    explicit AboutLauncher(QWidget *parent = 0);
    ~AboutLauncher();

private:
    Ui::AboutLauncher *about_ui;
};