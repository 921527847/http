#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<pthread.h>
#include<fcntl.h>
#include<sys/sendfile.h>
#include<sys/stat.h>
void show_404(int fd)
{
	drop_header(fd);
	char path[1024] = "./wwwroot/404.html";
	int s = open(path,O_RDONLY);
	struct stat st;
	stat(path,&st);
	const char* echo_header="HTTP/1.0 404 Not Found";
	send(fd,echo_header,strlen(echo_header),0);
	const char* type="Content-Type:text/html;charset=ISO-8859-1\r\n";
	send(fd,type,strlen(type),0);
	const char* blank_line="\r\n";
	send(fd,blank_line,strlen(blank_line),0);
	sendfile(fd,s,NULL,st.st_size);
}
void echo_error(int fd,int error_num)
{
	switch(error_num)
	{
		case 404:
			show_404(fd);
			break;
		case 501:
			show_404(fd);
			break;
		default: 
			break;
	}
} 
int exe_cgi(int fd,char* method,char* query_string,char* path)
{
	printf("method:%s,query_string:%s,path:%s\n",method,query_string,path);
	char Method[1024/10];
	char Query_String[1024];
	char Content_Length[1024];
	int content_len = -1;
	if(strcasecmp(method,"GET") == 0){
		drop_header(fd);
	}else{
		char buff[1024];
		int ret = -1;
		do{
			ret = get_line(fd,buff,sizeof(buff));
			printf("%s\n",buff);
			if(strncasecmp(buff,"Content-Length: ",16) == 0){   //判断响应报头中是否有Content-Length(记录报头长度)这一行
				content_len = atoi(&buff[16]);
			}
		}while(ret > 0&&strcmp(buff,"\n"));
	}
	if(content_len == -1){
		echo_error(fd,401);
		return -1;
	}	
	const char* echo_line = "HTTP/1.0 200 ok\r\n";                            //响应
	write(fd,echo_line,strlen(echo_line));
	const char* type = "Content-Type:text/html;charset=ISO-8859-1\r\n";
	write(fd,type,strlen(type));
	const char* blank_line = "\r\n";
	write(fd,blank_line,strlen(blank_line));
	int input[2];
	int output[2];
	if(pipe(input) < 0){
		echo_error(fd,401);
		return -2;
	}
	if(pipe(output) < 0){
		echo_error(fd,401);
		return -3;
	}	
	pipe(input);
	pipe(output);
	
	pid_t pid = fork();
	if(pid < 0){
		echo_error(fd,401);
		return -4;
	}else if(pid == 0){
		//child                          //子进程对输入管道进行读操作，对输出管道进行写操作
		printf("child is running....\n");
		close(input[1]);
		close(output[0]);
		
		dup2(input[0],0);
		dup2(output[1],1);
		sprintf(Method,"METHOD=%s",method);	   //设置环境变量 putenv()
		putenv(Method);
		if(strcasecmp(method,"GET") == 0){
			sprintf(Query_String,"QUERY_STRING=%s",query_string);
			putenv(Query_String);
		}else{
			sprintf(Content_Length,"CONTENT_LENGTH=%d",content_len);
			putenv(Content_Length);
		}
	
		execl(path,path,NULL);   //1 path:代表路径  2 path:可执行程序
		exit(1);
	}else{
		//father
		printf("father is running....\n");
		close(input[0]);              //父进程对输入管道进行写操作，对输出管道进行读操作
		close(output[1]);
		char c = '\0';
		if(strcasecmp(method,"POST") == 0){
			int i = 0;
			for(i = 0;i<content_len;i++){
				recv(fd,&c,1,0);                     //从套接字中读，往输入管道中写
				write(input[1],&c,1);
			}
		}
		while(1){
			ssize_t size = read(output[0],&c,1);      //从输出管道中读，往套接字中写
			if(size > 0)
				send(fd,&c,1,0);
			else
				break;
		}
		waitpid(pid,NULL,0);
		close(input[1]);
		close(output[0]);
	}
}
int drop_header(int fd)               //清空
{
	char buff[1024];
	int ret=1;
	do
	{
		ret=get_line(fd,buff,sizeof(buff));
	}while(ret>0&&strcmp(buff,"\n"));
}
void respond(int fd,char* path,int size)  ///响应
{
	drop_header(fd);
	int s = open(path,O_RDONLY);
	char* echo_line = "HTTP/1.1 200 OK\r\n";
	write(fd,echo_line,strlen(echo_line));
	const char* blank_line = "\r\n";
	write(fd,blank_line,strlen(blank_line));
	sendfile(fd,s,NULL,size);
	close(s);
}
int get_line(int fd,char *buf,int len)//读取一行
{
	char c = '\0';
	int i = 0;
	while(c != '\n' && i<len-1){
		ssize_t s = recv(fd,&c,1,0);
		if(s > 0){
			if(c == '\r'){
				recv(fd,&c,1,MSG_PEEK);
				if(c == '\n'){
					recv(fd,&c,1,0);
				}else{
					c = '\n';
				}
			}
			buf[i++] = c;
		}
	}	
	buf[i] = 0;
	return i;
}
void* handler(void *arg)
{
	int fd = (int)arg;
	int error_num = 200;
	int cgi = 0;
	char* query_string = NULL;
#ifdef _DEBUG_	
	char buf[1024];
	int ret = 1;
	do{
		ret = get_line(fd,buf,sizeof(buf));
		printf("%s",buf);
	}while(ret > 0&&strcmp(buf,"\n"));	
	
	error_num = 404;
	goto end;
#else
	char method[1024/10];
	char url[1024];
	char buf[1024];
	char path[1024];
	int i,j;
	if(get_line(fd,buf,sizeof(buf)) <= 0){
		error_num = 404;
		goto end;
	}
	i=0;
	j=0;

	while(i<sizeof(method)-1&&j<sizeof(buf)&&!isspace(buf[j])){ //提取报头中方法
		method[i] = buf[j];
		i++;
		j++;
	}
	method[i]=0;
	while(isspace(buf[j])&&j<sizeof(buf)){                     //跳过空格
		j++;
	}
	i = 0;
	while(i<sizeof(url)-1&&j<sizeof(buf)&&!isspace(buf[j])){  //提取报头中url
		url[i] = buf[j];
		i++;
		j++;
	}
	url[i] = 0;
	printf("method:%s url:%s\n",method,url);
	if(strcasecmp("GET",method) == 0){                      //GET方法后(?)如果跟参数使用cgi，不跟直接响应
		query_string = url;
		while(*query_string != 0){
			if(*query_string == '?'){
				cgi = 1;
				*query_string = '\0';
				query_string++;
				break;
			}
			query_string++;
		}
	}else if(strcasecmp("POST",method) == 0){             //POST方法直接使用cgi，参数在正文中
		printf("woshi post\n");
		cgi = 1;
	}else{
		error_num = 404;
		goto end;
	}
	printf("url:%s\n",url);
	sprintf(path,"wwwroot%s",url);  
	if(path[strlen(path)-1] ==  '/'){
		strcat(path,"index.html");
	}
	printf("path:%s\n",path);
	struct stat st;
	if(stat(path,&st) < 0){                                     //生成文件模式
		error_num = 404;
		goto end;
	}else{
		if(S_ISREG(st.st_mode)){									 //判断是否为文件
			if(S_ISDIR(st.st_mode)){                               //判断是否为目录
				strcat(path,"/index.html");
			}else{
				if(st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode & S_IXOTH)  //为可执行文件
					cgi = 1;
			}	
		}
		if(cgi){
			
			exe_cgi(fd,method,query_string,path);
		}else{
			respond(fd,path,st.st_size);
		}
	}

		
#endif
end:
	echo_error(fd,error_num);	
	close(fd);
}
int startup(int port)                          //建立连接
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0){
		perror("socket");
		exit(2);
	}
	int opt=1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));	
	struct sockaddr_in lockl_socket;
	lockl_socket.sin_family=AF_INET;
	lockl_socket.sin_addr.s_addr=htonl(INADDR_ANY);
	lockl_socket.sin_port=htons(port);
	if(bind(sock,(struct sockaddr*)&lockl_socket,sizeof(struct sockaddr_in)) < 0){
		perror("bind");
		close(sock);
		exit(3);
	}	
	if(listen(sock,5) < 0){
		perror("listen");
		close(sock);
		exit(4);
	}
	
	return sock;
}
void Usage(char *port)
{
	printf("Usage:./http [%s,port]\n",port);
}
int main(int argc,char *argv[])
{
	if(argc != 2){
		Usage(argv[1]);
		return 1;
	}
	int listen_fd = startup(atoi(argv[1]));
	struct sockaddr_in client_socket;
    socklen_t len = sizeof(client_socket);
	for(;;){
   		int fd = accept(listen_fd,(struct sockaddr*)&client_socket,&len);
    	if(fd < 0){
    		perror("accept");
      		exit(5);
   		}
	   	printf("get a new connect...[%s] %d\n",inet_ntoa(client_socket.sin_addr),ntohs(client_socket.sin_port));
		
		pthread_t id;
		pthread_create(&id,NULL,handler,(void*)fd);//创建线程
		pthread_detach(id);
	}
	return 0;
}
