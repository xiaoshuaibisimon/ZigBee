// 官网==>> 胜达网   www.sddz.xyz
// 作者==>> 胜达电子老达 于 深圳
// 老达新浪博客==>>   百度搜索  倒戈人生
// 大众公益平台Zigbee技术交流群==>> 胜达电子Zigbee大众群1 、2...
// 配套视频==>>   胜达电子原创超高清视频教程   《Zigbee深入浅出实战篇》
// 视频配套学习板、Dongle提供点==>>  淘宝店铺   胜达电子零售店  老达原创设计


#include<ioCC2530.h>
#include"74LS164_8LED.h"
#define SENDVAL 5
char SendPacket[]={0x19,0x61,0x88,0x00,0x07,0x20,0xEF,0xBE,0x20,0x50,'h','e','l','l','o',' ','l','a','o',' ','d','a','\r','\n'};
//第一个字节0x0C含义，这个自己后面还有12个字节要发送
//第5 6个字节表示的是PANID
//第7 8个字节是无线模块目标设备的网络地址 0xBEEF
//第9 10就是本地模块的网络地址
//11 个字节是我们有用的数据
// CRC码 12 13个字节 是硬件自动追加

void Delay()
{
    int y,x;
    for(y=1000;y>0;y--)
      for(x=30;x>0;x--);
}
void Init32M()
{
   SLEEPCMD &=0xFB;//1111 1011 开启2个高频时钟源
   while(0==(SLEEPSTA & 0x40));// 0100 0000 等待32M稳定
   Delay();
   CLKCONCMD &=0xF8;//1111 1000 不分频输出
   CLKCONCMD &=0XBF;//1011 1111 设置32M作为系统主时钟
   while(CLKCONSTA & 0x40); //0100 0000 等待32M成功成为当前系统主时钟
}
void KeysIntCfg()
{//Key3  Key4   Key5
     
     IEN2|=0x10;//开P1IE组中断
     P1IEN|=0x02;//开Key3组内中断
     PICTL|=0x02;//设置P1_1为下降沿 
     EA=1;      //开总中断
}


void halRfInit(void)
{
    EA=0;
    FRMCTRL0 |= 0x60;

    // Recommended RX settings  
    TXFILTCFG = 0x09;
    AGCCTRL1 = 0x15;
    FSCAL1 = 0x00;
    
    // enable RXPKTDONE interrupt  
    RFIRQM0 |= 0x40;//把射频接收中断打开
    // enable general RF interrupts
    IEN2 |= 0x01;
    
    FREQCTRL =(11+(25-11)*5);//(MIN_CHANNEL + (channel - MIN_CHANNEL) * CHANNEL_SPACING);   
                     //设置载波为2475M
    PAN_ID0=0x07;
    PAN_ID1=0x20; //0x2007   
//halRfRxInterruptConfig(basicRfRxFrmDoneIsr);    
    RFST = 0xEC;//清接收缓冲器
    RFST = 0xE3;//开启接收使能 
    EA=1;    
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

void main()
{
   LS164_Cfg();//74LS164控制数码管的初始化
   Init32M(); //主时钟晶振工作在32M 
   KeysIntCfg(); 
   
   halRfInit();//无线通信的初始化  初始化相关的寄存器，配置工作信道，和PANID
   
  SHORT_ADDR0=0x50;
  SHORT_ADDR1=0x20;//设置本模块地址  设置本模块的网络地址0x2050
  //大小端模式问题，
  
    LS164_BYTE(1); 
    while(1);
}
#pragma vector=P1INT_VECTOR
__interrupt void Key3_ISR() //P1_1
{
     if(0x02 & P1IFG)
     {
         Delay();
         if(0==P1_1)
         {           
           P1DIR |=0X01;
           P1_0 ^=1;
           RFSend(SendPacket,24);
         }     
     }

     P1IFG=0;
     P1IF=0;
}


#pragma vector=RF_VECTOR
__interrupt void RF_IRQ(void)
{//这个是射频中断函数，当小灯模块接收到开关模块发送来的数据时，小灯模块的CPU就会进入中断函数执行
    EA=0;
    if( RFIRQF0 & 0x40 )
    {            
        RFIRQF0&= ~0x40;   // Clear RXPKTDONE interrupt
    }
    S1CON= 0;                   // Clear general RF interrupt flag
    RFST = 0xEC;//清接收缓冲器
    RFST = 0xE3;//开启接收使能 
    EA=1;
}
