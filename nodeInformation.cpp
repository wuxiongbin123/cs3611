#include <iostream>

#include "headers.h"
#include "M.h"
#include "nodeInformation.h"
#include "functions.h"
#include "helperClass.h"

using namespace std;


NodeInformation::NodeInformation(){
	fingerTable = vector< pair< pair<string,int> , lli > >(M+1);
	successorList = vector< pair< pair<string,int> , lli > >(R+1);
	isInRing = false;
}

void NodeInformation::setStatus(){
	isInRing = true;
}

void NodeInformation::setSuccessor(string ip,int port,lli hash){
	successor.first.first = ip;
	successor.first.second = port;
	successor.second = hash;
}

void NodeInformation::setSuccessorList(string ip,int port,lli hash){
	for(int i=1;i<=R;i++){
		successorList[i] = make_pair(make_pair(ip,port),hash);
	}
}

void NodeInformation::setPredecessor(string ip,int port,lli hash){
	predecessor.first.first = ip;
	predecessor.first.second = port;
	predecessor.second = hash;	
}

void NodeInformation::setId(lli nodeId){
	id = nodeId;
}

void NodeInformation::setFingerTable(string ip,int port,lli hash){
	for(int i=1;i<=M;i++){
		fingerTable[i] = make_pair(make_pair(ip,port),hash);
	}
}

void NodeInformation::storeKey(lli key,string val){
	dictionary[key] = val;
}

void NodeInformation::printKeys(){
	map<lli,string>::iterator it;

	for(it = dictionary.begin(); it != dictionary.end() ; it++){
		cout<<it->first<<" "<<it->second<<endl;
	}
}

void NodeInformation::updateSuccessorList(){

	HelperFunctions help;

	vector< pair<string,int> > list = help.getSuccessorListFromNode(successor.first.first,successor.first.second);

	if(list.size() != R)
		return;

	successorList[1] = successor;

	for(int i=2;i<=R;i++){
		successorList[i].first.first = list[i-2].first;
		successorList[i].first.second = list[i-2].second;
		successorList[i].second = help.getHash(list[i-2].first + ":" + to_string(list[i-2].second));
	}

}


/* send all keys of this node to it's successor after it leaves the ring */
vector< pair<lli , string> > NodeInformation::getAllKeysForSuccessor(){
	map<lli,string>::iterator it;
	vector< pair<lli , string> > res;

	for(it = dictionary.begin(); it != dictionary.end() ; it++){
		res.push_back(make_pair(it->first , it->second));
		dictionary.erase(it);
	}

	return res;
}

vector< pair<lli , string> > NodeInformation::getKeysForPredecessor(lli nodeId){
	map<lli,string>::iterator it;

	vector< pair<lli , string> > res;
	for(it = dictionary.begin(); it != dictionary.end() ; it++){
		lli keyId = it->first;

		/* if predecessor's id is more than current node's id */
		if(id < nodeId){
			if(keyId > id && keyId <= nodeId){
				res.push_back(make_pair(keyId , it->second));
				dictionary.erase(it);
			}
		}

		/* if predecessor's id is less than current node's id */
		else{
			if(keyId <= nodeId || keyId > id){
				res.push_back(make_pair(keyId , it->second));
				dictionary.erase(it);
			}
		}
	}

	return res;
}

pair< pair<string,int> , lli > NodeInformation::findSuccessor(lli nodeId, bool printRouting){

	pair < pair<string,int> , lli > self; // self 的元素构成 ip, port, id.
	self.first.first = sp.getIpAddress(); //sp 是socketAndPort， 组成是 socket 和 端口号
	self.first.second = sp.getPortNumber();
	self.second = id;
	
	if(printRouting){

    	// // 打印当前节点信息
    	cout << "Current node ID: " << id << ", IP: " << sp.getIpAddress() << ", Port: " << sp.getPortNumber() << endl;

	}
	
	if(nodeId > id && nodeId <= successor.second){
		//需要找的id是在本Node和后继Node的id之间，所以应该是要在后继node里面寻找。
		if(printRouting) cout << "Key is stored in the node with IP: " << successor.first.first << ", Port: " << successor.first.second << endl; 
		return successor;
	}

	/* */
	else if(id == successor.second || nodeId == id){
		//在 自己的id和后继的id一样，说明环上只有一个点，而且发现就是我要找id 就是这个点的id，所以就返回这个点。
		if(printRouting) cout << "Key is stored in the node with IP: " << self.first.first << ", Port: " << self.first.second << endl; 

		return self;
	}

	else if(successor.second == predecessor.second){
		if(successor.second >= id){
			if(nodeId > successor.second || nodeId < id){
				if(printRouting){
cout << "Key is stored in the node with IP: " << self.first.first << ", Port: " << self.first.second << endl; 
				}

				return self;
			}
		}
		else{
			if((nodeId > id && nodeId > successor.second) || (nodeId < id && nodeId < successor.second)){
				if(printRouting){
cout << "Key is stored in the node with IP: " << successor.first.first << ", Port: " << successor.first.second << endl; 
				}

				return successor;
			}
			else{
				if(printRouting){
cout << "Key is stored in the node with IP: " << self.first.first << ", Port: " << self.first.second << endl; 		
				}

				return self;
			}
		}
	}

	else{
		//这里实现的是非线性查找。
		pair < pair<string,int> , lli > node = closestPrecedingNode(nodeId);//这个node是前驱。
		if(printRouting){
			cout << "Routing to the node for ID: " << nodeId << " is ID: " << node.second 
			<< ", IP: " << node.first.first << ", Port: " << node.first.second << endl;
		}

		if(node.second == id){
			//key的前驱的id等于当前节点的id， 肯定key是存在于当前节点这里。
			if(printRouting){
				cout << "Key is stored in the node with IP: " << successor.first.first << ", Port: " << successor.first.second << endl; 
			}

			return successor;
		}
		else{

			/* connect to node which will now find the successor */
			struct sockaddr_in serverToConnectTo;
			socklen_t len = sizeof(serverToConnectTo);

			string ip;
			int port;

			/* if this node couldn't find closest preciding node for given node id then now ask it's successor to do so */
			if(node.second == -1){
				node = successor;
			}

			HelperFunctions help;

			help.setServerDetails(serverToConnectTo,node.first.first,node.first.second);

			/* set timer on this socket */
    		struct timeval timer;
    		help.setTimer(timer);

			//bug！！！！
			//这里不应该是监听后继的ip和port，而是要发送给后继需要寻找的key，然后后继回传key所在的node的id。
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！
			//------------------------------------------------------------
			//这里其实没有bug，这里实现了递归查询，虽然我不知道接收到socket来信之后执行的代码片段，
			//但是这里给当前节点所能找到的距离keyId最近的node进行了通信，让它递归查询keyId，并且回传key所在的node的IP,port,id信息。
			int sockT = socket(AF_INET,SOCK_DGRAM,0);

			setsockopt(sockT,SOL_SOCKET,SO_RCVTIMEO,(char*)&timer,sizeof(struct timeval));

			if(sockT < 0){
				cout<<"socket cre error";
				perror("error");
				exit(-1);
			}

			/* send the node's id to the other node */
			//这里把要查询的keyId递归地传给了当前节点所能找到的距离key最近的前驱node，to_string(nodeId)就是证据。
			char nodeIdChar[40];
			strcpy(nodeIdChar,to_string(nodeId).c_str());
			sendto(sockT, nodeIdChar, strlen(nodeIdChar), 0, (struct sockaddr*) &serverToConnectTo, len);

			/* receive ip and port of node's successor as ip:port*/
			// 这里收到的ip  ,port 其实就是已经经过递归，回传的精确的位置了。
			char ipAndPort[40];

			int l = recvfrom(sockT, ipAndPort, 1024, 0, (struct sockaddr *) &serverToConnectTo, &len);

			close(sockT);
			//此时，l的信息是节点的后继的ip和port
			if(l < 0){
				pair < pair<string,int> , lli > node;
				node.first.first = "";
				node.second = -1;
				node.first.second = -1;
				return node;
			}

			ipAndPort[l] = '\0';

			/* set ip,port and hash for this node and return it */
			string key = ipAndPort;
    		lli hash = help.getHash(ipAndPort);
    		pair<string,int> ipAndPortPair = help.getIpAndPort(key);
    		node.first.first = ipAndPortPair.first;
    		node.first.second = ipAndPortPair.second;
    		node.second = hash;
			// if(printRouting) cout << "Key is stored in the node with IP: " << node.first.first << ", Port: " << node.first.second << endl; 

    		return node;
		}
	}
}

pair< pair<string,int> , lli > NodeInformation::closestPrecedingNode(lli nodeId){
	HelperFunctions help;

	for(int i=M;i>=1;i--){
		if(fingerTable[i].first.first == "" || fingerTable[i].first.second == -1 || fingerTable[i].second == -1){
			continue;
		}

		if(fingerTable[i].second > id && fingerTable[i].second < nodeId){
			//这里的nodeId是key的ID，条件判断是如果finger表储存的节点介于当前节点和查询的keyId之间，则返回fingerTable[i].
			return fingerTable[i];
		}
		else{

			lli successorId = help.getSuccessorId(fingerTable[i].first.first,fingerTable[i].first.second);

			if(successorId == -1)
				continue;

			if(fingerTable[i].second > successorId){
				//此时在f[i]和它的后继跨过环的末尾。
				if((nodeId <= fingerTable[i].second && nodeId <= successorId) || (nodeId >= fingerTable[i].second && nodeId >= successorId)){
					//这里没有问题，都小于说明id很小，很接近环头，都大于说明很接近环尾，这里node就被finger和它的后继bound住了。
					return fingerTable[i];
				}
			}
			else if(fingerTable[i].second < successorId && nodeId > fingerTable[i].second && nodeId < successorId){
				return fingerTable[i];
			}
			
			pair< pair<string,int> , lli > predNode = help.getPredecessorNode(fingerTable[i].first.first,fingerTable[i].first.second,"",-1,false);
			lli predecessorId = predNode.second;

			if(predecessorId != -1 && fingerTable[i].second < predecessorId){
				if((nodeId <= fingerTable[i].second && nodeId <= predecessorId) || (nodeId >= fingerTable[i].second && nodeId >= predecessorId)){
					return predNode;
				}		
			}
			if(predecessorId != -1 && fingerTable[i].second > predecessorId && nodeId >= predecessorId && nodeId <= fingerTable[i].second){
				return predNode;
			}
		}
	}

	/* */
	pair< pair<string,int> , lli > node;
	node.first.first = "";
	node.first.second = -1;
	node.second = -1;
	return node;
}

void NodeInformation::stabilize(){

	/* get predecessor of successor */

	HelperFunctions help;

	string ownIp = sp.getIpAddress();
	int ownPort = sp.getPortNumber();

	if(help.isNodeAlive(successor.first.first,successor.first.second) == false)
		return;

	/* get predecessor of successor */
	pair< pair<string,int> , lli > predNode = help.getPredecessorNode(successor.first.first,successor.first.second,ownIp,ownPort,true);

	lli predecessorHash = predNode.second;

	if(predecessorHash == -1 || predecessor.second == -1)
		return;

	if(predecessorHash > id || (predecessorHash > id && predecessorHash < successor.second) || (predecessorHash < id && predecessorHash < successor.second)){
		successor = predNode;
	}

	
}

/* check if current node's predecessor is still alive */
void NodeInformation::checkPredecessor(){
	if(predecessor.second == -1)
		return;

	HelperFunctions help;
	string ip = predecessor.first.first;
	int port = predecessor.first.second;

	if(help.isNodeAlive(ip,port) == false){
		/* if node has same successor and predecessor then set node as it's successor itself */
		if(predecessor.second == successor.second){
			successor.first.first = sp.getIpAddress();
			successor.first.second = sp.getPortNumber();
			successor.second = id;
			setSuccessorList(successor.first.first,successor.first.second,id);
		}
		predecessor.first.first = "";
		predecessor.first.second = -1;
		predecessor.second = -1;
	}

}

/* check if current node's successor is still alive */
void NodeInformation::checkSuccessor(){
	if(successor.second == id)
		return;

	HelperFunctions help;
	string ip = successor.first.first;
	int port = successor.first.second;

	if(help.isNodeAlive(ip,port) == false){
		successor = successorList[2];
		updateSuccessorList();
	}

}

void NodeInformation::notify(pair< pair<string,int> , lli > node){

	/* get id of node and predecessor */
	lli predecessorHash = predecessor.second;
	lli nodeHash = node.second;

	predecessor = node;

	/* if node's successor is node itself then set it's successor to this node */
	if(successor.second == id){
		successor = node;
	}
}

void NodeInformation::fixFingers(){

	HelperFunctions help;

	//if(help.isNodeAlive(successor.first.first,successor.first.second) == false)
		//return;
	//cout<<"in fix fingers - "<<successor.second<<endl;
	
	int next = 1;
	lli mod = pow(2,M);

	while(next <= M){
		if(help.isNodeAlive(successor.first.first,successor.first.second) == false)
			return;
		
		lli newId = id + pow(2,next-1);
		newId = newId % mod;

		pair< pair<string,int> , lli > node = findSuccessor(newId, false);

		if(node.first.first == "" || node.second == -1 || node.first.second == -1 )
			break;
		fingerTable[next] = node;
		next++;	
	}

}

vector< pair< pair<string,int> , lli > > NodeInformation::getFingerTable(){
	return fingerTable;
}

lli NodeInformation::getId(){
	return id;
}

pair< pair<string,int> , lli > NodeInformation::getSuccessor(){
	return successor;
}

pair< pair<string,int> , lli > NodeInformation::getPredecessor(){
	return predecessor;
}

string NodeInformation::getValue(lli key){
	if(dictionary.find(key) != dictionary.end()){
		return dictionary[key];
	}
	else
		return "";
}

vector< pair< pair<string,int> , lli > > NodeInformation::getSuccessorList(){
	return successorList;
}

bool NodeInformation::getStatus(){
	return isInRing;
}