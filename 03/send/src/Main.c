// ����==>> ʤ����   www.sddz.xyz
// ����==>> ʤ������ϴ� �� ����
// �ϴ����˲���==>>   �ٶ�����  ��������
// ���ڹ���ƽ̨Zigbee��������Ⱥ==>> ʤ�����Zigbee����Ⱥ1 ��2...
// ������Ƶ==>>   ʤ�����ԭ����������Ƶ�̳�   ��Zigbee����ǳ��ʵսƪ��
// ��Ƶ����ѧϰ�塢Dongle�ṩ��==>>  �Ա�����   ʤ��������۵�  �ϴ�ԭ�����


#include<ioCC2530.h>
#include"74LS164_8LED.h"
#define SENDVAL 5
char SendPacket[]={0x19,0x61,0x88,0x00,0x07,0x20,0xEF,0xBE,0x20,0x50,'h','e','l','l','o',' ','l','a','o',' ','d','a','\r','\n'};
//��һ���ֽ�0x0C���壬����Լ����滹��12���ֽ�Ҫ����
//��5 6���ֽڱ�ʾ����PANID
//��7 8���ֽ�������ģ��Ŀ���豸�������ַ 0xBEEF
//��9 10���Ǳ���ģ��������ַ
//11 ���ֽ����������õ�����
// CRC�� 12 13���ֽ� ��Ӳ���Զ�׷��

void Delay()
{
    int y,x;
    for(y=1000;y>0;y--)
      for(x=30;x>0;x--);
}
void Init32M()
{
   SLEEPCMD &=0xFB;//1111 1011 ����2����Ƶʱ��Դ
   while(0==(SLEEPSTA & 0x40));// 0100 0000 �ȴ�32M�ȶ�
   Delay();
   CLKCONCMD &=0xF8;//1111 1000 ����Ƶ���
   CLKCONCMD &=0XBF;//1011 1111 ����32M��Ϊϵͳ��ʱ��
   while(CLKCONSTA & 0x40); //0100 0000 �ȴ�32M�ɹ���Ϊ��ǰϵͳ��ʱ��
}
void KeysIntCfg()
{//Key3  Key4   Key5
     
     IEN2|=0x10;//��P1IE���ж�
     P1IEN|=0x02;//��Key3�����ж�
     PICTL|=0x02;//����P1_1Ϊ�½��� 
     EA=1;      //�����ж�
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
    RFIRQM0 |= 0x40;//����Ƶ�����жϴ�
    // enable general RF interrupts
    IEN2 |= 0x01;
    
    FREQCTRL =(11+(25-11)*5);//(MIN_CHANNEL + (channel - MIN_CHANNEL) * CHANNEL_SPACING);   
                     //�����ز�Ϊ2475M
    PAN_ID0=0x07;
    PAN_ID1=0x20; //0x2007   
//halRfRxInterruptConfig(basicRfRxFrmDoneIsr);    
    RFST = 0xEC;//����ջ�����
    RFST = 0xE3;//��������ʹ�� 
    EA=1;    
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

void main()
{
   LS164_Cfg();//74LS164��������ܵĳ�ʼ��
   Init32M(); //��ʱ�Ӿ�������32M 
   KeysIntCfg(); 
   
   halRfInit();//����ͨ�ŵĳ�ʼ��  ��ʼ����صļĴ��������ù����ŵ�����PANID
   
  SHORT_ADDR0=0x50;
  SHORT_ADDR1=0x20;//���ñ�ģ���ַ  ���ñ�ģ��������ַ0x2050
  //��С��ģʽ���⣬
  
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
{//�������Ƶ�жϺ�������С��ģ����յ�����ģ�鷢����������ʱ��С��ģ���CPU�ͻ�����жϺ���ִ��
    EA=0;
    if( RFIRQF0 & 0x40 )
    {            
        RFIRQF0&= ~0x40;   // Clear RXPKTDONE interrupt
    }
    S1CON= 0;                   // Clear general RF interrupt flag
    RFST = 0xEC;//����ջ�����
    RFST = 0xE3;//��������ʹ�� 
    EA=1;
}
