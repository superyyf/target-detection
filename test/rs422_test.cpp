#include <stdint.h>
 #include<stdio.h>      /*标准输入输出定义*/    
 #include<stdlib.h>     /*标准函数库定义*/    
 #include<unistd.h>     /*Unix 标准函数定义*/    
 #include<sys/types.h>     
 #include<sys/stat.h>       
 #include<fcntl.h>      /*文件控制定义*/    
 #include<termios.h>    /*PPSIX 终端控制定义*/    
 #include<errno.h>      /*错误号定义*/    
 #include<string.h>

//宏定义    
#define FALSE  -1    
#define TRUE   0
//串口数据结构体
struct ReceiveInfo {
        uint8_t flag1;
        uint8_t f_num;
        uint8_t t_h;
        uint8_t t_m;
        uint8_t t_s;
        uint16_t t_ms;
        uint8_t flag2;
} __attribute__((packed));

/*******************************************************************  
    * 名称：            UART0_Recv  
    * 功能：            接收串口数据  
    * 入口参数：        fd         文件描述符      
    *                   rcv_buf    接收串口中数据存入rcv_buf缓冲区中  
    *                   data_len   一帧数据的长度  
    * 出口参数：        正确返回为1，错误返回为0  
    *******************************************************************/  
int UART0_Recv(int fd, char *rcv_buf,int data_len)    
    {    
        int len,fs_sel;    
        fd_set fs_read;    
           
        struct timeval time;    
           
        FD_ZERO(&fs_read);    
        FD_SET(fd,&fs_read);    
           
        time.tv_sec = 0;    
        time.tv_usec = 15000;    
           
        //使用select实现串口的多路通信    
        fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);    
        printf("fs_sel = %d\n",fs_sel);    
        if(fs_sel)    
        {    
            len = read(fd,rcv_buf,data_len);    
            return len;    
        }    
        else    
        {    
            return FALSE;    
        }         
    } 

void data_analysis(char* rcv_buf,int data_len)
{
	if(data_len != 7)
	{
		printf("data frame num error !");
	}
	
	printf("帧头 ： %d \n",rcv_buf[0]);
	printf("视频帧号 ： %d \n",rcv_buf[1]);
	printf("时间-小时 ： %d \n",rcv_buf[2]);
	printf("时间-分钟 ： %d \n",rcv_buf[3]);
	printf("时间-秒 ： %d \n",rcv_buf[4]);
	printf("时间-10毫秒 ： %d \n",rcv_buf[5]);
	printf("校验位 ： %d \n",rcv_buf[6]);		
}
 

int main()
{

	/*打开串口*/
	//int fd = open( "/dev/ttyTHS2", O_RDWR|O_NOCTTY|O_NDELAY);//   串口号(ttyS0,ttyS1,ttyS2)
	int fd = open( "/dev/ttyS0", O_RDWR|O_NOCTTY|O_NDELAY);
	if (fd<0)    
        {    
            perror("Can't Open Serial Port");   
	    return(FALSE);     
        }  
 	if(fcntl(fd, F_SETFL, 0) < 0)    
        {    
            printf("fcntl failed!\n");  
	    return(FALSE);     
        }    
	if(0 == isatty(STDIN_FILENO))    
        {    
            printf("standard input is not a terminal device\n");    
            return(FALSE);    
        }    
        else    
        {    
            printf("isatty success!\n");    
        }                  
        printf("fd->open=%d\n",fd);


	/*设置串口参数*/
	int speed = 115200;//比特率
	int flow_ctrl = 0;//流控制
	int databits = 8;//数据位
	int stopbits = 1;//停止位
	int parity = 'N';//效验类型 取值为N,E,O,,S
  
	int   i;    
        int   status;    
        int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};    
        int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};    
                 
        struct termios options;    
           
        /*  tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数还可以测试配置是否正确，
            该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.  */    
        if( tcgetattr( fd,&options)  !=  0)    
        {    
            perror("SetupSerial 1");        
            return(FALSE);     
        }    
          
        //设置串口输入波特率和输出波特率    
        for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)    
        {    
            if  (speed == name_arr[i])    
            {                 
                cfsetispeed(&options, speed_arr[i]);     
                cfsetospeed(&options, speed_arr[i]);      
            }    
        }         
           
        //修改控制模式，保证程序不会占用串口    
        options.c_cflag |= CLOCAL;    
        //修改控制模式，使得能够从串口中读取输入数据    
        options.c_cflag |= CREAD;    
          
        //设置数据流控制    
        switch(flow_ctrl)    
        {    
              
            case 0 ://不使用流控制    
                  options.c_cflag &= ~CRTSCTS;    
                  break;       
              
            case 1 ://使用硬件流控制    
                  options.c_cflag |= CRTSCTS;    
                  break;    
            case 2 ://使用软件流控制    
                  options.c_cflag |= IXON | IXOFF | IXANY;    
                  break;    
        }    
        //设置数据位    
        //屏蔽其他标志位    
        options.c_cflag &= ~CSIZE;    
        switch (databits)    
        {      
            case 5    :    
                         options.c_cflag |= CS5;    
                         break;    
            case 6    :    
                         options.c_cflag |= CS6;    
                         break;    
            case 7    :        
                     options.c_cflag |= CS7;    
                     break;    
            case 8:        
                     options.c_cflag |= CS8;    
                     break;      
            default:       
                     fprintf(stderr,"Unsupported data size\n");    
                     return (FALSE);     
        }    
        //设置校验位    
        switch (parity)    
        {      
            case 'n':    
            case 'N': //无奇偶校验位。    
                     options.c_cflag &= ~PARENB;     
                     options.c_iflag &= ~INPCK;        
                     break;     
            case 'o':      
            case 'O'://设置为奇校验        
                     options.c_cflag |= (PARODD | PARENB);     
                     options.c_iflag |= INPCK;                 
                     break;     
            case 'e':     
            case 'E'://设置为偶校验      
                     options.c_cflag |= PARENB;           
                     options.c_cflag &= ~PARODD;           
                     options.c_iflag |= INPCK;          
                     break;    
            case 's':    
            case 'S': //设置为空格     
                     options.c_cflag &= ~PARENB;    
                     options.c_cflag &= ~CSTOPB;    
                     break;     
            default:      
                     fprintf(stderr,"Unsupported parity\n");        
                     return (FALSE);     
        }     
        // 设置停止位     
        switch (stopbits)    
        {      
            case 1:       
                     options.c_cflag &= ~CSTOPB; break;     
            case 2:       
                     options.c_cflag |= CSTOPB; break;    
            default:       
                           fprintf(stderr,"Unsupported stop bits\n");     
                           return (FALSE);    
        }    
           
        //修改输出模式，原始数据输出    
        options.c_oflag &= ~OPOST;    
          
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);    
        //options.c_lflag &= ~(ISIG | ICANON);    
           
        //设置等待时间和最小接收字符    
        options.c_cc[VTIME] = 0.1; /* 读取一个字符等待1*(1/10)s */      
        options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */    
           
        //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读    
        tcflush(fd,TCIFLUSH);    
           
        //激活配置 (将修改后的termios数据设置到串口中）    
        if (tcsetattr(fd,TCSANOW,&options) != 0)      
        {    
            perror("com set error!\n");      
            return (FALSE);     
        } 





	/*接收信号*/
	char rcv_buf[8];
	ReceiveInfo *rcv_info; 
	while (1) //循环读取数据    
            {   
                int len = UART0_Recv(fd, rcv_buf,sizeof(rcv_buf));    

                if(len > 0)    
                {   
			rcv_info = reinterpret_cast<ReceiveInfo *>(rcv_buf);
			printf("f_num : %d\nt_h : %d\nt_m : %d\nt_s : %d\nt_ms : %d/n", rcv_info->f_num, rcv_info->t_h, rcv_info->t_m, rcv_info->t_s, rcv_info->t_ms);
                }    
                else    
                {    
                    printf("cannot receive data\n");    
                }    
            } 



	/*关闭串口*/
	close(fd);         
	return 0;
     
    }              

