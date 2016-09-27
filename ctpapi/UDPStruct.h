#ifndef __UDP_STRUCT_HPP__
#define __UDP_STRUCT_HPP__

typedef struct tagMarket
{
    char bourseid;
    char commodity[31];
    double last;
    int volume;
    double turnover;
    int interest;
    double highest;
    double lowest;
    double bidprice1;
    int bidvolume1;
    double askprice1;
    int askvolume1;
    char updatetime[9];
    int updatemillisec;
}TMARKET, *LPTMARKET;

#endif //__UDP_STRUCT_HPP__
