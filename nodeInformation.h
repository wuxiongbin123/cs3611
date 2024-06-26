#ifndef nodeInfo_h
#define nodeInfo_h

#include <iostream>
#include <vector>
#include <map>

#include "port.h"
#include "M.h"

using namespace std;

typedef long long int lli;

// id 是 long long int 的类型， ip 是 string 类型， port 是 整数类型， hash 也是 lli 类型。

class NodeInformation{
	private:
		lli id;
		pair< pair<string,int> , lli > predecessor;
		pair< pair<string,int> , lli > successor; 
		vector< pair< pair<string,int> , lli > > fingerTable;
		map<lli,string> dictionary;
		vector< pair< pair<string,int> , lli > > successorList;

		bool isInRing;

	public:
		SocketAndPort sp;
		
		NodeInformation();

		pair< pair<string,int> , lli > findSuccessor(lli nodeId, bool printRouting); //flag代表是否需要输出路由寻址过程.
        pair< pair<string,int> , lli > findSuccessor_routing(lli nodeId, bool printRouting, pair< pair<string, int>, lli> &serverNodeId); //flag代表是否需要输出路由寻址过程.
		pair< pair<string,int> , lli > closestPrecedingNode(lli nodeId);
		void fixFingers();
		void stabilize();
		void notify(pair< pair<string,int> , lli > node);
		void checkPredecessor();
		void checkSuccessor();
		void updateSuccessorList();

		void printKeys();
		void storeKey(lli key,string val);
		vector< pair<lli , string> > getAllKeysForSuccessor();
		vector< pair<lli , string> > getKeysForPredecessor(lli nodeId);

		void setSuccessor(string ip,int port,lli hash);
		void setSuccessorList(string ip,int port,lli hash);
		void setPredecessor(string ip,int port,lli hash);
		void setFingerTable(string ip,int port,lli hash);
		void setId(lli id);
		void setStatus();
		
		lli getId();
		string getValue(lli key);
		vector< pair< pair<string,int> , lli > > getFingerTable();
		pair< pair<string,int> , lli > getSuccessor();
		pair< pair<string,int> , lli > getPredecessor(); 
		vector< pair< pair<string,int> , lli > > getSuccessorList();
		bool getStatus();
};

class Package{
public:
	char ipAndPortChar[40];
	char keyId[40];
	bool printRouting;
	// int jumps;
	Package(){
		// jumps = 0;
		printRouting = false;
		}
};

#endif