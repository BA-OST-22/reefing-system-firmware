/**
 ******************************************************************************
 * @file           : usbd_cdc_if.c
 * @version        : v2.0_Cube
 * @brief          : Usb device for Virtual Com Port.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

#include "util/fifo.h"
#include "config/globals.h"

/* USB config */
#define TIMUsb TIM5
#define CDC_POLLING_INTERVAL 2 // ms
#define TIMUsb_CLK_ENABLE __HAL_RCC_TIM5_CLK_ENABLE
#define TIMUsb_IRQn TIM5_IRQn
#define TIMUsb_IRQHandler TIM5_IRQHandler

// Timer for periodic transmission
TIM_HandleTypeDef TimHandle;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
trace_command_buffer_t trace_command_buffer;

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
 * @brief Private variables.
 * @{
 */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
 * @brief Public variables.
 * @{
 */

extern USBD_HandleTypeDef hUsbDeviceFS;

static bool usb_initialized = false;

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes
 * USBD_CDC_IF_Private_FunctionPrototypes
 * @brief Private functions declaration.
 * @{
 */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t *pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

static void TIM_Config(void);

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = {CDC_Init_FS, CDC_DeInit_FS, CDC_Control_FS, CDC_Receive_FS,
                                              CDC_TransmitCplt_FS};

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Initializes the CDC media low layer over the FS USB IP
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Init_FS(void) {
  TIM_Config();

  if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK) {
    /* Starting Error */
    Error_Handler();
  }
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 512);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);

  usb_initialized = true;
  return (USBD_OK);
}

/**
 * @brief  DeInitializes the CDC media low layer
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_DeInit_FS(void) {
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
 * @brief  Manage the CDC class requests
 * @param  cmd: Command code
 * @param  pbuf: Buffer containing command data (request parameters)
 * @param  length: Number of data to be sent (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else
 * USBD_FAIL
 */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length) {
  /* USER CODE BEGIN 5 */
  UNUSED(length);
  uint32_t speed = 115200;
  switch (cmd) {
    case CDC_SEND_ENCAPSULATED_COMMAND:

      break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

      break;

    case CDC_SET_COMM_FEATURE:

      break;

    case CDC_GET_COMM_FEATURE:

      break;

    case CDC_CLEAR_COMM_FEATURE:

      break;

      /*******************************************************************************/
      /* Line Coding Structure */
      /*-----------------------------------------------------------------------------*/
      /* Offset | Field       | Size | Value  | Description */
      /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per
       * second*/
      /* 4      | bCharFormat |   1  | Number | Stop bits */
      /*                                        0 - 1 Stop bit */
      /*                                        1 - 1.5 Stop bits */
      /*                                        2 - 2 Stop bits */
      /* 5      | bParityType |  1   | Number | Parity */
      /*                                        0 - None */
      /*                                        1 - Odd */
      /*                                        2 - Even */
      /*                                        3 - Mark */
      /*                                        4 - Space */
      /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16). */
      /*******************************************************************************/
    case CDC_SET_LINE_CODING:

      break;

    case CDC_GET_LINE_CODING:
      pbuf[0] = (uint8_t)(speed);
      pbuf[1] = (uint8_t)(speed >> 8);
      pbuf[2] = (uint8_t)(speed >> 16);
      pbuf[3] = (uint8_t)(speed >> 24);
      pbuf[4] = 0;
      pbuf[5] = 0;
      pbuf[6] = 8;
      break;

    case CDC_SET_CONTROL_LINE_STATE:

      break;

    case CDC_SEND_BREAK:

      break;

    default:
      break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
 * @brief  Data received over USB OUT endpoint are sent over CDC interface
 *         through this function.
 *
 *         @note
 *         This function will issue a NAK packet on any OUT packet received on
 *         USB endpoint until exiting this function. If you exit this function
 *         before transfer is complete on CDC interface (ie. using DMA
 * controller) it will result in receiving more data while previous ones are
 * still not sent.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else
 * USBD_FAIL
 */
static int8_t CDC_Receive_FS(uint8_t *Buf, uint32_t *Len) {
  /* USER CODE BEGIN 6 */
  uint32_t buf_length = *Len;
  if (buf_length != 0) {
    fifo_write_bytes(&usb_input_fifo, Buf, buf_length);
  }
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
 * @brief  CDC_Transmit_FS
 *         Data to send over USB IN endpoint are sent over CDC interface
 *         through this function.
 *         @note
 *
 *
 * @param  Buf: Buffer of data to be sent
 * @param  Len: Number of data to be sent (in bytes)
 * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
 */
uint8_t CDC_Transmit_FS(uint8_t *Buf, uint16_t Len) {
  if (usb_initialized == false) return 0;
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0) {
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}

/**
 * @brief  CDC_TransmitCplt_FS
 *         Data transmited callback
 *
 *         @note
 *         This function is IN transfer complete callback used to inform user
 * that the submitted Data is successfully sent over USB.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else
 * USBD_FAIL
 */
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum) {
  int8_t result = USBD_OK;
  /* USER CODE BEGIN 13 */
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
  /* USER CODE END 13 */
  return result;
}

void TIMUsb_IRQHandler(void) {
	HAL_TIM_IRQHandler(&TimHandle);
}

void CDC_Transmit_Elapsed() {
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
  if (hcdc->TxState == 0) {
    // Check usb fifo and print out to usb
    uint32_t len = fifo_get_length(&usb_output_fifo);
    if (len) {
      if (fifo_read_bytes(&usb_output_fifo, UserTxBufferFS, len)) {
        USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, len);
        USBD_CDC_TransmitPacket(&hUsbDeviceFS);
      }
    }
  }
}

static void TIM_Config(void) {
   /* Enable TIM peripherals Clock */
  TIMUsb_CLK_ENABLE();

  /* Set TIMUsb instance */
  TimHandle.Instance = TIMUsb;

  TimHandle.Init.Period = (CDC_POLLING_INTERVAL * 1000) - 1;
  TimHandle.Init.Prescaler = (SystemCoreClock / 2 / (1000000)) - 1;
  TimHandle.Init.ClockDivision = 0;
  TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK) {
    /* Initialization Error */
    Error_Handler();
  }


  /* Configure the NVIC for TIMx */
  /* Set Interrupt Group Priority */
  HAL_NVIC_SetPriority(TIMUsb_IRQn, 0, 0);

  /* Enable the TIMx global Interrupt */
  HAL_NVIC_EnableIRQ(TIMUsb_IRQn);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
