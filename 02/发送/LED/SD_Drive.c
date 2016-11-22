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
      CLKCONCMD &= ~0x40;                          //����ϵͳʱ��ԴΪ32MHZ����
      while(CLKCONSTA & 0x40);                     //�ȴ������ȶ�
      CLKCONCMD &= ~0x47;                          //����ϵͳ��ʱ��Ƶ��Ϊ32MHZ 
}
void ON_16MRC()
{
      CLKCONCMD |=0x40;                      //����ϵͳʱ��Ϊ16M RC
      while(1==(CLKCONSTA & 0x40));          //�ȴ������ȶ�
      CLKCONCMD |=0x41;                      //����ϵͳ��ʱ��Ƶ��Ϊ16MHZ
}
void LEDs_Cfg()
{//LED��
     P0SEL&=~0x12;//P0_1(LED2),P0_4(LED3)  0001 0010  ��ͨIO��ģʽ
     P0DIR|=0x12;//���
  
     P1SEL&=~0x01;//P1_0��LED0��  0000 0001
     P1DIR|=0x01;//���
}
void Bee_Cfg()
{//������
     P0SEL&=~0x01;
     P0DIR|=0x01;//��� 
     OFF_BEE;
}
void LxChangR()
{//��������
     P0SEL&=~0x40;//P0_6  0100 0000  ��ͨIO��ģʽ
     P0DIR&=~0x40;
     P0INP&=~0x40;
     P2INP&=~0x20;//P2INP ��5  0010 0000 P0�����������蹦��
}
void Uart0_Cfg()
{
  PERCFG&=~0x01;   //��2������λ�ã�0ʹ�ñ���λ��1��1ʹ�ñ���λ��2
  P0SEL |= 0x0C;   //P0_2 RXD P0_3 TXD ���蹦�� 0000 1100
 
  U0CSR |= 0xC0;  //���ڽ���ʹ��  1100 0000 ����UARTģʽ+�������
  U0UCR |= 0x00;  //����żУ�飬1λֹͣλ
 
  U0GCR |= 0x08;  
  U0BAUD = 0x3b;  //�����ʣ�9600bps  
 
  IEN0 |= 0X04;     //�����ڽ����ж� 'URX0IE = 1',Ҳ����д�� URX0IE=1;
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
     
     IEN2|=0x10;//��P1IE���ж�
     P1IEN|=0x02;//��Key3�����ж�
     PICTL|=0x02;//����P1_1Ϊ�½���
     
     IEN2|=0x02;
     P2IEN|=0x01;
     PICTL|=0x08;//����P2_0Ϊ�½���
     
     P0IE=1;//����д�� IEN1|=0x20
     P0IEN|=0x20;
     PICTL|=0x01;//����P0_5Ϊ�½���
     
     
     EA=1;      //�����ж�
}

//����void halRfInit(void)������Ƶ��ʼ�����ã�����Ҫ��ע��������Ҫ��2��ģ��ĸ��������ŵ����úò���һ�£�ʵ����Ҫ������ɻ�
//��һ������³���ģ�������̵�ַ��������һ����2��ģ�飬ֻ������������ͨ�������ֱ����ã�����ģ���ַΪ0x2050��С��ģ��0xbeef
void halRfInit(void)
{
   EA=0;
    FRMCTRL0 |= 0x60;

    // Recommended RX settings  �Ƽ���Ƶ��������
    TXFILTCFG = 0x09;
    AGCCTRL1 = 0x15;
    FSCAL1 = 0x00;

    //����2���Ĵ��������ǿ���Ƶ�ж�
    // enable RXPKTDONE interrupt  
    RFIRQM0 |= 0x40;
    // enable general RF interrupts
    IEN2 |= 0x01;
    
    
//���ù����ŵ�
      FREQCTRL =(11+(25-11)*5);//(MIN_CHANNEL + (channel - MIN_CHANNEL) * CHANNEL_SPACING);    
//����PANID,������ID�����ڷ���ģ��ͽ���ģ�鶼��ִ��������������Ժ���Ȼ���ǵĸ�����ID��һ���ģ��ŵ�Ҳ��һ����
      PAN_ID0=0x07;
      PAN_ID1=0x20;    
//halRfRxInterruptConfig(basicRfRxFrmDoneIsr);    
    RFST = 0xEC;//����ջ�����
    RFST = 0xE3;//��������ʹ�� 
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
 
    len=RFD;//����һ���ֽ��ж���һ�����ݺ����м����ֽڣ�
    Uart0_SendCh(len);
    while (len>0) 
    {//ֻҪ���滹��������ô�Ͱ������ӽ��ܻ�����ȡ����
        ch=RFD;
        Uart0_SendCh(ch);
        if((3==len)&&(LIGHTCMD==ch))
        {//��������������ֽڵ���7����ô���ǰ�LED0ȡ��
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
    RFST = 0xEC; //ȷ�������ǿյ�
    RFST = 0xE3; //����ձ�־λ
    while (FSMSTAT1 & 0x22);//�ȴ���Ƶ����׼����
    RFST = 0xEE;//ȷ�����Ͷ����ǿ�
    RFIRQF1 &= ~0x02;//�巢�ͱ�־λ
//Ϊ���ݷ�������׼������

    for(i=0;i<len;i++)
    {
       RFD=pstr[i];
    }  //ѭ���������ǰ�����Ҫ���͵�����ȫ��ѹ�����ͻ���������
    
    RFST = 0xE9; //����Ĵ���һ��������Ϊ0xE9,���ͻ����������ݾͱ����ͳ�ȥ
    while(!(RFIRQF1 & 0x02) );//�ȴ��������
    RFIRQF1 = ~0x02;//�巢����ɱ�־
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
{//�������Ƶ�жϺ�������С��ģ����յ�����ģ�鷢����������ʱ��С��ģ���CPU�ͻ�����жϺ���ִ��
  unsigned long i=100000;
    EA=0;
    
    if( RFIRQF0 & 0x40 )
    {
        RevRFProc();//С��ȡ��֮��Ĵ���
        
        S1CON= 0;                   // Clear general RF interrupt flag
        RFIRQF0&= ~0x40;   // Clear RXPKTDONE interrupt
    }
    while(i--);
    RFST = 0xEC;//����ջ�����
    RFST = 0xE3;//��������ʹ�� 
    EA=1;
}