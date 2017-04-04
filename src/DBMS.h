#ifndef _DBMS_H
#define _DBMS_H
class DBMS{
public:
	DBMS(){
		host = string("127.0.0.1");
		port = string("27017");
		_db_name = string("TradeMS");
		errmsg = string("");
	}
	///
	int _Connect(){
		
	}

	///
	int _Insert(string table_name, string data);
	///
	int _Query(string table_name, string condition);
	///
	int _Command(string table_name, string command);
private:
	/// 数据库名称
	string _db_name;
	/// 服务器IP
	string _host;
	/// 服务器端口
	string _port;
	///
	string _errmsg;
};

#endif
