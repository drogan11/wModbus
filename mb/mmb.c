/*
********************************************************************************************************
*                                                wModbus
*                                 The Embedded Modbus Stack Software
*
*               Copyright: Wu YuanFu. All rights reserved.Protected by international copyright laws.
*
* Programmer  : Wu YuanFu
* Version     : V2.50
* LICENSING TERMS:
* ---------------
*           wModbus is provided in source form for FREE short-term evaluation, for educational use or
*           for peaceful research.  If you plan or intend to use wModbus in a commercial application/
*           product then, you need to contact Wu YuanFu<240697738@qq.com> to properly license wModbus
*           for its use in your application/product.  The fact that the source is provided does NOT 
*           mean that you can use it commercially without paying a licensing fee.
*
*           Knowledge of the source code may NOT be used to develop a similar product.
*               
********************************************************************************************************
*/

/* Filename    : mmb.c*/

#include "mb.h"
#include <stdlib.h>

#define mMB_PDU_FN01_SndBuf_SIZE                ( 5 )
#define mMB_PDU_FN02_SndBuf_SIZE                ( 5 )
#define mMB_PDU_FN03_SndBuf_SIZE                ( 5 )
#define mMB_PDU_FN04_SndBuf_SIZE                ( 5 )
#define mMB_PDU_FN05_SndBuf_SIZE                ( 5 )
#define mMB_PDU_FN06_SndBuf_SIZE                ( 5 )
#define mMB_PDU_FN15_SndBuf_SIZE                ( 6 )
#define mMB_PDU_FN16_SndBuf_SIZE                ( 6 )
#define mMB_PDU_FN23_SndBuf_SIZE                ( 10)

#define mMB_PDU_FN01_RcvLen_Min                 ( 3 )
#define mMB_PDU_FN02_RcvLen_Min                 ( 3 )
#define mMB_PDU_FN03_RcvLen_Min                 ( 4 )
#define mMB_PDU_FN04_RcvLen_Min                 ( 4 )
#define mMB_PDU_FN05_RcvLen_Min                 ( 5 )
#define mMB_PDU_FN06_RcvLen_Min                 ( 5 )
#define mMB_PDU_FN15_RcvLen_Min                 ( 5 )
#define mMB_PDU_FN16_RcvLen_Min                 ( 5 )
#define mMB_PDU_FN23_RcvLen_Min                 ( 4 )

#if (MB_MASTER_ENABLED  == MBENABLED)

/*
********************************************************************************************************
*                                                mMB RegInputCB
*
* Description: �˺����������� RegInput �����ݴ�����
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : 
* Note(s)    : 1) ����FN04 ����
********************************************************************************************************
*/
static void mMB_RegInputCB( wMB *p_mb ,UCHAR* pBuf)
{
    UCHAR   *pRegFrame;
    
    if(p_mb->Mode == MB_MODE_TCP)
    {
         pRegFrame= &pBuf[MB_PDU_TCP_OFF + p_mb->Value_off];
    }else
    {pRegFrame = &pBuf[MB_PDU_RTU_OFF + p_mb->Value_off];}      
    
    USHORT  Index = 0;
    SHORT RegCnt = ( SHORT )p_mb->RegCnt;
    while( RegCnt-- > 0)
    {
        p_mb->RegDataPtr[Index] = *pRegFrame++ << 8;
        p_mb->RegDataPtr[Index++] |= *pRegFrame++;
    }
}

/*
********************************************************************************************************
*                                                mMB RegHolding
*
* Description: �˺����������� RegHolding �����ݴ�����
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : 
* Note(s)    : 1) ����FN03 FN06 FN16 FN23 ����
********************************************************************************************************
*/
static void mMB_RegHoldingCB(wMB *p_mb ,UCHAR* pBuf)
{   
    UCHAR   *pRegFrame;
    
    if(p_mb->Mode == MB_MODE_TCP)
    {
         pRegFrame = &pBuf[MB_PDU_TCP_OFF + p_mb->Value_off];
    }else
    {pRegFrame = &pBuf[MB_PDU_RTU_OFF + p_mb->Value_off];}  
    
    USHORT  Index = 0;
    SHORT RegCnt = ( SHORT )p_mb->RegCnt;
    switch (p_mb->ReadWrite)
    {
    case MBREAD:
        while( RegCnt-- > 0)
		{
			*pRegFrame++ = p_mb->RegDataPtr[Index] >> 8;
			*pRegFrame++ = p_mb->RegDataPtr[Index++] ;
		}break;        
    case MBWRITE:
        while( RegCnt-- > 0)
		{
            p_mb->RegDataPtr[Index] = *pRegFrame++ << 8;
            p_mb->RegDataPtr[Index++] |= *pRegFrame++;
		}break;         
    }
}

/*
********************************************************************************************************
*                                                mMB RegCoilsCB
*
* Description: �˺����������� RegCoilsCB �����ݴ�����
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : 
* Note(s)    : 1) ����FN01 FN05 FN15����
********************************************************************************************************
*/
static void mMB_RegCoilsCB(wMB *p_mb ,UCHAR* pBuf)
{
    USHORT   *pRegFrame;
    
    if(p_mb->Mode == MB_MODE_TCP)
    {
         pRegFrame = (USHORT *)&pBuf[MB_PDU_TCP_OFF + p_mb->Value_off];
    }else
    {pRegFrame = (USHORT *)&pBuf[MB_PDU_RTU_OFF + p_mb->Value_off];}  
    
    SHORT RegCnt = ( SHORT )p_mb->RegCnt;      
    switch (p_mb->ReadWrite)
    {
    case MBREAD:
        while (RegCnt > 0)
        {
            *pRegFrame++ = MB_UtilGetBits( p_mb->RegDataPtr, p_mb->BitOffset,
				( UCHAR )( RegCnt > MB_BITS ? MB_BITS : RegCnt ) );
            RegCnt -= MB_BITS;
            p_mb->BitOffset += MB_BITS;
        }
        break;
    case MBWRITE:
        while (RegCnt > 0)
        {
            MB_UtilSetBits(p_mb->RegDataPtr, p_mb->BitOffset,
				( UCHAR )( RegCnt > MB_BITS ? MB_BITS : RegCnt ),*pRegFrame++ );
            RegCnt -= MB_BITS;
            p_mb->BitOffset += MB_BITS;            
        }
        break;
    } 
}

/*
********************************************************************************************************
*                                                mMB RegDiscreteCB
*
* Description: �˺����������� RegDiscreteCB �����ݴ�����
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : 
* Note(s)    : 1) ����FN02 ����
********************************************************************************************************
*/
static void mMB_RegDiscreteCB( wMB *p_mb ,UCHAR* pBuf)
{
    USHORT   *pRegFrame;
    
    if(p_mb->Mode == MB_MODE_TCP)
    {
         pRegFrame = (USHORT *)&pBuf[MB_PDU_TCP_OFF + p_mb->Value_off];
    }else
    {pRegFrame = (USHORT *)&pBuf[MB_PDU_RTU_OFF + p_mb->Value_off];}  
    
    SHORT RegCnt = ( SHORT )p_mb->RegCnt;    
    while (RegCnt > 0)
    {
        MB_UtilSetBits(p_mb->RegDataPtr, p_mb->BitOffset,
			( UCHAR )( RegCnt > MB_BITS ? MB_BITS : RegCnt ),*pRegFrame++ );
        RegCnt -= MB_BITS;
        p_mb->BitOffset += MB_BITS;            
    }   
}

#define mMB_FN_FramePtr_Set(Func,Addr,Cnt)						\
{																\
	p_mb->NodeAddr = SlaveAddr;								    \
	p_mb->RegCnt = Cnt;										    \
	if(p_mb->Mode == MB_MODE_TCP)								\
	{															\
		FramePtr = &pBuf[MB_PDU_TCP_OFF];						\
		FramePtr[0] = p_mb->ComCnt >> 8;						\
		FramePtr[1] = p_mb->ComCnt++;							\
		FramePtr[2] = 0;										\
		FramePtr[3] = 0;										\
	}else														\
	{FramePtr = &pBuf[MB_PDU_RTU_OFF];}							\
	/*FramePtr = &pBuf[3 * p_mb->Mode * (p_mb->Mode - 3) + 7];*/\
	FramePtr[MB_PDU_FUNC_OFF]        = Func;					\
	FramePtr[MB_PDU_REQ_ADDR_OFF]    = Addr >> 8;				\
	FramePtr[MB_PDU_REQ_ADDR_OFF + 1]= Addr;					\
	FramePtr[MB_PDU_REQ_CNT_OFF ]    = Cnt >> 8;				\
	FramePtr[MB_PDU_REQ_CNT_OFF + 1] = Cnt;						\
}

/*
********************************************************************************************************
*                                                mMB FN01 Call
*
* Description: �˺����������� FN01 �û����ú���
* Arguments  : p_mb           ָ��1��wModbus��ָ��
*              SlaveAddr      ��ȡ�Ĵӻ���ַ
*              RegAddr    	  ��ȡ��bit ��ַ
*              RegCnt       ��ȡ��bit ����
*              pRegBuffer     ָ��1����ȡbit���ݵ�ָ��
*              ReadBitOffset  ��ȡ��bit�����ݵ�ƫ����
*              lTimeOut       ��ȡ������ʱ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
#if MB_FN01_READ_COILS_ENABLED == MBENABLED
MB_Exception mMB_FN01_Read_CoilsRegister( wMB *p_mb,
										 UCHAR   SlaveAddr, 
                                         USHORT  RegAddr, 
                                         USHORT  RegCnt,  
                                         USHORT  *pRegBuffer,
                                         USHORT  ReadBitOffset,
                                         LONG    lTimeOut )
{
    UCHAR          *FramePtr;
    MB_Exception    eStatus = MB_EX_NONE;   
    if( SlaveAddr == MB_ADDRESS_BROADCAST ) eStatus = MB_EX_ILLEGAL_DATA_ADDRESS;
	else if((RegCnt < MB_PDU_FNxx_COUNT_MIN) || (RegCnt > MB_PDU_FN01_READCOUNT_MAX))
		eStatus = MB_EX_ILLEGAL_DATA_VALUE;	
    else
    {
        if ( MB_MutexPend( p_mb ) == MBFALSE ) {eStatus = MB_EX_SLAVE_BUSY;}
        else
        {
            UCHAR *pBuf = (UCHAR *)malloc( MB_PDU_SIZE_MAX );
            if(pBuf != MBNULL)
            {
				p_mb->BitOffset= ReadBitOffset;
				p_mb->RegDataPtr = pRegBuffer;
				mMB_FN_FramePtr_Set(MB_FN01_READ_COILS,RegAddr,RegCnt);
				p_mb->BufCnt = mMB_PDU_FN01_SndBuf_SIZE;
                MB_Send( p_mb,pBuf); 
                eStatus = MB_Poll(p_mb,pBuf,lTimeOut);
                free(pBuf);
                pBuf = MBNULL;
            }
            else
            {
                eStatus = MB_EX_NO_MEMORY;
            }
            MB_MutexPost(p_mb); 
        }    
    }
    return eStatus;
}

/*
********************************************************************************************************
*                                                mMB FN01 Handler
*
* Description: �˺����������� FN01 ������Ĵ�����
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
static MB_Exception mMB_FN01_Read_CoilsHandler( wMB *p_mb ,UCHAR* pBuf)
{
    UCHAR          *FramePtr;
    UCHAR           ucNBytes;
    MB_Exception    eStatus = MB_EX_NONE;
    if ( p_mb->BufCnt >= mMB_PDU_FN01_RcvLen_Min )
    { 
        if( ( p_mb->RegCnt & 0x0007 ) != 0 )
        {
        	ucNBytes = ( UCHAR )( p_mb->RegCnt / 8 + 1 );
        }
        else
        {
        	ucNBytes = ( UCHAR )( p_mb->RegCnt / 8 );
        }
        if(p_mb->Mode == MB_MODE_TCP)
        {
            FramePtr = &pBuf[MB_PDU_TCP_OFF];
        }else
        {FramePtr = &pBuf[MB_PDU_RTU_OFF];}
        
        if( ucNBytes == FramePtr[MB_PDU_FN01_BYTECNT_OFF] )
        {            
            p_mb->Value_off = MB_PDU_FN01_VALUE_OFF ;
            p_mb->ReadWrite = MBWRITE;
            mMB_RegCoilsCB( p_mb ,pBuf);
        }
        else{eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    }
    else{eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    return eStatus;
}
#endif

/*
********************************************************************************************************
*                                                mMB FN02 Call
*
* Description: �˺����������� FN02 �û����ú���
* Arguments  : p_mb           ָ��1��wModbus��ָ��
*              SlaveAddr      ��ȡ�Ĵӻ���ַ
*              RegAddr        ��ȡ��bit ��ַ
*              RegCnt       ��ȡ��bit ����
*              pRegBuffer 	  ָ��1����ȡbit���ݵ�ָ��
*              ReadBitOffset  ��ȡ��bit�����ݵ�ƫ����
*              lTimeOut       ��ȡ������ʱ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
#if MB_FN02_READ_DISCRETE_ENABLED == MBENABLED
MB_Exception mMB_FN02_Read_DiscreteRegister( wMB     *p_mb,
											UCHAR   SlaveAddr,
											USHORT  RegAddr, 
											USHORT  RegCnt,  
											USHORT  *pRegBuffer,
                                            USHORT  ReadBitOffset,
                                            LONG    lTimeOut )
{
    UCHAR          *FramePtr;    
    MB_Exception    eStatus = MB_EX_NONE;
    if( SlaveAddr == MB_ADDRESS_BROADCAST ) eStatus = MB_EX_ILLEGAL_DATA_ADDRESS;
	else if((RegCnt < MB_PDU_FNxx_COUNT_MIN) || (RegCnt > MB_PDU_FN02_READCOUNT_MAX))
		eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    else 
    {
        if ( MB_MutexPend( p_mb ) == MBFALSE ) eStatus = MB_EX_SLAVE_BUSY;
        else
        {
            UCHAR *pBuf = (UCHAR *)malloc( MB_PDU_SIZE_MAX );
            if(pBuf != MBNULL)
            {
				p_mb->BitOffset= ReadBitOffset;
				p_mb->RegDataPtr = pRegBuffer;
				mMB_FN_FramePtr_Set(MB_FN02_READ_DISCRETE_INPUTS,RegAddr,RegCnt);
				p_mb->BufCnt = mMB_PDU_FN02_SndBuf_SIZE;
                MB_Send( p_mb,pBuf);        
                eStatus = MB_Poll(p_mb,pBuf,lTimeOut);
                free(pBuf);
                pBuf = MBNULL;
            }
            else
            {
                eStatus = MB_EX_NO_MEMORY;
            }
            MB_MutexPost(p_mb);
        }
    }
    return eStatus;
}

/*
********************************************************************************************************
*                                                mMB FN02 Handler
*
* Description: �˺����������� FN02 ������Ĵ�����
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
static MB_Exception mMB_FN02_Read_DiscreteHandler( wMB *p_mb ,UCHAR* pBuf)
{
    UCHAR          *FramePtr; 
    UCHAR           ucNBytes;
    MB_Exception    eStatus = MB_EX_NONE;
    if( p_mb->BufCnt >= mMB_PDU_FN02_RcvLen_Min )
    {         
        if( ( p_mb->RegCnt & 0x0007 ) != 0 )
        {
        	ucNBytes = ( UCHAR )( p_mb->RegCnt / 8 + 1 );
        }
        else
        {
        	ucNBytes = ( UCHAR )( p_mb->RegCnt / 8 );
        }
        if(p_mb->Mode == MB_MODE_TCP)
        {
            FramePtr = &pBuf[MB_PDU_TCP_OFF];
        }else
        {FramePtr = &pBuf[MB_PDU_RTU_OFF];}
        
		if (ucNBytes == FramePtr[MB_PDU_FN02_BYTECNT_OFF])
        {
            p_mb->Value_off = MB_PDU_FN02_VALUE_OFF ;
			mMB_RegDiscreteCB( p_mb,pBuf);
        }
        else{eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    }
    else{eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    return eStatus;
}
#endif

/*
********************************************************************************************************
*                                                mMB FN03 Call
*
* Description: �˺����������� FN03 �û����ú���
* Arguments  : p_mb           ָ��1��wModbus��ָ��
*              SlaveAddr      ��ȡ�Ĵӻ���ַ
*              RegAddr        ��ȡ��Reg ��ַ
*              RegCnt       ��ȡ��Reg ����
*              pRegBuffer     ָ��1����ȡReg���ݵ�ָ��
*              lTimeOut       ��ȡ������ʱ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
#if MB_FN03_READ_HOLDING_ENABLED == MBENABLED
MB_Exception mMB_FN03_Read_HoldingRegister( wMB     *p_mb,
                                           UCHAR   SlaveAddr, 
                                           USHORT  RegAddr, 
                                           USHORT  RegCnt,  
                                           USHORT  *pRegBuffer,
                                           LONG    lTimeOut )
{
    UCHAR          *FramePtr; 
    MB_Exception    eStatus = MB_EX_NONE;
      
    if( SlaveAddr == MB_ADDRESS_BROADCAST ) eStatus = MB_EX_ILLEGAL_DATA_ADDRESS;
	else if((RegCnt < MB_PDU_FNxx_COUNT_MIN) || (RegCnt > MB_PDU_FN03_READCOUNT_MAX))
		eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    else
    {   if ( MB_MutexPend( p_mb ) == MBFALSE ) eStatus = MB_EX_SLAVE_BUSY;
        else
        {
            UCHAR *pBuf = (UCHAR *)malloc( MB_PDU_SIZE_MAX );
            if(pBuf != MBNULL)
            {
				p_mb->RegDataPtr = pRegBuffer;
				mMB_FN_FramePtr_Set(MB_FN03_READ_HOLDING_REGISTER,RegAddr,RegCnt);
				p_mb->BufCnt = mMB_PDU_FN03_SndBuf_SIZE;
                MB_Send( p_mb,pBuf);         
                eStatus = MB_Poll(p_mb,pBuf,lTimeOut);
                free(pBuf);
                pBuf = MBNULL;
            }
            else
            {
                eStatus = MB_EX_NO_MEMORY;
            }
            MB_MutexPost(p_mb);
        }
    }
    return eStatus;
}

/*
********************************************************************************************************
*                                                mMB FN03 Handler
*
* Description: �˺����������� FN03 ������Ĵ�����
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
static MB_Exception mMB_FN03_Read_HoldingHandler( wMB *p_mb ,UCHAR* pBuf)
{
    UCHAR          *FramePtr; 
    MB_Exception    eStatus = MB_EX_NONE;
    if( p_mb->BufCnt >= mMB_PDU_FN03_RcvLen_Min )
    {   
        if(p_mb->Mode == MB_MODE_TCP)
        {
            FramePtr = &pBuf[MB_PDU_TCP_OFF];
        }else
        {FramePtr = &pBuf[MB_PDU_RTU_OFF];}
        
        if( 2 * p_mb->RegCnt == FramePtr[MB_PDU_FN03_BYTECNT_OFF] )
        {
            p_mb->Value_off = MB_PDU_FN03_VALUE_OFF ;
            p_mb->ReadWrite = MBWRITE;
            mMB_RegHoldingCB( p_mb ,pBuf);
        }
        else{eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    }
    else{eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    return eStatus;
}
#endif

/*
********************************************************************************************************
*                                                mMB FN04 Call
*
* Description: �˺����������� FN04 �û����ú���
* Arguments  : p_mb           ָ��1��wModbus��ָ��
*              SlaveAddr      ��ȡ�Ĵӻ���ַ
*              RegAddr        ��ȡ��Reg ��ַ
*              RegCnt       ��ȡ��Reg ����
*              pRegBuffer     ָ��1����ȡReg���ݵ�ָ��
*              lTimeOut       ��ȡ������ʱ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
#if MB_FN04_READ_INPUT_ENABLED == MBENABLED
MB_Exception mMB_FN04_Read_InputRegister( wMB     *p_mb,
										 UCHAR   SlaveAddr, 
                                         USHORT  RegAddr, 
                                         USHORT  RegCnt,  
                                         USHORT  *pRegBuffer,
                                         LONG    lTimeOut )
{
    UCHAR          *FramePtr; 
    MB_Exception    eStatus = MB_EX_NONE;  
    if( SlaveAddr == MB_ADDRESS_BROADCAST ) eStatus = MB_EX_ILLEGAL_DATA_ADDRESS;
	else if((RegCnt < MB_PDU_FNxx_COUNT_MIN) || (RegCnt > MB_PDU_FN04_READCOUNT_MAX))
		eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    else 
    {
        if ( MB_MutexPend( p_mb ) == MBFALSE ) eStatus = MB_EX_SLAVE_BUSY;
        else
        { 
            UCHAR *pBuf = (UCHAR *)malloc( MB_PDU_SIZE_MAX );
            if(pBuf != MBNULL)
            {
				p_mb->RegDataPtr = pRegBuffer;
				mMB_FN_FramePtr_Set(MB_FN04_READ_INPUT_REGISTER,RegAddr,RegCnt);
				p_mb->BufCnt = mMB_PDU_FN04_SndBuf_SIZE;
                MB_Send( p_mb,pBuf);        
                eStatus = MB_Poll(p_mb,pBuf,lTimeOut);
                free(pBuf);
                pBuf = MBNULL;
            }
            else
            {
                eStatus = MB_EX_NO_MEMORY;
            }
            MB_MutexPost(p_mb);            
        }
    }
    return eStatus;
}

/*
********************************************************************************************************
*                                                mMB FN04 Handler
*
* Description: �˺����������� FN04 ������Ĵ�����
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
static MB_Exception mMB_FN04_Read_InputHandler( wMB *p_mb ,UCHAR* pBuf)
{
    UCHAR          *FramePtr; 
    MB_Exception    eStatus = MB_EX_NONE;
    if( p_mb->BufCnt >= mMB_PDU_FN04_RcvLen_Min )
    {     
        if(p_mb->Mode == MB_MODE_TCP)
        {
            FramePtr = &pBuf[MB_PDU_TCP_OFF];
        }else
        {FramePtr = &pBuf[MB_PDU_RTU_OFF];}
        if( 2 * p_mb->RegCnt == FramePtr[MB_PDU_FN04_BYTECNT_OFF] )
        {
            p_mb->Value_off = MB_PDU_FN04_VALUE_OFF ;
            mMB_RegInputCB( p_mb ,pBuf);
        }
        else{eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    }
    else{eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    return eStatus;
}
#endif

/*
********************************************************************************************************
*                                                mMB FN05 Call
*
* Description: �˺����������� FN05 �û����ú���
* Arguments  : p_mb           ָ��1��wModbus��ָ��
*              SlaveAddr      д��Ĵӻ���ַ
*              RegAddr        д���bit ��ַ
*              WriteData      д���bit ֵ
*              lTimeOut       ��ȡ������ʱ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
#if MB_FN05_WRITE_COIL_ENABLED == MBENABLED
MB_Exception mMB_FN05_Write_CoilsRegister( wMB    *p_mb,
										  UCHAR  SlaveAddr, 
                                          USHORT RegAddr, 
                                          USHORT WriteData, 
                                          LONG   lTimeOut )
{
    UCHAR          *FramePtr; 
    MB_Exception    eStatus = MB_EX_NONE;
    if(WriteData == 1) WriteData = 0xFF00 ;
    if ( ( WriteData != 0xFF00 ) && ( WriteData != 0 ) ) 
		eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    else 
    {
        if ( MB_MutexPend( p_mb ) == MBFALSE ) eStatus = MB_EX_SLAVE_BUSY;
        else
        {
            UCHAR *pBuf = (UCHAR *)malloc( MB_PDU_SIZE_MAX );
            if(pBuf != MBNULL)
            {
				mMB_FN_FramePtr_Set(MB_FN05_WRITE_SINGLE_COIL,RegAddr,WriteData);
				p_mb->BufCnt = mMB_PDU_FN05_SndBuf_SIZE;
                MB_Send( p_mb,pBuf); 
                if(SlaveAddr != MB_ADDRESS_BROADCAST)
                {
                    eStatus = MB_Poll(p_mb,pBuf,lTimeOut);
                }
                free(pBuf);
                pBuf = MBNULL;
            }
            else
            {
                eStatus = MB_EX_NO_MEMORY;
            }
            MB_MutexPost(p_mb); 
        }
    }
    return eStatus;
}

/*
********************************************************************************************************
*                                                mMB FN05 Handler
*
* Description: �˺����������� FN05 ������Ĵ�����
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
static MB_Exception mMB_FN05_Write_CoilsHandler( wMB *p_mb ,UCHAR* pBuf)
{
    UCHAR          *FramePtr; 
    MB_Exception    eStatus = MB_EX_NONE;
    if( p_mb->BufCnt == mMB_PDU_FN05_RcvLen_Min )
    {
        if(p_mb->Mode == MB_MODE_TCP)
        {
            FramePtr = &pBuf[MB_PDU_TCP_OFF];
        }else
        {FramePtr = &pBuf[MB_PDU_RTU_OFF];}
        if(( FramePtr[MB_PDU_FN05_VALUE_OFF + 1] == 0x00 ) 
			&& (( FramePtr[MB_PDU_FN05_VALUE_OFF] == 0xFF ) 
			|| ( FramePtr[MB_PDU_FN05_VALUE_OFF] == 0x00 ))){}
        else{eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    }
    else{eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    return eStatus;
}
#endif

/*
********************************************************************************************************
*                                                mMB FN06 Call
*
* Description: �˺����������� FN06 �û����ú���
* Arguments  : p_mb           ָ��1��wModbus��ָ��
*              SlaveAddr      д��Ĵӻ���ַ
*              RegAddr        д���Reg ��ַ
*              WriteData      д���Reg ����
*              lTimeOut       д�������ʱ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
#if MB_FN06_WRITE_HOLDING_ENABLED == MBENABLED
MB_Exception mMB_FN06_Write_HoldingRegister( wMB    *p_mb,
											UCHAR  SlaveAddr, 
                                            USHORT RegAddr, 
                                            USHORT WriteData, 
                                            LONG   lTimeOut )
{
    UCHAR          *FramePtr; 
    MB_Exception    eStatus = MB_EX_NONE;
    if ( MB_MutexPend( p_mb ) == MBFALSE ) eStatus = MB_EX_SLAVE_BUSY;
    else
    {
        UCHAR *pBuf = (UCHAR *)malloc( MB_PDU_SIZE_MAX );
        if(pBuf != MBNULL)
        {
			mMB_FN_FramePtr_Set(MB_FN06_WRITE_REGISTER,RegAddr,WriteData);
			p_mb->BufCnt = mMB_PDU_FN06_SndBuf_SIZE;
            MB_Send( p_mb,pBuf);        
            if(SlaveAddr != MB_ADDRESS_BROADCAST)
            {
                eStatus = MB_Poll(p_mb,pBuf,lTimeOut);
            }
            free(pBuf);
            pBuf = MBNULL;
        }
        else
        {
            eStatus = MB_EX_NO_MEMORY;
        }
        MB_MutexPost(p_mb); 
    }
    return eStatus;
}

/*
********************************************************************************************************
*                                                mMB FN06 Handler
*
* Description: �˺����������� FN06 ������Ĵ�����
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
static MB_Exception mMB_FN06_Write_HoldingHandler( wMB *p_mb ,UCHAR* pBuf)
{
    MB_Exception    eStatus = MB_EX_NONE;
    if(p_mb->BufCnt == mMB_PDU_FN06_RcvLen_Min){}
    else{eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    return eStatus;
}
#endif

/*
********************************************************************************************************
*                                                mMB FN15 Call
*
* Description: �˺�����������  FN15 �û����ú���
* Arguments  : p_mb            ָ��1��wModbus��ָ��
*              SlaveAddr       д��Ĵӻ���ַ
*              RegAddr         д���bit ��ַ
*              RegCnt        д���bit ����
*              pBuffer         ָ��1��д��it���ݵ�ָ��
*              WriteBitOffset  д���bit�����ݵ�ƫ����
*              lTimeOut        д�������ʱ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
#if MB_FN15_WRITE_COILS_ENABLED == MBENABLED
MB_Exception mMB_FN15_Write_CoilsRegister( wMB    *p_mb,
										  UCHAR  SlaveAddr,
                                          USHORT RegAddr, 
                                          USHORT RegCnt, 
                                          USHORT *pRegBuffer,
                                          USHORT WriteBitOffset,
                                          LONG   lTimeOut)
{
    UCHAR          *FramePtr; 
    UCHAR          ucByteCount;
    MB_Exception    eStatus = MB_EX_NONE;
	if((RegCnt < MB_PDU_FNxx_COUNT_MIN) || (RegCnt > MB_PDU_FN15_WRITECOUNT_MAX))
		eStatus = MB_EX_ILLEGAL_DATA_VALUE;		
    else 
    {
        if ( MB_MutexPend( p_mb ) == MBFALSE ) eStatus = MB_EX_SLAVE_BUSY;
        else
        {
            UCHAR *pBuf = (UCHAR *)malloc( MB_PDU_SIZE_MAX );
            if(pBuf != MBNULL)
            {
				mMB_FN_FramePtr_Set(MB_FN15_WRITE_MULTIPLE_COILS,RegAddr,RegCnt);
                if( ( RegCnt & 0x0007 ) != 0 )
                {
                    ucByteCount = ( UCHAR )( RegCnt / 8 + 1 );
                }
                else
                {
                    ucByteCount = ( UCHAR )( RegCnt / 8 );
                }
                FramePtr[MB_PDU_FN15_BYTECNT_OFF]= ucByteCount;    
                p_mb->RegDataPtr = pRegBuffer;
                p_mb->RegCnt = RegCnt;
                p_mb->BitOffset= WriteBitOffset;
                p_mb->Value_off = MB_PDU_FN15_VALUE_OFF ;
                p_mb->ReadWrite = MBREAD;
                mMB_RegCoilsCB( p_mb ,pBuf);
				p_mb->BufCnt = mMB_PDU_FN15_SndBuf_SIZE + ucByteCount;
                MB_Send( p_mb,pBuf);        
                if(SlaveAddr != MB_ADDRESS_BROADCAST)
                {
                    eStatus = MB_Poll(p_mb,pBuf,lTimeOut);
                }
                free(pBuf);
                pBuf = MBNULL;
            }
            else
            {
                eStatus = MB_EX_NO_MEMORY;
            }
            MB_MutexPost(p_mb); 
        }
    }
    return eStatus;
}

/*
********************************************************************************************************
*                                                mMB FN15 Handler
*
* Description: �˺����������� FN15 ������Ĵ�����
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
static MB_Exception mMB_FN15_Write_CoilsHandler( wMB *p_mb ,UCHAR* pBuf)
{
    MB_Exception    eStatus = MB_EX_NONE;
    if( p_mb->BufCnt == mMB_PDU_FN15_RcvLen_Min ){}
    else{eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    return eStatus;
}
#endif

/*
********************************************************************************************************
*                                                mMB FN16 Call
*
* Description: �˺����������� FN16 �û����ú���
* Arguments  : p_mb           ָ��1��wModbus��ָ��
*              SlaveAddr      д��Ĵӻ���ַ
*              RegAddr        д���Reg ��ַ
*              RegCnt       д���Reg ����
*              pBuffer        ָ��1��д��Reg���ݵ�ָ��
*              lTimeOut       д�������ʱ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
#if MB_FN16_WRITE_HOLDING_ENABLED == MBENABLED
MB_Exception mMB_FN16_Write_HoldingRegister( wMB    *p_mb,
                                            UCHAR  SlaveAddr,
                                            USHORT RegAddr, 
                                            USHORT RegCnt, 
                                            USHORT *pRegBuffer,
                                            LONG   lTimeOut )
{
    UCHAR          *FramePtr; 
    MB_Exception   eStatus = MB_EX_NONE;
    if((RegCnt < MB_PDU_FNxx_COUNT_MIN) || (RegCnt > MB_PDU_FN16_WRITECOUNT_MAX))
		eStatus = MB_EX_ILLEGAL_DATA_VALUE;	
    else 
    {
        if ( MB_MutexPend( p_mb ) == MBFALSE ) eStatus = MB_EX_SLAVE_BUSY;
        else
        {
            UCHAR *pBuf = (UCHAR *)malloc( MB_PDU_SIZE_MAX );
            if(pBuf != MBNULL)
            {
				mMB_FN_FramePtr_Set(MB_FN16_WRITE_MULTIPLE_REGISTERS,RegAddr,RegCnt);
                FramePtr[MB_PDU_FN16_BYTECNT_OFF]= RegCnt * 2;
                p_mb->RegCnt = RegCnt;        
                p_mb->RegDataPtr = pRegBuffer;
                p_mb->Value_off = MB_PDU_FN16_VALUE_OFF ;
                p_mb->ReadWrite = MBREAD;        
                mMB_RegHoldingCB( p_mb ,pBuf);  
				p_mb->BufCnt = mMB_PDU_FN16_SndBuf_SIZE + 2*RegCnt;
                MB_Send( p_mb,pBuf);         
                if(SlaveAddr != MB_ADDRESS_BROADCAST)
                {
                    eStatus = MB_Poll(p_mb,pBuf,lTimeOut);
                }
                
                free(pBuf);
                pBuf = MBNULL;
            }
            else
            {
                eStatus = MB_EX_NO_MEMORY;
            }
            MB_MutexPost(p_mb); 
        }
    }
    return eStatus;
}

/*
********************************************************************************************************
*                                                mMB FN16 Handler
*
* Description: �˺����������� FN16 ������Ĵ�����
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
static MB_Exception mMB_FN16_Write_HoldingHandler( wMB *p_mb ,UCHAR* pBuf)
{
    MB_Exception    eStatus = MB_EX_NONE;

    if(p_mb->BufCnt == mMB_PDU_FN16_RcvLen_Min){}
    else{eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    return eStatus;
}
#endif

/*
********************************************************************************************************
*                                                mMB FN23 Call
*
* Description: �˺����������� FN23 �û����ú���
* Arguments  : p_mb           ָ��1��wModbus��ָ��
*              SlaveAddr      ��д�Ĵӻ���ַ
*              ReadRegAddr    ��ȡ��Reg ��ַ
*              ReadRegCnt   ��ȡ��Reg ����
*              pReadRegBuffer ָ��1����ȡReg���ݵ�ָ��
*              WriteRegAddr    д���Reg ��ַ
*              WriteRegCnt   д���Reg ����
*              pWriteRegBuffer ָ��1��д��Reg���ݵ�ָ��
*              lTimeOut       ��ȡ������ʱ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
#if MB_FN23_READWRITE_HOLDING_ENABLED == MBENABLED
MB_Exception mMB_FN23_ReadWrite_HoldingRegister( wMB    *p_mb,
                                                UCHAR  SlaveAddr,
                                                USHORT ReadRegAddr, 
                                                USHORT ReadRegCnt, 
                                                USHORT *pReadRegBuffer,
                                                USHORT WriteRegAddr, 
                                                USHORT WriteRegCnt, 
                                                USHORT *pWriteRegBuffer,
                                                LONG   lTimeOut )
{
    UCHAR          *FramePtr; 
    MB_Exception    eStatus = MB_EX_NONE;
    if( SlaveAddr == MB_ADDRESS_BROADCAST ) eStatus = MB_EX_ILLEGAL_DATA_ADDRESS;    
	else if((ReadRegCnt < MB_PDU_FNxx_COUNT_MIN) || (ReadRegCnt > MB_PDU_FN23_READCOUNT_MAX))
		eStatus = MB_EX_ILLEGAL_DATA_VALUE;
	else if((WriteRegCnt < MB_PDU_FNxx_COUNT_MIN) || (WriteRegCnt > MB_PDU_FN23_WRITECOUNT_MAX))
		eStatus = MB_EX_ILLEGAL_DATA_VALUE;	
    else 
    {
        if ( MB_MutexPend( p_mb ) == MBFALSE ) eStatus = MB_EX_SLAVE_BUSY;
        else
        {
            UCHAR *pBuf = (UCHAR *)malloc( MB_PDU_SIZE_MAX );
            if(pBuf != MBNULL)
            {
				mMB_FN_FramePtr_Set(MB_FN23_READWRITE_MULTIPLE_REGISTERS,ReadRegAddr,ReadRegCnt);
                FramePtr[MB_PDU_REQ_2ndADDR_OFF]      = WriteRegAddr >> 8;
                FramePtr[MB_PDU_REQ_2ndADDR_OFF + 1]  = WriteRegAddr;
                FramePtr[MB_PDU_REQ_2ndCNT_OFF]       = WriteRegCnt >> 8;
                FramePtr[MB_PDU_REQ_2ndCNT_OFF + 1]   = WriteRegCnt ;
                FramePtr[MB_PDU_FN23_WRITEBYTECNT_OFF]= WriteRegCnt * 2;
                p_mb->RegCnt = WriteRegCnt;         
                p_mb->RegDataPtr = pWriteRegBuffer; 
                p_mb->Value_off = MB_PDU_FN23_WRITEVALUE_OFF ;
                p_mb->ReadWrite = MBREAD;
                mMB_RegHoldingCB( p_mb ,pBuf);
                p_mb->RegCnt = ReadRegCnt;         
                p_mb->RegDataPtr = pReadRegBuffer;
				p_mb->BufCnt = mMB_PDU_FN23_SndBuf_SIZE + 2*WriteRegCnt;
                MB_Send( p_mb,pBuf);        
                eStatus = MB_Poll(p_mb,pBuf,lTimeOut);
                free(pBuf);
                pBuf = MBNULL;
            }
            else
            {
                eStatus = MB_EX_NO_MEMORY;
            }
            MB_MutexPost(p_mb);             
        }
    }
    return eStatus;
}

/*
********************************************************************************************************
*                                                mMB FN23 Handler
*
* Description: �˺����������� FN23 ������Ĵ�����
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : 
* Note(s)    : 1) 
********************************************************************************************************
*/
static MB_Exception mMB_FN23_ReadWrite_HoldingHandler( wMB *p_mb ,UCHAR* pBuf)
{
    UCHAR          *FramePtr; 
    MB_Exception    eStatus = MB_EX_NONE;
    if( p_mb->BufCnt >= mMB_PDU_FN23_RcvLen_Min )
    {   
        if(p_mb->Mode == MB_MODE_TCP)
        {
             FramePtr= &pBuf[MB_PDU_TCP_OFF];
        }else
        {FramePtr = &pBuf[MB_PDU_RTU_OFF];}   
        if( ( 2 * p_mb->RegCnt ) == FramePtr[MB_PDU_FN23_READBYTECNT_OFF] )
        {
            p_mb->Value_off = MB_PDU_FN23_READVALUE_OFF ; 
            p_mb->ReadWrite = MBWRITE;
            mMB_RegHoldingCB(p_mb ,pBuf);
        }
        else{ eStatus = MB_EX_ILLEGAL_DATA_VALUE;}
    }
    return eStatus;
}

/*
********************************************************************************************************
*                                                Function Handlers
*
* Description: �˺�������������wModbusЭ��ջ�Ĺ��ܺ�������á�
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Note(s)    : 1) mMB_FuncHandlers() �Զ��� MB_Poll()����
********************************************************************************************************
*/
static MB_Exception mMB_FuncHandlers(wMB   *p_mb,UCHAR* pBuf)
{
    UCHAR          *FramePtr;
    MB_Exception   Exception = MB_EX_NONE; 

    if(p_mb->Mode == MB_MODE_TCP)
    {
         FramePtr= &pBuf[MB_PDU_TCP_OFF];
    }else
    {FramePtr = &pBuf[MB_PDU_RTU_OFF];}   
	
    if(FramePtr[MB_PDU_FUNC_OFF]& MB_FUNC_ERROR) 
    {
        Exception = (MB_Exception)FramePtr[MB_PDU_FUNC_OFF + 1];
    }
    else
    {          
        switch (FramePtr[MB_PDU_FUNC_OFF])
        {   
#if MB_FN01_READ_COILS_ENABLED == MBENABLED
        case MB_FN01_READ_COILS:
            Exception = mMB_FN01_Read_CoilsHandler(p_mb,pBuf);break;
#endif
#if MB_FN02_READ_DISCRETE_ENABLED == MBENABLED     
        case MB_FN02_READ_DISCRETE_INPUTS:
            Exception = mMB_FN02_Read_DiscreteHandler(p_mb,pBuf);break;
#endif   
#if MB_FN03_READ_HOLDING_ENABLED == MBENABLED
        case MB_FN03_READ_HOLDING_REGISTER:
            Exception = mMB_FN03_Read_HoldingHandler(p_mb,pBuf);break;
#endif    
#if MB_FN04_READ_INPUT_ENABLED == MBENABLED
        case MB_FN04_READ_INPUT_REGISTER:
            Exception = mMB_FN04_Read_InputHandler(p_mb,pBuf);break;
#endif 
#if MB_FN05_WRITE_COIL_ENABLED == MBENABLED
        case MB_FN05_WRITE_SINGLE_COIL:
            Exception = mMB_FN05_Write_CoilsHandler(p_mb,pBuf);break;
#endif   
#if MB_FN06_WRITE_HOLDING_ENABLED == MBENABLED
        case MB_FN06_WRITE_REGISTER:
            Exception = mMB_FN06_Write_HoldingHandler(p_mb,pBuf);break;
#endif  
#if MB_FN15_WRITE_COILS_ENABLED == MBENABLED
        case MB_FN15_WRITE_MULTIPLE_COILS:
            Exception = mMB_FN15_Write_CoilsHandler(p_mb,pBuf);break;
#endif  
#if MB_FN16_WRITE_HOLDING_ENABLED == MBENABLED
        case MB_FN16_WRITE_MULTIPLE_REGISTERS:
            Exception = mMB_FN16_Write_HoldingHandler(p_mb,pBuf);break;
#endif
#if MB_FN23_READWRITE_HOLDING_ENABLED == MBENABLED
        case MB_FN23_READWRITE_MULTIPLE_REGISTERS:
            Exception = mMB_FN23_ReadWrite_HoldingHandler(p_mb,pBuf);break;
#endif
        default:
            Exception = MB_EX_ILLEGAL_FUNCTION;break;
        }        
    }
    return Exception;
}

/*
********************************************************************************************************
*                                                Deal A wModbus
*
* Description: �˺�����������wModbusЭ��ջPoll��������ѭ�����á�
* Arguments  : p_mb           ָ��1��wModbus��ָ��
* Returns    : MB_Exception   ���ش�����롣
* Note(s)    : 1) mMB_Deal() Ӧ������һ��wModbusЭ��ջ��ʹ�ܺ�ʹ��
********************************************************************************************************
*/
MB_Exception mMB_Deal( wMB   *p_mb,UCHAR* pBuf,USHORT length )
{
    MB_Exception eStatus;
    
    if((MB_Receive( p_mb,pBuf,length) != MBTRUE) || (p_mb->RecvAddr != p_mb->NodeAddr))
    {
        eStatus = MB_EX_REVDATAERR;
        #if (MB_STAT_ENABLED == MBENABLED)
        p_mb->StatReceiveErrCtr++;
        #endif                  
    }
    else 
    {
		eStatus = mMB_FuncHandlers(p_mb,pBuf);
		#if (MB_STAT_ENABLED == MBENABLED)
        if (eStatus != MB_EX_NONE) 
        {
            p_mb->StatHandlersErrCtr++;                         
        }
        else
        {                 
            p_mb->StatOKCtr++;            
        }
		#endif   		
    }
    return eStatus;
}

#endif

#endif
