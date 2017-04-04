#include "deploy.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "lib_io.h"
#include "lib_time.h"
#include <queue>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <string.h>
using namespace std;
#define INF (1<<30)
#define MAX_FEE (1<<30)
#define _DEBUG
/// 消费节点的信息
struct consumerNode{
	int netNode;
	int need;
	consumerNode():netNode(0), need(0){}
	consumerNode(int node, int ne):netNode(node), need(ne){}	
};
/// 网络节点的信息
struct networkNode{
	int bandwith;
	int oppsiteDirection;
	int price; 
	networkNode(int bw=0, int p=0x1<<30):bandwith(bw),price(p),oppsiteDirection(0){}
};

vector<vector<networkNode> > graph(1002, vector<networkNode>(1002, networkNode(0,1<<30)));
vector<vector<int> > link_graph(1002);
vector<int> closestNode(1002,-1); 
vector<consumerNode> consumerInfo(500);
vector<vector<int>> outPathDetail; // 最终的输出链路信息
vector<int> initNodes; // 初始化集合
map<int, int> initNodesMp; // 初始化集合
vector<pair<int,int> > sortNodesMp;//  初始化
vector<bool> resPerson; // 精英个体
int networkNodeNum;
int linkNstartm;
int consumerNodeNum;
int serverNumMax; // 服务器最大个数
int serverNum;
int costServer;
int feedNeedAll; // 总流量需求
int S; // 源点
int T; // 汇点
int resFeeMin; // 最终费用
int feedCurrent; // 累加流量
set<int> serverSetRes; // 最优的服务器集合
set<int> numOfServerUsed; // 每个案例的服务器集合

double currentAvgFitness;
double currentMaxFitness;
/// spfa算法全局变量
vector<vector<networkNode> > graphTemp;
#define Max 2002
int pre[Max];  
/// 随机数函数
int rand_num(unsigned int range){
	if(range == 0)
		return 0;
	int num = rand() % RAND_MAX;
	num = (num<<15) + rand() % RAND_MAX;
	return num % range;
}
void print_person(vector<bool> person){
#ifdef _DEBUG
	for(int i  = 0; i < person.size();i++){
		cout<<person[i];
	}
	cout<<endl;
#endif
}
void print_groups(vector<vector<bool>> groups){
#ifdef _DEBUG
	for(int i = 0; i < groups.size(); i++){
		cout<<i<<": ";
		for(int j = 0; j < groups[i].size(); j++){
			if(groups[i][j]) cout<<1;
			else cout<<0;
		}
		cout<<endl;
	}
#endif
}
void print_bool_vec(vector<bool> group){
#ifdef _DEBUG
	for(int j = 0; j < group.size(); j++){
		if(group[j]) cout<<1;
		else cout<<0;
	}
	cout<<endl;
#endif
}
void print_vector(vector<int> fitvals){
#ifdef _DEBUG
	for(int i = 0; i < fitvals.size(); i++){
		cout<<fitvals[i]<<" ";
	}
	cout<<endl;
#endif
}
void print_result(){
#ifdef _DEBUG
	cout<<resFeeMin<<" "<<serverNum<<endl;
	for(auto it = serverSetRes.begin(); it != serverSetRes.end(); it++)
		cout<<*it<<" ";
	cout<<endl;
/*	
	for(int i = 0; i < outPathDetail.size(); i++){
		for(int j = 0; j < outPathDetail[i].size(); j++){
			cout<<outPathDetail[i][j]<<" ";
		}
		cout<<endl;	
	}
*/

#endif
}

/// 解析文件的某一行数据，成整数集合
vector<int> parse_line(char *line){
//	cout<<line<<endl;
	int len = strlen(line);	
	//cout<<len<<endl;
	vector<int> ret;
	if(len <= 0)
		return ret;
	int nstartm = 0;
//	cout<<(int)line[7]<<" "<<(int)line[8]<<endl;
	while(*line != '\0' && *line != '\r' && *line != '\n'){
	//	cout<<nstartm<<endl;
		if(*line == ' '){
			ret.push_back(nstartm);
			nstartm = 0;
		}
		else
			nstartm = nstartm*10 + ((*line) - '0');
		line++;
		//count++;
	}
	//cout<<count<<endl;
	//cout<<nstartm<<endl;
	ret.push_back(nstartm);
	return ret;
}
/// 解析整个文件，成整数集合
void parse_infile(char * topo[MAX_EDGE_NUM], int line_num){
	//	cout<<line_num<<endl;//test
	vector<int> netInfo = parse_line(topo[0]);
	networkNodeNum = netInfo[0]; // 网络节点数
	linkNstartm = netInfo[1]; // 链路条数
	consumerNodeNum = netInfo[2]; // 消费节点数
	//cout<<networkNodeNum<<" "<<linkNstartm<<" "<<consumerNodeNum<<endl;
	serverNumMax = consumerNodeNum;
	vector<int> costInfo = parse_line(topo[2]);
	//cout<<costInfo[0];
	costServer = costInfo[0]; // 服务器成本
	int lineIndex;
	vector<int> lineTemp;
	// 设置源点 和 汇点下标
	S = networkNodeNum;
	T = networkNodeNum + 1;
	// 获取网络节点信息
	
	for(lineIndex = 4; lineIndex < line_num-consumerNodeNum-1; lineIndex++){
		lineTemp = parse_line(topo[lineIndex]);
//		cout<<lineTemp[0]<<endl;//test
		graph[lineTemp[0]][lineTemp[1]].bandwith = lineTemp[2];
		graph[lineTemp[0]][lineTemp[1]].price = lineTemp[3];
		graph[lineTemp[1]][lineTemp[0]].bandwith = lineTemp[2];
		graph[lineTemp[1]][lineTemp[0]].price = lineTemp[3];
		// 距离最近节点
		int from = lineTemp[0], to = lineTemp[1];
		int bandwith = lineTemp[2], price = lineTemp[3];
		if(closestNode[from] == -1){
			closestNode[from] = to;
		}else{
			if(graph[from][to].price < graph[from][closestNode[from]].price){
			closestNode[from] = to; 
		}
												}
		if(closestNode[to] == -1){
			closestNode[to] = from;
		}else{
			if(graph[to][from].price < graph[to][closestNode[to]].price){
				closestNode[to] = from; 
			}
		}
		// 邻接节点
		link_graph[lineTemp[0]].push_back(lineTemp[1]);
		link_graph[lineTemp[1]].push_back(lineTemp[0]);
	}
	// 获取消费节点信息
	vector<int> temp(3);
	vector<bool> personTemp(networkNodeNum,false);
	for(lineIndex = line_num-consumerNodeNum; lineIndex < line_num; lineIndex++){
		lineTemp = parse_line(topo[lineIndex]);		
		consumerInfo[lineTemp[0]].netNode = lineTemp[1];
		consumerInfo[lineTemp[0]].need = lineTemp[2];
		feedNeedAll += lineTemp[2];
		// 获取最差的方案
		temp[0] = lineTemp[1];
		temp[1] = lineTemp[0];
		temp[2] = lineTemp[2];
		serverNum = consumerNodeNum;
		outPathDetail.push_back(temp);
		personTemp[lineTemp[1]] = true;
		resFeeMin = consumerNodeNum * costServer;
		// 服务器集合
		serverSetRes.insert(lineTemp[2]);
	//	cout<<temp[0]<<" "<<temp[1]<<" "<<temp[2]<<endl;
	}	
	resPerson = personTemp;// 最优精英个体
	// 
	// 设置需求汇点T
	for(int i = 0; i < consumerNodeNum; i++){
		//
		link_graph[consumerInfo[i].netNode].push_back(T);
		//
		graph[consumerInfo[i].netNode][T].bandwith = consumerInfo[i].need; // 
		graph[consumerInfo[i].netNode][T].price = 0;
	}
}
/// 增加服务器源点
void update_server_resostartrce(vector<int> serverPosition){
	int len = serverPosition.size();
	// 服务器源S
	// 清零
	/*
	for(int i = 0; i < networkNodeNum; i++){
		graph[S][i].bandwith = 0; // 无穷大
		graph[S][i].price = INF; // 
	}
	*/
	graphTemp = graph;
	link_graph[S].clear();// 清空虚拟源汇点的邻接节点
	for(int i = 0; i < len; i++){
		//
		link_graph[S].push_back(serverPosition[i]); // 插入邻接节点
		//
		graphTemp[S][serverPosition[i]].bandwith = 1<<30; // 无穷大		
		graphTemp[S][serverPosition[i]].price = 0; // 初始化为服务器价格
	}
}
/// spfa算法求最小费用最大流问题

bool spfa_once(int source = S, int termit = T){                  //  源点为0，汇点为n。  
	memset(pre,0,sizeof(pre));
	int que[Max];   
	bool vis[Max]; 
	int dis[Max];
    int i, head = 0, tail = 1;  
    int sum = 0;
	int mean = 0;
	int count = 1;
	for(i = 0; i <= termit; i ++){  
        dis[i] = INF;  
        vis[i] = false;  
    }
    dis[source] = 0;// dis 表示 最小 花费  
    que[0] = source;  	
    vis[source] = true;  
	#ifdef _DEBUG
	//cout<<source<<endl;
	#endif

    while(tail != head){      //  循环队列。         
		mean = sum / count;	
		int start = que[head];
		// LLL 算法
		while(dis[start] > mean){
			que[tail++] = start;
			if(tail == Max) tail = 0;
			head++;
			if(head == Max) head = 0;

		    start = que[head];
		}
		++head;
		if(head == Max) head = 0;
		// 优化
		int nextIdx;
		for(int i = 0; i < link_graph[start].size(); i++){
			nextIdx = link_graph[start][i];
			#ifdef _DEBUG
//			cout<<link_graph[start].size();	
//			printf("fff");
			#endif
			if(	(graphTemp[start][nextIdx].bandwith - graphTemp[start][nextIdx].oppsiteDirection) > 0	&& dis[nextIdx] > dis[start] + graphTemp[start][nextIdx].price){    //  存在路径，且权值变小。  
                dis[nextIdx] = dis[start] + graphTemp[start][nextIdx].price;  
                pre[nextIdx] = start;  
                if(!vis[nextIdx]){  
                    vis[nextIdx] = true; 
				    // SLF 算法
					++count; sum += dis[nextIdx];
					//que[tail] = nextIdx;
					if(dis[que[head]] > dis[nextIdx]){
						--head;
						if(head== -1) head = Max-1;
						que[head] = nextIdx;
						//swap(que[head], que[tail]);
					}else{	
						que[tail] = nextIdx;
						tail++;
						if(tail == Max) tail = 0;  
					}
				}
			}  
		}
		/*
		for(i = 0; i <= termit; i ++){  
            if(graphTemp[start][i].bandwith && dis[i] > dis[start] + graphTemp[start][i].price){    //  存在路径，且权值变小。  
                dis[i] = dis[start] + graphTemp[start][i].price;  
                pre[i] = start;  
                if(!vis[i]){  
                    vis[i] = true; 
				    // SLF 算法
					++count; sum += dis[i];
					//que[tail] = i;
					if(dis[que[head]] > dis[i]){
						--head;
						if(head== -1) head = Max-1;
						que[head] = i;
						//swap(que[head], que[tail]);
					}else{	
						que[tail] = i;
						tail++;
						if(tail == Max) tail = 0;  
					}
                }  
            }  
		}*/
		vis[start] = false;  
		--count;sum -= dis[start];
    }  
    if(dis[termit] == INF){
//		cout<<dis[termit]<<endl;//test	
		return false;  
	}
    return true;  
}  

bool spfa_once_init(int source, int termit){                  //  源点为0，汇点为n。  
	memset(pre,0,sizeof(pre));
	int que[Max];   
	bool vis[Max]; 
	int dis[Max];
    int i, head = 0, tail = 1;  
    for(i = 0; i <= termit; i ++){  
        dis[i] = INF;  
        vis[i] = false;  
    }
    dis[source] = 0;// dis 表示 最小 花费  
    que[0] = source;  	
    vis[source] = true;  
  
    while(tail != head){      //  循环队列。  
        int start = que[head];    
        for(i = 0; i <= termit; i ++)  
            if(graph[start][i].bandwith && dis[i] > dis[start] + graph[start][i].price){    //  存在路径，且权值变小。  
                dis[i] = dis[start] + graph[start][i].price;  
                pre[i] = start;  
                if(!vis[i]){  
                    vis[i] = true;  
                    que[tail ++] = i;  
                    if(tail == Max) tail = 0;  
                }  
            }  
        vis[start] = false;  
        head ++;  
        if(head == Max) head = 0;  
    } 
    if(dis[termit] == INF) return false;  
    return true;  
}  
void calculate_cost(int &cost_price){  
    int i, sum = INF;  
    for(i = T; i != S; i = pre[i])  {
		sum = min(sum, (graphTemp[pre[i]][i].bandwith-graphTemp[pre[i]][i].oppsiteDirection)); ///
	}

	feedCurrent += sum;
	#ifdef _DEBUG
//	cout<<feedCurrent<<endl;
	#endif
	for(i = T; i != S; i = pre[i]){  
		int index_pre = pre[i];
		// 源节点不需要计算租金和带宽变化
		if(pre[i] == S){ // 计算服务器的个数
			numOfServerUsed.insert(i);
			break;
		}
		// 目标节点只需要减小正向带宽
		if(i == T){
			graphTemp[index_pre][i].bandwith -= sum;
			continue;
		}
		int existFeed = graphTemp[index_pre][i].oppsiteDirection + sum;
		if(existFeed <= 0){// 逆流大
			graphTemp[index_pre][i].oppsiteDirection = existFeed;			
			graphTemp[i][index_pre].bandwith += sum;
			cost_price -= graphTemp[index_pre][i].price * sum;   //  cost[][]记录的为单位流量费用，必须得乘以流量。
		}else{
			graphTemp[i][index_pre].bandwith -= graphTemp[index_pre][i].oppsiteDirection; // 还原
			graphTemp[i][index_pre].oppsiteDirection = -existFeed;
							
			cost_price += graphTemp[index_pre][i].price*(existFeed + graphTemp[index_pre][i].oppsiteDirection);
																												
			graphTemp[index_pre][i].oppsiteDirection = 0;
			graphTemp[index_pre][i].bandwith -= existFeed;			
		}
		/*
        graphTemp[pre[i]][i].bandwith -= sum;
        cost_price += graphTemp[pre[i]][i].price * sum;   //  cost[][]记录的为单位流量费用，必须得乘以流量。  
		*/
    }
	//	cout<<cost_price<<endl;//test
}
int spfa_augmented(){
	int cost_price = 0;	

	while(spfa_once()) calculate_cost(cost_price);
	return cost_price;
}

/// 遗传算法每次迭代运行这个函数 ,求一种方案的最终流量租用费
int get_fee_base_on_server_position(vector<int> serverPosition){
	// 更新
	update_server_resostartrce(serverPosition);
	//graphTemp = graph;
	return spfa_augmented();
}

int compare(pair<int, int> p1, pair<int, int> p2){
	return p1.second > p2.second;	
}
void new_answer(vector<bool> &S1, vector<bool> &S2, int bigPm, int smallPm){

	S2 = S1;
		
   	
	//	int M = S1.size();
	for(int i = 0; i < networkNodeNum; i++){
		if(resPerson[i] == S2[i]){
			if(rand_num(1000) < smallPm){
				S2[i] = !S2[i];
			}
		}else{
			if(rand_num(1000) < bigPm){
				S2[i] = !S2[i];
			}
		}
		/*
		// 随机变异
		if(rand_num(1000) < delPm){
			S2[i]  = !S2[i];
			continue;
		}
		*/
		// 随机 转移到最近的邻接点，并随机删除
		/*
		if(S2[i]){
			if(rand_num(1000) < movePm){
			//  S2[i] = false;
				S2[closestNode[i]] = true;;		
			}	
		}
		*/
		/*
		//随机转移到邻接节点
		if(S2[i]){
			if(rand_num(1000) < delPm){
				S2[i] = false;
				continue;
			}
			if(rand_num(1000) < movePm){				
				int idx_next = 0;
				//while(S2[idx_next]){
				idx_next = link_graph[i][rand_num(link_graph[i].size())];
				//}
				//cout<<idx_next<<"FFFFFFF";//test
				S2[idx_next] = true;
				S2[i] = false;
			}
		}*/
	}
	
}
/// 获取最终的链路信息
void get_result(vector<bool> personGene);
int getPersonCost(vector<bool> &personGene);
vector<bool> SA(vector<bool> initPerson){
	vector<bool> S1 = initPerson;
	vector<bool> S2 = S1;
	double T0 = 500;
	double Tend = 0.001;
	
	double q = 0.95;
	
	int L = 50;
	if(networkNodeNum > 600){
		L = 15;
		q = 0.95;
	}

	int timeL;
	int TE = 50; // 
	int timeEnd = 0;
	int outCost = getPersonCost(S1);
	int newCost;
	int diff = 0;
	int LinkCostPre = outCost;
	double expDiff  = 0;
	/// 随机扰动 概率初始化
	int smallPm = 3;
	int biggerPm = 6;
	
#ifdef _DEBUG
	//cout<<"SA Init:"<<endl;
#endif
	while(T0 > Tend && timeEnd < TE){
		if(timeEnd >= 25){
			smallPm<<4;
			biggerPm<<4;
		}
#ifdef _DEBUG
		
		cout<<"T0:"<<T0<<"; CurrCost:"<<outCost<<"; MinFee:"<<resFeeMin<<"; timeEnd:"<<timeEnd<<endl;
#endif
		timeL = 0;
		while(timeL++ < L){
#ifdef _DEBUG
			
#endif
		//	print_person(S1);
			new_answer(S1, S2, biggerPm, smallPm);
		//	print_person(S2);
		//	print_time("SPFA Time:");
			newCost = getPersonCost(S2);
		//	print_time("end");
			if(newCost == INF){
				continue;
			}
			diff = newCost - outCost;
			if(diff < 0){
				S1 = S2;
				outCost = newCost;
				if(outCost < resFeeMin){
					resPerson = S1;
					resFeeMin = outCost;
				}
				continue;
			}
			expDiff = exp(-diff/T0);
			if(rand_num(1000)/1000.0 < expDiff){
				S1 = S2;
				outCost = newCost;					
			}						
			
		}
		if(LinkCostPre == outCost){
			++timeEnd;			
		}else{
			timeEnd = 0;
			LinkCostPre = outCost;
		}
		T0 = q*T0;
		if(is_timeout()){// 超时退出
			#ifdef _DEBUG
			printf("timeout\n");
			#endif
			break;
		}
	}	
	//resPerson = S1;
	//resFeeMin = outCost;
	//get_result(resPerson);
	return S1;
}
/// 初始化种群
void init_groups(vector<vector<bool> > &groups, int M){
#ifdef _DEBUG
	cout<<"init_groups"<<endl;
#endif
	
	/*for(int i = 0; i < M; i++){
		vector<bool> svrIndex(networkNodeNum, false);
		for(int count = 0; count < consumerNodeNum; count++){
			int idx;
			do{
				idx = rand() % networkNodeNum;
			}while(svrIndex[idx]);
			svrIndex[idx] = true;
			bool isServer = (rand() % 100 < 50);	
			groups[i][idx] = isServer;
		}
	}*/
	/*
	*/
	// 消费节点设置成无服务器地址
	groups[0] = resPerson;

	// SA算法求初始值
	groups[1] = SA(resPerson);
	groups[2] = groups[1];
	if(is_timeout()){
		return;
	}
	// 随机产生服务节点
	for(int i = 3; i < M; i++){		
		for(int j = 0; j < networkNodeNum; j++){	
			groups[i][j] = rand_num(networkNodeNum) < consumerNodeNum? true: false;
		}
	}

	// 计算交汇最多的点	
	/*	
	for(int i = 0; i < consumerNodeNum - 1; i++){
		for(int j = i+1; j < consumerNodeNum; j++){
			int source = consumerInfo[i].netNode;
			int termin = consumerInfo[j].netNode;
//			cout<<source<<" "<<termin<<endl;
			if(spfa_once_init(source, termin)) {
				
				for(int k = termin; k != source; k = pre[k]){  
					//cout<<k<<" ";
					initNodes.push_back(k);
					if(k != termin)
						initNodesMp[k]++;
				}	
				initNodes.push_back(source);	
			//	initNodesMp[source]++;
			}
		}
	}
	// 排序候选节点
	for(auto it = initNodesMp.begin(); it != initNodesMp.end(); it++){
		sortNodesMp.push_back(make_pair(it->first, it->second));
	}
	sort(sortNodesMp.begin(), sortNodesMp.end(), compare);
	// 将候选节点全部设成服务点
	for(int i = 0; i < sortNodesMp.size() && i < networkNodeNum; i++){
		groups[2][i] = sortNodesMp[i].first;
	}
	for(int i = 0; i < serverNumMax; i++){
		groups[3][i] = sortNodesMp[i].first;
	}

	// 随机从候选节点中选服务器
	int size = initNodes.size();
//	sort(initNodes.begin(), initNodes.end());
	//print_vector(initNodes);
	for(int i = 4; i < M; i++){
		int edge = (consumerNodeNum + networkNodeNum)>>1;
		for(int count = 0; count < serverNumMax; count++){
			int idx = initNodes[rand_num(size)];
			groups[i][idx] = true;
		}
	}*/
#ifdef _DEBUG
//	for(int i = 0; i < sortNodesMp.size(); i++){
//		cout<<sortNodesMp[i].first<<":"<<sortNodesMp[i].second<<endl;
//	}

#endif
	/*
	// 将消费节点设置成服务器
	for(int i = 0; i < consumerInfo.size(); i++){
		int idx = consumerInfo[i].netNode;
		groups[1][idx] = true;
		
	}*/
	

} 
/// 获取个体成本费用
int getPersonCost(vector<bool> &personGene){
	vector<int> svrPosition;
	for(int i = 0; i < networkNodeNum; i++){
		if(personGene[i])
			svrPosition.push_back(i);
	}	
	// 总费用
	feedCurrent = 0;
	numOfServerUsed.clear();
	int cost_all = get_fee_base_on_server_position(svrPosition)
				 + costServer * numOfServerUsed.size();
	#ifdef _DEBUG
	//cout<<personGene[0]<<endl;
	#endif
	if(feedCurrent != feedNeedAll){
		//cout<<"error server"<<endl;
		return INF;
	}
	if(cost_all <= 0){
		return INF;
	}
	
	// 超过最大服务器个数
	if(numOfServerUsed.size() > serverNumMax){
		return INF;
	}
	// 更新个体基因
	for(int i = 0; i < personGene.size(); i++){
		personGene[i] = false;				
	}
	for(auto it = numOfServerUsed.begin(); it != numOfServerUsed.end(); it++){
		personGene[*it] = true;
	}
	
	return cost_all;
	
}
/// 获取适应度值
void getFitnessVal(vector<int> &fitnessVals){
	int len = fitnessVals.size();
	int minVal = INF;//fitnessVals[0];
	int maxVal = 0;
	for(int i = 0; i < len; i++){
		if(fitnessVals[i] >= INF)
			continue;
		if(minVal > fitnessVals[i]){
			minVal = fitnessVals[i];			
		}
		if(maxVal < fitnessVals[i]){
			maxVal = fitnessVals[i];
		}
	}


	int guiyi = (maxVal - minVal) / serverNum;
	if(guiyi == 0)
	{
		guiyi = 1;
	}
#ifdef _DEBUG
	cout<<"chayi:"<<maxVal-minVal<<",guiyi:"<<guiyi;
#endif
	int fitnessMax = 0;
	int sum = 0;
	int count = 0;
	int top = maxVal+1;//(maxVal<<1) - minVal;
	for(int i = 0; i < len; i++){
		if(fitnessVals[i] >= INF)
			fitnessVals[i] = 0;
		else{
			
			fitnessVals[i] = top - fitnessVals[i];
			fitnessVals[i] = (int)pow(fitnessVals[i]/guiyi, 3.0);
			if(fitnessVals[i] > fitnessMax)
				fitnessMax = fitnessVals[i];
			sum += fitnessVals[i];
			count++;
		}
	}
	currentAvgFitness = (double)fitnessMax / (double)count;
	currentMaxFitness = (double)fitnessMax;
	
}

/// 获取最终的链路信息
void get_result(vector<bool> personGene){
	getPersonCost(personGene);
	vector<vector<networkNode> > newGraph(networkNodeNum+2, vector<networkNode>(networkNodeNum+2));// 初始化
	int bandwith = 0;
	vector<vector<int> > linkTemp(T+2);
	link_graph.clear();// 清空虚拟源汇点的邻接节点
	for(int i = 0; i <= T; i++){
		for(int j = 0; j <= T; j++){
			bandwith = graphTemp[i][j].oppsiteDirection;
			if(bandwith < 0){
				linkTemp[j].push_back(i);// 添加临界节点
				newGraph[j][i].price = graphTemp[j][i].price;
				newGraph[j][i].bandwith = -bandwith;
				newGraph[j][i].oppsiteDirection = 0;				
			}
		}		
	}
	link_graph = linkTemp;
	// 源汇点
	for(auto it = numOfServerUsed.begin(); it != numOfServerUsed.end(); it++){
		//
		link_graph[S].push_back(*it); // 插入邻接节点
		//
		newGraph[S][*it].bandwith = 1<<30; // 无穷大		
		newGraph[S][*it].price = 0; // 
	}
	// 目标汇点
	for(int i = 0; i < consumerNodeNum; i++){
		//
		link_graph[consumerInfo[i].netNode].push_back(T);
		//
		graph[consumerInfo[i].netNode][T].bandwith = consumerInfo[i].need; // 
		graph[consumerInfo[i].netNode][T].price = 0;
	}
	
	graphTemp = newGraph;
	// 初始化消费节点与网络节点的键值对
	map<int, int> mpNetNode2ConNode;
	for(int i = 0; i < consumerNodeNum; i++){
		mpNetNode2ConNode[consumerInfo[i].netNode] = i;
	}
	int cost_price = 0;
	serverSetRes.clear();
	while(spfa_once()) {
		vector<int> pathTemp;
		int i, sum = INF;  
		for(i = T; i != S; i = pre[i]){  
			sum = min(sum, graphTemp[pre[i]][i].bandwith);  
			pathTemp.push_back(sum);
																
			for(i = T; i != S; i = pre[i]){  
				if(pre[i] == S){
					serverSetRes.insert(i);
				}
				if(i == T){
					int conNodeTemp = mpNetNode2ConNode[pre[i]];
					pathTemp.push_back(conNodeTemp);
				}else{
					pathTemp.push_back(i);
				}
				graphTemp[pre[i]][i].bandwith -= sum;  
				cost_price += graphTemp[pre[i]][i].price * sum;  
				cout<<"cost:"<<cost_price<<"; bandwidth:"<<sum<<endl;
			}		
			reverse(pathTemp.begin(), pathTemp.end());
			outPathDetail.push_back(pathTemp);
		}
		serverNum = serverSetRes.size();
	}
#ifdef _DEBUG
	printf("Result Fee:%d\n", cost_price);
#endif
/*
	outPathDetail.clear();
	vector<int> svrPosition;
	for(int i = 0; i < networkNodeNum; i++){
		if(personGene[i])
			svrPosition.push_back(i);
	}
	//serverNum = svrPosition.size();
	// 更新
	update_server_resostartrce(svrPosition);
	//graphTemp = graph;	
	//int cost_price = 0;
	map<int, int> mpNetNode2ConNode;
	for(int i = 0; i < consumerNodeNum; i++){
		mpNetNode2ConNode[consumerInfo[i].netNode] = i;
	}

	serverSetRes.clear();
	while(spfa_once()) {
		vector<int> pathTemp;
		int i, sum = INF;  
		for(i = T; i != S; i = pre[i])  
			sum = min(sum, graphTemp[pre[i]][i].bandwith);  
		pathTemp.push_back(sum);
		
		for(i = T; i != S; i = pre[i]){  
			if(pre[i] == S)
				serverSetRes.insert(i);
			if(i == T){
				int conNodeTemp = mpNetNode2ConNode[pre[i]];
				pathTemp.push_back(conNodeTemp);
			}else{
				pathTemp.push_back(i);
			}
			graphTemp[pre[i]][i].bandwith -= sum;  
			//graphTemp[i][pre[i]] += sum;  
			//cost_price += graphTemp[pre[i]][i].price * sum;   //  cost[][]记录的为单位流量费用，必须得乘以流量。  
		}		
		reverse(pathTemp.begin(), pathTemp.end());
		outPathDetail.push_back(pathTemp);
	}
	serverNum = serverSetRes.size();
*/
}
/// 交叉
void cross_bind(vector<bool> &group1, vector<bool> &group2, int crossIdx1, int crossIdx2){
	for(int i = 0; i < networkNodeNum; i++){
		if(i < crossIdx1 || i > crossIdx2){
			bool tmp = group1[i];
			group1[i] = group2[i];
			group2[i] = tmp;
		}
	}
}
/// 交叉变异
void recombind(vector<vector<bool> > &groups, int crossPm){
	//
	/*
	int M = groups.size();
	for(int i = 0; i < M; i++){
		for(int j = 0; j < networkNodeNum; j++){
			if(groups[i][j]){
				if(rand_num(100) < crossPm){
					//groups[i][j] = false;
					//int idx_next = link_graph[i][rand_num(link_graph[i].size())];
					//groups[i][idx_next] = true;
					
					// 左右移动
					if(rand_num(100) < 50){
						if(j == 0)
							groups[i][networkNodeNum-1] = true;
						else
							groups[i][j-1] = true;
					}	
					else{						
						if(j == networkNodeNum-1)
							groups[i][0] = true;
						else
							groups[i][j+1] = true;
					}
					
				}
			}
		}
	}*/
	
	int M = groups.size();
	vector<bool> flag(M,false);
	// 生成随机下标
	vector<int> indexs;
	for(int i = M; i > 0; i--){
		int idx = rand() % i;
		while(flag[idx]){
			idx++;
		}
		flag[idx] = true;
		indexs.push_back(idx);
	}
	
	// 交叉变异
	for(int i = 0; i < (M/2); i++){
		int crossIdx1 = rand() % networkNodeNum;
		int crossIdx2 = rand() % networkNodeNum;
		//if(crossIdx1 > crossIdx2){
		//	swap(crossIdx1, crossIdx2);
		//}
		//cout<<crossIdx1<<" "<<crossIdx2<<endl;
		for(int j = 0; j < M; j += 2){
			if(rand_num(100) < crossPm){
				cross_bind(groups[indexs[j]], groups[indexs[j+1]], crossIdx1, crossIdx2);
			}
		}
		//print_groups(groups);
		
	}
	//print_groups(groups);
	
}
/// 变异
int number_of_server_in_group(vector<bool> group){
	int count = 0;
	for(int i = 0; i < group.size(); i++){
		if(group[i])
			count++;
	}
	return count;
}
void mutation(vector<vector<bool> > &groups, int M, int mutatPm, int addPm){
/*	int rate = mutatPm;
	if(currentAvgFitness > currentMaxFitness*0.95){
		rate = mutatPm * 8;
#ifdef _DEBUG
		cout<<"rate:"<<rate<<endl;
#endif
	}*/
/*
	
	for(int i=0;i<M;i++){//种群个数
		if(rand_num(1000) < mutatPm){//该个体是否变异
			int mutat_num = rand_num(4)+1;//该个体需要变异基因的个数
			for(int j=0;j<mutat_num;j++){
				int index=rand_num(networkNodeNum);//变异的位置;
				groups[i][index]=!groups[i][index];//变异
			}
		 }
    }
*/
	for(int j = 0; j < M; j++){
		
		for(int i = 0; i < networkNodeNum; i++){
			if(resPerson[i] == groups[j][i]){
				if(rand_num(1000) < addPm){
					groups[j][i] = !groups[j][i];
				}
			}else{
				if(rand_num(1000) < mutatPm){
					groups[j][i] = !groups[j][i];
				}
			}
		}	
	}
	/*
	for(int i = 0; i < M; i++){
		for(int j = 0; j < networkNodeNum; j++){
			if(rand_num(1000) < addPm){
				groups[i][j] = !groups[i][j];
			}
		}
	}
*/
	/*for(int i = 0; i < M; i++){
		for(int j = 0; j < networkNodeNum; j++){	
			if(groups[i][j]){
				if(rand_num(100) < mutatPm){
					//groups[i][j] = false;		
					// 左右移动
					if(rand_num(100) < 50){
						if(j == 0)
							groups[i][networkNodeNum-1] = true;
						else
							groups[i][j-1] = true;
					}	
					else{						
						if(j == networkNodeNum-1)
							groups[i][0] = true;
						else
							groups[i][j+1] = true;
					}
				}
			}			
		}
	}*/
	// 新增节点
	
}

void write_resule_to_file(vector<vector<int> > paths, char* filename){
	string res;
	char temp[20];
	sprintf(temp,"%d\n\n", serverNum);
	//cout<<temp<<endl;//test
	res += temp;
	for(int i = 0; i < paths.size(); i++){
		for(int j = 0; j < paths[i].size(); j++){
			if(j == 0){
				sprintf(temp, "%d", paths[i][j]);
			}else{
				sprintf(temp, " %d", paths[i][j]);
			}
			res += temp;
		}	
		res += "\n";
	}
	char * topo_file = (char *)res.c_str();
    write_result(topo_file, filename);
}

void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
	parse_infile(topo, line_num);
	//cout<<"parse_infile"<<endl;
	srand((unsigned)time(NULL));   
	// 遗传算法迭代循环
	int N = 300; // 迭代次数
	int M = 40; // 种群规模
	if(networkNodeNum > 600)
		M = 10;
	
#ifdef _DEBUG
	cout<<M<<endl;
#endif
	int codeLength = networkNodeNum;
	vector<vector<bool>> groups(M,vector<bool>(codeLength, false));

	int optimalFee = INF; // 最优个体费用
	int optimalFeePre = INF; 
	int optimalFitValPre = 1; // 最优个体适应度值前一个
	int optimalFitValNow = 1; // 最优个体适应度值当前个
	int optimalNum=0; // 最优个体编
	int worstNum = 0; // 最差个体编号
	int worstFee = INF; // 最差个体费用
	vector<int> fitnessVals(M); // 种群对应的适应度值
	int crossPm = 90; // 交叉概率
	int mutatPm = 4;  // 变异概率
	int addPm = 2; // 新增概率，千分之
	int count = 0; 
	int countEnd = 10; // 终止条件
	double minP = 0.002; // 最优适应度值增量, 可以考虑动态变化的概率
	int deviation = 30; 
	bool timeout = false;
	/* 初始化种群 */ 
	init_groups(groups, M);
	
	//resFeeMin = INF;

	while(N--){
	/*	if(N < 90){
			addPm = 5;
		}else if(N < 120){
			addPm = 10;
		}*/
		/* 计算适应度值 */
#ifdef _DEBUG
		cout<<N<<": "<<endl;
#endif
		optimalFee = INF; 
		worstFee = 0;
		optimalNum = -1;
		worstNum = -1;
		//print_groups(groups);// test
		for(int i = 0; i < M; i++){
			timeout = is_timeout();
			/* 判断终止条件 */
			if(timeout){
				break;
			}

			fitnessVals[i] = getPersonCost(groups[i]);
		//	cout<<fitnessVals[i];// test
		//	system("pause");//test
			if(INF == fitnessVals[i]){
				continue;
			}
			if(fitnessVals[i] < optimalFee){
				optimalFee = fitnessVals[i]; // 
				optimalNum = i;
			}			
			if(fitnessVals[i] > worstFee){
				worstFee = fitnessVals[i];
				worstNum = i;
			}
			//cout<<fitnessVals[i]<<" ";
		}
		if(timeout){
			break;
		}
		
		if(optimalNum == -1 || worstNum == -1){
			init_groups(groups, M);
			N++;
			continue;
		}
		groups[worstNum] = resPerson;
		fitnessVals[worstNum] = resFeeMin;
		//cout<<optimalNum<<" "<<worstNum<<" ";
#ifdef _DEBUG
		cout<<"最小费用:"<<optimalFee<<";";
#endif
		vector<int> fitTemp = fitnessVals;
		getFitnessVal(fitnessVals);
		//print_time("计算适应度值");
		/* 选择最优个体适应度值 */
		optimalFitValNow = fitnessVals[optimalNum];
#ifdef _DEBUG
		cout<<"最优适应度值:"<<optimalFitValNow<<";服务器个数："<<number_of_server_in_group(groups[optimalNum])<<endl;
#endif
		//print_bool_vec(groups[optimalNum]);
		if(optimalFee < resFeeMin){
			resPerson = groups[optimalNum];
			resFeeMin = optimalFee;
			//get_result(groups[optimalNum]);
		//	print_result();
		}
		
		if(optimalFeePre - optimalFee < deviation){
			count++;
			
			if(count == countEnd)
				break;
		}else{
			count = 0;
		}
		/*if((double)(optimalFitValNow - optimalFitValPre) / (double)optimalFitValPre <= minP){
			count++;
			if(count == countEnd)
				break;
		}else{
			count = 0;
		}
		optimalFitValPre = optimalFitValNow;
		*/
		optimalFeePre = optimalFee;
		/* 轮盘赌选择 */
		//fitnessVals[worstNum] = fitnessVals[optimalNum];
		//groups[worstNum] = groups[optimalNum];
		//print_groups(groups);
		//print_vector(fitnessVals);
		vector<int> pan(M);
		for(int i = 0; i < M; i++){
			if(i == 0)
				pan[i] = fitnessVals[i];
			else
				pan[i] = pan[i-1] + fitnessVals[i];
		}
		if(pan[M-1] == 0){
			init_groups(groups, M);
			continue;
			//print_groups(groups);
			//cout<<"";
		}
		//print_vector(pan);
		vector<vector<bool>> groupsNew;
		for(int i = 0; i < M; i++){
			int ran = rand_num(pan[M-1]);
			for(int j = 0; j < M; j++){
				if(ran >= pan[j])
					continue;
				groupsNew.push_back(groups[j]);
				break;
			}
		}
		//print_time("轮盘赌选择");
		//print_groups(groupsNew);
		/* 交叉变异 */
		recombind(groupsNew, crossPm);
		//print_time("交叉变异");
		/* 变异算子 */
		mutation(groupsNew, M, mutatPm, addPm);
		groups = groupsNew;
		//print_bool_vec(groups[optimalNum]);
		//print_time("变异算子");
	}
	//print_time("End");
	get_result(resPerson);
	print_result();
	write_resule_to_file(outPathDetail, filename);
}
