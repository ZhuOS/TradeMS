#ifndef CTPMDSPI_H
#define CTPMDSPI_H
#include "Comment.h"

extern CThostFtdcMdApi *pMdUserApi;
extern char* ppInstrumentID[];	
extern int iInstrumentID;

// 请求编号
extern int iRequestID;
class CTPMdSpi: public CThostFtdcMdSpi{
public:

	void OnFrontConnected(){
		printf("< Md Connect Success\n");
		///用户登录请求
		ReqUserLogin();	
	}

	void OnFrontDisconnected(int nReason){
		printf("< Md Connect Fail\n");
	}

	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< Md Rsp Error\n");
		IsErrorRspInfo(pRspInfo);
	}
	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	///@param nTimeLapse 距离上次接收报文的时间
	virtual void OnHeartBeatWarning(int nTimeLapse){
		printf("< Md HearBeat Warning\n");
	
	}
				
	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< Md Login\n");
		if (bIsLast && !IsErrorRspInfo(pRspInfo))
		{
			///获取当前交易日
			cout << "> 获取当前交易日 = " << pMdUserApi->GetTradingDay() << endl;
			// 请求订阅行情
			SubscribeMarketData();	
		}
	}
	///登出请求响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< Md Logout\n");
	
	}
	
	///订阅行情应答
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< Md Subscribe Market Data\n");
		
	}
	
	///取消订阅行情应答
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("< Md UnSubscribe Market Data\n");
	
	}
	
	///深度行情通知
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
		printf("< Md Market Data\n");

		cout<< pDepthMarketData->InstrumentID<<","<< pDepthMarketData->UpdateTime<<"."<< pDepthMarketData->UpdateMillisec<<","<< pDepthMarketData->LastPrice
			<< "," << pDepthMarketData-> Volume << "," << pDepthMarketData-> BidPrice1 << "," << pDepthMarketData-> BidVolume1 << "," << pDepthMarketData-> AskPrice1 
			<< "," << pDepthMarketData-> AskVolume1 << "," << pDepthMarketData-> OpenInterest << "," << pDepthMarketData->Turnover<< pDepthMarketData->AskPrice2<< pDepthMarketData->AskVolume2<<endl;

	}

private:
	void SubscribeMarketData()
	{
		int iResult = pMdUserApi->SubscribeMarketData(ppInstrumentID, iInstrumentID);
		cerr << "--->>> 发送行情订阅请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
	}

	void ReqUserLogin()
	{
		CThostFtdcReqUserLoginField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.BrokerID, CTP_BROKER_ID);
		strcpy(req.UserID, CTP_USER_ID);
		strcpy(req.Password, CTP_INVESTOR_PASSWD);
		int iResult = pMdUserApi->ReqUserLogin(&req, ++iRequestID);
		cerr << "--->>> 发送用户登录请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
	}
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
	{
		bool isError = (pRspInfo && pRspInfo->ErrorID != 0);
		if(isError){
			int fd = open(ExceptionPath, O_RDWR | O_CREAT, 777);
			if(fd == -1){
				return isError;
			}
			write(fd, pRspInfo->ErrorMsg, strlen(pRspInfo->ErrorMsg));
			close(fd);
			printf("ErrorCode=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		}
		return isError;
	}


private:
};

#endif
