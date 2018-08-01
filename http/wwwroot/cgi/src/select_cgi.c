#include<mysql.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main()
{
	char mysql[512];	
	MYSQL *mfp = mysql_init(NULL);
	mfp = mysql_real_connect(mfp,"127.0.0.1","root","123456","guo",3306,NULL,0);
	
	
	if(mfp == NULL)
		exit(0);
	sprintf(mysql,"select * from student");

	mysql_query(mfp,mysql);
	if(mfp == NULL){
		printf("query error!\n");
		exit(0);
	}
	MYSQL_RES *res = mysql_store_result(mfp);
	int fields = mysql_num_fields(res);
	MYSQL_FIELD *field = mysql_fetch_fields(res);
	int i = 0;
	for(; i < fields; i++){
		printf("%s ",field[i].name);
	}
	printf("\n");
	i = 0;
	MYSQL_ROW line;
	int nums = mysql_num_rows(res);
	for(; i < nums; i++){
		line = mysql_fetch_row(res);
		int j = 0;
		for(; j < fields; j++){
			printf("%s ",line[j]);
		}
		printf("\n");
	}
	mysql_close(mfp);
	return 0;
}
