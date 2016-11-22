// 官网==>> 胜达网   www.sddz.xyz
// 作者==>> 胜达电子老达 于 深圳
// 老达新浪博客==>>   百度搜索  倒戈人生
// 大众公益平台Zigbee技术交流群==>> 胜达电子Zigbee大众群1 、2...
// 配套视频==>>   胜达电子原创超高清视频教程   《Zigbee深入浅出实战篇》
// 视频配套学习板、Dongle提供点==>>  淘宝店铺   胜达电子零售店  老达原创设计


#include<ioCC2530.h>
#include"74LS164_8LED.h"
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
void Uart0_Cfg()
{
   PERCFG &=0xFE;//把这个寄存器的第零位强行清零  1111 1110 
   //就是把串口0的脚位置配置在备用位置1 即P0_2  P0_3
   
   P0SEL  |=0x0C;//让P0_2  P0_3这两个脚工作在片上外设模式,而不是普通IO口       0000 1100
   
   U0CSR |=0xC0;
   U0UCR =0; //串口0 典型的串口配置  校验位 停止位之类的东西
   
   U0GCR =11;
   U0BAUD =216;//就是重官方数据手册中波特率表格中参照115200时的 配置值，前提是系统时钟在32M
   
   IEN0 |=0x04; //开接收数据的中断  0000 0100
   EA=1;
}

void Uart0SendByte(char SendByte)
{
    U0DBUF=SendByte;  //把我们收到的数据通过串口再返回发出去
    while(UTX0IF==0);
    UTX0IF=0;
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
    RFIRQM0 |= 0x40;
    // enable general RF interrupts
    IEN2 |= 0x01;
    
    FREQCTRL =(11+(25-11)*5);//(MIN_CHANNEL + (channel - MIN_CHANNEL) * CHANNEL_SPACING);    
    PAN_ID0=0x07;
    PAN_ID1=0x20;    
//halRfRxInterruptConfig(basicRfRxFrmDoneIsr);    
    RFST = 0xEC;//清接收缓冲器
    RFST = 0xE3;//开启接收使能 
    EA=1;    
}


void main()
{
   LS164_Cfg();//74LS164控制数码管的初始化
   Init32M(); //主时钟晶振工作在32M 
   halRfInit();
   Uart0_Cfg();
   
  SHORT_ADDR0=0xEF;
  SHORT_ADDR1=0xBE;//设置本模块地址  0xBEEF
  
    LS164_BYTE(2); 
    while(1);
}

void RevRFProc()
{
 static char len;
 static char  ch;
 static char Alllen;
    len=ch=0;
    RFIRQM0 &= ~0x40;
    IEN2 &= ~0x01;
    EA=1;
 
    len=RFD;//读第一个字节判断这一串数据后面有几个字节；
    //len=0x0C 12
    Alllen=len;
    while (len>0) 
    {//只要后面还有数据那么就把他都从接受缓冲区取出来
        ch=RFD;
        if(3==len)
        {//如果倒数第三个字节等于7，那么我们把LED0取反
           LS164_BYTE(ch);
        }
        
        if((len>=3)&&(Alllen-len>=9))
        {
            Uart0SendByte(ch);
        }
        len--;
     }     
    EA=0;
    // enable RXPKTDONE interrupt
    RFIRQM0 |= 0x40;
    // enable general RF interrupts
    IEN2 |= 0x01;        
}

#pragma vector=RF_VECTOR
__interrupt void RF_IRQ(void)
{//这个是射频中断函数，当小灯模块接收到开关模块发送来的数据时，小灯模块的CPU就会进入中断函数执行
    EA=0;
    if( RFIRQF0 & 0x40 )
    {
        
        RevRFProc();
             
        RFIRQF0&= ~0x40;   // Clear RXPKTDONE interrupt
    }
    S1CON= 0;                   // Clear general RF interrupt flag
    RFST = 0xEC;//清接收缓冲器
    RFST = 0xE3;//开启接收使能 
    EA=1;
}
