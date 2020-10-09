#include <sys/file.h>

#include "mainctrl.h"


static CMainCtrl *g_pMainCtrl;

void ShowUsage(const char* aName )
{
    printf("%s -f [config]", aName);
		
}

void SignalHandle(int aSigNo)
{
    g_pMainCtrl->SetRunFlag(Flag_ReloadCfg);
	signal(SIGUSR1, SignalHandle);
}


void Signa2Handle(int aSigNo)
{
    g_pMainCtrl->SetRunFlag(Flag_Exit);

	signal(SIGUSR2, Signa2Handle);


}
int InitDameon()
{
    if ( fork() != 0 )
	{    	
		exit(0);
	}

	setsid();

	if ( fork() ! = 0 )
	{
	    exit(0);
	}

    umask(0);
	setpgrp();

	return 0;
}

int main(int argc, char* argv[])
{

    if(1 < argc && !strcasecmp(argv[1], "-v"))
	{
	    printf("%s build in  %s %s", argv[0], __DATE__, __TIME__);
		exit(0);		
	}

	if (argc < 2)
	{
	   ShowUsage(argv[0]);
	   exit(-1);
	}



    //½ø³Ì»¥³âËø
    int  lock_fd  = open(argv[1], O_RDWR, 0640);
	if (lock_fd < 0 )
	{
	    printf("open %s failed!\n", argv[1]);
		exit(-2);
	}

	int ret = flock(lock_fd, LOCK_EX | LOCK_NB);
	if (ret < 0)
	{
	    printf("%s already runing!\n", argv[0]);
		exit(-3);
	}

	if (argc >=3 && !strcasecmp(argv[2], "-d"))
	{
	    ;
	}
	else
	{
		InitDameon();
	}



	signal(SIGUSR1, SignalHandle);
	signal(SIGUSR2, Signa2Handle);


	
	g_pMainCtrl = new CMainCtrl();

	if (NULL == g_pMainCtrl)
	{
		printf("Alloc CMainCtrl failed!\n");
		exit(-3);
	}

	char szProcName[128];
	GetNameFromPath(argv[0], szProcName);


	ret = g_pMainCtrl->Initialize(szProcName, argv[1]);
	if(ret < 0)
	{
	    exit(0);
	}


	g_pMainCtrl->Run();
	return 0;
	
	


}
