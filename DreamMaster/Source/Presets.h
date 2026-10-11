#pragma once
#include "MasterEngine.h"
namespace dream {
inline Settings preset(int index)
{
    Settings s;
    if(index==1) { s.clarity=12;s.smooth=15;s.air=-.3;s.glue=12; }
    if(index==2) { s.bass=.5;s.clarity=20;s.smooth=25;s.air=-.5;s.glue=20;s.drive=.5; }
    if(index==3) { s.bass=.4;s.clarity=15;s.smooth=10;s.glue=8;s.width=102;s.drive=1; }
    if(index==4) { s.clarity=10;s.smooth=45;s.air=-1;s.glue=10; }
    return s;
}
}
