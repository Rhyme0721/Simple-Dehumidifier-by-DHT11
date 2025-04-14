/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "dht11.h"
#include "lcd.h"
#include "font_onmy.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
static uint32_t fac_us = 0; //us延时倍乘数
uint32_t Tick=20000;

uint8_t humiH;
uint8_t humiL;
uint8_t tempH;
uint8_t tempL;

int T_up;
int T_low;
int H_up;
int H_low;
int flag_t=0;

uint8_t Threshold_t_set[2];//设置的温度阈值
uint8_t Threshold_t_conf[2];//确定温度阈值
uint8_t Threshold_h_set[2];//设置的温度阈值
uint8_t Threshold_h_conf[2];//确定温度阈值

uint16_t value[2];//接收ADC的转换值

uint8_t Disp_num=0x00;//界面转换，参数选择调节

uint8_t buf_ctl[64];//串口接收字符数组

uint8_t *Lcd_Disp_String[17];//字符16x16

uint16_t cl1=WHITE;
uint16_t cl2=WHITE;
uint16_t cl3=WHITE;
uint16_t cl4=WHITE;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

void delay_init(uint8_t SYSCLK);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);

int getKey_value(uint32_t key);
void Lcd_Proc(void);
void Get_AdcData(void);
void Key_Proc(void);
void Uart_Proc(void);

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	Lcd_Init();
	Lcd_Clear(YELLOW);
	
	delay_init(16);
	
	HAL_ADC_Start_DMA(&hadc1,(uint32_t *)value,2);
	HAL_ADC_Stop_DMA(&hadc1);
	
	int flag=FS_DHT11_Init();
	if(flag==0)
	{
		printf("传感设备正常，可以通信;工作电压：%.2fV\r\n",(float)value[1]*(3.3/4096));
	}
	//给定初始阈值
	T_up=35;
	T_low=27;
	H_up=70;
	H_low=55;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		DHT11_Read_Data(&humiH,&humiL,&tempH,&tempL);		
		Uart_Proc();
		Lcd_Proc();	
		flag_t=1;
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/*   功能：按键外部中断回调，识别按键是否按下，处理识别按键，并进行相应处理   */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)//
{
	Get_AdcData();
	delay_us(1);
	Get_AdcData();
	int k=getKey_value(value[0]);//value[0],按键adc值
	if(k==5)
	{
			Lcd_Clear(YELLOW);
			Disp_num ^= 0x10;	
			if(Disp_num>>4 == 0x1)//阈值设置界面
			{		
				Lcd_Clear(YELLOW);
			}
			else if(Disp_num>>4 == 0x0)//数据显示界面
			{
				Lcd_Clear(YELLOW);
				Threshold_t_conf[0] = Threshold_t_set[0];
				Threshold_t_conf[1] = Threshold_t_set[1];
				Threshold_h_conf[0] = Threshold_h_set[0];			
				Threshold_h_conf[1] = Threshold_h_set[1];	
			}		
	}
	
	if(k==3)//左键，向上选择参数
	{
		if((Disp_num>>4)==0x1)
			if(--Disp_num == 0xf)
				Disp_num = 0x13;	
	}
	if(k==4)//右键，向下选择参数
	{
		if((Disp_num>>4)==0x1)
			if(++Disp_num == 0x14)
				Disp_num = 0x10;	
	}
	if(k==1)//上键，加
	{
		if(Disp_num == 0x10)
		{
			Threshold_t_set[0] ++;
		}
		else if(Disp_num == 0x11)
		{
			Threshold_t_set[1] ++;
		}
		else if(Disp_num == 0x12)
		{
			Threshold_h_set[0] ++;
		}			
		else if(Disp_num == 0x13)
		{
			Threshold_h_set[1] ++;
		}			
	}
	if(k==2)//下键，减
	{
		if(Disp_num == 0x10)
		{
			Threshold_t_set[0]--;
		}
		else if(Disp_num == 0x11)
		{
			Threshold_t_set[1]--;
		}
		else if(Disp_num == 0x12)
		{
			Threshold_h_set[0]--;	
		}			
		else if(Disp_num == 0x13)
		{
			Threshold_h_set[1]--;
		}			
	}		
}

/*   功能：LCD显示   */
void Lcd_Proc(void)
{
		if(Disp_num>>4 == 0x1)//阈值设置界面
		{				
			if(Disp_num==0x10)
			{
				cl1=WHITE;
			}
			else
				cl1=YELLOW;
			if(Disp_num==0x11)
			{
				cl2=WHITE;
			}
			else
				cl2=YELLOW;
			if(Disp_num==0x12)
			{
				cl3=WHITE;
			}
			else
				cl3=YELLOW;
			if(Disp_num==0x13)
			{
				cl4=WHITE;
			}
			else
				cl4=YELLOW;
			
			Gui_DrawFont_1616(32,0,BLACK, YELLOW,font12,4);//第一行,阈值设置
			
			Gui_DrawFont_1616(48,16,BLACK, YELLOW,font2,3);//第二行，温度
			
			Gui_DrawFont_1616(16,32,BLACK, YELLOW,font10,3);//第三行
			sprintf((char *)Lcd_Disp_String," %.2fC",(float)Threshold_t_set[0]);//温度上阈值
			Gui_DrawFont_GBK16(64, 32, BLACK, cl1,(uint8_t *)Lcd_Disp_String);
			
			Gui_DrawFont_1616(16,48,BLACK, YELLOW,font11,3);//第四行
			sprintf((char *)Lcd_Disp_String," %.2fC",(float)Threshold_t_set[1]);//温度下阈值
			Gui_DrawFont_GBK16(64, 48, BLACK, cl2,(uint8_t *)Lcd_Disp_String);	
			
			Gui_DrawFont_1616(16,64,BLACK, YELLOW,font7,5);//第五行，湿度
			sprintf((char *)Lcd_Disp_String,"  ");
			Gui_DrawFont_GBK16(96, 64, BLACK, YELLOW,(uint8_t *)Lcd_Disp_String);
			
			Gui_DrawFont_1616(16,80,BLACK, YELLOW,font10,3);//第六行
			sprintf((char *)Lcd_Disp_String," %d%% ",Threshold_h_set[0]);//湿度上阈值
			Gui_DrawFont_GBK16(64, 80, BLACK, cl3,(uint8_t *)Lcd_Disp_String);
			
			
			Gui_DrawFont_1616(16,96,BLACK, YELLOW,font11,3);//第七行		
			sprintf((char *)Lcd_Disp_String," %d%% ",Threshold_h_set[1]);//湿度下阈值
			Gui_DrawFont_GBK16(64, 96, BLACK, cl4,(uint8_t *)Lcd_Disp_String);
			
			Gui_DrawFont_1616(0,112,BLACK, YELLOW,font13,8);//第八行，空白	
				
		}
		else if(Disp_num>>4 == 0x0)//数据显示界面
		{
			float temp=tempH + tempL*0.1;
			
			Gui_DrawFont_1616(16,0,BLACK, YELLOW,font1,5);//第一行，设备名
			
			Gui_DrawFont_1616(16,32,BLACK, YELLOW,font2,3);//第三行，温度：
			sprintf((char *)Lcd_Disp_String,"%.2f C",temp);//温度数值
			Gui_DrawFont_GBK16(64, 32, BLACK, YELLOW,(uint8_t *)Lcd_Disp_String);
			
			Gui_DrawFont_1616(0,48,BLACK, YELLOW,font7,5);//第四行，湿度：
			sprintf((char *)Lcd_Disp_String,"%d%%  ",humiH);//湿度数值
			Gui_DrawFont_GBK16(80, 48, BLACK, YELLOW,(uint8_t *)Lcd_Disp_String);	
			
			Gui_DrawFont_1616(32,64,BLACK, YELLOW,font4,3);//第五行，制热			
			if(temp<Threshold_t_conf[1])
			{
				Gui_DrawFont_1616(80,64,BLACK, YELLOW,font5,1);//制热开	
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
			}
			if(temp>Threshold_t_conf[1])
			{
				Gui_DrawFont_1616(80,64,BLACK, YELLOW,font6,1);//制热关
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
			}			
			

			Gui_DrawFont_1616(16,80,BLACK, YELLOW,font3,4);//第六行，制冷	
			if(humiH>Threshold_h_conf[0] || temp>Threshold_t_conf[0])
			{
				Gui_DrawFont_1616(80,80,BLACK, YELLOW,font5,1);//制冷开
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
			}
			else if(humiH<=Threshold_h_conf[1] || temp<=Threshold_t_conf[1])
			{
				Gui_DrawFont_1616(80,80,BLACK, YELLOW,font6,1);//制冷关
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
			}
					
						
			Gui_DrawFont_1616(16,96,BLACK, YELLOW,font8,4);//第七行，风机：		
			if(humiH>Threshold_h_conf[0])
			{
				Gui_DrawFont_1616(80,96,BLACK, YELLOW,font5,1);//风机开
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
			}
			else if(humiH<=Threshold_h_conf[1])
			{
				Gui_DrawFont_1616(80,96,BLACK, YELLOW,font6,1);//风机关	
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);				
			}
			
			Gui_DrawFont_1616(0,112,BLACK, YELLOW,font9,5);//第八行，工作电压：
			sprintf((char *)Lcd_Disp_String,"%.2fV",(float)value[1]*(3.3/4096));
			Gui_DrawFont_GBK16(80, 112, BLACK, YELLOW,(uint8_t *)Lcd_Disp_String);	
			
		}	
}

/*   功能：串口相关功能   */
void Uart_Proc(void)
{
		if(flag_t==1)
		{
			if((uwTick-Tick) > 2000)
			{
				Tick=uwTick;
				printf("温度：%.2fC  湿度：%d%%  Vot:%.2fV  正常工作中\r\n",
								tempH + tempL*0.1,humiH,(float)value[1]*(3.3/4096));	
			}	
		}	
		HAL_UART_Receive_DMA(&huart1,buf_ctl,64);	//串口接收DMA使能
		__HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);	//开启串口空闲中断

}



/*   -重写 ADC转换完成中断完成回调函数  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)//
{
	HAL_ADC_Stop_DMA(&hadc1);
//	printf("KEY=%d   VOT=%d\n",value[0],value[1]);
}

/*   功能：获取adc数据，包括按键电压value[0]，电源电压value[1],此两值未换算   */
/*   以DMA方式开启ADC获取数据   */
void Get_AdcData()
{
	HAL_ADC_Start_DMA(&hadc1,(uint32_t *)value,2);
}

/*   功能：根据adc值，识别按键   */
int getKey_value(uint32_t key)
{
	int k;
	if(key>2156 && key<2200)//上
		k=1;
	else if(key>570 && key<610)//下
		k=2;
	else if(key>1600 && key<1630)//左
		k=3;
	else if(key>3030 && key<3080)//右
		k=4;
	else if(key>2600 && key<2650)//中
		k=5;
	return k;
}



/*   ****************************************************************************   */

void delay_init(uint8_t SYSCLK)
{
  fac_us = SYSCLK;
}

void delay_us(uint32_t nus)//100  6800
{
  uint32_t ticks;
  uint32_t told, tnow, tcnt = 0;
  uint32_t reload = SysTick->LOAD; //LOAD的值
  ticks = nus * fac_us;            //需要的节拍数
  told = SysTick->VAL;             // 24  刚进入时的计数器值
  while (1)
  {
    tnow = SysTick->VAL;//22  20  0
    if (tnow != told)
    {
      if (tnow < told)
        tcnt += told - tnow; //这里注意一下SYSTICK是一个递减的计数器就可以了.
      else
        tcnt += reload - tnow + told;
      told = tnow;
      if (tcnt >= ticks)
        break; //时间超过/等于要延迟的时间,则退出.
    }
  }
}


void delay_ms(uint16_t nms)
{
  uint32_t i;
  for (i = 0; i < nms; i++)
    delay_us(1000);
}


int fputc(int ch,FILE *p)
{
		while(!(USART1->ISR &(1<<7))){}
	  USART1->TDR = ch;
	  return ch;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
