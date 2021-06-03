/*******************************************************************
filename      mpuConversion.c
author        root
created date  2012-1-9
description   specifications of implemented functions
warning
modify history
    author        date        modify        description
*******************************************************************/

#include "include/MPUConvert.h"

const int HEX_PATTERN[8] =
  { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

/*******************************************************************
 function name			accumulate
 description			implement math function power(int,int)
 parameter
 int				IN	 		num
 int				IN			times
 Return value
 value of power(number,times)
 *******************************************************************/
u_int64
accumulate(int num, int times)
{

  int i = 0;
  u_int64 val;

	val = num;
	if (times == 0)
	{
	  return 1;
	}

  for (i = 0; i < times - 1; i++)
    {
    val *= num;
  }
return val;
}

/*******************************************************************
 function name			setBitsOfByte
 description			set definition bits of one byte
 parameter
 BYTE *				IN	 		bye
 int				IN			posii
 int				IN			width
 int 				IN			value
 Return value
 true or false
 *******************************************************************/
int
setBitsOfByte(BYTE *bye, int posii, int width, int value)
{
	int bits[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; //low -- high
	int i, value_bit;
	if ((value >= (int)accumulate(2, width)) || (value <= -(int)accumulate(2, width)))
	{
		return -1;
	}
	for (i = posii, value_bit = 0; i < posii + width; i++, value_bit++)
	{
		bits[i] = (value & HEX_PATTERN[value_bit]) >> value_bit;
		//set value to position i of byte
		*bye &= ~(1 << i);
		//if(bits[i]==1)  *bye|=(1<<i);
		*bye |= (bits[i] << i);
	}
	return true;
}

/*******************************************************************
 Function name			countValueofBits
 description			count value of some bits of a byte
 parameter
 BYTE				IN	 		b
 int				IN			pos
 int				IN 			width
 Return value
 value of bits
 *******************************************************************/
int
countValueofBits(BYTE b, int pos, int width)
{

int i = 0, value = 0, bits = 0;

 //travel over all bits which have counted
for (i = pos, bits = 0; i < pos + width; i++, bits++)
{
//get one bit value
value += (int) ((b & HEX_PATTERN[i]) >> i) * (int) HEX_PATTERN[bits];
}
return value;
}

/*******************************************************************
 Function name			getBytesOfValue
 description		    convert integer value to serials bytes
 parameter
 BYTE *				IN/OUT 		buffer
 u_int64			IN			value
 int				IN			len
 Return value
 true or false

 *******************************************************************/
int
getBytesOfValue(BYTE * buffer, u_int64 value, int len)
{
	int i=0;
	u_int64  abytevalue;

for (i = len - 1; i >= 0; i--)
{
abytevalue = value / accumulate(2, i * 8);
buffer[i] = (BYTE) abytevalue;
value -= abytevalue * accumulate(2, i * 8);
}
return true;
}

/*******************************************************************
 Function name			reverseBytes
 description		    reverse bytes from src to des variable
 parameter
 const BYTE *		IN	 		src
 BYTE *des			IN/OUT		des
 int				IN			len
 Return value
 true or false
 *******************************************************************/
int
reverseBytes(const BYTE *src, BYTE *des, int len)
{
int i = len - 1, j = 0;

for (i = len - 1, j = 0; i >= 0; i--, j++)
{

			memcpy(des+j,src+i,sizeof(BYTE));

}
return true;
}

/*******************************************************************
 Function name			countValueofBytes
 description			count value of integer from  bytes format
 parameter
 const BYTE *		IN	 		buffer
 int				IN			len
 Return value
 value of integer converse from bytes
 *******************************************************************/
u_int64
countValueofBytes(const BYTE * buffer, int len)
{
int i = 0;
u_int64 val = 0;

for (i = 0; i < len; i++)
{
val += ((u_int32)(*buffer)) * accumulate(2, i * 8);
buffer++;
}
return val;
}



u_int32
countLengthofBytes(const BYTE * buffer)
{
int len = 0;

while (buffer!=NULL)
{
	len++;
	buffer++;
}
return len;
}




/*******************************************************************
 Function name			int2bytes
 description		    convert from integer to a serial of bytes
 parameter
 BYTE *				IN/OUT	 		byte
 const int			IN			mode
 u64				IN			u_int64
 Return value
 true or false
 *******************************************************************/
u_int64
int2bytes(BYTE bye[], const int mode, u_int64 num)
{
 //high endian	used
 //if number is not valid, then set all bits as 1
BYTE *reverse_bye = NULL;


if (mode == CHAR2BYTE)
{
bye[0] = (BYTE) num;
}
else if (HIGH_ENDIAN)
{
reverse_bye = malloc(mode);
memset(reverse_bye, 0, mode);

if (mode == SHT2BYTE)
{
getBytesOfValue(reverse_bye, num, SHT2BYTE);
reverseBytes(reverse_bye, bye, SHT2BYTE);
}
if (mode == INT2BYTE)
{
getBytesOfValue(reverse_bye, num, INT2BYTE);
reverseBytes(reverse_bye, bye, INT2BYTE);
}
if (mode == THR2BYTE)
{
getBytesOfValue(reverse_bye, num, THR2BYTE);
reverseBytes(reverse_bye, bye, THR2BYTE);
}
if (mode == LONGLONG2BYTE)
{
getBytesOfValue(reverse_bye, num, LONGLONG2BYTE);
reverseBytes(reverse_bye, bye, LONGLONG2BYTE);
}

		if (reverse_bye != NULL)
		{
			free(reverse_bye);
		}
	}
	 //low endian used
	else if (LOW_ENDIAN)
	{
		if (mode == SHT2BYTE)
		{
			getBytesOfValue(bye, num, SHT2BYTE);
		}
		if (mode == INT2BYTE)
		{
			getBytesOfValue(bye, num, INT2BYTE);
		}
		if (mode == THR2BYTE)
		{
			getBytesOfValue(bye, num, THR2BYTE);
		}
		if (mode == LONGLONG2BYTE)
		{
			getBytesOfValue(bye, num, LONGLONG2BYTE);
		}
	}
	return true;
}

/*******************************************************************
 Function name			bytes2int
 description		    convert bytes to integer
 parameter
 const BYTE*		IN	 		bye
 int				IN			mode
 Return value
 value of integer
 *******************************************************************/
u_int64
bytes2int(const BYTE bye[], int mode)
{
u_int64 nh = 0, nl = 0;
u_int64 val = 0;
BYTE *reverse_bye = NULL;

reverse_bye = malloc(mode*sizeof(BYTE));
memset(reverse_bye, 0, mode*sizeof(BYTE));

if (mode == CHAR2BYTE)
{
 //high endian
nh = 0;
 //low endian used
nl = (u_int32)bye[0];
}

else if (mode == SHT2BYTE)
{
 //high endian
reverseBytes(bye, reverse_bye, SHT2BYTE);
nh = countValueofBytes(reverse_bye, SHT2BYTE);

 //low endian used
nl = countValueofBytes(bye, SHT2BYTE);
}
else if (mode == THR2BYTE)
{
 //high endian
reverseBytes(bye, reverse_bye, THR2BYTE);
nh = countValueofBytes(reverse_bye, THR2BYTE);

 //low endian used
nl = countValueofBytes(bye, THR2BYTE);
}
else if (mode == INT2BYTE)
{
 //high endian
reverseBytes(bye, reverse_bye, INT2BYTE);
nh = countValueofBytes(reverse_bye, INT2BYTE);

 //low endian
nl = countValueofBytes(bye, INT2BYTE);
}
else if (mode == LONGLONG2BYTE)
{
 //high endian
reverseBytes(bye, reverse_bye, LONGLONG2BYTE);
nh = countValueofBytes(reverse_bye, LONGLONG2BYTE);

 //low endian used
nl = countValueofBytes(bye, LONGLONG2BYTE);
}

	val = nl * LOW_ENDIAN + nh * HIGH_ENDIAN;
	if (reverse_bye != NULL)
	{
		free(reverse_bye);
	}
	return val;
}

/*
void
bprint(BYTE * allbytes, int type)
{
int i = 0;

for (i = 0; i < type; i++)
{
ILOG("<%d[%02x]>", allbytes[i], allbytes[i]);

}
}


 int main(char **argv, int argc)
 {
 BYTE maxbytes[8];
 int value;

 u_int64 val, num64 = 24435259078878;


 int number = 26534;
 int num32 = 345323;

 int2bytes(maxbytes, SHT2BYTE, number);
 printf("Testing int2bytes,type=SHT2BYTE, input=%d\n", number);
 bprint(maxbytes, SHT2BYTE);

 value = bytes2int(maxbytes, SHT2BYTE);
 printf("Testing byetes2int ,type=SHT2BYTE, input=%d,value=%d\n", number,
 value);

 int2bytes(maxbytes, INT2BYTE, num32);
 printf("Testing int2bytes,type=INT2BYTE, input=%d\n", num32);
 bprint(maxbytes, INT2BYTE);

 value = bytes2int(maxbytes, INT2BYTE);
 printf("Testing byetes2int ,type=INT2BYTE, input=%d,value=%d\n", num32,
 value);

 int2bytes(maxbytes, LONGLONG2BYTE, num64);
 printf("Testing int2bytes,type=LONGLONG2BYTE, input=%lld\n", num64);
 bprint(maxbytes, LONGLONG2BYTE);

 val = bytes2int(maxbytes, LONGLONG2BYTE);
 printf("Testing byetes2int ,type=LONGLONG2BYTE,input=%lld,value=%lld\n",
 num64, val);

 return EXIT_SUCCESS;
 }

 */
