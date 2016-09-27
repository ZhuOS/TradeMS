#ifndef COMMENT_H
#define COMMENT_H
# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>
# include <string.h>
# include <fcntl.h>
# include "ThostFtdcMdApi.h"
# include "ThostFtdcTraderApi.h"
# include "ThostFtdcUserApiDataType.h"
# include "ThostFtdcUserApiStruct.h"
# include "UDPStruct.h"
# include <iostream>
# include <map>
# include <vector>
# include <string>
using namespace std;
char CTP_BROKER_ID[] = "9999";
char CTP_INVESTOR_ID[] = "039528";
char CTP_USER_ID[] = "039528";
char CTP_INVESTOR_PASSWD[] = "123456";
char CTP_FRONT_ADDRESS[]  = "tcp://180.168.146.187:10000";
char pathOfLocalFile[]	=	"./data_recv_file/";
char ExceptionPath[] = "./temp/exception.txt\n";
char INSTRUMENTID[]	= "ru1701";
char EXCHANGEID[] = "SHFE";
char COMB_OFFSET_FLAG[] = "0";
struct	OrderStatusStruct
{
	TThostFtdcInstrumentIDType	InstrumentID;
	///报单引用
	TThostFtdcOrderRefType	OrderRef;
	///本地报单编号
	TThostFtdcOrderLocalIDType	OrderLocalID;
	///报单编号
	TThostFtdcOrderSysIDType	OrderSysID;
	///报单状态
	TThostFtdcOrderStatusType	OrderStatus;
	///交易所代码
	TThostFtdcExchangeIDType	ExchangeID;
	
	OrderStatusStruct(char* instrument, char* order_ref,  char* local_order_id, char* order_id, char* exchange_id, char status)
	{
		strcpy(OrderRef, order_ref);
		strcpy(OrderLocalID, local_order_id);
		strcpy(OrderSysID, order_id);
		strcpy(InstrumentID, instrument);
		strcpy(ExchangeID, exchange_id);
		OrderStatus = status;
	}
	
};
/// Market Data
struct MarketDataStruct{
	TThostFtdcInstrumentIDType InstrumentID;
	TThostFtdcPriceType	PreSettlementPrice, LastPrice, BidPrice1, AskPrice1;
	int BidVolume1, AskVolume1;
};
/// Input Order message
struct InputOrderMsgStruct{
	///合约代码
	TThostFtdcInstrumentIDType	InstrumentID;
	///买卖方向
	TThostFtdcDirectionType	Direction;
	///组合开平标志
	TThostFtdcCombOffsetFlagType combOffsetFlag;
	///价格
	TThostFtdcPriceType	LimitPrice;
	///数量
	TThostFtdcVolumeType	VolumeTotalOriginal;	
};
/// 持仓数据
struct PositionDetail{
	TThostFtdcInstrumentIDType InstrumentID;
	TThostFtdcDirectionType Direction;
	TThostFtdcPriceType OpenPrice;
	TThostFtdcVolumeType Volume;
	PositionDetail(char* ticker, char dir, double price, int vol){
		strcpy(InstrumentID, ticker);
		Direction = dir;
		OpenPrice = price;
		Volume = vol;
	}
};
#endif
