#ifndef _MA_CONFIG_H
#define _MA_CONFIG_H

#ifdef WIN32
#include "ma_config_win.in"
#else
#include "ma_config_lnx.in"
#endif

#endif