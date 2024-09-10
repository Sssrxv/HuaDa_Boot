#include "ArrayList.h"

/**
*		@date: 			2024-02-28	by	CYL
*		@name: 			InitU8ArrayList
*		@brief:			Init an <unsigned char> member array.
*		@param[in]:	inputU8Array	[The Array you want to Init.]
*		@param[in]:	arrayLens			[The Lens of inputU8Array.]
*		@param[in]:	initValue			[The init Value which you want to store into inputU8Array.]
*		@ret:				res @ref ArrayListFuncRunState
*/
ArrayListFuncRunState InitU8ArrayList(u8Array inputU8Array[],unsigned int arrayLens,int initValue)
{
		unsigned int i=0;
		ArrayListFuncRunState res=Mistake;
		memset(inputU8Array,initValue,arrayLens);
		for(i=0;i<arrayLens;i++)
		{
			if(inputU8Array[i]!=initValue)
			{
				res = Mistake;
			}else{
				res = noMistake;
			}
		}
		/* For USART LOG */
		#if (NEED_MCU_USART_LOG==1)
			USART_SendMessageAdvanced("InitU8ArrayList RunState:");
		
			if(res==Mistake){
				USART_SendMessageAdvanced("Mistake.\n");
			}else{
				USART_SendMessageAdvanced("noMistake.\n");
			}
			
			USART_SendMessageAdvanced("InitU8ArrayList = ");
			for(i=0;i<arrayLens;i++)
			{
				USART_SendDec(inputU8Array[i],1);
				USART_SendMessageAdvanced(" ");
			}
			USART_SendMessageAdvanced("\n");
		#endif
		
		return res;
}


/**
*		@date:2024-02-29	by	CYL
*		@name:CmpTwoU8ArrayList
*		@brief: 		To compare members is same or not between two array (have the same length).
*		@param[in]:	u8Array_A		[An array.]
*		@param[in]:	u8Array_B		[An array.]
*		@param[in]:	arrayLens		[Length of array.]
*/
ArrayListFuncRunState CmpTwoU8ArrayList(u8Array u8Array_A[],u8Array u8Array_B[],unsigned int arrayLens,char *LogMessage)
{
		unsigned int i=0;
		ArrayListFuncRunState res = Mistake;
	
		for(i=0;i<arrayLens;i++)
		{
			if(u8Array_A[i]!=u8Array_B[i])
			{
				res = Mistake;
				break;
			}else{
				res = noMistake;
			}
		}
		/* For USART LOG */
		#if (NEED_MCU_USART_LOG==1)
			USART_SendMessageAdvanced(LogMessage);
			if(res==Mistake){
				USART_SendMessageAdvanced("Mistake.\n");
				USART_SendMessageAdvanced("i = ");
				USART_SendDec(i,1);
				USART_SendMessageAdvanced("\n");
			}else{
				USART_SendMessageAdvanced("noMistake.\n");
			}
		#endif
		return res;
}
