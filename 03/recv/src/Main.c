// ����==>> ʤ����   www.sddz.xyz
// ����==>> ʤ������ϴ� �� ����
// �ϴ����˲���==>>   �ٶ�����  ��������
// ���ڹ���ƽ̨Zigbee��������Ⱥ==>> ʤ�����Zigbee����Ⱥ1 ��2...
// ������Ƶ==>>   ʤ�����ԭ����������Ƶ�̳�   ��Zigbee����ǳ��ʵսƪ��
// ��Ƶ����ѧϰ�塢Dongle�ṩ��==>>  �Ա�����   ʤ��������۵�  �ϴ�ԭ�����


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
   SLEEPCMD &=0xFB;//1111 1011 ����2����Ƶʱ��Դ
   while(0==(SLEEPSTA & 0x40));// 0100 0000 �ȴ�32M�ȶ�
   Delay();
   CLKCONCMD &=0xF8;//1111 1000 ����Ƶ���
   CLKCONCMD &=0XBF;//1011 1111 ����32M��Ϊϵͳ��ʱ��
   while(CLKCONSTA & 0x40); //0100 0000 �ȴ�32M�ɹ���Ϊ��ǰϵͳ��ʱ��
}
void Uart0_Cfg()
{
   PERCFG &=0xFE;//������Ĵ����ĵ���λǿ������  1111 1110 
   //���ǰѴ���0�Ľ�λ�������ڱ���λ��1 ��P0_2  P0_3
   
   P0SEL  |=0x0C;//��P0_2  P0_3�������Ź�����Ƭ������ģʽ,��������ͨIO��       0000 1100
   
   U0CSR |=0xC0;
   U0UCR =0; //����0 ���͵Ĵ�������  У��λ ֹͣλ֮��Ķ���
   
   U0GCR =11;
   U0BAUD =216;//�����عٷ������ֲ��в����ʱ���в���115200ʱ�� ����ֵ��ǰ����ϵͳʱ����32M
   
   IEN0 |=0x04; //���������ݵ��ж�  0000 0100
   EA=1;
}

void Uart0SendByte(char SendByte)
{
    U0DBUF=SendByte;  //�������յ�������ͨ�������ٷ��ط���ȥ
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
    RFST = 0xEC;//����ջ�����
    RFST = 0xE3;//��������ʹ�� 
    EA=1;    
}


void main()
{
   LS164_Cfg();//74LS164��������ܵĳ�ʼ��
   Init32M(); //��ʱ�Ӿ�������32M 
   halRfInit();
   Uart0_Cfg();
   
  SHORT_ADDR0=0xEF;
  SHORT_ADDR1=0xBE;//���ñ�ģ���ַ  0xBEEF
  
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
 
    len=RFD;//����һ���ֽ��ж���һ�����ݺ����м����ֽڣ�
    //len=0x0C 12
    Alllen=len;
    while (len>0) 
    {//ֻҪ���滹��������ô�Ͱ������ӽ��ܻ�����ȡ����
        ch=RFD;
        if(3==len)
        {//��������������ֽڵ���7����ô���ǰ�LED0ȡ��
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
{//�������Ƶ�жϺ�������С��ģ����յ�����ģ�鷢����������ʱ��С��ģ���CPU�ͻ�����жϺ���ִ��
    EA=0;
    if( RFIRQF0 & 0x40 )
    {
        
        RevRFProc();
             
        RFIRQF0&= ~0x40;   // Clear RXPKTDONE interrupt
    }
    S1CON= 0;                   // Clear general RF interrupt flag
    RFST = 0xEC;//����ջ�����
    RFST = 0xE3;//��������ʹ�� 
    EA=1;
}
