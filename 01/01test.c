#include<iocc2530.h>

void main()
{
  int i = 0;
  P1DIR |= 0x01;
  P1_0 = 0;
  while(1)
  {
    for(i = 0;i<10000;i++)
    {
      ;
    }
    P1_0 = 1;
    for(i = 0;i<10000;i++)
    {
      ;
    }
    P1_0 = 0;
  }
}