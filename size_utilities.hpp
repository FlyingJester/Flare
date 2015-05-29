#pragma once

enum FileSizePrefix {fsp_B, fsp_KB, fsp_MB, fsp_GB, fsp_TB};

const char *sizeNumberString(char buffer[8], unsigned long long i);
char sizePrefixChar(unsigned long long i);
FileSizePrefix sizePrefix(unsigned long long i);
