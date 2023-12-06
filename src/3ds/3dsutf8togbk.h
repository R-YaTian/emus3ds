#ifndef _3DSUTF8TOGBK_H_
#define _3DSUTF8TOGBK_H_

#include <string>
#include <3ds.h>

uint16 getGbkChar(uint16 chUtf8);
std::string mapGBKToInitial(const std::string& GBKStr);

#endif
