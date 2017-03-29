#include <iostream>
#include "Comment.h"
#include "CTPTraderSpi.h"
#include "CTPMdSpi.h"
#include "TradeManager.h"
using namespace std;
CThostFtdcMdApi *pMdUserApi;

char *ppInstrumentID[] = {"ru1705", "ru1704"};			// 行情订阅列表，注意，这个合约ID会过时的，注意与时俱进修改
int iInstrumentID = 2;									// 行情订阅数量
// 请求编号
int iRequestID = 0;
int main()
{
	pMdUserApi = CThostFtdcMdApi::CreateFtdcMdApi(pathOfLocalFile, false);
	CTPMdSpi *pUserSpi = new CTPMdSpi();
	pMdUserApi->RegisterSpi(pUserSpi);
	pMdUserApi->RegisterFront(CTP_MD_FRONT_ADDRESS);
	pMdUserApi->Init();
	pMdUserApi->Join();
	/*
	CThostFtdcTraderApi *pUserTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi(pathOfLocalFile);
	CTPTraderSpi *pUserTraderSpi = new CTPTraderSpi();
	TradeManager *pTradeManager = new TradeManager(pUserTraderApi, pUserTraderSpi);
	pTradeManager->Start();
	pTradeManager->WorkHand();
	*/
	/*pTradeManager->SetUserInfo();
	pTradeManager->RegisterSpi();
	pTradeManager->RegisterFront();
	pTradeManager->SubscribePrivateTopic(THOST_TERT_RESUME);
	pTradeManager->SubscribePublicTopic(THOST_TERT_RESTART);
	pTradeManager->Init();
	
	pTradeManager->UserLogin();
	pTradeManager->SettlementInfoConfirm();
	cout<<"############################\n"
		<<"# Order Insert 		(i)\n"
		<<"# Order Delete 		(d)\n"
		<<"# Query Account		(a)\n"
		<<"# Query Order 		(o)\n"
		<<"# Query Trade		(t)\n"
		<<"# Print Status		(s)\n"
		<<"# Query Market Data	(m)\n"
		<<"# Query Position Detail	(p)\n"
		<<"###########################"<<endl;
		
	char command;
	bool flag = true;
	while(flag)
	{
		cout<<"# Command: ";
		cin>>command;
		cin.clear();
		cin.ignore();
		cin.sync();
		switch(command)
		{
			///合约代码
			TThostFtdcInstrumentIDType	InstrumentID;
			case 'i':
				///买卖方向
				TThostFtdcDirectionType	Direction;
				///组合开平标志
				TThostFtdcCombOffsetFlagType combOffsetFlag;
				///价格
				TThostFtdcPriceType	LimitPrice;
				///数量
				TThostFtdcVolumeType VolumeTotalOriginal;
				cout<<"Ticker,Direction,combOffsetFlag,Price,Volume"<<endl;

				cin>>InstrumentID>>Direction>>combOffsetFlag>>LimitPrice>>VolumeTotalOriginal;
				cin.clear();
				cin.ignore();
				cin.sync();
				pTradeManager->OrderInsert(InstrumentID, Direction, combOffsetFlag, LimitPrice, VolumeTotalOriginal);
				break;
			case 'd':
				///报单引用
				TThostFtdcOrderRefType	OrderRef;
				cout<<"OrderReference"<<endl;
				cin.clear();
				cin.ignore();
				cin.sync();
				pTradeManager->OrderAction(OrderRef);
				break;
			case 'a':
				pTradeManager->QryTradingAccount();
				break;
			case 'm':
				//cout<<"Ticker"<<endl;
				//cin>>InstrumentID;
				//cin.clear();
				//cin.ignore();
				//cin.sync();
				pTradeManager->QryMarketData(INSTRUMENTID);
				break;
			case 'o':
				pTradeManager->QryOrder();
				break;
			case 't':
				pTradeManager->QryTrade();
				break;
			case 'p':
				//cout<<"Ticker"<<endl;
				//cin>>InstrumentID;
				//cin.clear();
				//cin.ignore();
				//cin.sync();
				pTradeManager->QryInvestorPositionDetail(INSTRUMENTID);
				break;
			case 's':
				pTradeManager->PrintOrderStatus();
				break;
			case 'e':
				pTradeManager->UserLogout();
				flag = false;
				break;
			default:
				break;
		}
	}
	*/
	//pTradeManager->Release();
	
	
	return 0;
}
