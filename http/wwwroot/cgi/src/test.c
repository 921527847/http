#include<stdio.h>
#include<mysql.h>


int main()
{
	printf("mysql client Version info: %s\n",mysql_get_client_info());
	return 0;
}

