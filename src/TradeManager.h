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
		if(volume >= 0) 	// ����
		{
			return OpenPositionOrder(volume);
		}
		else 				// ƽ��
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
	///���ֱ���
	InputOrderMsgStruct OpenPositionOrder(int volume)
	{
		strcpy(_order.InstrumentID, INSTRUMENTID);
		_order.Direction = '0';
		strcpy(_order.combOffsetFlag, "0");
		_order.LimitPrice = _price1Arr[_index];
		_order.VolumeTotalOriginal = volume;
		return _order;
	}
	///ƽ�ֱ���
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
	///ע��ص��ӿ�
	///@param pSpi �����Իص��ӿ����ʵ��
	int RegisterSpi()
	{
		_pUserTraderApi->RegisterSpi(_pUserTraderSpi);
		return 0;
	}
	///ע��ǰ�û������ַ
	///@param pszFrontAddress��ǰ�û������ַ��
	///@remark �����ַ�ĸ�ʽΪ����protocol://ipaddress:port�����磺��tcp://127.0.0.1:17001���� 
	///@remark ��tcp��������Э�飬��127.0.0.1�������������ַ����17001������������˿ںš�
	int RegisterFront()
	{
		_pUserTraderApi->RegisterFront(_pszFrontAddress);
		return 0;
	}
	///����˽������
	///@param nResumeType ˽�����ش���ʽ  
	///        THOST_TERT_RESTART:�ӱ������տ�ʼ�ش�
	///        THOST_TERT_RESUME:���ϴ��յ�������
	///        THOST_TERT_QUICK:ֻ���͵�¼��˽����������
	///@remark �÷���Ҫ��Init����ǰ���á����������򲻻��յ�˽���������ݡ�
	int SubscribePrivateTopic(THOST_TE_RESUME_TYPE nResumeType)
	{
		_pUserTraderApi->SubscribePrivateTopic(nResumeType);
		return 0;
	}
	
	///���Ĺ�������
	///@param nResumeType �������ش���ʽ  
	///        THOST_TERT_RESTART:�ӱ������տ�ʼ�ش�
	///        THOST_TERT_RESUME:���ϴ��յ�������
	///        THOST_TERT_QUICK:ֻ���͵�¼�󹫹���������
	///@remark �÷���Ҫ��Init����ǰ���á����������򲻻��յ������������ݡ�
	int SubscribePublicTopic(THOST_TE_RESUME_TYPE nResumeType)
	{
		_pUserTraderApi->SubscribePublicTopic(nResumeType);
		return 0;
	}
	///��ʼ��
	///@remark ��ʼ�����л���,ֻ�е��ú�,�ӿڲſ�ʼ����
	int Init()
	{
		_pUserTraderApi->Init();
		const char *trade_day = _pUserTraderApi->GetTradingDay();
		printf("trade day=[%s]\n", trade_day);
		strcpy(_todayDate, trade_day);
		strcpy(_yestodayDate, trade_day);
		
		
	}
	///�û���¼���� 
	int UserLogin()
	{
		printf("> UserLogin\n");
		///�û���¼����
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
		// �û���¼���� 
		_pUserTraderApi->ReqUserLogin(&_userLoginStruct, _nRequestID++);
		// waiting for User Login Success
		while(!_pUserTraderSpi->IsOnRspUserLoginFinished)
			;
		
		return 0;
	}
	///�ǳ�����
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
	///�����ѯͶ���߽�����
	int SettlementInfoConfirm()
	{
		cout<<"> SettlementInfoConfirm"<<endl;
		CThostFtdcSettlementInfoConfirmField _settlementInfoConfirmStruct;
		strcpy(_settlementInfoConfirmStruct.BrokerID, _brokerID);
		strcpy(_settlementInfoConfirmStruct.InvestorID, _investorID);
		strcpy(_settlementInfoConfirmStruct.ConfirmDate, _todayDate);
		//strcpy(_settlementInfoConfirmStruct.ConfirmTime, "1415");
		///Ͷ���߽�����ȷ��
		_pUserTraderSpi->IsOnRspSettlementInfoConfirmFinished = false;
		int ret = _pUserTraderApi->ReqSettlementInfoConfirm(&_settlementInfoConfirmStruct, _nRequestID++);
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspSettlementInfoConfirmFinished)
				;
		_pUserTraderSpi->IsOnRspSettlementInfoConfirmFinished = false;
		return ret;
	}
	///�����ѯ����
	int QryMarketData(char *instrumentId = INSTRUMENTID)
	{
		cout<<"> QryMarketData"<<endl;
		///��ѯ����
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
	
	///�����ѯ����
	int QryOrder(char* instrumentId = NULL)
	{
		cout<<"> QryOrder"<<endl;

		///��ѯ����
		CThostFtdcQryOrderField _qryOrderStruct;
		memset(&_qryOrderStruct, 0, sizeof(_qryOrderStruct));
		///���͹�˾����
		strcpy(_qryOrderStruct.BrokerID, CTP_BROKER_ID);
		///Ͷ���ߴ���
		strcpy(_qryOrderStruct.InvestorID, CTP_INVESTOR_ID);
		if(instrumentId != NULL)
			///��Լ����
			strcpy(_qryOrderStruct.InstrumentID, instrumentId);
		///����������
		//strcpy(_qryOrderStruct.ExchangeID, _exchangeID);
		///�������
		//TThostFtdcOrderSysIDType	OrderSysID;
		///��ʼʱ��
		//strcpy(_qryOrderStruct.InsertTimeStart, _todayDate);
		///����ʱ��
		//strcpy(_qryOrderStruct.InsertTimeEnd, "20160907");
		_pUserTraderSpi->IsOnRspQryOrderFinished = false;
		int ret = _pUserTraderApi->ReqQryOrder(&_qryOrderStruct, _nRequestID++);
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspQryOrderFinished)
				;
		_pUserTraderSpi->IsOnRspQryOrderFinished = false;
		return ret;
	}	
	///�����ѯ�ɽ�
	int QryTrade(char *instrumentId = NULL)
	{
		printf("> ReqQryTrade\n");
		CThostFtdcQryTradeField pQryTrade;
		memset(&pQryTrade, 0, sizeof(pQryTrade));
		///���͹�˾����
		strcpy(pQryTrade.BrokerID, _brokerID);
		///Ͷ���ߴ���
		strcpy(pQryTrade.InvestorID, _investorID);
		if(instrumentId != NULL)
			///��Լ����
			strcpy(pQryTrade.InstrumentID, instrumentId);
		///����������
		//strcpy(pQryTrade.ExchangeID, _exchangeID);
		///�ɽ����
		//TThostFtdcTradeIDType	TradeID;
		///��ʼʱ��
		//strcpy(pQryTrade.TradeTimeStart, "08:58:07");
		///����ʱ��
		//TThostFtdcTimeType	TradeTimeEnd;
		_pUserTraderSpi->IsOnRspQryTradeFinished = false;
		int ret = _pUserTraderApi->ReqQryTrade(&pQryTrade, _nRequestID++);
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspQryTradeFinished)
				;
		_pUserTraderSpi->IsOnRspQryTradeFinished = false;
		return ret;
	}
	///�����ѯͶ���ֲ߳���ϸ
	int QryInvestorPositionDetail(char *instrumentId = INSTRUMENTID)
	{
		cout<<"> QryInvestorPositionDetail"<<endl;
		CThostFtdcQryInvestorPositionDetailField pQryInvestorPositionDetail;
		memset(&pQryInvestorPositionDetail, 0, sizeof(pQryInvestorPositionDetail));
		///���͹�˾����
		strcpy(pQryInvestorPositionDetail.BrokerID, _brokerID);
		///Ͷ���ߴ���
		strcpy(pQryInvestorPositionDetail.InvestorID,_investorID);
		///��Լ����
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
	///�����ѯ�ʽ��˻�
	int QryTradingAccount()
	{
		cout<<"> QryTradingAccount"<<endl;
		CThostFtdcQryTradingAccountField pQryTradingAccount;
		memset(&pQryTradingAccount, 0, sizeof(pQryTradingAccount));
		strcpy(pQryTradingAccount.BrokerID, _brokerID);
		strcpy(pQryTradingAccount.InvestorID, _investorID);
		cout<<"  BrokerID=["<<pQryTradingAccount.BrokerID<<"]"<<endl;
		cout<<"InvestorID=["<<pQryTradingAccount.InvestorID<<"]"<<endl;
		///���ִ���
		//strcpy(pQryTradingAccount.CurrencyID,"CNY"); // �����
		_pUserTraderSpi->IsOnRspQryTradingAccountFinished = false;
		int ret = _pUserTraderApi->ReqQryTradingAccount(&pQryTradingAccount, _nRequestID++);
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspQryTradingAccountFinished)
				;
		_pUserTraderSpi->IsOnRspQryTradingAccountFinished = false;
		return ret;
	}
	///�����ѯͶ���߽�����
	int QrySettlementInfo()
	{
		cout<<"> QrySettlementInfo"<<endl;
		CThostFtdcQrySettlementInfoField QrySettlementInfoData;
		///���͹�˾����
		strcpy(QrySettlementInfoData.BrokerID, _brokerID);
		///Ͷ���ߴ���
		strcpy(QrySettlementInfoData.InvestorID, _investorID);
		///������
		//strcpy(QrySettlementInfoData.TradingDay, _todayDate); 
		_pUserTraderSpi->IsOnRspQryInstrumentFinished = false;
		int ret = _pUserTraderApi->ReqQrySettlementInfo(&QrySettlementInfoData, _nRequestID++); 
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspQryInstrumentFinished)
				;
		_pUserTraderSpi->IsOnRspQryInstrumentFinished = false;
		return ret;
	}
	
	///�����ѯ��Լ
	int QryInstrument(char *instrumentId = INSTRUMENTID)
	{
		//cout<<"> QryInstrument"<<endl;
		CThostFtdcQryInstrumentField QryInstrumentData;
		///��Լ����
		strcpy(QryInstrumentData.InstrumentID, instrumentId);
		///����������
		strcpy(QryInstrumentData.ExchangeID, _exchangeID);
		///��Լ�ڽ������Ĵ���
		strcpy(QryInstrumentData.ExchangeInstID, _exchangeID);
		///��Ʒ����
		//TThostFtdcInstrumentIDType	ProductID;
		_pUserTraderSpi->IsOnRspQryInstrumentFinished = false;
		int ret = _pUserTraderApi->ReqQryInstrument(&QryInstrumentData, _nRequestID++); 
		if(ret == 0)
			while(!_pUserTraderSpi->IsOnRspQryInstrumentFinished)
				;
		_pUserTraderSpi->IsOnRspQryInstrumentFinished = false;
		return ret;
	}

	///�ȴ��ӿ��߳̽�������
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
	///����¼������
	int OrderInsert(char *instrumentId = INSTRUMENTID, char direction = THOST_FTDC_D_Sell, char* combOffsetFlag = COMB_OFFSET_FLAG, 
					double price = 12330, int volume = 1)
	{
		cout<<"> OrderInsert"<<endl;
		// �˵ǳɹ�,��������¼������
		memset(&_inputOrderStruct, 0, sizeof(_inputOrderStruct));
		//���͹�˾����
		strcpy(_inputOrderStruct.BrokerID, _brokerID);
		//Ͷ���ߴ���
		strcpy(_inputOrderStruct.InvestorID, _investorID);
		// ��Լ����
		strcpy(_inputOrderStruct.InstrumentID, instrumentId);
		///��������
		//printf("_orderRef=%s\n", _orderRef);
		strcpy(_inputOrderStruct.OrderRef, _orderRef);
		OrderRefPlus();
		//printf("_orderRef=%s\n", _orderRef);
		// �û�����
		strcpy(_inputOrderStruct.UserID, CTP_USER_ID);
		// �����۸�����
		_inputOrderStruct.OrderPriceType = THOST_FTDC_OPT_LimitPrice; // �޼�
		// ��������
		_inputOrderStruct.Direction = direction;
		// ��Ͽ�ƽ��־
		strcpy(_inputOrderStruct.CombOffsetFlag, combOffsetFlag);
		// ���Ͷ���ױ���־
		strcpy(_inputOrderStruct.CombHedgeFlag, "1");
		// �۸�
		_inputOrderStruct.LimitPrice = price; //
		// ����
		_inputOrderStruct.VolumeTotalOriginal = volume;
		// ��Ч������
		_inputOrderStruct.TimeCondition = THOST_FTDC_TC_GFD; // ����GTDDate��Ч 
		// GTD����
		strcpy(_inputOrderStruct.GTDDate, ""); 
		// �ɽ�������
		_inputOrderStruct.VolumeCondition = THOST_FTDC_VC_AV; // �κ�����
		// ��С�ɽ���
		_inputOrderStruct.MinVolume = 0;
		// ��������
		_inputOrderStruct.ContingentCondition = THOST_FTDC_CC_Immediately; // ����
		// ֹ���
		_inputOrderStruct.StopPrice = 0;
		// ǿƽԭ��
		_inputOrderStruct.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
		// �Զ������־
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
	
	///������������
	int OrderAction(char *order_ref)
	{
		printf("> OrderAction\n");
		OrderStatusStruct *order_status = _pUserTraderSpi->Get_pMapSysID_Status(order_ref);
		if(NULL == order_status){
			printf("Not Exist\n");
			return -1;
		}
		
		///���͹�˾����
		memset(&_inputOrderActionStruct, 0, sizeof(_inputOrderActionStruct));
		strcpy(_inputOrderActionStruct.BrokerID,_brokerID);
		///Ͷ���ߴ���
		strcpy(_inputOrderActionStruct.InvestorID,_investorID);
		///������������
		//_inputOrderActionStruct.OrderActionRef = _orderActionRefID++;
		///��������
		strcpy(_inputOrderActionStruct.OrderRef,order_ref);
		///������
		_inputOrderActionStruct.RequestID = _nRequestID;
		///ǰ�ñ��
		_inputOrderActionStruct.FrontID = _pUserTraderSpi->_frontID;
		///�Ự���
		_inputOrderActionStruct.SessionID = _pUserTraderSpi->_sessionID;
		///����������
		strcpy(_inputOrderActionStruct.ExchangeID, order_status->ExchangeID);
		///�������
		strcpy(_inputOrderActionStruct.OrderSysID, order_status->OrderSysID);
		///������־
		_inputOrderActionStruct.ActionFlag = THOST_FTDC_AF_Delete;
		///�۸�
		//TThostFtdcPriceType	LimitPrice;
		///�����仯
		//TThostFtdcVolumeType	VolumeChange;
		///�û�����
		strcpy(_inputOrderActionStruct.UserID, CTP_USER_ID);
		///��Լ����
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
	///Ԥ��¼������
	int ParkedOrderInsert()
	{
		memset(&_ParkedOrder, 0, sizeof(_ParkedOrder));
		//���͹�˾����
		strcpy(_ParkedOrder.BrokerID, _brokerID);
		//Ͷ���ߴ���
		strcpy(_ParkedOrder.InvestorID, _investorID);
		// ��Լ����
		strcpy(_ParkedOrder.InstrumentID, INSTRUMENTID);
		///��������
		strcpy(_ParkedOrder.OrderRef, "000000000010");
		// �û�����
		strcpy(_ParkedOrder.UserID, CTP_USER_ID);
		// �����۸�����
		_ParkedOrder.OrderPriceType = THOST_FTDC_OPT_LimitPrice; // �޼�
		// ��������
		_ParkedOrder.Direction = THOST_FTDC_D_Sell;
		// ��Ͽ�ƽ��־
		strcpy(_ParkedOrder.CombOffsetFlag, "0");
		// ���Ͷ���ױ���־
		strcpy(_ParkedOrder.CombHedgeFlag, "1");
		// �۸�
		_ParkedOrder.LimitPrice = 3000; //
		// ����
		_ParkedOrder.VolumeTotalOriginal = 10;
		// ��Ч������
		_ParkedOrder.TimeCondition = THOST_FTDC_TC_GFD; // ����GTDDate��Ч 
		// GTD����
		strcpy(_ParkedOrder.GTDDate, ""); 
		// �ɽ�������
		_ParkedOrder.VolumeCondition = THOST_FTDC_VC_AV; // �κ�����
		// ��С�ɽ���
		_ParkedOrder.MinVolume = 0;
		// ��������
		_ParkedOrder.ContingentCondition = THOST_FTDC_CC_ParkedOrder; // ����
		// ֹ���
		_ParkedOrder.StopPrice = 0;
		// ǿƽԭ��
		_ParkedOrder.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
		// �Զ������־
		_ParkedOrder.IsAutoSuspend = 0;
		///Ԥ��״̬
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
				///��Լ����
				TThostFtdcInstrumentIDType	InstrumentID;
				case 'i':
					///��������
					TThostFtdcDirectionType	Direction;
					///��Ͽ�ƽ��־
					TThostFtdcCombOffsetFlagType CombOffsetFlag;
					///�۸�
					TThostFtdcPriceType	LimitPrice;
					///����
					TThostFtdcVolumeType VolumeTotalOriginal;
					cout<<"Ticker,Direction,CombOffsetFlag,Price,Volume"<<endl;

					cin>>InstrumentID>>Direction>>CombOffsetFlag>>LimitPrice>>VolumeTotalOriginal;
					cin.clear();
					cin.ignore();
					cin.sync();
					OrderInsert(InstrumentID, Direction, CombOffsetFlag, LimitPrice, VolumeTotalOriginal);
					break;
				case 'd':
					///��������
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

	///���͹�˾����
	TThostFtdcBrokerIDType	_brokerID;
	///Ͷ���ߴ���
	TThostFtdcInvestorIDType	_investorID;
	///�û�����
	TThostFtdcUserIDType	_userID;
	///����
	TThostFtdcPasswordType	_password;
	///����������
	TThostFtdcExchangeIDType _exchangeID;
	///ǰ�û������ַ
	char *_pszFrontAddress;
	///
	CThostFtdcTraderApi	*_pUserTraderApi;
	///
	CTPTraderSpi	*_pUserTraderSpi;
	/// ��ǰ������
	TThostFtdcDateType _todayDate;
	/// ǰһ������
	TThostFtdcDateType _yestodayDate;
	///
	int _nRequestID;
	///
	TThostFtdcOrderRefType  _orderRef;
	///���뱨��
	CThostFtdcInputOrderField _inputOrderStruct;
	///Ԥ��
	CThostFtdcParkedOrderField _ParkedOrder;
	///���뱨������
	CThostFtdcInputOrderActionField _inputOrderActionStruct;
	///������������
	TThostFtdcOrderActionRefType _orderActionRefID = 0;
	///�洢���б���״̬��Ϣ���Ա���������Ϊ��ֵ
	map<string, OrderStatusStruct*> _mapSysID_Status;
	///
	RealTimeDB	*_pReadTimeDB;
	///
	AlgoEngine	*_pAlgoEngine;
	///�г���������
	MarketDataStruct _marketData;
	///�ֲ���ϸ
	//vector<PositionDetail> *_pVecPosDetail;

};



#endif