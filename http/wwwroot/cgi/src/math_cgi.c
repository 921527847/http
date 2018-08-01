#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main()
{
	int a = 0;
	int b = 0;
	char* method=NULL;
	char* query_string=NULL;
	char* content_len=NULL;
	char buf[1024];
	method=getenv("METHOD");
	printf("method:%s",method);
	if(method&&strcasecmp(method,"GET")==0)
	{
		query_string=getenv("QUERY_STRING");
		printf("hello GET\n");
		if(!query_string)
		{
			printf("get method GET arg error!\n");
			return 1;
		}
		strcpy(buf,query_string);
	}
	else if(method&&strcasecmp(method,"POST")==0)
	{
		int i=0;
		printf("hello POST\n");
		int nums=atoi((char*)getenv("CONTENT_LENGTH"));
		for(;i<nums;i++)
		{
			read(0,buf+i,1);
		}
		buf[i]='\0';
	}
	else
	{
		exit(0);
	}
	printf("buf:%s\n",buf);
//	strtok(buf,"=&");
//	a = atoi(strtok(NULL,"=&"));
//	strtok(NULL,"=&");
//	b = atoi(strtok(NULL,"=&"));
//	printf("%d + %d = %d",a,b,a+b);
//	printf("%d - %d = %d",a,b,a-b);
//	printf("%d * %d = %d",a,b,a*b);
//	printf("%d / %d = %d",a,b,a/b);

	return 0;
}
