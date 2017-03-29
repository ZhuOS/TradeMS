#ifndef CTPTRADERSPI_H
#define CTPTRADERSPI_H
#include "Comment.h"

class CTPTraderSpi: public CThostFtdcTraderSpi{
public:
	// 构造函数，需要一个有效的指向CThostFtdcMduserApi实例的指针
	CTPTraderSpi() : IsOnFrontConnectedFinished(false),IsOnRspUserLoginFinished(false), IsOnRspOrderInsertFinished(false),	
			IsOnRspQryDepthMarketDataFinished(false), IsOnRspSettlementInfoConfirmFinished(false), 
			IsOnRtnOrderFinished(false), IsOnRspQryOrderFinished(false), IsOnRspQryInvestorPositionDetailFinished(false),
			IsOnRspQryTradeFinished(false), IsOnRspQryTradingAccountFinished(false),IsOnRspQrySettlementInfoFinished(false),
			IsOnRspOrderActionFinished(false),IsOnRspUserLogoutFinished(false) {
				_pVecPosDetail = new vector<PositionDetail>();
				isNew = true;		
	}
	~CTPTraderSpi(){}
	/// init _pMapSysID_Status
	void InitMapID(map<string, OrderStatusStruct*> *p_map)
	{
		_pMapSysID_Status = p_map;
	}
	/// 当客户端与交易托管系统建立起通信连接，客户端需要进行登录
	virtual void OnFrontConnected()
	{ 		
		IsOnFrontConnectedFinished = true;
	}
	/// 当客户端与交易托管系统通信连接断开时，该方法被调用
	virtual void OnFrontDisconnected(int nReason)
	{
		// 当发生这个情况后，API会自动重新连接，客户端可不做处理
		printf("OnFrontDisconnected.\n");
	} 
	/// 当客户端发出登录请求之后，该方法会被调用，通知客户端登录是否成功
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
					CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		printf("< OnRspUserLogin:\n");
		//printf("ErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		if(IsRespError(pRspInfo))
		{
			exit(-1);
		}
		/*if (pRspInfo->ErrorID != 0) {
			// 端登失败，客户端需进行错误处理
			printf("Failed to login, errorcode=%d errormsg=%s requestid=%d chain=%d", 
					pRspInfo->ErrorID, pRspInfo->ErrorMsg, nRequestID, bIsLast);
			exit(-1);
		}*/
		_frontID = pRspUserLogin->FrontID;
		_sessionID = pRspUserLogin->SessionID;
		if(bIsLast == 1)
			IsOnRspUserLoginFinished = true;
	}
	
	///登出请求响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< OnRspUserLogout\n");
		if(IsRespError(pRspInfo))
		{
			IsOnRspUserLogoutFinished = true;
			return ;
		}
		printf("BrokerID=[%s],UserID=[%s]\n", pUserLogout->BrokerID, pUserLogout->UserID);
		if(bIsLast)
			IsOnRspUserLogoutFinished = true;
	};

	///请求查询投资者结算结果响应
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< OnRspSettlementInfoConfirm\n");
		if(IsRespError(pRspInfo)){
			IsOnRspSettlementInfoConfirmFinished = true;
			return ;
		}
		/*
		if(pRspInfo != NULL)
		{
			printf("ErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			//return ;
		}*/
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		printf("BrokerID=[%s],InvestorID=[%s],ConfirmDate=[%s],ConfirmTime[%s]\n",
				pSettlementInfoConfirm->BrokerID,pSettlementInfoConfirm->InvestorID,pSettlementInfoConfirm->ConfirmDate,pSettlementInfoConfirm->ConfirmTime);
		if(bIsLast == 1)
			IsOnRspSettlementInfoConfirmFinished = true;
	}
	
	///报单回报
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder)
	{
		printf("< OnRtnOrder:\n");
		printf("InstrumentID=[%s],LimitPrice=[%6.1f],Volume=[%d],OrderRef=[%s],OrderSysID=[%s],SubmitStatus=[%c],OrderStatus=[%c]\n", 
				pOrder->InstrumentID, pOrder->LimitPrice, pOrder->VolumeTotalOriginal, 
				pOrder->OrderRef,pOrder->OrderSysID,pOrder->OrderSubmitStatus,pOrder->OrderStatus);
				
		UpdateOrderStatus(pOrder->InstrumentID, pOrder->OrderRef, pOrder->OrderLocalID, pOrder->OrderSysID,
						pOrder->ExchangeID,pOrder->OrderStatus);
		/*
		map<string, OrderStatusStruct*>::iterator iter;
		if((iter = _pMapSysID_Status->find(string(pOrder->OrderRef))) != _pMapSysID_Status->end()){
			OrderStatusStruct *pOstatus = new OrderStatusStruct(pOrder->InstrumentID, pOrder->OrderRef,
										pOrder->OrderLocalID, pOrder->OrderSysID, pOrder->OrderStatus);
			memcpy(iter->second, pOstatus,sizeof(OrderStatusStruct));
		}
		else
			_pMapSysID_Status->insert(pair<string, OrderStatusStruct*>(
										pOrder->OrderRef, 
										new OrderStatusStruct(	pOrder->InstrumentID, pOrder->OrderRef,
																pOrder->OrderLocalID, pOrder->OrderSysID, pOrder->OrderStatus
															)
										)
									);
		*/
		IsOnRtnOrderFinished = true;
		IsOnRspOrderActionFinished = true;

	}
	///成交通知
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade) {
		printf("< OnRtnTrade:\n");
		printf("InstrumentID=[%s],Price=[%6.1f],Volume=[%d],OrderRef=[%s],OrderSysID=[%s]\n", 
				pTrade->InstrumentID,pTrade->Price,pTrade->Volume,
				pTrade->OrderRef,pTrade->OrderSysID);
		
	};

	///针对用户请求的出错通知
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		IsRespError(pRspInfo);
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		// 客户端需进行错误处理{}
		
	}
	
	///请求查询成交响应
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< OnRspQryTrade:\n");
		if (IsRespError(pRspInfo)) {
			IsOnRspQryTradeFinished = true;
			return;
		}
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		
		if(pTrade == NULL)
		{
			printf("pTrade=[NULL]\n");
			IsOnRspQryTradeFinished = true;
			return;
		}
		printf("TradeDate=[%s],TradeTime=[%s],InstrumentID=[%s],Direction=[%c]Price=[%6.1f],Volume=[%d]\n",
				pTrade->TradeDate,pTrade->TradeTime,pTrade->InstrumentID,pTrade->Direction,pTrade->Price,pTrade->Volume);
		if(bIsLast == 1)
			IsOnRspQryTradeFinished = true;
	}
	///请求查询报单响应
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< onRspQryOrder\n");
		//PrintOrderStatus();
		if(IsRespError(pRspInfo)){
			IsOnRspQryOrderFinished = true;
			return ;
		}
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		if(pOrder == NULL)
		{
			printf("OnRspQryOrder=[NULL]\n");
			IsOnRspQryOrderFinished = true;
			return ;
		}
		//UpdateOrderStatus(pOrder->InstrumentID, pOrder->OrderRef, pOrder->OrderLocalID, pOrder->OrderSysID,
		//		pOrder->ExchangeID, pOrder->OrderStatus);
		//printf("InstrumentID=[%s],Direction=[%c],LimitPrice=[%6.1f],VolumeTotalOriginal=[%d]\n", 
		//		pOrder->InstrumentID,pOrder->Direction,pOrder->LimitPrice,pOrder->VolumeCondition);
		printf("InstrumentID=[%s],OrderRef=[%s],OrderLocalID=[%s],OrderSysID=[%s],OrderStatus=[%c],ExchangeID=[%s]\n", 
				pOrder->InstrumentID,pOrder->OrderRef,pOrder->OrderLocalID,pOrder->OrderSysID, pOrder->OrderStatus,pOrder->ExchangeID);
		if(bIsLast == 1){
			IsOnRspQryOrderFinished = true;
			//PrintOrderStatus();
		}
	}
	virtual void OnRspQryExchange(CThostFtdcExchangeField *pExchange, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< OnRspQryExchange\n");
		if (pRspInfo != NULL) {
			// 失败，客户端需进行错误处理
			printf("Failed to Query Exchange, errorcode=%d, errormsg=%s, requestid=%d, chain=%d", 
				pRspInfo->ErrorID, pRspInfo->ErrorMsg, nRequestID, bIsLast);
			exit(-1);
		}
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		
		if(pExchange == NULL)
		{
			printf("pExchange=NULL\n");
			exit(-1);
		}
		
		printf("ExchangeID=[%s], ExchangeName=[%s], ExchangeProperty=[%c]\n", 
				pExchange->ExchangeID, pExchange->ExchangeName, pExchange->ExchangeProperty);
	}
	///请求查询行情响应
	virtual void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< OnRspQryDepthMarketData\n");
		if (IsRespError(pRspInfo)) {
			IsOnRspQryDepthMarketDataFinished = true;
			return;
		}
		if(pDepthMarketData == NULL){
			printf("NULL\n");
			IsOnRspQryDepthMarketDataFinished = true;
			return;
		}
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		printf("TradingDay=[%s], InstrumentID=[%s],ExchangeInstID=[%s],PreSettlementPrice=[%6.1f],LastPrice=[%6.1f]\nBid1=[%6.1f*%d],Ask1=[%6.1f*%d]\n",
//Bid2=[%6.1f*%d],Bid3=[%6.1f*%d],Bid4=[%6.1f*%d],Bid5=[%6.1f*%d]\n\
//,Ask2=[%6.1f*%d],Ask3=[%6.1f*%d],Ask4=[%6.1f*%d],Ask5=[%6.1f*%d]\n",
				pDepthMarketData->TradingDay,pDepthMarketData->InstrumentID,pDepthMarketData->ExchangeInstID,
				pDepthMarketData->PreSettlementPrice,pDepthMarketData->LastPrice,
				pDepthMarketData->BidPrice1,pDepthMarketData->BidVolume1,
				//pDepthMarketData->BidPrice2,pDepthMarketData->BidVolume2,
				//pDepthMarketData->BidPrice3,pDepthMarketData->BidVolume3,
				//pDepthMarketData->BidPrice4,pDepthMarketData->BidVolume4,
				//pDepthMarketData->BidPrice5,pDepthMarketData->BidVolume5,
				pDepthMarketData->AskPrice1,pDepthMarketData->AskVolume1);
				//pDepthMarketData->AskPrice2,pDepthMarketData->AskVolume2,
				//pDepthMarketData->AskPrice3,pDepthMarketData->AskVolume3,
				//pDepthMarketData->AskPrice4,pDepthMarketData->AskVolume4,
				//pDepthMarketData->AskPrice5,pDepthMarketData->AskVolume5);
		
		strcpy(_marketData.InstrumentID, pDepthMarketData->InstrumentID);
		_marketData.PreSettlementPrice = pDepthMarketData->PreSettlementPrice;
		_marketData.LastPrice = pDepthMarketData->LastPrice;
		_marketData.BidPrice1 = pDepthMarketData->BidPrice1;
		_marketData.AskPrice1 = pDepthMarketData->AskPrice1;
		_marketData.BidVolume1 = pDepthMarketData->BidVolume1;
		_marketData.AskVolume1 = pDepthMarketData->AskVolume1;
		if(bIsLast == 1)
			IsOnRspQryDepthMarketDataFinished = true;

	}
	
	

	///请求查询投资者持仓明细响应
	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		cout<<"< OnRspQryInvestorPositionDetail"<<endl;
		if(pRspInfo != NULL){
			printf("ErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID,pRspInfo->ErrorMsg);
			return ;
		}
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		if(pInvestorPositionDetail == NULL)
		{
			printf("pInvestorPositionDetail=[NULL]\n");
			IsOnRspQryInvestorPositionDetailFinished = true;
			return ;
		}
		printf("TradingDay=[%s],InstrumentID=[%s],Direction=[%c],OpenPrice=[%6.1f],Volume=[%d]\n",
				pInvestorPositionDetail->TradingDay,pInvestorPositionDetail->InstrumentID, 
				pInvestorPositionDetail->Direction, pInvestorPositionDetail->OpenPrice,
				pInvestorPositionDetail->Volume);
				
		PositionDetail posDetail(pInvestorPositionDetail->InstrumentID, pInvestorPositionDetail->Direction, 
									pInvestorPositionDetail->OpenPrice, pInvestorPositionDetail->Volume);
		if(isNew){
			_pVecPosDetail->clear();
		}
		if(bIsLast == 1){
			isNew = true;
			_pVecPosDetail->push_back(posDetail);
			IsOnRspQryInvestorPositionDetailFinished = true;			
		}
		else{
			isNew = false;	
			_pVecPosDetail->push_back(posDetail);
			IsOnRspQryInvestorPositionDetailFinished = false;
		}
		
	}
	void PrintPositionDetail()
	{
		/*for(int i = 0; i < _pVecPosDetail.size(); i++)
		{
			cout<<"ticker="<<_pVecPosDetail[i].InstrumentID<<",Direction="<<_pVecPosDetail[i].Direction
				<<",OpenPrice="<<_pVecPosDetail[i].OpenPrice<<",Volume="<<_pVecPosDetail[i].Volume<<endl;
		}
		*/
		vector<PositionDetail>::iterator iter;
		for(iter = _pVecPosDetail->begin(); iter != _pVecPosDetail->end(); iter++)
			cout<<"ticker="<<iter->InstrumentID<<",Direction="<<iter->Direction
				<<",OpenPrice="<<iter->OpenPrice<<",Volume="<<iter->Volume<<endl;
		
	}
	///请求查询资金账户响应
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		cout<<"< OnRspQryTradingAccount"<<endl;
		if(IsRespError(pRspInfo)){
			IsOnRspQryTradingAccountFinished = true;
			return ;
		}
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		if(pTradingAccount == NULL)
		{
			printf("pTradingAccount=[NULL]\n");
			IsOnRspQryTradingAccountFinished = true;
			return ;
		}
		printf("Available=[%6.1f],PreCredit=[%6.1f],PreDeposit=[%6.1f],PreBalance=[%6.1f],PreMargin=[%6.1f]\n",
				pTradingAccount->Available,pTradingAccount->PreCredit,pTradingAccount->PreDeposit,
				pTradingAccount->PreBalance,pTradingAccount->PreMargin);
		if(bIsLast == 1)
			IsOnRspQryTradingAccountFinished = true;
	}
	///请求查询投资者结算结果响应
	virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< OnRspQrySettlementInfo\n");
		if(IsRespError(pRspInfo)){
			IsOnRspQrySettlementInfoFinished = true;
			return ;
		}
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		if(pSettlementInfo == NULL)
		{
			printf("pSettlementInfo=[NULL]\n");
			IsOnRspQryOrderFinished = true;
			return ;
		}
		printf("TradingDay=[%s],SettlementID=[%d],BrokerID=[%s],InvestorID=[%s],SequenceNo=[%d]\n",//,Content=[%s]
				pSettlementInfo->TradingDay,pSettlementInfo->SettlementID,pSettlementInfo->BrokerID,
				pSettlementInfo->InvestorID,pSettlementInfo->SequenceNo);//,pSettlementInfo->Content
		if(bIsLast == 1)
			IsOnRspQrySettlementInfoFinished = true;
	}
	///请求查询合约响应
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< OnRspQryInstrument\n");
		if(IsRespError(pRspInfo)){
			IsOnRspQryInstrumentFinished = true;
			return ;
		}
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		if(pInstrument == NULL)
		{
			printf("pInstrument=[NULL]\n");
			IsOnRspQryInstrumentFinished = true;
			return ;
		}
		printf("InstrumentID=[%s],PriceTick=[%6.1f],ExpireDate=[%s],InstLifePhase=[%c]\n", 
				pInstrument->InstrumentID,pInstrument->PriceTick,pInstrument->ExpireDate,pInstrument->InstLifePhase);
		if(bIsLast == 1)
			IsOnRspQryInstrumentFinished = true;
	}
	///报单操作请求响应
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< OnRspOrderAction\n");
		if(IsRespError(pRspInfo)){
			IsOnRspOrderActionFinished = true;
			return ;
		}
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		if(pInputOrderAction == NULL)
		{
			printf("pInstrument=[NULL]\n");
			IsOnRspOrderActionFinished = true;
			return ;
		}
		printf("InstrumentID=[%s],OrderActionRef=[%d],OrderRef=[%s],FrontID=[%d],SessionID=[%d],ExchangeID=[%s],OrderSysID=[%s],ActionFlag=[%c],LimitPrice=[%6.1f],VolumeChange=[%d],",
				pInputOrderAction->InstrumentID,pInputOrderAction->OrderActionRef,pInputOrderAction->OrderRef,
				pInputOrderAction->FrontID,pInputOrderAction->SessionID,pInputOrderAction->ExchangeID,pInputOrderAction->OrderSysID,
				pInputOrderAction->ActionFlag,pInputOrderAction->LimitPrice,pInputOrderAction->VolumeChange);
		if(bIsLast)
			IsOnRspOrderActionFinished = true;
	}
	
	bool IsRespError(CThostFtdcRspInfoField *pRspInfo){
		bool isError = (pRspInfo && pRspInfo->ErrorID != 0);
		if(isError)
		{	/// 修改成log
			int fd = open(ExceptionPath, O_RDWR | O_CREAT,666 );
			if(fd == -1)
				return isError;
			write(fd, pRspInfo->ErrorMsg, strlen(pRspInfo->ErrorMsg));
			close(fd);
			printf("ErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID,pRspInfo->ErrorMsg);
		}
		return isError;
	}
	// 报单录入应答
	virtual void OnRspOrderInsert(	CThostFtdcInputOrderField *pInputOrder,
									CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		printf("< OnRspOrderInsert:\n");
		if(IsRespError(pRspInfo)){
			IsOnRspOrderInsertFinished = true;
			return ;
		}
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		if(bIsLast == 1)
			IsOnRspOrderInsertFinished = true;
		// 通知报单录入完成 
		//SetEvent(g_hEvent);
	};
	/// 更新报单信息
	void UpdateOrderStatus( char* instrument_id, char* order_ref, char* order_local_id, char* order_id, 
							char* exchange_id, char status)
	{
		map<string, OrderStatusStruct*>::iterator iter;
		if((iter = _pMapSysID_Status->find(string(order_ref))) != _pMapSysID_Status->end()){
			OrderStatusStruct *pOstatus = new OrderStatusStruct(instrument_id, order_ref,order_local_id,
										order_id, exchange_id, status);
			memcpy(iter->second, pOstatus,sizeof(OrderStatusStruct));
		}
		else{
			_pMapSysID_Status->insert(pair<string, OrderStatusStruct*>(
					string(order_ref), 
					new OrderStatusStruct(instrument_id, order_ref, order_local_id, order_id,exchange_id, status)));
		}
	}
	/// 打印出所有报单的状态信息
	void PrintOrderStatus()
	{
		for(map<string, OrderStatusStruct*>::iterator iter = _pMapSysID_Status->begin();iter != _pMapSysID_Status->end(); iter++)
		{
			//printf("SysID=[%s],InstrumentID=[%s],OrderStatus=[%c]\n", 
			//		iter->first,iter->second->InstrumentID,iter->second->OrderStatus);
			cout<<"OrderRef="<<iter->first<<",OrderSysID="<<iter->second->OrderSysID
				<<",InstrumentID="<<iter->second->InstrumentID<<",OrderStatus="
				<<iter->second->OrderStatus<<endl;
		}
	}
	OrderStatusStruct* Get_pMapSysID_Status(char* order_ref)
	{
		map<string, OrderStatusStruct*>::iterator iter;
		if((iter = _pMapSysID_Status->find(string(order_ref))) != _pMapSysID_Status->end())
			return iter->second;
		else
			return NULL;
	}

	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("OnRtnDepthMarketData: \n");
		if (IsRespError(pRspInfo)) {
			IsOnRspQryDepthMarketDataFinished = true;
			return;
		}
		if(pDepthMarketData == NULL){
			printf("NULL\n");
			IsOnRspQryDepthMarketDataFinished = true;
			return;
		}
		printf("RequestID=[%d], Chain=[%d]\n", nRequestID, bIsLast);
		printf("TradingDay=[%s], InstrumentID=[%s],ExchangeInstID=[%s],PreSettlementPrice=[%6.1f],LastPrice=[%6.1f]\nBid1=[%6.1f*%d],Ask1=[%6.1f*%d]\n",
//Bid2=[%6.1f*%d],Bid3=[%6.1f*%d],Bid4=[%6.1f*%d],Bid5=[%6.1f*%d]\n\
//,Ask2=[%6.1f*%d],Ask3=[%6.1f*%d],Ask4=[%6.1f*%d],Ask5=[%6.1f*%d]\n",
				pDepthMarketData->TradingDay,pDepthMarketData->InstrumentID,pDepthMarketData->ExchangeInstID,
				pDepthMarketData->PreSettlementPrice,pDepthMarketData->LastPrice,
				pDepthMarketData->BidPrice1,pDepthMarketData->BidVolume1,
				//pDepthMarketData->BidPrice2,pDepthMarketData->BidVolume2,
				//pDepthMarketData->BidPrice3,pDepthMarketData->BidVolume3,
				//pDepthMarketData->BidPrice4,pDepthMarketData->BidVolume4,
				//pDepthMarketData->BidPrice5,pDepthMarketData->BidVolume5,
				pDepthMarketData->AskPrice1,pDepthMarketData->AskVolume1);
				//pDepthMarketData->AskPrice2,pDepthMarketData->AskVolume2,
				//pDepthMarketData->AskPrice3,pDepthMarketData->AskVolume3,
				//pDepthMarketData->AskPrice4,pDepthMarketData->AskVolume4,
				//pDepthMarketData->AskPrice5,pDepthMarketData->AskVolume5);
		/*
		strcpy(_marketData.InstrumentID, pDepthMarketData->InstrumentID);
		_marketData.PreSettlementPrice = pDepthMarketData->PreSettlementPrice;
		_marketData.LastPrice = pDepthMarketData->LastPrice;
		_marketData.BidPrice1 = pDepthMarketData->BidPrice1;
		_marketData.AskPrice1 = pDepthMarketData->AskPrice1;
		_marketData.BidVolume1 = pDepthMarketData->BidVolume1;
		_marketData.AskVolume1 = pDepthMarketData->AskVolume1;*/
		//if(bIsLast == 1)
		//	IsOnRspQryDepthMarketDataFinished = true;

	}
	/// 标志
	volatile bool IsOnFrontConnectedFinished;
	volatile bool IsOnRspUserLoginFinished;
	volatile bool IsOnRspQryDepthMarketDataFinished;
	volatile bool IsOnRspOrderInsertFinished;
	volatile bool IsOnRspSettlementInfoConfirmFinished;
	volatile bool IsOnRtnOrderFinished;
	volatile bool IsOnRspQryOrderFinished;
	volatile bool IsOnRspQryInvestorPositionDetailFinished;
	volatile bool IsOnRspQryTradeFinished;
	volatile bool IsOnRspQryTradingAccountFinished;
	volatile bool IsOnRspQrySettlementInfoFinished;
	volatile bool IsOnRspQryInstrumentFinished;
	volatile bool IsOnRspOrderActionFinished;
	volatile bool IsOnRspUserLogoutFinished;
	///前置编号
	volatile TThostFtdcFrontIDType _frontID;
	///会话编号
	volatile TThostFtdcSessionIDType _sessionID;
	///市场行情数据
	MarketDataStruct	_marketData;
	///持仓明细
	vector<PositionDetail> *_pVecPosDetail;
	///上次结算价
	//volatile TThostFtdcPriceType	PreSettlementPrice;
private:
	///存储所有报单状态信息，以报单引用作为键值
	map<string, OrderStatusStruct*> *_pMapSysID_Status;

	// 指向CThostFtdcMduserApi实例的指针
	//CThostFtdcTraderApi *m_pUserApi;
	TThostFtdcBrokerIDType g_chBrokerID;
	TThostFtdcUserIDType g_chUserID;
	//
	bool isNew;

};

#endif
