/* USER CODE BEGIN Header */

//This stamp code is made by Rohit Maurya (Embedded Software Developer at Ubyte Consulting).
//This code demonstrates all the parameters that can be read from the Bosch BME690 Sensor on the Ubyte BME690 Evaluation Board connected to Ubyte STM32WB55CGU6 Development Board

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "bme69x.h"
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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

// Variables to store readings from the Gas Sensor
float temperature_c;
float humidity_percent;
float pressure_pa;
float pressure_hpa;
float gas_resistance_ohms;
uint32_t gas_valid;
uint32_t heater_stable;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE {
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY); // Change huart1 to hlpuart1 if needed
  return ch;
}

// 2. Bosch API I2C Read Wrapper
BME69X_INTF_RET_TYPE bme69x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    uint8_t dev_addr = *(uint8_t*)intf_ptr;
    // HAL requires the 7-bit address to be shifted left by 1
    if (HAL_I2C_Mem_Read(&hi2c1, (dev_addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, len, 1000) == HAL_OK) {
        return BME69X_OK;
    }
    return BME69X_E_COM_FAIL;
}

// 3. Bosch API I2C Write Wrapper
BME69X_INTF_RET_TYPE bme69x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    uint8_t dev_addr = *(uint8_t*)intf_ptr;
    if (HAL_I2C_Mem_Write(&hi2c1, (dev_addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)reg_data, len, 1000) == HAL_OK) {
        return BME69X_OK;
    }
    return BME69X_E_COM_FAIL;
}

// 4. Bosch API Delay Wrapper (Converts Microseconds to Milliseconds for HAL)
void bme69x_delay_us(uint32_t period, void *intf_ptr) {
    HAL_Delay((period / 1000) + 1); // Round up to ensure minimum delay is met
}
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

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  // Setting the Default state of the RGB LED to off
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, 1);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 1);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, 1);

  printf("\r\n--- BME690 Sensor Initialization ---\r\n");

    struct bme69x_dev bme;
    struct bme69x_conf conf;
    struct bme69x_heatr_conf heatr_conf;
    struct bme69x_data data;

    uint8_t dev_addr = BME69X_I2C_ADDR_LOW; // Use BME69X_I2C_ADDR_HIGH (0x77) if SDO is tied to 3.3V
    uint32_t del_period;
    uint8_t n_fields;

    // Link our wrapper functions to the Bosch API
    bme.intf_ptr = &dev_addr;
    bme.intf = BME69X_I2C_INTF;
    bme.read = bme69x_i2c_read;
    bme.write = bme69x_i2c_write;
    bme.delay_us = bme69x_delay_us;
    bme.amb_temp = 25; // Default ambient temp for calibration

    // Initialize the sensor
    int8_t rslt = bme69x_init(&bme);
    if (rslt != BME69X_OK) {
        printf("Failed to initialize BME690. Error code: %d\r\n", rslt);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, 0); //Turn on the Green LED to indicate that the sensor is initialized successfully
        while(1); // Halt the code. The user needs to reset the controller in order to re-initialize the sensor
    }
    printf("Sensor initialized successfully!\r\n");

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 0);//Turn on the Red LED to indicate fault

    // Configure Oversampling and Filter settings
    conf.filter = BME69X_FILTER_SIZE_3;
    conf.odr = BME69X_ODR_NONE;
    conf.os_hum = BME69X_OS_2X;
    conf.os_pres = BME69X_OS_4X;
    conf.os_temp = BME69X_OS_8X;
    bme69x_set_conf(&conf, &bme);

    // Configure the Gas Heater (300°C for 100ms is standard for general gas profiling)
    heatr_conf.enable = BME69X_ENABLE;
    heatr_conf.heatr_temp = 300;
    heatr_conf.heatr_dur = 100;
    bme69x_set_heatr_conf(BME69X_FORCED_MODE, &heatr_conf, &bme);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	    bme69x_set_heatr_conf(BME69X_FORCED_MODE, &heatr_conf, &bme);
	  // 1. Trigger a measurement in Forced Mode
	        rslt = bme69x_set_op_mode(BME69X_FORCED_MODE, &bme);

	        // 2. Calculate measurement time and ADD 10,000 microseconds (10ms) of padding
	        del_period = bme69x_get_meas_dur(BME69X_FORCED_MODE, &conf, &bme) + (heatr_conf.heatr_dur * 1000);
	        bme.delay_us(del_period + 10000, bme.intf_ptr); // The extra 10ms guarantees it finishes

	        // 3. Read the compensated data
	              rslt = bme69x_get_data(BME69X_FORCED_MODE, &data, &n_fields, &bme);

	              if (rslt == BME69X_OK && n_fields > 0) {

	                  // Check if ANY new data (Temp/Press/Hum) is ready (Status 0x80)
	                  if (data.status & BME69X_NEW_DATA_MSK) {

	                	  // Store standard environmental data in variables to read in live expression
	                	  temperature_c = data.temperature;
	                	  humidity_percent = data.humidity;
	                	  pressure_pa = data.pressure;
	                	  pressure_hpa = data.pressure / 100.0f;


	                	  // Print the standard environmental data over UART
	                      printf("Temp: %.2f *C | Press: %.2f hPa | Hum: %.2f %% | ",
	                             data.temperature,
	                             data.pressure / 100.0,
	                             data.humidity);

	                      // Check specifically if the Gas data is also ready
	                      if ((data.status & BME69X_GASM_VALID_MSK) && (data.status & BME69X_HEAT_STAB_MSK)) {

	                    	  // Store heater status and gas resistance status in variables to read in live expression
	                    	  gas_valid = (data.status & BME69X_GASM_VALID_MSK) ? 1 : 0;
	                    	  heater_stable = (data.status & BME69X_HEAT_STAB_MSK) ? 1 : 0;

	                    	  // Store gas resistance data in variables to read in live expression
	                    	  gas_resistance_ohms = data.gas_resistance;

	                    	  // Print the gas resistance data over UART
	                    	  printf("Gas Res: %.2f Ohms\r\n", data.gas_resistance);
	                      } else {
	                          printf("Gas: Heater Disabled / Warming Up\r\n");
	                      }

	                  } else {
	                      printf("Waiting for sensor data...\r\n");
	                  }

	              } else {
	                  printf("Failed to read data. Check I2C wiring!\r\n");
	              }

	        HAL_Delay(2000); // Wait 2 seconds before the next reading
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS;
  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSI;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE1;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00100D14;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RGB_Red_GPIO_Port, RGB_Red_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RGB_Green_Pin|RGB_Blue_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : RGB_Red_Pin */
  GPIO_InitStruct.Pin = RGB_Red_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RGB_Red_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RGB_Green_Pin RGB_Blue_Pin */
  GPIO_InitStruct.Pin = RGB_Green_Pin|RGB_Blue_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
#ifdef USE_FULL_ASSERT
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
