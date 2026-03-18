message subtask {
	int client_id;
	int server_id;
	int arr_len;
	int query[];
	int TaskNum;
	int subTaskNum;
	bool IsMalicious;
	simtime_t timestamp;
}

message subtaskResp{
	int serverId;
	int clientId;
	int answer;
	int TaskNum;
	int SubTaskNum;
	simtime_t timestamp;
}

message gossip{
	int txId;
	int rxId;
	int TaskNum;
	int serverScore[];
	simtime_t timestamp;
}