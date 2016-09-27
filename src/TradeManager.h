#ifndef TRADEMANAGER_H
#define TRADEMANAGER_H
#include "Comment.h"
class RealTimeDB{
public:
	RealTimeDB():_index(-1){
		memset(_dataArr,0,sizeof(_dataArr));
	}
	int WriteDB(MarketDataStruct *dataStruct)
	{
		//cout<<"WriteDB:"<<dataStruct->LastPrice<<endl;
		_index = (++_index)&(2048-1);
		
		//strcpy((char*)&_dataArr[_index], (char*)dataStruct);
		strcpy(_dataArr[_index].InstrumentID,dataStruct->InstrumentID);
		_dataArr[_index].PreSettlementPrice = dataStruct->PreSettlementPrice;
		_dataArr[_index].LastPrice = dataStruct->LastPrice;
		_dataArr[_index].BidPrice1 = dataStruct->BidPrice1;
		_dataArr[_index].AskPrice1 = dataStruct->AskPrice1;
		_dataArr[_index].BidVolume1 = dataStruct->BidVolume1;
		_dataArr[_index].AskVolume1 = dataStruct->AskVolume1;

		//cout<<"WriteDB:"<<_dataArr[_index].LastPrice<<endl;
		return _index;
	}
	MarketDataStruct ReadDB(int offset = 0)
	{
		return _dataArr[(_index-offset)&(2048-1)];
	}
	//MarketDataStruct ReadDB()
	//{
	//	return _dataArr[_index];
	//}
private:
	int _index;
	MarketDataStruct _dataArr[2048];
};

class AlgoEngine{
public:
	AlgoEngine():_index(-1){
		memset(_price20Arr, 0, sizeof(_price20Arr));
		memset(_price50Arr, 0, sizeof(_price50Arr));
		memset(_price20Arr, 0, sizeof(_price20Arr));
		memset(_price1Arr, 0, sizeof(_price1Arr));
	}
	void UpdatePrice(RealTimeDB *pRealTimeDB){
		
		_index = (++_index)&(2048-1);
		_preSettlementPrice = pRealTimeDB->ReadDB().PreSettlementPrice;
		_price1Arr[_index] = pRealTimeDB->ReadDB().LastPrice;
		//cout<<"ff"<<_price1Arr[_index]<<endl;
		double sum = 0;
		for(int i = 0; i < 20; i++)
			sum += pRealTimeDB->ReadDB(i).LastPrice;
		_price20Arr[_index] = sum / 20;
		for(int i = 20; i < 50; i++)
			sum += pRealTimeDB->ReadDB(i).LastPrice;		
		_price50Arr[_index] = sum / 50;
	}
	void UpdatePosition(vector<PositionDetail> *pVecPositionDetail)
	{
		_pPositionDetail = pVecPositionDetail;
	}
	InputOrderMsgStruct BuildInsertOrder()
	{
		int curr_position = GetCurrPosition();
		int next_position = GetNextPosition();
		cout<<"settlePrice:  "<<_preSettlementPrice<<endl;
		cout<<"curr_position:"<<curr_position<<endl;
		cout<<"next_position:"<<next_position<<endl;
		int volume = next_position - curr_position;
		if(volume >= 0) 	// 开仓
		{
			return OpenPositionOrder(volume);
		}
		else 				// 平仓
		{
			return ClosePositionOrder(-volume);
		}
	}
	/// 
	void PrintK()
	{
		cout<<"###########K#############"<<endl;
		cout<<"1:"<<endl;
		for(int i = 0; i <= _index; i++)
		{
			cout<<_price1Arr[i]<<",";
		}
		cout<<endl<<"20:"<<endl;
		for(int i = 0; i <= _index; i++)
		{
			cout<<_price20Arr[i]<<",";
		}
		cout<<endl<<"50:"<<endl;
		for(int i = 0; i <= _index; i++)
		{
			cout<<_price50Arr[i]<<",";
		}
		cout<<endl<<"#########################"<<endl;
	}
private:
	int GetCurrPosition()
	{
		int vol = 0;
		vector<PositionDetail>::iterator iter;
		for(iter = _pPositionDetail->begin(); iter != _pPositionDetail->end(); iter++)
		{
			vol += iter->Volume;
		}
		_currPosition = vol;
		return vol;
	}
	int GetNextPosition()
	{
		
		if(_price20Arr[_index] > _price50Arr[_index])
			return _price1Arr[_index] > _preSettlementPrice? 10: 20;
		else
			return _price1Arr[_index] > _preSettlementPrice? 0: 10;
	}
	///开仓报单
	InputOrderMsgStruct OpenPositionOrder(int volume)
	{
		strcpy(_order.InstrumentID, INSTRUMENTID);
		_order.Direction = '0';
		strcpy(_order.combOffsetFlag, "0");
		_order.LimitPrice = _price1Arr[_index];
		_order.VolumeTotalOriginal = volume;
		return _order;
	}
	///平仓报单
	InputOrderMsgStruct ClosePositionOrder(int volume)
	{
		strcpy(_order.InstrumentID, INSTRUMENTID);
		_order.Direction = '1';
		strcpy(_order.combOffsetFlag, "3");
		_order.LimitPrice = _price1Arr[_index];
		_order.VolumeTotalOriginal = volume;
		return _order;
	}
	///
	int _index;
	int _currPosition;
	int _nextPosition;
	double _preSettlementPrice;
	double _price1Arr[2048], _price20Arr[2048], _price50Arr[2048];
	vector<PositionDetail> *_pPositionDetail;
	
	InputOrderMsgStruct _order;
};

class TradeManager{
public:
	TradeManager(CThostFtdcTraderApi *pApi, CTPTraderSpi *pSpi): _nRequestID(0),
			_pUserTraderApi(pApi), _pUserTraderSpi(pSpi){
				strcpy(_orderRef, "100000000000");
				_pUserTraderSpi->InitMapID(&_mapSysID_Status);
			}
			
	int SetUserInfo(char* brokerId = CTP_BROKER_ID, char* investorId = CTP_INVESTOR_ID, 
					char* userId   = CTP_USER_ID, 	char* password   = CTP_INVESTOR_PASSWD, 
					char *frontAddress = CTP_FRONT_ADDRESS, char *exchangeId = EXCHANGEID)
	{
		strcpy(_brokerID, brokerId);
		strcpy(_investorID, investorId);
		strcpy(_userID, userId);
		strcpy(_password, password);
		strcpy(_exchangeID, exchangeId);
		_pszFrontAddress = frontAddress;
		return 0; 
	} 
	///注册回调接口
	///@param pSpi 派生自回调接口类的实例
	int RegisterSpi()
	{
		_pUserTraderApi->RegisterSpi(_pUserTraderSpi);
		return 0;
	}
	///注册前置机网络地址
	///@param pszFrontAddress：前置机网络地址。
	///@remark 网络地址的格式为：“protocol://ipaddress:port”，如：”tcp://127.0.0.1:17001”。 
	///@remark “tcp”代表传输协议，“127.0.0.1”代表服务器地址。”17001”代表服务器端口号。
	int RegisterFront()
	{
		_pUserTraderApi->RegisterFront(_pszFrontAddress);
		return 0;
	}
	///订阅私有流。
	///@param nResumeType 私有流重传方式  
	///        THOST_TERT_RESTART:从本交易日开始重传
	///        THOST_TERT_RESUME:从上次收到的续传
	///        THOST_TERT_QUICK:只传送登录后私有流的内容
	///@remark 该方法要在Init方法前调用。若不调用则不会收到私有流的数据。
	int SubscribePrivateTopic(THOST_TE_RESUME_TYPE nResumeType)
	{
		_pUserTraderApi->SubscribePrivateTopic(nResumeType);
		return 0;
	}
	
	///订阅公共流。
	///@param nResumeType 公共流重传方式  
	///        THOST_TERT_RESTART:从本交易日开始重传
	///        THOST_TERT_RESUME:从上次收到的续传
	///        THOST_TERT_QUICK:只传送登录后公共流的内容
	///@remark 该方法要在Init方法前调用。若不调用则不会收到公共流的数据。
	int SubscribePublicTopic(THOST_TE_RESUME_TYPE nResumeType)
	{
		_pUserTraderApi->SubscribePublicTopic(nResumeType);
		return 0;
	}
	///初始化
	///@remark 初始化运行环境,只有调用后,接口才开始工作
	int Init()
	{
		_pUserTraderApi->Init();
		const char *trade_day = _pUserTraderApi->GetTradingDay();
		printf("trade day=[%s]\n", trade_day);
		strcpy(_todayDate, trade_day);
		strcpy(_yestodayDate, trade_day);
		
		
	}
	///用户登录请求 
	int UserLogin()
	{
		printf("> UserLogin\n");
		///用户登录请求
		CThostFtdcReqUserLoginField _userLoginStruct;
		printf("BrokerID:%s\n", _brokerID);
		// scanf("%s", (char*)&g_chBrokerID);
		strcpy(_userLoginStruct.BrokerID, _brokerID);
		// get userid 
		printf("Userid:%s\n", _userID);
		//scanf("%s", (char*)&g_chUserID);
		strcpy(_userLoginStruct.UserID, _userID);
		// get password
		printf("Password:%s\n", _password);
		strcpy(_userLoginStruct.Password, _password);
		// wait util OnFrontconnected is Finished
		while(!_pUserTraderSpi->IsOnFrontConnectedFinished)
			;
		
		_pUserTraderSpi->IsOnRspUserLoginFinished = false;
		// 用户登录请求 
		_pUserTraderApi->ReqUserLogin(&_userLoginStruct, _nRequestID++);
		// waiting for User Login Success
		while(!_pUserTraderSpi->IsOnRspUserLoginFinished)
			;
		
		return 0;
	}
	///登出请求
	int UserLogout()
	{
		cout<<"> UserLogout"<<endl;
		CThostFtdcUserLogoutField userLogoutStruct;
		strcpy(userLogoutStruct.BrokerID, CTP_BROKER_ID);
		strcpy(userLogoutStruct.UserID, CTP_USER_ID);
		_pUserTraderSpi->IsOnRspUserLogoutFinished = false;
		int ret = _pUserTraderApi->ReqUserLogout(&userLogoutStruct, _nRequestID++);
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspUserLogoutFinished)
				;
		_pUserTraderSpi->IsOnRspUserLogoutFinished = false;
		return ret;
	}
	///请求查询投资者结算结果
	int SettlementInfoConfirm()
	{
		cout<<"> SettlementInfoConfirm"<<endl;
		CThostFtdcSettlementInfoConfirmField _settlementInfoConfirmStruct;
		strcpy(_settlementInfoConfirmStruct.BrokerID, _brokerID);
		strcpy(_settlementInfoConfirmStruct.InvestorID, _investorID);
		strcpy(_settlementInfoConfirmStruct.ConfirmDate, _todayDate);
		//strcpy(_settlementInfoConfirmStruct.ConfirmTime, "1415");
		///投资者结算结果确认
		_pUserTraderSpi->IsOnRspSettlementInfoConfirmFinished = false;
		int ret = _pUserTraderApi->ReqSettlementInfoConfirm(&_settlementInfoConfirmStruct, _nRequestID++);
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspSettlementInfoConfirmFinished)
				;
		_pUserTraderSpi->IsOnRspSettlementInfoConfirmFinished = false;
		return ret;
	}
	///请求查询行情
	int QryMarketData(char *instrumentId = INSTRUMENTID)
	{
		cout<<"> QryMarketData"<<endl;
		///查询行情
		CThostFtdcQryDepthMarketDataField _depthMarketDataStruct;
		strcpy(_depthMarketDataStruct.InstrumentID, instrumentId);

		_pUserTraderSpi->IsOnRspQryDepthMarketDataFinished = false;
		int ret = _pUserTraderApi->ReqQryDepthMarketData(&_depthMarketDataStruct, _nRequestID++);
		if(ret == 0){
			while(!_pUserTraderSpi->IsOnRspQryDepthMarketDataFinished)
				;
			//
			_marketData = _pUserTraderSpi->_marketData;
		}
		_pUserTraderSpi->IsOnRspQryDepthMarketDataFinished = false;
		return 0;
	}
	
	///请求查询报单
	int QryOrder(char* instrumentId = NULL)
	{
		cout<<"> QryOrder"<<endl;

		///查询报单
		CThostFtdcQryOrderField _qryOrderStruct;
		memset(&_qryOrderStruct, 0, sizeof(_qryOrderStruct));
		///经纪公司代码
		strcpy(_qryOrderStruct.BrokerID, CTP_BROKER_ID);
		///投资者代码
		strcpy(_qryOrderStruct.InvestorID, CTP_INVESTOR_ID);
		if(instrumentId != NULL)
			///合约代码
			strcpy(_qryOrderStruct.InstrumentID, instrumentId);
		///交易所代码
		//strcpy(_qryOrderStruct.ExchangeID, _exchangeID);
		///报单编号
		//TThostFtdcOrderSysIDType	OrderSysID;
		///开始时间
		//strcpy(_qryOrderStruct.InsertTimeStart, _todayDate);
		///结束时间
		//strcpy(_qryOrderStruct.InsertTimeEnd, "20160907");
		_pUserTraderSpi->IsOnRspQryOrderFinished = false;
		int ret = _pUserTraderApi->ReqQryOrder(&_qryOrderStruct, _nRequestID++);
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspQryOrderFinished)
				;
		_pUserTraderSpi->IsOnRspQryOrderFinished = false;
		return ret;
	}	
	///请求查询成交
	int QryTrade(char *instrumentId = NULL)
	{
		printf("> ReqQryTrade\n");
		CThostFtdcQryTradeField pQryTrade;
		memset(&pQryTrade, 0, sizeof(pQryTrade));
		///经纪公司代码
		strcpy(pQryTrade.BrokerID, _brokerID);
		///投资者代码
		strcpy(pQryTrade.InvestorID, _investorID);
		if(instrumentId != NULL)
			///合约代码
			strcpy(pQryTrade.InstrumentID, instrumentId);
		///交易所代码
		//strcpy(pQryTrade.ExchangeID, _exchangeID);
		///成交编号
		//TThostFtdcTradeIDType	TradeID;
		///开始时间
		//strcpy(pQryTrade.TradeTimeStart, "08:58:07");
		///结束时间
		//TThostFtdcTimeType	TradeTimeEnd;
		_pUserTraderSpi->IsOnRspQryTradeFinished = false;
		int ret = _pUserTraderApi->ReqQryTrade(&pQryTrade, _nRequestID++);
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspQryTradeFinished)
				;
		_pUserTraderSpi->IsOnRspQryTradeFinished = false;
		return ret;
	}
	///请求查询投资者持仓明细
	int QryInvestorPositionDetail(char *instrumentId = INSTRUMENTID)
	{
		cout<<"> QryInvestorPositionDetail"<<endl;
		CThostFtdcQryInvestorPositionDetailField pQryInvestorPositionDetail;
		memset(&pQryInvestorPositionDetail, 0, sizeof(pQryInvestorPositionDetail));
		///经纪公司代码
		strcpy(pQryInvestorPositionDetail.BrokerID, _brokerID);
		///投资者代码
		strcpy(pQryInvestorPositionDetail.InvestorID,_investorID);
		///合约编码
		strcpy(pQryInvestorPositionDetail.InstrumentID, instrumentId);
		_pUserTraderSpi->IsOnRspQryInvestorPositionDetailFinished = false;
		int ret = _pUserTraderApi->ReqQryInvestorPositionDetail(&pQryInvestorPositionDetail, _nRequestID++);
		if(ret == 0){
			while(!_pUserTraderSpi->IsOnRspQryInvestorPositionDetailFinished)
				;
			
		}
		_pUserTraderSpi->IsOnRspQryInvestorPositionDetailFinished = false;
		return ret;
	}
	///请求查询资金账户
	int QryTradingAccount()
	{
		cout<<"> QryTradingAccount"<<endl;
		CThostFtdcQryTradingAccountField pQryTradingAccount;
		memset(&pQryTradingAccount, 0, sizeof(pQryTradingAccount));
		strcpy(pQryTradingAccount.BrokerID, _brokerID);
		strcpy(pQryTradingAccount.InvestorID, _investorID);
		cout<<"  BrokerID=["<<pQryTradingAccount.BrokerID<<"]"<<endl;
		cout<<"InvestorID=["<<pQryTradingAccount.InvestorID<<"]"<<endl;
		///币种代码
		//strcpy(pQryTradingAccount.CurrencyID,"CNY"); // 人民币
		_pUserTraderSpi->IsOnRspQryTradingAccountFinished = false;
		int ret = _pUserTraderApi->ReqQryTradingAccount(&pQryTradingAccount, _nRequestID++);
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspQryTradingAccountFinished)
				;
		_pUserTraderSpi->IsOnRspQryTradingAccountFinished = false;
		return ret;
	}
	///请求查询投资者结算结果
	int QrySettlementInfo()
	{
		cout<<"> QrySettlementInfo"<<endl;
		CThostFtdcQrySettlementInfoField QrySettlementInfoData;
		///经纪公司代码
		strcpy(QrySettlementInfoData.BrokerID, _brokerID);
		///投资者代码
		strcpy(QrySettlementInfoData.InvestorID, _investorID);
		///交易日
		//strcpy(QrySettlementInfoData.TradingDay, _todayDate); 
		_pUserTraderSpi->IsOnRspQryInstrumentFinished = false;
		int ret = _pUserTraderApi->ReqQrySettlementInfo(&QrySettlementInfoData, _nRequestID++); 
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspQryInstrumentFinished)
				;
		_pUserTraderSpi->IsOnRspQryInstrumentFinished = false;
		return ret;
	}
	
	///请求查询合约
	int QryInstrument(char *instrumentId = INSTRUMENTID)
	{
		//cout<<"> QryInstrument"<<endl;
		CThostFtdcQryInstrumentField QryInstrumentData;
		///合约代码
		strcpy(QryInstrumentData.InstrumentID, instrumentId);
		///交易所代码
		strcpy(QryInstrumentData.ExchangeID, _exchangeID);
		///合约在交易所的代码
		strcpy(QryInstrumentData.ExchangeInstID, _exchangeID);
		///产品代码
		//TThostFtdcInstrumentIDType	ProductID;
		_pUserTraderSpi->IsOnRspQryInstrumentFinished = false;
		int ret = _pUserTraderApi->ReqQryInstrument(&QryInstrumentData, _nRequestID++); 
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspQryInstrumentFinished)
				;
		_pUserTraderSpi->IsOnRspQryInstrumentFinished = false;
		return ret;
	}

	///等待接口线程结束运行
	int Join()
	{
		printf("# Join\n");
		_pUserTraderApi->Join();
	}
	void Release()
	{
		printf("# Release\n");
		_pUserTraderApi->Release();
	}
	///报单录入请求
	int OrderInsert(char *instrumentId = INSTRUMENTID, char direction = THOST_FTDC_D_Sell, char* combOffsetFlag = COMB_OFFSET_FLAG, 
					double price = 12330, int volume = 1)
	{
		cout<<"> OrderInsert"<<endl;
		// 端登成功,发出报单录入请求
		memset(&_inputOrderStruct, 0, sizeof(_inputOrderStruct));
		//经纪公司代码
		strcpy(_inputOrderStruct.BrokerID, _brokerID);
		//投资者代码
		strcpy(_inputOrderStruct.InvestorID, _investorID);
		// 合约代码
		strcpy(_inputOrderStruct.InstrumentID, instrumentId);
		///报单引用
		//printf("_orderRef=%s\n", _orderRef);
		strcpy(_inputOrderStruct.OrderRef, _orderRef);
		OrderRefPlus();
		//printf("_orderRef=%s\n", _orderRef);
		// 用户代码
		strcpy(_inputOrderStruct.UserID, CTP_USER_ID);
		// 报单价格条件
		_inputOrderStruct.OrderPriceType = THOST_FTDC_OPT_LimitPrice; // 限价
		// 买卖方向
		_inputOrderStruct.Direction = direction;
		// 组合开平标志
		strcpy(_inputOrderStruct.CombOffsetFlag, combOffsetFlag);
		// 组合投机套保标志
		strcpy(_inputOrderStruct.CombHedgeFlag, "1");
		// 价格
		_inputOrderStruct.LimitPrice = price; //
		// 数量
		_inputOrderStruct.VolumeTotalOriginal = volume;
		// 有效期类型
		_inputOrderStruct.TimeCondition = THOST_FTDC_TC_GFD; // 当日GTDDate有效 
		// GTD日期
		strcpy(_inputOrderStruct.GTDDate, ""); 
		// 成交量类型
		_inputOrderStruct.VolumeCondition = THOST_FTDC_VC_AV; // 任何数量
		// 最小成交量
		_inputOrderStruct.MinVolume = 0;
		// 触发条件
		_inputOrderStruct.ContingentCondition = THOST_FTDC_CC_Immediately; // 立即
		// 止损价
		_inputOrderStruct.StopPrice = 0;
		// 强平原因
		_inputOrderStruct.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
		// 自动挂起标志
		_inputOrderStruct.IsAutoSuspend = 0;

		_pUserTraderSpi->IsOnRspOrderInsertFinished = false;
		_pUserTraderSpi->IsOnRtnOrderFinished = false;
		int ret = _pUserTraderApi->ReqOrderInsert(&_inputOrderStruct, _nRequestID++);
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRtnOrderFinished && !_pUserTraderSpi->IsOnRspOrderInsertFinished)
				;
		_pUserTraderSpi->IsOnRspOrderInsertFinished = false;
		_pUserTraderSpi->IsOnRtnOrderFinished = false;
		return ret;
	}
	
	///报单操作请求
	int OrderAction(char *order_ref)
	{
		printf("> OrderAction\n");
		OrderStatusStruct *order_status = _pUserTraderSpi->Get_pMapSysID_Status(order_ref);
		if(NULL == order_status){
			printf("Not Exist\n");
			return -1;
		}
		
		///经纪公司代码
		memset(&_inputOrderActionStruct, 0, sizeof(_inputOrderActionStruct));
		strcpy(_inputOrderActionStruct.BrokerID,_brokerID);
		///投资者代码
		strcpy(_inputOrderActionStruct.InvestorID,_investorID);
		///报单操作引用
		//_inputOrderActionStruct.OrderActionRef = _orderActionRefID++;
		///报单引用
		strcpy(_inputOrderActionStruct.OrderRef,order_ref);
		///请求编号
		_inputOrderActionStruct.RequestID = _nRequestID;
		///前置编号
		_inputOrderActionStruct.FrontID = _pUserTraderSpi->_frontID;
		///会话编号
		_inputOrderActionStruct.SessionID = _pUserTraderSpi->_sessionID;
		///交易所代码
		strcpy(_inputOrderActionStruct.ExchangeID, order_status->ExchangeID);
		///报单编号
		strcpy(_inputOrderActionStruct.OrderSysID, order_status->OrderSysID);
		///操作标志
		_inputOrderActionStruct.ActionFlag = THOST_FTDC_AF_Delete;
		///价格
		//TThostFtdcPriceType	LimitPrice;
		///数量变化
		//TThostFtdcVolumeType	VolumeChange;
		///用户代码
		strcpy(_inputOrderActionStruct.UserID, CTP_USER_ID);
		///合约代码
		strcpy(_inputOrderActionStruct.InstrumentID, order_status->InstrumentID);
		_pUserTraderSpi->IsOnRspOrderActionFinished = false;
		int ret = _pUserTraderApi->ReqOrderAction(&_inputOrderActionStruct, _nRequestID++);
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspOrderActionFinished)
				;
		_pUserTraderSpi->IsOnRspOrderActionFinished = false;
		return ret;
	}
	void PrintOrderStatus()
	{
		printf("> PrintOrderStatus\n");
		_pUserTraderSpi->PrintOrderStatus();
	}
	void PrintPositionDetail()
	{
		printf("> PrintPositionDetail\n");
		_pUserTraderSpi->PrintPositionDetail();
	}
	/*
	///预埋单录入请求
	int ParkedOrderInsert()
	{
		memset(&_ParkedOrder, 0, sizeof(_ParkedOrder));
		//经纪公司代码
		strcpy(_ParkedOrder.BrokerID, _brokerID);
		//投资者代码
		strcpy(_ParkedOrder.InvestorID, _investorID);
		// 合约代码
		strcpy(_ParkedOrder.InstrumentID, INSTRUMENTID);
		///报单引用
		strcpy(_ParkedOrder.OrderRef, "000000000010");
		// 用户代码
		strcpy(_ParkedOrder.UserID, CTP_USER_ID);
		// 报单价格条件
		_ParkedOrder.OrderPriceType = THOST_FTDC_OPT_LimitPrice; // 限价
		// 买卖方向
		_ParkedOrder.Direction = THOST_FTDC_D_Sell;
		// 组合开平标志
		strcpy(_ParkedOrder.CombOffsetFlag, "0");
		// 组合投机套保标志
		strcpy(_ParkedOrder.CombHedgeFlag, "1");
		// 价格
		_ParkedOrder.LimitPrice = 3000; //
		// 数量
		_ParkedOrder.VolumeTotalOriginal = 10;
		// 有效期类型
		_ParkedOrder.TimeCondition = THOST_FTDC_TC_GFD; // 当日GTDDate有效 
		// GTD日期
		strcpy(_ParkedOrder.GTDDate, ""); 
		// 成交量类型
		_ParkedOrder.VolumeCondition = THOST_FTDC_VC_AV; // 任何数量
		// 最小成交量
		_ParkedOrder.MinVolume = 0;
		// 触发条件
		_ParkedOrder.ContingentCondition = THOST_FTDC_CC_ParkedOrder; // 立即
		// 止损价
		_ParkedOrder.StopPrice = 0;
		// 强平原因
		_ParkedOrder.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
		// 自动挂起标志
		_ParkedOrder.IsAutoSuspend = 0;
		///预埋单状态
		strcpy(_ParkedOrder.Status, THOST_FTDC_PAOS_Send); // 
		
		
		int ret = _pUserTraderApi->ReqParkedOrderInsert(_ParkedOrder, int nRequestID);
		if(ret == 0)
			
	}
	*/
	
	int Start()
	{
		_pAlgoEngine = new AlgoEngine();
		_pReadTimeDB = new RealTimeDB();
		SetUserInfo();
		RegisterSpi();
		RegisterFront();
		SubscribePrivateTopic(THOST_TERT_RESUME);
		SubscribePublicTopic(THOST_TERT_RESTART);
		Init();
		UserLogin();
		SettlementInfoConfirm();
	}
	
	int WorkAuto()
	{
		char Command;
		cout<<"Command:";
		cin>>Command;
		while(Command == 'c'){
			QryMarketData();
			_pReadTimeDB->WriteDB(&_pUserTraderSpi->_marketData);
			_pAlgoEngine->UpdatePrice(_pReadTimeDB);
			sleep(1);
			QryInvestorPositionDetail();
			_pAlgoEngine->UpdatePosition(_pUserTraderSpi->_pVecPosDetail);
			InputOrderMsgStruct order = _pAlgoEngine->BuildInsertOrder();
			sleep(1);
			if(order.VolumeTotalOriginal == 0){
				_pAlgoEngine->PrintK();
				//cout<<"Command:";
				//cin>>Command;
				continue;
			}
				
			OrderInsert(order.InstrumentID, order.Direction, order.combOffsetFlag, order.LimitPrice, order.VolumeTotalOriginal);
	
			PrintOrderStatus();
			PrintPositionDetail();
			_pAlgoEngine->PrintK();
			cout<<"Command:";
			cin>>Command;
		}
	}

	int WorkHand()
	{
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
					TThostFtdcCombOffsetFlagType CombOffsetFlag;
					///价格
					TThostFtdcPriceType	LimitPrice;
					///数量
					TThostFtdcVolumeType VolumeTotalOriginal;
					cout<<"Ticker,Direction,CombOffsetFlag,Price,Volume"<<endl;

					cin>>InstrumentID>>Direction>>CombOffsetFlag>>LimitPrice>>VolumeTotalOriginal;
					cin.clear();
					cin.ignore();
					cin.sync();
					OrderInsert(InstrumentID, Direction, CombOffsetFlag, LimitPrice, VolumeTotalOriginal);
					break;
				case 'd':
					///报单引用
					TThostFtdcOrderRefType	OrderRef;
					cout<<"OrderReference"<<endl;
					cin>>OrderRef;
					cin.clear();
					cin.ignore();
					cin.sync();
					OrderAction(OrderRef);
					break;
				case 'a':
					QryTradingAccount();
					break;
				case 'm':
					//cout<<"Ticker"<<endl;
					//cin>>InstrumentID;
					//cin.clear();
					//cin.ignore();
					//cin.sync();
					QryMarketData(INSTRUMENTID);
					break;
				case 'o':
					QryOrder();
					break;
				case 't':
					QryTrade();
					break;
				case 'p':
					//cout<<"Ticker"<<endl;
					//cin>>InstrumentID;
					//cin.clear();
					//cin.ignore();
					//cin.sync();
					QryInvestorPositionDetail(INSTRUMENTID);
					break;
				case 's':
					PrintOrderStatus();
					break;
				case 'e':
					UserLogout();
					flag = false;
					break;
				default:
					break;
			}
		}
	}
private:
	char* OrderRefPlus()
	{
		int len = strlen(_orderRef);
		int carry_bit = 1;
		for(int i = len-1; i >=0; i++)
		{
			_orderRef[i] += 1;
			if(_orderRef[i] > '9')
				_orderRef[i] = '0';
			else
				break;
		}
		return _orderRef;
	}
	
private:

	///经纪公司代码
	TThostFtdcBrokerIDType	_brokerID;
	///投资者代码
	TThostFtdcInvestorIDType	_investorID;
	///用户代码
	TThostFtdcUserIDType	_userID;
	///密码
	TThostFtdcPasswordType	_password;
	///交易所代码
	TThostFtdcExchangeIDType _exchangeID;
	///前置机网络地址
	char *_pszFrontAddress;
	///
	CThostFtdcTraderApi	*_pUserTraderApi;
	///
	CTPTraderSpi	*_pUserTraderSpi;
	/// 当前交易日
	TThostFtdcDateType _todayDate;
	/// 前一交易日
	TThostFtdcDateType _yestodayDate;
	///
	int _nRequestID;
	///
	TThostFtdcOrderRefType  _orderRef;
	///输入报单
	CThostFtdcInputOrderField _inputOrderStruct;
	///预埋单
	CThostFtdcParkedOrderField _ParkedOrder;
	///输入报单操作
	CThostFtdcInputOrderActionField _inputOrderActionStruct;
	///报单操作引用
	TThostFtdcOrderActionRefType _orderActionRefID = 0;
	///存储所有报单状态信息，以报单引用作为键值
	map<string, OrderStatusStruct*> _mapSysID_Status;
	///
	RealTimeDB	*_pReadTimeDB;
	///
	AlgoEngine	*_pAlgoEngine;
	///市场行情数据
	MarketDataStruct _marketData;
	///持仓明细
	//vector<PositionDetail> *_pVecPosDetail;

};



#endif