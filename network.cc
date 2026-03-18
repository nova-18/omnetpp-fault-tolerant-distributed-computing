#include <string.h>
#include <omnetpp.h>
#include <bits/stdc++.h>
#include "message_m.h"
#include "omnetpp/distrib.h"
#include "config.h"
#include <fstream>
using namespace omnetpp;
// const int Total_Task_Count = 2;

typedef struct minitask{
    std::unordered_map<int,int> mapping;
}minitask;


class Taskmanager{

    private :
    int subtaskcount;
    std::set<int>serverLis;
    std::vector<minitask>subtasklis;
    
    public :
    Taskmanager(int count, std::set<int>serv) : subtaskcount(count), serverLis(serv), subtasklis(count){}


    int findMostFrequent(minitask& task) {
        std::unordered_map<int, int> frequency;
    
        // Calculate frequencies
        for (auto& pair : task.mapping) {
            frequency[pair.second]++;
        }
    
        // Find the most frequent second number
        int maxFrequency = 0;
        int mostFrequentSecondNumber = -1;
    
        for (auto& pair : frequency) {
            if (pair.second > maxFrequency) {
                maxFrequency = pair.second;
                mostFrequentSecondNumber = pair.first;
            }
        }
        return mostFrequentSecondNumber;
    }

    void update(int subtaskNum, int serv, int ans){
        if(serverLis.count(serv) == 1) subtasklis[subtaskNum].mapping[serv] = ans;
    }

    std::vector<int> processubtasks(){
        std::vector<int>out(subtaskcount);
        for(int i = 0;i < subtaskcount;i++){
            int ans = findMostFrequent(subtasklis[i]);
            out[i] = ans;
        }
        return out;
    }

    std::unordered_map<int,int>scoreUpd(){
        std::vector<int>vec = processubtasks();
        std::unordered_map<int,int>out;
        for( int serv : serverLis){
            out[serv] = 0;
        }
        for (int  i  = 0 ;i<subtaskcount;i++){
            int ans = vec[i];
            for(int ser : serverLis){
                if(subtasklis[i].mapping.count(ser) == 1){
                    if(subtasklis[i].mapping[ser] == ans){
                        out[ser]++;
                    }
                }
            }
        }
        return out;
    }

};


class Client: public cSimpleModule
{

protected:
    int taskSent = 0;
    int gossipsrecv = 0;
    std::vector<int>taskFlag;
    std::vector<Taskmanager>managers;
    std::unordered_map<std::string,bool>messageList;
    std::vector<int>scores;
    std::hash<std::string> hasher;
    std::ofstream logFile;
    std::ofstream gossiplogFile;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void createAndSendTask(int taskno, std::set<int>&pickedServers);
    virtual void processResponse(subtaskResp *Msg );
    virtual void processTask(int task);
    virtual void sendGossip(int tasknum);
    virtual void processGossip(gossip *g);

private:
    std::vector<int> generateRandomArray(int arraySize, int lowerBound, int upperBound) {
        std::vector<int> randomArray(arraySize);
        for (int i = 0; i < arraySize; i++) {
            randomArray[i] = intuniform(lowerBound, upperBound);
        }
        return randomArray;
    }

    std::vector<std::vector<int>> divideArray(const std::vector<int>& arr, int n) {
        int arraySize = arr.size();
        int baseSize = arraySize / n;
        int remainder = arraySize % n;
    
        std::vector<std::vector<int>> subarrays(n);
        int index = 0;
    
        for (int i = 0; i < n; i++) {
            int currentSize = baseSize + (i < remainder ? 1 : 0);
            subarrays[i].resize(currentSize);
    
            for (int j = 0; j < currentSize; j++) {
                subarrays[i][j] = arr[index++];
            }
        }
        return subarrays;
    }

    std::set<int> maliciousIndices(int siz, int range){
 
        int n = intuniform(0, siz-1);
        // int n = siz; 
        std::set<int> selectedNumbers;
        std::vector<int> numbers(range);
        std::iota(numbers.begin(), numbers.end(), 0);
        // std::random_device rd;
        std::random_device rd;
        std::shuffle(numbers.begin(), numbers.end(), rd);

        for (int i = 0;i<n;i++){
            selectedNumbers.insert(numbers[i]);
        }
        return selectedNumbers;
    }

    std::set<int> pickIndices(int siz, int range){
 
        int n = siz;
        // int n = siz; 
        std::set<int> selectedNumbers;
        std::vector<int> numbers(range);
        std::iota(numbers.begin(), numbers.end(), 0);
        // std::random_device rd;
        std::random_device rd;
        std::shuffle(numbers.begin(), numbers.end(), rd);

        for (int i = 0;i<n;i++){
            selectedNumbers.insert(numbers[i]);
        }
        
        return selectedNumbers;
    }

    std::set<int> getTopServers(int num){
        
        std::set<int>out;
        std::vector<std::pair<int, int>> indexedArr;
        // Store values with their indices
        for (size_t i = 0; i < scores.size(); i++) {
            indexedArr.emplace_back(i, scores[i]);
        }

        // Sort by values in descending order
        std::sort(indexedArr.begin(), indexedArr.end(), [](const auto &a, const auto &b) {
            return a.second > b.second;
        });

        // Output top n indices
        // std::cout << "Top " << n << " indices with max values:\n";
        for (int i = 0; i < num && i < indexedArr.size(); i++) {
            EV << "Index: " << indexedArr[i].first << ", Value: " << indexedArr[i].second << endl;
            out.insert(indexedArr[i].first);
        }
        return out;
    }

    void updateScores(std::vector<int>updts, int tasknum, int maxGossip, int siz){
        gossipsrecv++;
        EV << getName()<< " : gossip processed : " << "gossip recv : "<< gossipsrecv;
        if (logFile.is_open()) {
            logFile<< getName()<< " : gossip processed : " << "gossip recv : "<< gossipsrecv;
        }
        for(int i = 0;i<updts.size();i++){
            scores[i] += updts[i];
            EV<< " : " <<scores[i];
            if (logFile.is_open()) {
                logFile<< " : " <<scores[i];
            }
        }EV<<endl;
        if (logFile.is_open()) {
            logFile<<"\n";
        }
        if(gossipsrecv == maxGossip ){
            gossipsrecv = 0;
            if(taskSent < Total_Task_Count){
                std::set<int>serverSeed = getTopServers(siz/2 + 1);
                createAndSendTask(tasknum+1,serverSeed);
            }
        }
    }

};


Define_Module(Client);

void Client::processTask(int task){
    EV <<getName()<< " : Processing task : " << task<<endl;
    if (logFile.is_open()) {
        logFile<<getName()<<" : "<< getIndex()<<" : Processing task : " << task<< "\n";
    }

    int subtasks = gateSize("serverOut");
    std::vector<int>ansrs = managers[task-1].processubtasks();
    int taskResult = -1;
    for(int num : ansrs){
        taskResult = std::max(taskResult,num);
    }
    
    if (logFile.is_open()) {
        logFile<< getName()<<" : "<< getIndex()<<" : Task number : "<< task << " Result : " << taskResult<<"\n";
    }
    
    EV << getName()<<" : Task number : "<< task << " Result : " << taskResult<<endl;
    std::unordered_map<int,int>scoreupdates = managers[task-1].scoreUpd();
    for(auto &pair : scoreupdates){
        int serv = pair.first;
        int upd = pair.second;
        scores[serv] += upd;
        EV <<getName()<<" : "<< getIndex()<< " : Scoreupdated : server : " << serv << " upd : +"<<upd<<" final :" << scores[serv]<<endl;
        if (logFile.is_open()) {
            logFile<<getName()<<" : "<< getIndex()<< " : Scoreupdated : server : " << serv << " upd : +"<<upd<<" final :" << scores[serv]<<"\n";
        }
    }
}

void Client::createAndSendTask(int taskno, std::set<int>&pickedServers)
{
    int client_count = gateSize("clientOut");
    int server_count = gateSize("serverOut");
    int randSize = intuniform(30,50);
    Taskmanager tm(server_count, pickedServers);
    managers.push_back(tm);

    std::vector<int>task = generateRandomArray(randSize, 0, 1000);
    std::vector<std::vector<int>>task_div = divideArray(task,server_count);

    EV<< getName()<<" : "<< getIndex()<<" Task Created : "<<taskno<< " Task length : "<<randSize<<endl;
    std::string taskdesc = "Task desc :";
            for(int i = 0;i<randSize;i++){
                taskdesc += std::to_string(task[i]) + ",";
            }taskdesc.pop_back();
    EV << taskdesc <<endl;
    if (logFile.is_open()) {
        logFile<<getName()<<" : "<< getIndex()<<" Task Created : "<<taskno<< " Task length : "<<randSize<< "\n";
        logFile<<taskdesc<< "\n";
    }

    for(int subtask_count = 0;subtask_count < server_count;subtask_count++){
        std::vector<std::pair<int,int>>t;
        std::set malicious = maliciousIndices(server_count/4, server_count);
        int thisClient = getIndex();
        int subtasklen = task_div[subtask_count].size();


        for(int thisServer =0 ; thisServer < server_count; thisServer++){
            if(pickedServers.count(thisServer) == 0) continue;
            subtask * sub = new subtask("subtask");
            sub->setClient_id(thisClient);
            sub->setServer_id(thisServer);
            sub->setArr_len(subtasklen);
            sub->setQueryArraySize(subtasklen);
            sub->setTaskNum(taskno);
            sub->setSubTaskNum(subtask_count);
            for (int i = 0;i<subtasklen;i++){
                sub->setQuery(i,task_div[subtask_count][i]);
            }
            if(malicious.count(thisServer) == 1){
                sub->setIsMalicious(true);
            } 
            else{
                sub->setIsMalicious(false);
            }
            sub->setTimestamp(simTime());
            send(sub, "serverOut",thisServer);
            EV<<getName()<<" : "<< getIndex()<< " : "<<"Subtask sent : "<<"Destination : "<< thisServer << " Task number : "<<taskno <<" Sub task number : "<<subtask_count<<endl;
            std::string subtaskdesc = "Sub Task desc :";
            for(int i = 0;i<subtasklen;i++){
                subtaskdesc += std::to_string(task_div[subtask_count][i]) + ",";
            }subtaskdesc.pop_back();
            EV<< subtaskdesc<<endl;
            if (logFile.is_open()) {
                logFile<<getName()<<" : "<< getIndex()<<" : Subtask sent : "<<"Destination : "<< thisServer << " Task number : "<<taskno <<" Sub task number : "<<subtask_count << "\n";
                logFile<<subtaskdesc<< "\n";
            }
        }
    }
    taskSent++;

}

void Client::sendGossip(int tasknum){
    int client_count = gateSize("clientOut");
    int server_count = gateSize("serverOut");
    int txNode = getIndex();
    for(int rxNode = 0; rxNode < client_count; rxNode++){
        if(txNode == rxNode) continue;
        gossip * g = new gossip("gossip");
        g->setTxId(txNode);
        g->setRxId(rxNode);
        g->setTaskNum(tasknum);
        g->setServerScoreArraySize(server_count);
        for(int i = 0; i<server_count;i++){
            g->setServerScore(i,scores[i]);
        }
        g->setTimestamp(simTime());
        send(g,"clientOut", rxNode);
        EV <<getName()<<" : "<< getIndex()<< " : Gossip Sent : "<< "from : " << txNode << " to : "<< rxNode <<endl;
        if (logFile.is_open()) {
            logFile<<getName()<<" : "<< getIndex()<< " : Gossip Sent : "<< "from : " << txNode << " to : "<< rxNode<<"\n";
        }
    }
}

void::Client::processGossip(gossip *g){
    int clientsiz = gateSize("clientOut");
    int siz = gateSize("serverOut");
    int from = g->getTxId();
    int to = g->getRxId();
    int tasknum = g->getTaskNum();
    std::string timestamp = g->getTimestamp().str();
    std::vector<int>scoreupdts;
    EV<< getName()<<" : "<< getIndex()<< " : Gossip recieved : "<< timestamp<<":"<<from<<":[";
    if (logFile.is_open()) {
        logFile<<getName()<<" : "<< getIndex()<< " : Gossip recieved : "<< timestamp<<":";
    }
    std::string srec = std::to_string(from) + ":";
    for(int i = 0;i<siz;i++){
        int sco = g->getServerScore(i);
        EV<<sco<<",";
        scoreupdts.push_back(sco);
        srec += std::to_string(sco);
        srec = ":";
    }
    EV<<"]"<<endl;
    
    srec.pop_back();
    if (logFile.is_open()) {
        logFile<<srec<<"\n";
    }
    size_t hashValue = hasher(srec);
    std::string hashedst = std::to_string(hashValue);
    if(messageList.count(hashedst) != 1){
        messageList[hashedst] = true;
        updateScores(scoreupdts, tasknum, clientsiz - 1, siz);
    }
}

void Client::processResponse(subtaskResp *Msg )
{
    int siz = gateSize("serverOut");
    int target = siz * (siz/2 + 1);
    int incomingFrom = Msg->getServerId();
    int ans = Msg->getAnswer();
    int tasknumber = Msg->getTaskNum();
    int subtasknum = Msg->getSubTaskNum();

    managers[tasknumber-1].update(subtasknum,incomingFrom,ans);
    // std::pair<int,int>newEntry = {incomingFrom, ans};
    // responseArr[tasknumber-1][subtasknum].push_back(newEntry);
    taskFlag[tasknumber-1]++;
    EV<< getName()<<" : "<< getIndex()<<" : response by server: " << incomingFrom<< " Task : "<<tasknumber<<" Subtask : "<< subtasknum << " ans : "<< ans<<endl;
    if (logFile.is_open()) {
        logFile<<getName()<<" : response by server: " << incomingFrom<< " Task : "<<tasknumber<<" Subtask : "<< subtasknum << " ans : "<< ans<< "\n";
    }
    if(taskFlag[tasknumber-1] == target){
        processTask(tasknumber);
        sendGossip(tasknumber);
    }

}

void Client::initialize()
{
    logFile.open("logs/client.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << "Client log file initialized: " << getName()<<" : "<< getIndex() << "\n";
    }
    gossiplogFile.open("logs/gossip.log", std::ios::app);
    if (gossiplogFile.is_open()) {
        gossiplogFile << "Gossip log file initialized: " << getName()<<" : "<< getIndex() << "\n";
    }
    for(int i = 0;i<Total_Task_Count;i++){
        taskFlag.push_back((int)0);
    }
    int client_count = gateSize("clientOut");
    int server_count = gateSize("serverOut");
    EV << getName() << " : initialized"<<endl;
    if (logFile.is_open()) {
        logFile << "Message received at " << simTime() << " in " << getName()<<" : "<< getIndex() << "\n";
    }
    for (int i = 0;i<server_count;i++){
        scores.push_back((int)0);
    }
    std::set<int>picked_servers = pickIndices(server_count/2 +1 , server_count);
    createAndSendTask(1,picked_servers);
    
}

void Client::handleMessage(cMessage *msg)

{
    
    if (dynamic_cast<subtaskResp *>(msg)) {
        subtaskResp *Msg = check_and_cast<subtaskResp *>(msg);
        processResponse(Msg);
        
    } 
    else if(dynamic_cast<gossip *>(msg)){
        gossip *Msg = check_and_cast<gossip *>(msg);
        processGossip(Msg);
    }
    else {
        EV <<getName()<< " : "<< getIndex()<<" " << msg->getName() << endl;
        if (logFile.is_open()) {
            logFile<<getName()<< " : "<< getIndex()<<" " << msg->getName()<< "\n";
        }
    }

    // EV << getName() << " received message: " << msg->getName() << "\n";
    delete msg; // Clean up the received message
}


class Server: public cSimpleModule
{
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    std::ofstream logFile2;
private:
    int getMax(std::vector<int>&task_arr, bool ismalicius){
        int maxi = task_arr[0], mini = task_arr[0];
        for (int  i = 0; i< task_arr.size();i++){
            int num = task_arr[i];
            maxi = std::max(maxi,num);
            mini = std::min(mini,num);
        }
        if(ismalicius){
            return mini;
        }
        else{
            return maxi;
        }
    }
};

Define_Module(Server);

void Server::initialize()
{
    logFile2.open("logs/server.log", std::ios::app);
    EV << getName()<<" : " << getIndex()<<" initialized, sending Hello message to all Clients.\n";
    if (logFile2.is_open()) {
        logFile2<<getName()<<" : " << getIndex()<<" initialized, sending Hello message to all Clients.\n";
    }

    for (int i = 0; i < gateSize("out"); i++) {
        cMessage *msg = new cMessage("Hello from Server");
        send(msg, "out", i);
    }
}

void Server::handleMessage(cMessage *msg)
{
    if (dynamic_cast<subtask *>(msg)) {
        subtask *myMsg = check_and_cast<subtask *>(msg);
        int task_size = myMsg->getArr_len();
        bool ismal = myMsg->getIsMalicious();
        std::vector<int>taskData;
        EV<<getName()<<" : "<<getIndex()<< " : Received subtask with Client ID: " << myMsg->getClient_id()<<" Task length = "<<  task_size<< endl;
        if (logFile2.is_open()) {
            logFile2<<getName()<<" : "<<getIndex()<< "Received subtask with Client ID: " << myMsg->getClient_id()<<" Task length = "<<  task_size<<"\n";
        }
        for (int i = 0; i < task_size; i++) {
            EV << myMsg->getQuery(i) << " ";
            if (logFile2.is_open()) {
                logFile2<< myMsg->getQuery(i) << " ";
            }
            taskData.push_back(myMsg->getQuery(i));
        }
        EV << endl;
        if (logFile2.is_open()) {
            logFile2<<"\n";
        }

        int ans = getMax(taskData, ismal);
        EV <<getName()<<" : "<<getIndex()<<" : malicious flag :" << ismal << " ans :" << ans<<endl;
        if (logFile2.is_open()) {
            logFile2<<getName()<<" : "<<getIndex()<<" : malicious flag :" << ismal << " ans :" << ans<<"\n";
        }

        // int target = myMsg->getArrivalModule()->getIndex();
        
        cGate *arrivalGate = myMsg->getArrivalGate();
        int gateIndex = arrivalGate->getIndex();
        
        // int inSize = gateSize("in");
        // int outSIze = gateSize("out");

        // EV << "insize : " << inSize << " outsize : "<< outSIze<<" gateIndex : "<<gateIndex << endl;
        subtaskResp * resp = new subtaskResp("subtaskResp");
        resp->setServerId(getIndex());
        resp->setClientId(gateIndex);
        resp->setAnswer(ans);
        resp->setTimestamp(simTime());
        resp->setTaskNum(myMsg->getTaskNum());
        resp->setSubTaskNum(myMsg->getSubTaskNum());
        send(resp, "out", gateIndex);

    } else {
        EV <<getName()<<" : "<<getIndex()<< "Received unknown message type: " << msg->getName() << endl;
    }
    delete msg;
}
