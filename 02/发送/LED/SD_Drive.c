#include<ioCC2530.h>
#include"SD_Drive.h"
#define  LIGHTCMD   4
char SendPacket[]={0x0C,0x61,0x88,0x00,0x07,0x20,0xEF,0xBE,0x20,0x50,LIGHTCMD,0};
char *pSendPacket=SendPacket;

char SendPacket1[]={0x0C,0x61,0x88,0x01,0x07,0x20,0xEF,0xBE,0x20,0x50,LIGHTCMD,0};
char *pSendPacket1=SendPacket;

char SendPacket2[]={0x05,0x02,0x00,0x02,0};
char *pSendPacket2=SendPacket;
void ON_32MOSC();
void ON_16MRC();
void DriveCfg();
void LEDs_Cfg();
void Bee_Cfg();
void LxChangR();
void KeysIntCfg();

void Uart0_Cfg();
void Uart0_SendCh(char ch);

void halRfInit(void);
void RevRFProc();
void Delay()
{
    int y,x;
    for(y=1000;y>0;y--)
      for(x=40;x>0;x--);
}
void ON_32MOSC()
{
      CLKCONCMD &= ~0x40;                          //设置系统时钟源为32MHZ晶振
      while(CLKCONSTA & 0x40);                     //等待晶振稳定
      CLKCONCMD &= ~0x47;                          //设置系统主时钟频率为32MHZ 
}
void ON_16MRC()
{
      CLKCONCMD |=0x40;                      //设置系统时钟为16M RC
      while(1==(CLKCONSTA & 0x40));          //等待晶振稳定
      CLKCONCMD |=0x41;                      //设置系统主时钟频率为16MHZ
}
void LEDs_Cfg()
{//LED组
     P0SEL&=~0x12;//P0_1(LED2),P0_4(LED3)  0001 0010  普通IO口模式
     P0DIR|=0x12;//输出
  
     P1SEL&=~0x01;//P1_0（LED0）  0000 0001
     P1DIR|=0x01;//输出
}
void Bee_Cfg()
{//蜂鸣器
     P0SEL&=~0x01;
     P0DIR|=0x01;//输出 
     OFF_BEE;
}
void LxChangR()
{//光敏电阻
     P0SEL&=~0x40;//P0_6  0100 0000  普通IO口模式
     P0DIR&=~0x40;
     P0INP&=~0x40;
     P2INP&=~0x20;//P2INP 第5  0010 0000 P0开起上拉电阻功能
}
void Uart0_Cfg()
{
  PERCFG&=~0x01;   //有2个备用位置，0使用备用位置1；1使用备用位置2
  P0SEL |= 0x0C;   //P0_2 RXD P0_3 TXD 外设功能 0000 1100
 
  U0CSR |= 0xC0;  //串口接收使能  1100 0000 工作UART模式+允许接受
  U0UCR |= 0x00;  //无奇偶校验，1位停止位
 
  U0GCR |= 0x08;  
  U0BAUD = 0x3b;  //波特率：9600bps  
 
  IEN0 |= 0X04;     //开串口接收中断 'URX0IE = 1',也可以写成 URX0IE=1;
  EA=1;
}



void Uart0_SendCh(char ch)
{
    U0DBUF = ch;
    while(UTX0IF == 0);
    UTX0IF = 0;
}
void KeysIntCfg()
{//Key3  Key4   Key5
     
     IEN2|=0x10;//开P1IE组中断
     P1IEN|=0x02;//开Key3组内中断
     PICTL|=0x02;//设置P1_1为下降沿
     
     IEN2|=0x02;
     P2IEN|=0x01;
     PICTL|=0x08;//设置P2_0为下降沿
     
     P0IE=1;//或者写成 IEN1|=0x20
     P0IEN|=0x20;
     PICTL|=0x01;//设置P0_5为下降沿
     
     
     EA=1;      //开总中断
}

//函数void halRfInit(void)，是射频初始化配置，最需要关注的是我们要让2个模块的个域网、信道设置好并且一致，实际上要配置完成还
//有一项就是下程序模块的网络短地址，由于这一项由2个模块，只能在主函数中通过宏来分别设置，开关模块地址为0x2050，小灯模块0xbeef
void halRfInit(void)
{
   EA=0;
    FRMCTRL0 |= 0x60;

    // Recommended RX settings  推荐射频接收设置
    TXFILTCFG = 0x09;
    AGCCTRL1 = 0x15;
    FSCAL1 = 0x00;

    //下面2个寄存器设置是开射频中断
    // enable RXPKTDONE interrupt  
    RFIRQM0 |= 0x40;
    // enable general RF interrupts
    IEN2 |= 0x01;
    
    
//设置工作信道
      FREQCTRL =(11+(25-11)*5);//(MIN_CHANNEL + (channel - MIN_CHANNEL) * CHANNEL_SPACING);    
//设置PANID,个域网ID，由于发送模块和接受模块都会执行这个函数，所以很显然他们的个域网ID是一样的，信道也是一样的
      PAN_ID0=0x07;
      PAN_ID1=0x20;    
//halRfRxInterruptConfig(basicRfRxFrmDoneIsr);    
    RFST = 0xEC;//清接收缓冲器
    RFST = 0xE3;//开启接收使能 
    EA=1;    
}
void RevRFProc()
{
 static char len;
 static char  ch;
 len=ch=0;
    RFIRQM0 &= ~0x40;
    IEN2 &= ~0x01;
    EA=1;
 
    len=RFD;//读第一个字节判断这一串数据后面有几个字节；
    Uart0_SendCh(len);
    while (len>0) 
    {//只要后面还有数据那么就把他都从接受缓冲区取出来
        ch=RFD;
        Uart0_SendCh(ch);
        if((3==len)&&(LIGHTCMD==ch))
        {//如果倒数第三个字节等于7，那么我们把LED0取反
           P0_1 ^=1;
        }
        len--;
     }     
    EA=0;
    // enable RXPKTDONE interrupt
    RFIRQM0 |= 0x40;
    // enable general RF interrupts
    IEN2 |= 0x01;        
}

void RFSend(char *pstr,char len)
{
  char i;
    RFST = 0xEC; //确保接收是空的
    RFST = 0xE3; //清接收标志位
    while (FSMSTAT1 & 0x22);//等待射频发送准备好
    RFST = 0xEE;//确保发送队列是空
    RFIRQF1 &= ~0x02;//清发送标志位
//为数据发送做好准备工作

    for(i=0;i<len;i++)
    {
       RFD=pstr[i];
    }  //循环的作用是把我们要发送的数据全部压到发送缓冲区里面
    
    RFST = 0xE9; //这个寄存器一旦被设置为0xE9,发送缓冲区的数据就被发送出去
    while(!(RFIRQF1 & 0x02) );//等待发送完成
    RFIRQF1 = ~0x02;//清发送完成标志
}
void DriveCfg()
{
     LEDs_Cfg();
     Bee_Cfg();
     LxChangR();
     KeysIntCfg();
     Uart0_Cfg(); 
     halRfInit();
}


#pragma vector=P1INT_VECTOR
__interrupt void Key3_ISR() //P1_1
{
     Delay();
     if(0==P1_1)
     {
         if(0x02 & P1IFG)
         {
             ON_LED0;
             OFF_LED2;
             OFF_LED3;
             RFSend(SendPacket,11);
         }     
     }


     P1IFG &=~0x02;
     P1IF=0;
}
#pragma vector=P2INT_VECTOR
__interrupt void Key4_ISR()//P2_0
{
     ON_LED2;
     OFF_LED0;
     OFF_LED3;

     P2IFG &=~0x01;
     P2IF=0;
}
#pragma vector=P0INT_VECTOR
__interrupt void Key5_ISR()//P0_5
{
     ON_LED3;
     OFF_LED0;
     OFF_LED2;


     P0IFG &=~0x20;
     P0IF=0;
}

#pragma vector=URX0_VECTOR
__interrupt void URX0_IRQ(void)
{  
    char RevCh;
    URX0IF=0;
    
    RevCh=U0DBUF;
    SendPacket[10]=RevCh;
    RFSend(SendPacket,11);
}

#pragma vector=RF_VECTOR
__interrupt void RF_IRQ(void)
{//这个是射频中断函数，当小灯模块接收到开关模块发送来的数据时，小灯模块的CPU就会进入中断函数执行
  unsigned long i=100000;
    EA=0;
    
    if( RFIRQF0 & 0x40 )
    {
        RevRFProc();//小灯取反之类的处理
        
        S1CON= 0;                   // Clear general RF interrupt flag
        RFIRQF0&= ~0x40;   // Clear RXPKTDONE interrupt
    }
    while(i--);
    RFST = 0xEC;//清接收缓冲器
    RFST = 0xE3;//开启接收使能 
    EA=1;
}