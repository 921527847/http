#include<mysql.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
void insert_cgi()
{
	char name[20];
	char password[11];
	char buf[1024];
	char mysql[512];
	char* method=NULL;
	char* query_string=NULL;
	char* content_len=NULL;
	method=getenv("METHOD");
	printf("method:%s",method);
	if(method&&strcasecmp(method,"GET")==0)
	{
		query_string=getenv("QUERY_STRING");
		if(!query_string)
		{
			printf("get method GET arg error!\n");
			exit(1);
		}
		strcpy(buf,query_string);
	}
	else if(method&&strcasecmp(method,"POST")==0)
	{
		int i=0;
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
	MYSQL *mfp = mysql_init(NULL);
	mfp = mysql_real_connect(mfp,"127.0.0.1","root","123456","guo",3306,NULL,0);
	
	
	if(mfp == NULL)
		exit(0);
	printf("buf:%s\n",buf);
	strtok(buf,"=&");
	strcpy(name,strtok(NULL,"=&"));
	strtok(NULL,"=&");
	strcpy(password,strtok(NULL,"=&"));
	printf("name: %s password: %s",name,password);
	sprintf(mysql,"delete into student(name,password)values(%s,%s)",name,password);

	mysql_query(mfp,mysql);
}
int main()
{
	insert_cgi();
	return 0;
}
