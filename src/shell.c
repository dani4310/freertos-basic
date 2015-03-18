#include "shell.h"
#include <stddef.h>
#include "clib.h"
#include <string.h>
#include "fio.h"
#include "filesystem.h"
#include "FreeRTOS.h"
#include "task.h"
#include "host.h"

typedef struct {
	const char *name;
	cmdfunc *fptr;
	const char *desc;
} cmdlist;

void vTaskCode( void * pvParameters );
void donothing( void * pvParameters );

void ls_command(int, char **);
void man_command(int, char **);
void cat_command(int, char **);
void ps_command(int, char **);
void host_command(int, char **);
void help_command(int, char **);
void host_command(int, char **);
void mmtest_command(int, char **);
void test_command(int, char **);
void Dtask_command(int,char **);
void new_command(int,char **);
void _command(int, char **);

double sqrt(double );
int CharToInt(char *);

#define MKCL(n, d) {.name=#n, .fptr=n ## _command, .desc=d}
	
	har taskflag=0;
	const int maxtask=10;
	int testmatrix[4];	
	char tttable[10]={0,0,0,0,0,0,0,0,0,0};
	xTaskHandle tthandle[10];

cmdlist cl[]={
	MKCL(ls, "List directory"),
	MKCL(man, "Show the manual of the command"),
	MKCL(cat, "Concatenate files and print on the stdout"),
	MKCL(ps, "Report a snapshot of the current processes"),
	MKCL(host, "Run command on host"),
	MKCL(mmtest, "heap memory allocation test"),
	MKCL(help, "help"),
	MKCL(test, "test new function"),
	MKCL(Dtask, "Delete the task which is not using"),
	MKCL(new, "test new task"),
	MKCL(, ""),
};


double sqrt(double x){
	double small = 0.001;
	double low, high;
	if (x < 0) 
	fio_printf(1,"This is negative!!");
	if (x < 1) { 
		low = 0; high = 1; 
	}
	else {
    	low = 1; high = x;  
	}
	while (high - low > small)  
	{
		double mid = (low + high) / 2;
		if (mid*mid > x)          
			high = mid;
		else
			low = mid;            
	}
	return low+small;
}


int parse_command(char *str, char *argv[]){
	int b_quote=0, b_dbquote=0;
	int i;
	int count=0, p=0;
	for(i=0; str[i]; ++i){
		if(str[i]=='\'')
			++b_quote;
		if(str[i]=='"')
			++b_dbquote;
		if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
			str[i]='\0';
			argv[count++]=&str[p];
			p=i+1;
		}
	}
	/* last one */
	argv[count++]=&str[p];

	return count;
}

void ls_command(int n, char *argv[]){
    fio_printf(1,"\r\n"); 
    int dir;
    if(n == 0){
        dir = fs_opendir("");
    }else if(n == 1){
        dir = fs_opendir(argv[1]);
        //if(dir == )
    }else{
        fio_printf(1, "Too many argument!\r\n");
        return;
    }
(void)dir;   // Use dir
}

int filedump(const char *filename){
	char buf[128];

	int fd=fs_open(filename, 0, O_RDONLY);

	if( fd == -2 || fd == -1)
		return fd;

	fio_printf(1, "\r\n");

	int count;
	while((count=fio_read(fd, buf, sizeof(buf)))>0){
		fio_write(1, buf, count);
    }
	
    fio_printf(1, "\r");

	fio_close(fd);
	return 1;
}

void ps_command(int n, char *argv[]){
	signed char buf[1024];
	vTaskList(buf);
        fio_printf(1, "\n\rName          State   Priority  Stack  Num\n\r");
        fio_printf(1, "*******************************************\n\r");
	fio_printf(1, "%s\r\n", buf + 2);	
}

void cat_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: cat <filename>\r\n");
		return;
	}

    int dump_status = filedump(argv[1]);
	if(dump_status == -1){
		fio_printf(2, "\r\n%s : no such file or directory.\r\n", argv[1]);
    }else if(dump_status == -2){
		fio_printf(2, "\r\nFile system not registered.\r\n", argv[1]);
    }
}

void man_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: man <command>\r\n");
		return;
	}

	char buf[128]="/romfs/manual/";
	strcat(buf, argv[1]);

    int dump_status = filedump(buf);
	if(dump_status < 0)
		fio_printf(2, "\r\nManual not available.\r\n");
}

void host_command(int n, char *argv[]){
    int i, len = 0, rnt;
    char command[128] = {0};

    if(n>1){
        for(i = 1; i < n; i++) {
            memcpy(&command[len], argv[i], strlen(argv[i]));
            len += (strlen(argv[i]) + 1);
            command[len - 1] = ' ';
        }
        command[len - 1] = '\0';
        rnt=host_action(SYS_SYSTEM, command);
        fio_printf(1, "\r\nfinish with exit code %d.\r\n", rnt);
    } 
    else {
        fio_printf(2, "\r\nUsage: host 'command'\r\n");
    }
}

void help_command(int n,char *argv[]){
	int i;
	fio_printf(1, "\r\n");
	for(i = 0;i < sizeof(cl)/sizeof(cl[0]) - 1; ++i){
		fio_printf(1, "%s - %s\r\n", cl[i].name, cl[i].desc);
	}
}

void Dtask_command(int n, char *argv[]) {
	int i;
	for(i=0;i<maxtask;i++){
		if(tttable[i]==2){
			vTaskDelete(tthandle[i]);
			tttable[i]=0;
		}
	}
	fio_printf(1,"\r\n");
}

void test_command(int n, char *argv[]) {
	int i;
	int number=CharToInt(argv[2]);
	int handleposition=-1;
	if(n==3){
		if(number == -1)
			return;
		while(testmatrix[0]==1)//avoid other task using testmatrix
		{}
    	testmatrix[0]=1;
    	testmatrix[1]=number;

		if(strcmp(argv[1],"fib")==0)
    		testmatrix[2]=0;
		if(strcmp(argv[1],"ispri")==0)
			testmatrix[2]=1;
		if(strcmp(argv[1],"priat")==0)
			testmatrix[2]=2;

		for(i=0;i<maxtask;i++){
			if(tttable[i]==0){
				tttable[i]=1;
				handleposition=i;
				break;
			}
		}
    	testmatrix[3]=handleposition;
	
		if(handleposition==-1){
			fio_printf(1,"\r\ntask position is all using");
			return;
		}

		xTaskCreate( vTaskCode,(signed portCHAR *)"tTask", 256, testmatrix,0|portPRIVILEGE_BIT,&tthandle[handleposition]); 
	}	 
   
    int handle;
    int error;

    fio_printf(1, "\r\n");
    
    handle = host_action(SYS_SYSTEM, "mkdir -p output");
    handle = host_action(SYS_SYSTEM, "touch output/syslog");

    handle = host_action(SYS_OPEN, "output/syslog", 8);
    if(handle == -1) {
        fio_printf(1, "Open file error!\n\r");
        return;
    }

    char *buffer = "Test host_write function which can write data to output/syslog\n";
    error = host_action(SYS_WRITE, handle, (void *)buffer, strlen(buffer));
    if(error != 0) {
        fio_printf(1, "Write file error! Remain %d bytes didn't write in the file.\n\r", error);
        host_action(SYS_CLOSE, handle);
        return;
    }

    host_action(SYS_CLOSE, handle);
    	
}

void vTaskCode( void * pvParameters )
{
	char hint[] = USER_NAME "@" USER_NAME "-STM32:~$ ";
	int number=testmatrix[1];
	int argv=testmatrix[2];
	int handleposition=testmatrix[3];
	testmatrix[0]=0;

	char flag;
	int sqrtnumber;
	int i,j;
		 
	if(argv==0){                   
		int previous =-1;
		int result =1;
		int sum=0;
		for(i=0;i<=number;i++){
			sum=result+previous;
			previous=result;
			result=sum;
		}
		while(taskflag!=0){}
		taskflag=1;
		fio_printf(1,"\r\nThe fibonacci sequence at %d is: %d",number,result);
		taskflag=0;
	}
	if(argv==1){
		flag=0;
		sqrtnumber=(int)(sqrt((double)number));
		for(i=2;i<=sqrtnumber;i++){
			if(!(number%i)){
				flag=1;
				break;
			}
		}
		if(flag){
			while(taskflag!=0){}
			taskflag=1;
			fio_printf(1,"\r\n%d is composite number!",number);
			taskflag=0;
		}
		else{
			while(taskflag!=0){}
			taskflag=1;
			fio_printf(1,"\r\n%d is prime number!",number);
			taskflag=0;
		}
	}
	if(argv==2){
		int count=0;
		flag=0;
		j=1;
		while(count<number){
			j++;
			sqrtnumber=(int)(sqrt((double)j));
			for(i=2;i<=sqrtnumber;i++){
				if(!(j%i)){
					flag=1;
					break;
				}
			}
			if(!flag){
			count++;
			}
			flag=0;
		}
		while(taskflag!=0){}
		taskflag=1;
		fio_printf(1,"\r\nThe prime number at %d is:%d",number,j);
		taskflag=0;
	}
	while(taskflag!=0){}
	taskflag=1;
	fio_printf(1,"\r\n%s",hint);
	taskflag=0;

	tttable[handleposition]=2;
	for( ;; )
  	{}
}
void new_command(int n, char *argv[]){
	int i,j,handleposition;
	int ntonew=CharToInt(argv[1]);
	if(ntonew==-1)
		return;

	for(i=0;i<ntonew;i++){
		for(j=0;j<maxtask;j++){
    		if(tttable[j]==0){
    		tttable[j]=2;
    		handleposition=j;
    		break;
    		}
    	}
		if(handleposition==-1){
			fio_printf(1,"\r\ntask position is all using");
			return;
    	}

		if(xTaskCreate( donothing,(signed portCHAR *)"NewT", 512, NULL,0|portPRIVILEGE_BIT,&tthandle[handleposition])==pdPASS)
			fio_printf(1,"\r\nThe task creates successfully.");
		else{
			fio_printf(1,"\r\nCan't create task.");
			tttable[j]=0;
		}
	}
	fio_printf(1,"\r\n");
}

void donothing(void *pvParameters){
	for(;;)
	{}
}

int CharToInt(char *ch){
	int i;
    int length=strlen(ch);
    int number=0;
	
	for(i=0;i<length;i++){
        if(ch[i]>'9'||ch[i]<'0'){
        	fio_printf(1,"\r\ninput error\r\n");
            return -1;
        }
        number=number*10+(int)(ch[i]-'0');
    } 
    return number;
}

void _command(int n, char *argv[]){
    (void)n; (void)argv;
    fio_printf(1, "\r\n");
}

cmdfunc *do_command(const char *cmd){

	int i;

	for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
		if(strcmp(cl[i].name, cmd)==0)
			return cl[i].fptr;
	}
	return NULL;	
}
