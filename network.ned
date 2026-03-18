
simple Server
{
    gates :
        input in[];
        output out[];
}

simple Client
{
    gates :
        input serverIn[];
        input clientIn[];
        output serverOut[];
        output clientOut[];
}

network myNetwork
{
    parameters :
        
        int numServers;
        int numClients;
    
   	submodules :
        
        servers[numServers] : Server;
        clients[numClients] : Client;
        
    connections :
		clients[0].clientOut++ --> { delay = 0ms; } --> clients[0].clientIn++ ;
		clients[0].clientOut++ --> { delay = 100ms; } --> clients[1].clientIn++ ;
		clients[1].clientOut++ --> { delay = 200ms; } --> clients[0].clientIn++ ;
		clients[1].clientOut++ --> { delay = 0ms; } --> clients[1].clientIn++ ;
		clients[0].serverOut++ --> { delay = 100ms; } --> servers[0].in++ ;
		clients[0].serverOut++ --> { delay = 100ms; } --> servers[1].in++ ;
		clients[0].serverOut++ --> { delay = 200ms; } --> servers[2].in++ ;
		clients[0].serverOut++ --> { delay = 100ms; } --> servers[3].in++ ;
		clients[0].serverOut++ --> { delay = 100ms; } --> servers[4].in++ ;
		clients[0].serverOut++ --> { delay = 200ms; } --> servers[5].in++ ;
		clients[0].serverOut++ --> { delay = 100ms; } --> servers[6].in++ ;
		clients[0].serverOut++ --> { delay = 100ms; } --> servers[7].in++ ;
		clients[0].serverOut++ --> { delay = 200ms; } --> servers[8].in++ ;
		clients[1].serverOut++ --> { delay = 100ms; } --> servers[0].in++ ;
		clients[1].serverOut++ --> { delay = 100ms; } --> servers[1].in++ ;
		clients[1].serverOut++ --> { delay = 300ms; } --> servers[2].in++ ;
		clients[1].serverOut++ --> { delay = 100ms; } --> servers[3].in++ ;
		clients[1].serverOut++ --> { delay = 100ms; } --> servers[4].in++ ;
		clients[1].serverOut++ --> { delay = 200ms; } --> servers[5].in++ ;
		clients[1].serverOut++ --> { delay = 100ms; } --> servers[6].in++ ;
		clients[1].serverOut++ --> { delay = 100ms; } --> servers[7].in++ ;
		clients[1].serverOut++ --> { delay = 200ms; } --> servers[8].in++ ;
		clients[0].serverIn++ <-- { delay = 100ms; } <-- servers[0].out++ ;
		clients[1].serverIn++ <-- { delay = 100ms; } <-- servers[0].out++ ;
		clients[0].serverIn++ <-- { delay = 100ms; } <-- servers[1].out++ ;
		clients[1].serverIn++ <-- { delay = 100ms; } <-- servers[1].out++ ;
		clients[0].serverIn++ <-- { delay = 200ms; } <-- servers[2].out++ ;
		clients[1].serverIn++ <-- { delay = 300ms; } <-- servers[2].out++ ;
		clients[0].serverIn++ <-- { delay = 100ms; } <-- servers[3].out++ ;
		clients[1].serverIn++ <-- { delay = 100ms; } <-- servers[3].out++ ;
		clients[0].serverIn++ <-- { delay = 100ms; } <-- servers[4].out++ ;
		clients[1].serverIn++ <-- { delay = 100ms; } <-- servers[4].out++ ;
		clients[0].serverIn++ <-- { delay = 200ms; } <-- servers[5].out++ ;
		clients[1].serverIn++ <-- { delay = 200ms; } <-- servers[5].out++ ;
		clients[0].serverIn++ <-- { delay = 100ms; } <-- servers[6].out++ ;
		clients[1].serverIn++ <-- { delay = 100ms; } <-- servers[6].out++ ;
		clients[0].serverIn++ <-- { delay = 100ms; } <-- servers[7].out++ ;
		clients[1].serverIn++ <-- { delay = 100ms; } <-- servers[7].out++ ;
		clients[0].serverIn++ <-- { delay = 200ms; } <-- servers[8].out++ ;
		clients[1].serverIn++ <-- { delay = 200ms; } <-- servers[8].out++ ;
}


    