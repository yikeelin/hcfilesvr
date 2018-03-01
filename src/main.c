#include "commsocket.h"
#include "sckutil.h"
#include "session.h"
#include "parseconf.h"
#include "tunable.h"
#include "ftpproto.h"
#include "ftpcodes.h"
#include "common.h"
#include "hash.h"
#include "debugwriter.h"

void paint_conf()
{
	vdebugwrite( __FILE__, __LINE__, "tunable_pasv_enable: %d", 			tunable_pasv_enable);
	vdebugwrite( __FILE__, __LINE__, "tunable_port_enable: %d", 			tunable_port_enable);
	vdebugwrite( __FILE__, __LINE__, "tunable_listen_port: %u", 			tunable_listen_port);
	vdebugwrite( __FILE__, __LINE__, "tunable_portmodel_bind_port: %u", 	tunable_portmodel_bind_port);
	vdebugwrite( __FILE__, __LINE__, "tunable_max_clients: %u",			tunable_max_clients);
	vdebugwrite( __FILE__, __LINE__, "tunable_max_per_ip:  %u", 			tunable_max_per_ip);
	vdebugwrite( __FILE__, __LINE__, "tunable_accept_timeout: %u", 		tunable_accept_timeout);
	vdebugwrite( __FILE__, __LINE__, "tunable_connect_timeout: %u", 		tunable_connect_timeout);
	vdebugwrite( __FILE__, __LINE__, "tunable_idle_session_timeout: %u", 	tunable_idle_session_timeout);
	vdebugwrite( __FILE__, __LINE__, "tunable_data_connection_timeout: %u",tunable_data_connection_timeout);
	vdebugwrite( __FILE__, __LINE__, "tunable_local_umask: %u", 			tunable_local_umask);
	vdebugwrite( __FILE__, __LINE__, "tunable_upload_max_rate: %u", 		tunable_upload_max_rate);
	vdebugwrite( __FILE__, __LINE__, "tunable_download_max_rate: %u", 		tunable_download_max_rate);
		
	if (tunable_listen_address == NULL)
	{
		vdebugwrite( __FILE__, __LINE__, "tunable_listen_address: NULL");
	}
	else
	{
		vdebugwrite( __FILE__, __LINE__, "tunable_listen_address: %s",tunable_listen_address);
	}
}

extern session_t *p_sess;
static unsigned int s_children;

static hash_t *s_ip_count_hash;
static hash_t *s_pid_ip_hash;  


void check_limits(session_t *sess);
void handle_sigchld(int sig);
unsigned int hash_func(unsigned int buckets, void *key);

unsigned int handle_ip_count(void *ip);
void drop_ip_count(void *ip);

int check_repeat(const char * progname);

void deamonize ();

int main(void)
{	
	deamonize();
	
	parseconf_load_file("../hcfilesvr.cfg");
	//paint_conf();
	
	
	
	/*if (getuid() !=0)
	{
		ERR_EXIT( "miniftpd: must be started as root\n");
	}*/
		
	session_t sess = {
		
		0,-1,"","","",
		
		NULL,-1,-1,0,
		0,0,0,0,
		-1,-1,
		0,0,NULL,0,
		0,0
	};
	p_sess = &sess;
	sess.bw_upload_rate_max = tunable_upload_max_rate;
	sess.bw_download_rate_max = tunable_download_max_rate;	
	s_ip_count_hash = hash_alloc(256, hash_func);
	s_pid_ip_hash = hash_alloc(256, hash_func);
	signal(SIGCHLD, handle_sigchld);
	int listenfd = 0;
	int ret = sckServer_init(tunable_listen_address, tunable_listen_port, &listenfd);
	
	if( ret )
	{
		ERR_EXIT("sckServer_init failed");
	}
	int connfd = 0;
	pid_t pid;
	struct sockaddr_in addr;
	while (1)
	{
		
		//connfd = accept_timeout(listenfd, &addr, 0);
		ret = sckServer_accept(listenfd, &connfd, (void*)&addr, 0);
		if (ret == Sck_ErrTimeOut)
		{
			ERR_EXIT("accept timeout");
		}

		unsigned int ip = addr.sin_addr.s_addr;
		//printf("get_sock_addr ip: %u\n", ip);//++++
		sess.num_this_ip = handle_ip_count(&ip);

		
		++s_children;
		sess.num_clients = s_children;
		
		pid = fork();
		if (pid == -1)
		{
			--s_children;
			ERR_EXIT("fork");
		}
		if (pid == 0)
		{
			close(listenfd);
			sess.ctrl_fd = connfd;
			
			check_limits(&sess);
			
			signal(SIGCHLD, SIG_IGN);
		
			begin_session(&sess);
		}
		else
		{
			hash_add_entry(s_pid_ip_hash, &pid, sizeof(pid),
				&ip, sizeof(unsigned int));
			close(connfd);
		}
	}
	
	return 0;
}

void check_limits(session_t *sess)
{
	if (tunable_max_clients > 0 && sess->num_clients > tunable_max_clients)
	{
		
		ftp_relply(sess, FTP_TOO_MANY_USERS, "There are too many connected users please try later");
		exit(EXIT_FAILURE);
	}
	
	if (tunable_max_per_ip >0 && sess->num_this_ip > tunable_max_per_ip)
	{
		ftp_relply(sess, FTP_IP_LIMIT, "There are too many connecter from your internal addr");
		exit(EXIT_FAILURE);
	}
	
}

void handle_sigchld(int sig)
{
	
	pid_t pid;
	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
	{
		--s_children;
		unsigned int *ip = hash_lookup_entry(s_pid_ip_hash, &pid, sizeof(pid));
		if (ip == NULL)
		{
			continue;
		}
		
		drop_ip_count(ip);
		hash_free_entry(s_pid_ip_hash, &pid, sizeof(pid));
	}
	
}

unsigned int hash_func(unsigned int buckets, void *key)
{
	unsigned int *number = (unsigned int*)key;
	
	return (*number) % buckets;
}

unsigned int handle_ip_count(void *ip)
{
	unsigned int count;
	unsigned int *p_count = (unsigned int *)hash_lookup_entry(s_ip_count_hash, 
		ip, sizeof(unsigned int));
		
	if (p_count == NULL)
	{
		count = 1;
		hash_add_entry(s_ip_count_hash, ip, sizeof(unsigned int),
			&count, sizeof(unsigned int));
	}
	else
	{
		count = *p_count;
		++count;
		*p_count = count;
	}
	
	//printf("handle_ip_count :%u\n", count);//+++++
	
	return count;
}

void drop_ip_count(void *ip)
{
	unsigned int count;
	unsigned int *p_count = (unsigned int *)hash_lookup_entry(s_ip_count_hash, 
		ip, sizeof(unsigned int));
		
	if (p_count == NULL)
	{
		return;
	}

	count = *p_count;
	if (count <= 0)
	{
		return;
	}
	--count;
	*p_count = count;
	
	if (count == 0)
	{
		hash_free_entry(s_ip_count_hash, ip, sizeof(unsigned int));
	}
	
	//printf("drop_ip_count :%u\n", count);//++++++
	
}

		/*守护进程*/
/*******************************************************
*	1.创建子进程,父进程退出
*	2.在子进程中创建新会话
*	3.改变当前工作目录
*	4.重设文件权限掩码
*	5.关闭文件描述符
*	6.开始执行守护进程核心工作
*	7.守护进程退出处理
********************************************************/
void deamonize ()
{
	char szDebugMsg[512];
	pid_t pid;
	//成为一个新会话的首进程, 失去控制终端
	if((pid = fork()) < 0)
	{
		sprintf( szDebugMsg, "fork failed" );
		vdebugwrite(__FILE__, __LINE__, szDebugMsg );
		exit(1);
	}
	else if(pid != 0) //父进程
		exit(0);
	
	setsid();
	
	
	//改变当前工作目录到/目录下
	/*if(chdir("/") < 0)
	{
		sprintf( szDebugMsg, "chdir失败" );
		vdebugwrite(__FILE__, __LINE__, szDebugMsg );
		exit(1);
	}
	
	sprintf( szDebugMsg, "chdir(\"/\")成功" );
	vdebugwrite(__FILE__, __LINE__, szDebugMsg );
	*/
	
	//umask(0002);
	
	//重定向0,1,2文件描述符到/dev/null
	close(0);
	open("/dev/null", O_RDWR);  //这时fd0 是/dev/null
	dup2(0,1);
	dup2(0,2);
	
	sprintf( szDebugMsg, "deamonize successfully." );
	vdebugwrite(__FILE__, __LINE__, szDebugMsg );
}
