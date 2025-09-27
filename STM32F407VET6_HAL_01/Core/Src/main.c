/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

u8 AUTO=0;//如果不带触摸，想要一直刷屏，把这个参数设置为1
u8 Olddir;//用于记录屏幕原始的方向
u8  Contflag=0;//计数标志，用于测算刷图时间
u16 Counter=0; //用于计数，测算刷图时间

//外部flash地址定义
// 16*16汉字GBK 从0x00000000 -0x000BB080
// 24*24汉字GBK 从0x000BB080 -0x001A4D20
// 32*32汉字GBK 从0x0025FDA00-0x002EC200
// 128x128图片  从 0x0054BFA0           开始存放  每张图片大小 128*128*2字节  有5张图片
// 160x128图片  从 0x0054BFA0+0x00028000开始存放  每张图片大小 160*128*2字节  有5张图片   即偏移地址0x00028000
// 320x240图片  从 0x0054BFA0+0x0005A000开始存放  每张图片大小 320*240*2字节  有5张图片   即偏移地址0x0005A000
// 480x272图片  从 0x0054BFA0+0x00115800开始存放  每张图片大小 480*272*2字节  有5张图片   即偏移地址0x00115800
// 480x320图片  从 0x0054BFA0+0x00253C80开始存放  每张图片大小 480*320*2字节  有5张图片   即偏移地址0x00253C80

//图片存放地址 
#define pic_ad  ADD_BASE+0x0054BFA0 //这个地址不需要改动
//根据lcd.h中配置的分辨率选择对应的图片起始地址
#if((LCD_WIDTH==128)&&(LCD_HEIGHT==128))
#define off_ad  0x0
#elif((LCD_WIDTH==128)&&(LCD_HEIGHT==160))
#define off_ad  0x00028000
#elif((LCD_WIDTH==240)&&(LCD_HEIGHT==320))
#define off_ad  0x0005A000
#elif((LCD_WIDTH==480)&&(LCD_HEIGHT==272))
#define off_ad  0x00115800
#elif((LCD_WIDTH==320)&&(LCD_HEIGHT==480))
#define off_ad  0x00253C80
#endif
//图片取模方式为横屏取模，如果要正常显示图片，需要切换成横屏显示!!!

//定义按键
#define KEYup   1
#define KEYdown 2

#define DelayTime 1000000
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
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
void Simple_Touch_Test(void)
{
    static uint16_t last_x = 0, last_y = 0;
    static uint8_t first_run = 1;
    char info_buf[50];
    
    // 首次运行时初始化界面
    if (first_run)
    {
        LCD_Clear(WHITE);
        LCD_ShowString(10, 10, LCD_WIDTH-20, 30, 16, (uint8_t*)"Touch Screen Test", RED);
        LCD_ShowString(10, 40, LCD_WIDTH-20, 30, 16, (uint8_t*)"Touch to draw", BLUE);
        
        // 绘制分隔线
        LCD_DrawLine(0, 70, LCD_WIDTH, 70, GRAY);
        
        // 显示坐标区域标题
        LCD_ShowString(10, 80, 200, 20, 16, (uint8_t*)"Coordinates:", BLACK);
        
        first_run = 0;
    }
    
    // 扫描触摸屏
    tp_dev.scan(0);
    
    // 只在有触摸时执行操作
    if (tp_dev.sta & TP_PRES_DOWN)
    {
        // 显示坐标
        sprintf(info_buf, "X:%04d Y:%04d", tp_dev.x[0], tp_dev.y[0]);
        LCD_Fill(100, 80, LCD_WIDTH-10, 95, WHITE);
        LCD_ShowString(100, 80, 150, 20, 16, (uint8_t*)info_buf, BLUE);
        
        // 绘制连接线（从上一个点到当前点）
        if (last_x > 0 && last_y > 0)
        {
            LCD_DrawLine(last_x, last_y, tp_dev.x[0], tp_dev.y[0], RED);
        }
        
        // 绘制当前触摸点
        Draw_Circle(tp_dev.x[0], tp_dev.y[0], 3, RED);
        
        // 保存当前坐标
        last_x = tp_dev.x[0];
        last_y = tp_dev.y[0];
        
        // 添加小延时，防止过于频繁的刷新
        HAL_Delay(5);
    }
}
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
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  __HAL_SPI_ENABLE(&hspi1);
  
  printf("LCD_Init!\r\n");
  LCD_Init();
  // 打开背光
  LCD_LED_Set(GPIO_PIN_SET);
  
  // 初始化触摸屏
  if (TP_Init() == 0)
  {
    // 触摸屏初始化失败
    LCD_Clear(WHITE);
    LCD_ShowString(10, 10, 200, 30, 16, (uint8_t*)"Touch Init Failed!", RED);
    HAL_Delay(1000);
  }
  
  // 清屏并设置背景色
  LCD_Clear(WHITE);
  
  // 设置画笔颜色为红色
  POINT_COLOR = RED;
  
  // 显示欢迎信息
  LCD_ShowString(10, 10, 200, 30, 16, (uint8_t*)"LCD Test Demo", POINT_COLOR);
  LCD_ShowString(10, 40, 200, 30, 16, (uint8_t*)"Press RST for touch test", BLUE);
  
  // 显示更多测试内容
  LCD_ShowString(10, 70, 200, 30, 16, (uint8_t*)"Hello World!", POINT_COLOR);
  
  HAL_Delay(100);	
  // 显示数字
  LCD_ShowNum(10, 100, 12345, 5, 16, POINT_COLOR);
  
  // 画一些图形
  LCD_DrawLine(10, 130, 100, 130, BLUE);      // 画线
  LCD_DrawRectangle(10, 150, 60, 190, GREEN); // 画矩形
  Draw_Circle(150, 170, 20, RED);
  LCD_Clear(WHITE);
  LCD_Fill(0,0,200,200,0xf700);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  //Simple_Touch_Test();
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 144;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
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

