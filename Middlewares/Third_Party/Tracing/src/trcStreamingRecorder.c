/*
 * Trace Recorder for Tracealyzer v4.5.0
 * Copyright 2021 Percepio AB
 * www.percepio.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * The generic core of the trace recorder's streaming mode.
 */

#include "../../Tracing/inc/trcRecorder.h"

#if (TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING)

#if (TRC_USE_TRACEALYZER_RECORDER == 1)

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "../../Tracing/inc/trcExtensions.h"
#include "../../Tracing/inc/trcInternalBuffer.h"

/* Unless specified in trcConfig.h we assume this is a single core target */
#ifndef TRC_CFG_PLATFORM_NUM_CORES
#define TRC_CFG_PLATFORM_NUM_CORES 1
#endif

#ifndef TRC_GET_CURRENT_CORE
#define TRC_GET_CURRENT_CORE() 0
#endif

#if (TRC_CFG_PLATFORM_NUM_CORES) > 1
#define TRC_GET_EVENT_COUNT(eventCounter)                                      \
  (((TRC_GET_CURRENT_CORE() & 0xF) << 12) | (eventCounter & 0xFFF))
#else
#define TRC_GET_EVENT_COUNT(eventCounter) eventCounter
#endif

#define TRC_PLATFORM_CFG_LENGTH 8

typedef struct {
  uint16_t EventID;
  uint16_t EventCount;
  uint32_t TS;
} BaseEvent;

typedef struct {
  BaseEvent base;
  uint32_t param1;
} EventWithParam_1;

typedef struct {
  BaseEvent base;
  uint32_t param1;
  uint32_t param2;
} EventWithParam_2;

typedef struct {
  BaseEvent base;
  uint32_t param1;
  uint32_t param2;
  uint32_t param3;
} EventWithParam_3;

typedef struct {
  BaseEvent base;
  uint32_t param1;
  uint32_t param2;
  uint32_t param3;
  uint32_t param4;
} EventWithParam_4;

typedef struct {
  BaseEvent base;
  uint32_t param1;
  uint32_t param2;
  uint32_t param3;
  uint32_t param4;
  uint32_t param5;
} EventWithParam_5;

/* Used in event functions with variable number of parameters. */
typedef struct {
  BaseEvent base;
  uint32_t data[15]; /* maximum payload size */
} largestEventType;

typedef struct {
  uint32_t psf;
  uint16_t version;
  uint16_t platform;
  uint32_t options;
  uint32_t numCores;
  char platform_cfg[TRC_PLATFORM_CFG_LENGTH];
  uint16_t platform_cfg_patch;
  uint8_t platform_cfg_minor;
  uint8_t platform_cfg_major;
  uint32_t heapCounter;
  uint32_t heapMax;
  uint16_t symbolSize;
  uint16_t symbolCount;
  uint16_t objectDataSize;
  uint16_t objectDataCount;
} PSFHeaderInfo;

#ifndef TRC_CFG_RECORDER_DATA_INIT
#define TRC_CFG_RECORDER_DATA_INIT 1
#endif

/* The size of each slot in the Symbol Table */
#define SYMBOL_TABLE_SLOT_SIZE                                                 \
  (sizeof(uint32_t) +                                                          \
   (((TRC_CFG_SYMBOL_MAX_LENGTH) + (sizeof(uint32_t) - 1)) /                   \
    sizeof(uint32_t)) *                                                        \
       sizeof(uint32_t))

#define OBJECT_DATA_SLOT_SIZE (sizeof(uint32_t) + sizeof(uint32_t))

/* The total size of the Symbol Table */
#define SYMBOL_TABLE_BUFFER_SIZE                                               \
  ((TRC_CFG_SYMBOL_TABLE_SLOTS)*SYMBOL_TABLE_SLOT_SIZE)

/* The total size of the Object Data Table */
#define OBJECT_DATA_TABLE_BUFFER_SIZE                                          \
  ((TRC_CFG_OBJECT_DATA_SLOTS)*OBJECT_DATA_SLOT_SIZE)

#if (TRC_CFG_PAGED_EVENT_BUFFER_PAGE_COUNT > 128)
#error "TRC_CFG_PAGED_EVENT_BUFFER_PAGE_COUNT cannot be larger than 128"
#endif /* (TRC_CFG_PAGED_EVENT_BUFFER_PAGE_COUNT > 128) */

/* The Symbol Table type - just a byte array */
typedef struct {
  union {
    uint32_t
        pSymbolTableBufferUINT32[SYMBOL_TABLE_BUFFER_SIZE / sizeof(uint32_t)];
    uint8_t pSymbolTableBufferUINT8[SYMBOL_TABLE_BUFFER_SIZE];
  } SymbolTableBuffer;
} SymbolTable;

/* The Object Data Table type - just a byte array */
typedef struct {
  union {
    uint32_t pObjectDataTableBufferUINT32[OBJECT_DATA_TABLE_BUFFER_SIZE /
                                          sizeof(uint32_t)];
    uint8_t pObjectDataTableBufferUINT8[OBJECT_DATA_TABLE_BUFFER_SIZE];
  } ObjectDataTableBuffer;
} ObjectDataTable;

#if defined(TRC_CFG_ENABLE_STACK_MONITOR) &&                                   \
    (TRC_CFG_ENABLE_STACK_MONITOR == 1) && (TRC_CFG_SCHEDULING_ONLY == 0)
typedef struct {
  void *tcb;
  uint32_t uiPreviousLowMark;
} TaskStackMonitorEntry_t;
#endif /* defined(TRC_CFG_ENABLE_STACK_MONITOR) &&                             \
          (TRC_CFG_ENABLE_STACK_MONITOR == 1) && (TRC_CFG_SCHEDULING_ONLY ==   \
          0) */

/* Code used for "task address" when no task has started, to indicate
 * "(startup)". This value was used since NULL/0 was already reserved for the
 * idle task. */
#define HANDLE_NO_TASK 2

/* Calls prvTraceError if the _assert condition is false. For void functions,
where no return value is to be provided. */
#define PSF_ASSERT_VOID(_assert, _err)                                         \
  if (!(_assert)) {                                                            \
    prvTraceError(_err);                                                       \
    return;                                                                    \
  }

/* Calls prvTraceError if the _assert condition is false. For non-void
functions, where a return value is to be provided. */
#define PSF_ASSERT_RET(_assert, _err, _return)                                 \
  if (!(_assert)) {                                                            \
    prvTraceError(_err);                                                       \
    return _return;                                                            \
  }

/* Part of the PSF format - encodes the number of 32-bit params in an event */
#define PARAM_COUNT(n) ((n & 0xF) << 12)

/* We skip the slot for PSF_ERROR_NONE so error code 1 is the first bit */
#define GET_ERROR_WARNING_FLAG(errCode)                                        \
  (ErrorAndWarningFlags & (1 << ((errCode)-1)))
#define SET_ERROR_WARNING_FLAG(errCode)                                        \
  (ErrorAndWarningFlags |= (1 << ((errCode)-1)))

/* Counts the number of trace sessions (not yet used) */
static uint32_t SessionCounter = 0u;

/* Used to determine endian of data (big/little) */
static uint32_t PSFEndianessIdentifier = 0x50534600;

/* Used to interpret the data format */
static uint16_t FormatVersion = 0x0009;

/* The number of events stored. Used as event sequence number. */
static uint32_t eventCounter = 0;

/* The extension data */
PSFExtensionInfoType PSFExtensionInfo = TRC_EXTENSION_INFO;

/* Used for flags indicating if a certain error or warning has occurred */
static uint32_t ErrorAndWarningFlags TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/* The Symbol Table instance - keeps names of tasks and other named objects. */
static SymbolTable symbolTable TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/* This points to the first unused entry in the symbol table. */
static uint32_t firstFreeSymbolTableIndex TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/* The Object Data Table instance - keeps initial priorities of tasks. */
static ObjectDataTable objectDataTable TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/* This points to the first unused entry in the object data table. */
static uint32_t firstFreeObjectDataTableIndex TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/* Keeps track of ISR nesting */
static uint32_t
    ISR_stack[TRC_CFG_MAX_ISR_NESTING] TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/* Keeps track of ISR nesting */
static int8_t ISR_stack_index TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/* Any error that occurred in the recorder (also creates User Event) */
static int errorCode TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/* The amount of Heap currently being used */
uint32_t trcHeapCounter TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/* The maximum amount of Heap that has been used */
uint32_t trcHeapMax TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/* Temporary event data storage */
static largestEventType xEventDataDummy = {{0, 0, 0}, {0}};

/* The current event */
static largestEventType *pvCurrentEvent = 0;

/* The payload of the current event that has been initialized using
 * prvTraceBeginStoreEvent() */
static uint32_t uiCurrentEventPayloadSize = 0;

/* The size of the current event that has been initialized using
 * prvTraceBeginStoreEvent() */
static uint32_t uiCurrentEventSize = 0;

/* The current event's payload pointer */
static uint32_t uiCurrentEventPayloadOffset = 0;

/* The most recent tcb address */
static uint32_t xCurrentTask = 0;

/* Remembers if an earlier ISR in a sequence of adjacent ISRs has triggered a
task switch. In that case, vTraceStoreISREnd does not store a return to the
previously executing task. */
int32_t isPendingContextSwitch TRC_CFG_RECORDER_DATA_ATTRIBUTE;

uint32_t uiTraceTickCount TRC_CFG_RECORDER_DATA_ATTRIBUTE;
uint32_t timestampFrequency TRC_CFG_RECORDER_DATA_ATTRIBUTE;
uint32_t DroppedEventCounter TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/*******************************************************************************
 * NoRoomForSymbol
 *
 * Incremented on prvTraceSaveSymbol if no room for saving the symbol name. This
 * is used for storing the names of:
 * - Tasks
 * - Named ISRs (xTraceSetISRProperties)
 * - Named kernel objects (vTraceStoreKernelObjectName)
 * - User event channels (xTraceRegisterString)
 *
 * This variable should be zero. If not, it shows the number of missing slots so
 * far. In that case, increment SYMBOL_TABLE_SLOTS with (at least) this value.
 ******************************************************************************/
volatile uint32_t NoRoomForSymbol TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/*******************************************************************************
 * NoRoomForObjectData
 *
 * Incremented on prvTraceSaveObjectData if no room for saving the object data,
 * i.e., the base priorities of tasks. There must be one slot for each task.
 * If not, this variable will show the difference.
 *
 * This variable should be zero. If not, it shows the number of missing slots so
 * far. In that case, increment OBJECT_DATA_SLOTS with (at least) this value.
 ******************************************************************************/
volatile uint32_t NoRoomForObjectData TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/*******************************************************************************
 * LongestSymbolName
 *
 * Updated in prvTraceSaveSymbol. Should not exceed TRC_CFG_SYMBOL_MAX_LENGTH,
 * otherwise symbol names will be truncated. In that case, set
 * TRC_CFG_SYMBOL_MAX_LENGTH to (at least) this value.
 ******************************************************************************/
volatile uint32_t LongestSymbolName TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/*******************************************************************************
 * MaxBytesTruncated
 *
 * Set in prvTraceStoreStringEvent if the total data payload exceeds 60 bytes,
 * including data arguments and the string. For user events, that is 52 bytes
 * for string and data arguments. In that is exceeded, the event is  truncated
 * (usually only the string, unless more than 15 parameters) and this variable
 * holds the maximum number of truncated bytes, from any event.
 ******************************************************************************/
volatile uint32_t MaxBytesTruncated TRC_CFG_RECORDER_DATA_ATTRIBUTE;

uint16_t CurrentFilterMask TRC_CFG_RECORDER_DATA_ATTRIBUTE;

uint16_t CurrentFilterGroup TRC_CFG_RECORDER_DATA_ATTRIBUTE;

volatile uint32_t uiTraceSystemState TRC_CFG_RECORDER_DATA_ATTRIBUTE;

#if defined(TRC_CFG_ENABLE_STACK_MONITOR) &&                                   \
    (TRC_CFG_ENABLE_STACK_MONITOR == 1) && (TRC_CFG_SCHEDULING_ONLY == 0)
/*******************************************************************************
 * TaskStacksNotIncluded
 *
 * Increased in prvAddTaskToStackMonitor(...) if there is no room in the stack
 * monitor.
 ******************************************************************************/
volatile uint32_t TaskStacksNotIncluded TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/*******************************************************************************
 * tasksInStackMonitor
 *
 * Keeps track of the stacks for each task.
 ******************************************************************************/
TaskStackMonitorEntry_t tasksInStackMonitor
    [TRC_CFG_STACK_MONITOR_MAX_TASKS] TRC_CFG_RECORDER_DATA_ATTRIBUTE;
#endif /* defined(TRC_CFG_ENABLE_STACK_MONITOR) &&                             \
          (TRC_CFG_ENABLE_STACK_MONITOR == 1) && (TRC_CFG_SCHEDULING_ONLY ==   \
          0) */

/* RecorderEnabled is a master switch for recording (0 => Disabled, 1 =>
 * Enabled) */
uint32_t RecorderEnabled TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/*******************************************************************************
 * RecorderInitialized
 *
 * Makes sure the recorder data is only initialized once.
 *
 * NOTE: RecorderInitialized is only initialized to 0 if
 * TRC_CFG_RECORDER_DATA_INIT is non-zero.
 * This will avoid issues where the recorder must be started before main(),
 * which can lead to RecorderInitialized be cleared by late initialization after
 * vTraceEnable(TRC_INIT) was called and assigned RecorderInitialized its'
 * value.
 ******************************************************************************/
#if (TRC_CFG_RECORDER_DATA_INIT != 0)
uint32_t RecorderInitialized = 0;
#else /* (TRC_CFG_RECORDER_DATA_INIT != 0) */
uint32_t RecorderInitialized TRC_CFG_RECORDER_DATA_ATTRIBUTE;
#endif /* (TRC_CFG_RECORDER_DATA_INIT != 0) */

/* Internal common function for storing string events */
static void prvTraceStoreStringEventHelper(int nArgs, uint16_t eventID,
                                           traceString userEvtChannel, int len,
                                           const char *str, va_list vl);

/* Not static to avoid warnings from SysGCC/PPC */
void prvTraceStoreSimpleStringEventHelper(uint16_t eventID,
                                          traceString userEvtChannel,
                                          const char *str);

/* Stores the header information on Start */
static void prvTraceStoreHeader(void);

/* Stores the Start Event */
static void prvTraceStoreStartEvent(void);

/* Stores the symbol table on Start */
static void prvTraceStoreSymbolTable(void);

/* Stores the object table on Start */
static void prvTraceStoreObjectDataTable(void);

/* Store the Timestamp Config on Start */
static void prvTraceStoreTSConfig(void);

/* Store information about trace library extensions. */
static void prvTraceStoreExtensionInfo(void);

/* Internal function for starting/stopping the recorder. */
static void prvSetRecorderEnabled(uint32_t isEnabled);

/* Performs timestamping using definitions in trcHardwarePort.h */
static uint32_t prvGetTimestamp32(void);

/* Returns the string associated with the error code */
static const char *prvTraceGetError(int errCode);

/* Signal an error. */
void prvTraceError(int errCode);

/* Signal a warning (does not stop the recorder). */
void prvTraceWarning(int errCode);

/******************************************************************************
 * vTraceInstanceFinishedNow
 *
 * Creates an event that ends the current task instance at this very instant.
 * This makes the viewer to splits the current fragment at this point and begin
 * a new actor instance, even if no task-switch has occurred.
 *****************************************************************************/
void vTraceInstanceFinishedNow(void) {
  prvTraceStoreEvent0(PSF_EVENT_IFE_DIRECT);
}

/******************************************************************************
 * vTraceInstanceFinishedNext
 *
 * Marks the current "task instance" as finished on the next kernel call.
 *
 * If that kernel call is blocking, the instance ends after the blocking event
 * and the corresponding return event is then the start of the next instance.
 * If the kernel call is not blocking, the viewer instead splits the current
 * fragment right before the kernel call, which makes this call the first event
 * of the next instance.
 *****************************************************************************/
void vTraceInstanceFinishedNext(void) {
  prvTraceStoreEvent0(PSF_EVENT_IFE_NEXT);
}

/*******************************************************************************
 * vTraceStoreKernelObjectName
 *
 * Parameter object: pointer to the Event Group that shall be named
 * Parameter name: the name to set (const string literal)
 *
 * Sets a name for a kernel object for display in Tracealyzer.
 ******************************************************************************/
void vTraceStoreKernelObjectName(void *object, const char *name) {
  uint16_t eventID = PSF_EVENT_OBJ_NAME;

  PSF_ASSERT_VOID(eventID < 4096, PSF_ERROR_EVENT_CODE_TOO_LARGE);

  /* Always save in symbol table, in case the recording has not yet started */
  prvTraceSaveObjectSymbol(object, name);

  prvTraceStoreStringEvent(1, eventID, name, object);
}

/******************************************************************************
 * vTraceSetFrequency
 *
 * Registers the clock rate of the time source for the event timestamping.
 * This is normally not required, but if the default value (TRC_HWTC_FREQ_HZ)
 * should be incorrect for your setup, you can override it using this function.
 *
 * Must be called prior to vTraceEnable, and the time source is assumed to
 * have a fixed clock frequency after the startup.
 *****************************************************************************/
void vTraceSetFrequency(uint32_t frequency) { timestampFrequency = frequency; }

#if (TRC_CFG_SCHEDULING_ONLY == 0) && (TRC_CFG_INCLUDE_USER_EVENTS == 1)

/*******************************************************************************
 * xTraceRegisterString
 *
 * Stores a name for a user event channel, returns the handle.
 ******************************************************************************/
traceString xTraceRegisterString(const char *name) {
  traceString str;
  uint16_t eventID = PSF_EVENT_OBJ_NAME;

  str = prvTraceSaveSymbol(name);

  PSF_ASSERT_RET(eventID < 4096, PSF_ERROR_EVENT_CODE_TOO_LARGE, str);

  /* Always save in symbol table, if the recording has not yet started */
  prvTraceStoreStringEvent(1, eventID, (const char *)name, str);

  return str;
}

/******************************************************************************
 * vTracePrint
 *
 * Generates "User Events", with unformatted text.
 *
 * User Events can be used for very efficient application logging, and are shown
 * as yellow labels in the main trace view.
 *
 * You may group User Events into User Event Channels. The yellow User Event
 * labels shows the logged string, preceded by the channel  name within
 * brackets. For example:
 *
 *  "[MyChannel] Hello World!"
 *
 * The User Event Channels are shown in the View Filter, which makes it easy to
 * select what User Events you wish to display. User Event Channels are created
 * using xTraceRegisterString().
 *
 * Example:
 *
 *	 traceString chn = xTraceRegisterString("MyChannel");
 *	 ...
 *	 vTracePrint(chn, "Hello World!");
 *
 ******************************************************************************/
void vTracePrint(traceString chn, const char *str) {
  uint16_t eventID = PSF_EVENT_USER_EVENT;
  PSF_ASSERT_VOID(eventID < 4096, PSF_ERROR_EVENT_CODE_TOO_LARGE);

  prvTraceStoreSimpleStringEventHelper(eventID, chn, str);
}

/*******************************************************************************
 * vTraceConsoleChannelPrintF
 *
 * Wrapper for vTracePrint, using the default channel. Can be used as a drop-in
 * replacement for printf and similar functions, e.g. in a debug logging macro.
 *
 * Example:
 *
 *	 // Old: #define LogString debug_console_printf
 *
 *    // New, log to Tracealyzer instead:
 *	 #define LogString vTraceConsoleChannelPrintF
 *	 ...
 *	 LogString("My value is: %d", myValue);
 ******************************************************************************/
void vTraceConsoleChannelPrintF(const char *fmt, ...) {
  va_list vl;
  char tempBuf[60];
  static traceString consoleChannel = NULL;

  if (consoleChannel == NULL)
    consoleChannel = xTraceRegisterString("Debug Console");

  va_start(vl, fmt);
  vsnprintf(tempBuf, 60, fmt, vl);
  vTracePrint(consoleChannel, tempBuf);
  va_end(vl);
}

/******************************************************************************
 * vTracePrintF
 *
 * Generates "User Events", with formatted text and data, similar to a "printf".
 * It is very fast since the actual formatting is done on the host side when the
 * trace is displayed.
 *
 * User Events can be used for very efficient application logging, and are shown
 * as yellow labels in the main trace view.
 * An advantage of User Events is that data can be plotted in the "User Event
 * Signal Plot" view, visualizing any data you log as User Events, discrete
 * states or control system signals (e.g. system inputs or outputs).
 *
 * You may group User Events into User Event Channels. The yellow User Event
 * labels show the logged string, preceded by the channel name within brackets.
 *
 * Example:
 *
 *  "[MyChannel] Hello World!"
 *
 * The User Event Channels are shown in the View Filter, which makes it easy to
 * select what User Events you wish to display. User Event Channels are created
 * using xTraceRegisterString().
 *
 * Example:
 *
 *	 traceString adc_uechannel = xTraceRegisterString("ADC User Events");
 *	 ...
 *	 vTracePrintF(adc_uechannel,
 *				 "ADC channel %d: %d volts",
 *				 ch, adc_reading);
 *
 * All data arguments are assumed to be 32 bit wide. The following formats are
 * supported:
 * %d - signed integer. The following width and padding format is supported:
 *"%05d" -> "-0042" and "%5d" -> "  -42" %u - unsigned integer. The following
 *width and padding format is supported: "%05u" -> "00042" and "%5u" -> "   42"
 * %X - hexadecimal (uppercase). The following width and padding format is
 *supported: "%04X" -> "002A" and "%4X" -> "  2A" %x - hexadecimal (lowercase).
 *The following width and padding format is supported: "%04x" -> "002a" and
 *"%4x" -> "  2a" %s - string (currently, this must be an earlier stored symbol
 *name)
 *
 * Up to 15 data arguments are allowed, with a total size of maximum 60 byte
 * including 8 byte for the base event fields and the format string. So with
 * one data argument, the maximum string length is 48 chars. If this is exceeded
 * the string is truncated (4 bytes at a time).
 *
 ******************************************************************************/
void vTracePrintF(traceString chn, const char *fmt, ...) {
  va_list vl;

  va_start(vl, fmt);
  vTraceVPrintF(chn, fmt, vl);
  va_end(vl);
}

/******************************************************************************
 * vTraceVPrintF
 *
 * vTracePrintF variant that accepts a va_list.
 * See vTracePrintF documentation for further details.
 *
 ******************************************************************************/
void vTraceVPrintF(traceString chn, const char *fmt, va_list vl) {
  int i = 0;
  int nArgs = 0;
  int eventID = PSF_EVENT_USER_EVENT;

  /* Count the number of arguments in the format string (e.g., %d) */
  for (i = 0; (fmt[i] != 0) && (i < 52); i++) {
    if (fmt[i] == '%') {
      if (fmt[i + 1] == 0) {
        /* Found end of string, let for loop detect it */
        continue;
      }

      if (fmt[i + 1] != '%') {
        nArgs++; /* Found an argument */
      }

      i++; /* Move past format specifier or non-argument '%' */
    }
  }

  if (chn != NULL) {
    /* Make room for the channel */
    nArgs++;
  }
  eventID += nArgs;

  PSF_ASSERT_VOID(eventID < 4096, PSF_ERROR_EVENT_CODE_TOO_LARGE);

  prvTraceStoreStringEventHelper(nArgs, (uint16_t)eventID, chn, i, fmt, vl);
}
#endif /* (TRC_CFG_SCHEDULING_ONLY == 0) && (TRC_CFG_INCLUDE_USER_EVENTS == 1) \
        */

/*******************************************************************************
 * xTraceSetISRProperties
 *
 * Stores a name and priority level for an Interrupt Service Routine, to allow
 * for better visualization. Returns a traceHandle used by vTraceStoreISRBegin.
 *
 * Example:
 *	 #define PRIO_ISR_TIMER1 3 // the hardware priority of the interrupt
 *	 ...
 *	 traceHandle Timer1Handle = xTraceSetISRProperties("ISRTimer1",
 *PRIO_ISR_TIMER1);
 *	 ...
 *	 void ISR_handler()
 *	 {
 *		 vTraceStoreISRBegin(Timer1Handle);
 *		 ...
 *		 vTraceStoreISREnd(0);
 *	 }
 *
 ******************************************************************************/
traceHandle xTraceSetISRProperties(const char *name, uint8_t priority) {
  traceHandle isrHandle;
  uint16_t eventID = PSF_EVENT_DEFINE_ISR;

  /* Always save in symbol table, in case the recording has not yet started */
  isrHandle = prvTraceSaveSymbol(name);

  /* Save object data in object data table */
  prvTraceSaveObjectData((void *)isrHandle, priority);

  PSF_ASSERT_RET(eventID < 4096, PSF_ERROR_EVENT_CODE_TOO_LARGE, isrHandle);

  prvTraceStoreStringEvent(2, eventID, name, isrHandle, priority);

  return isrHandle;
}

/*******************************************************************************
 * vTraceStoreISRBegin
 *
 * Registers the beginning of an Interrupt Service Routine, using a traceHandle
 * provided by xTraceSetISRProperties.
 *
 * Example:
 *	 #define PRIO_ISR_TIMER1 3 // the hardware priority of the interrupt
 *	 ...
 *	 traceHandle Timer1Handle = xTraceSetISRProperties("ISRTimer1",
 *PRIO_ISR_TIMER1);
 *	 ...
 *	 void ISR_handler()
 *	 {
 *		 vTraceStoreISRBegin(Timer1Handle);
 *		 ...
 *		 vTraceStoreISREnd(0);
 *	 }
 *
 ******************************************************************************/
void vTraceStoreISRBegin(traceHandle handle) {
  TRACE_ALLOC_CRITICAL_SECTION();

  TRACE_ENTER_CRITICAL_SECTION();

  /* We are at the start of a possible ISR chain.
  No context switches should have been triggered now. */
  if (ISR_stack_index == -1)
    isPendingContextSwitch = 0;

  if (ISR_stack_index < (TRC_CFG_MAX_ISR_NESTING)-1) {
    ISR_stack_index++;
    ISR_stack[ISR_stack_index] = (uint32_t)handle;
#if (TRC_CFG_INCLUDE_ISR_TRACING == 1)
    prvTraceStoreEvent1(PSF_EVENT_ISR_BEGIN, (uint32_t)handle);
#endif
    TRACE_EXIT_CRITICAL_SECTION();
  } else {
    TRACE_EXIT_CRITICAL_SECTION();
    prvTraceError(PSF_ERROR_ISR_NESTING_OVERFLOW);
  }
}

/*******************************************************************************
 * vTraceStoreISREnd
 *
 * Registers the end of an Interrupt Service Routine.
 *
 * The parameter pendingISR indicates if the interrupt has requested a
 * task-switch (= 1), e.g., by signaling a semaphore. Otherwise (= 0) the
 * interrupt is assumed to return to the previous context.
 *
 * Example:
 *	 #define PRIO_OF_ISR_TIMER1 3 // the hardware priority of the interrupt
 *	 traceHandle traceHandleIsrTimer1 = 0; // The ID set by the recorder
 *	 ...
 *	 traceHandleIsrTimer1 = xTraceSetISRProperties("ISRTimer1",
 *PRIO_OF_ISR_TIMER1);
 *	 ...
 *	 void ISR_handler()
 *	 {
 *		 vTraceStoreISRBegin(traceHandleIsrTimer1);
 *		 ...
 *		 vTraceStoreISREnd(0);
 *	 }
 *
 ******************************************************************************/
void vTraceStoreISREnd(int isTaskSwitchRequired) {
  TRACE_ALLOC_CRITICAL_SECTION();

  TRACE_ENTER_CRITICAL_SECTION();

  (void)ISR_stack;

  /* Is there a pending task-switch? (perhaps from an earlier ISR) */
  isPendingContextSwitch |= isTaskSwitchRequired;

  if (ISR_stack_index > 0) {
    ISR_stack_index--;

#if (TRC_CFG_INCLUDE_ISR_TRACING == 1)
    /* Store return to interrupted ISR (if nested ISRs)*/
    prvTraceStoreEvent1(PSF_EVENT_ISR_RESUME,
                        (uint32_t)ISR_stack[ISR_stack_index]);
#endif
  } else {
    ISR_stack_index--;

    /* Store return to interrupted task, if no context switch will occur in
     * between. */
    if ((isPendingContextSwitch == 0) || (prvTraceIsSchedulerSuspended())) {
#if (TRC_CFG_INCLUDE_ISR_TRACING == 1)
      prvTraceStoreEvent1(PSF_EVENT_TASK_ACTIVATE,
                          (uint32_t)TRACE_GET_CURRENT_TASK());
#endif
    }
  }

  TRACE_EXIT_CRITICAL_SECTION();
}

/*******************************************************************************
 * xTraceGetLastError
 *
 * Returns the last error or warning, as a string, or NULL if none.
 *****************************************************************************/
const char *xTraceGetLastError(void) { return prvTraceGetError(errorCode); }

/*******************************************************************************
 * vTraceClearError
 *
 * Clears any errors.
 *****************************************************************************/
void vTraceClearError(void) {
  NoRoomForSymbol = 0;
  LongestSymbolName = 0;
  NoRoomForObjectData = 0;
  MaxBytesTruncated = 0;
#if defined(TRC_CFG_ENABLE_STACK_MONITOR) &&                                   \
    (TRC_CFG_ENABLE_STACK_MONITOR == 1) && (TRC_CFG_SCHEDULING_ONLY == 0)
  TaskStacksNotIncluded = 0;
#endif /* defined(TRC_CFG_ENABLE_STACK_MONITOR) &&                             \
          (TRC_CFG_ENABLE_STACK_MONITOR == 1) && (TRC_CFG_SCHEDULING_ONLY ==   \
          0) */
  errorCode = PSF_ERROR_NONE;
}

/*******************************************************************************
 * vTraceStop
 *
 * Stops the tracing.
 *****************************************************************************/
void vTraceStop(void) { prvSetRecorderEnabled(0); }

/*******************************************************************************
 * vTraceSetRecorderDataBuffer
 *
 * If custom allocation is used, this function must be called so the recorder
 * library knows where to save the trace data.
 ******************************************************************************/
#if (TRC_CFG_RECORDER_BUFFER_ALLOCATION ==                                     \
     TRC_RECORDER_BUFFER_ALLOCATION_CUSTOM)

extern char *_TzTraceData;

void vTraceSetRecorderDataBuffer(void *pRecorderData) {
  _TzTraceData = pRecorderData;
}
#endif

/*******************************************************************************
 * xTraceIsRecordingEnabled
 * Returns true (1) if the recorder is enabled (i.e. is recording), otherwise 0.
 ******************************************************************************/
int xTraceIsRecordingEnabled(void) { return (int)RecorderEnabled; }

void vTraceSetFilterMask(uint16_t filterMask) {
  CurrentFilterMask = filterMask;
}

void vTraceSetFilterGroup(uint16_t filterGroup) {
  CurrentFilterGroup = filterGroup;
}

/******************************************************************************
 * vTraceInitialize
 *
 * Initializes the recorder data.
 * This function will be called by vTraceEnable(...).
 * Only needs to be called manually if traced objects are created before the
 * trace recorder can be enabled, at which point make sure to call this function
 * as early as possible.
 * See TRC_CFG_RECORDER_DATA_INIT in trcConfig.h.
 ******************************************************************************/
void vTraceInitialize(void) {
  uint32_t i = 0;

  if (RecorderInitialized != 0) {
    return;
  }

  /* These are set on init so they aren't overwritten by late initialization
   * values. */
  RecorderEnabled = 0;
  NoRoomForSymbol = 0;
  LongestSymbolName = 0;
  NoRoomForObjectData = 0;
  MaxBytesTruncated = 0;
#if defined(TRC_CFG_ENABLE_STACK_MONITOR) &&                                   \
    (TRC_CFG_ENABLE_STACK_MONITOR == 1) && (TRC_CFG_SCHEDULING_ONLY == 0)
  TaskStacksNotIncluded = 0;
#endif /* defined(TRC_CFG_ENABLE_STACK_MONITOR) &&                             \
          (TRC_CFG_ENABLE_STACK_MONITOR == 1) && (TRC_CFG_SCHEDULING_ONLY ==   \
          0) */
  CurrentFilterMask = 0xFFFF;
  CurrentFilterGroup = FilterGroup0;
  uiTraceSystemState = TRC_STATE_IN_STARTUP;
  uiTraceTickCount = 0;
  timestampFrequency = 0;
  DroppedEventCounter = 0;
  ErrorAndWarningFlags = 0;
  errorCode = 0;
  isPendingContextSwitch = 0;
  trcHeapCounter = 0;
  trcHeapMax = 0;

  for (i = 0; i < (SYMBOL_TABLE_BUFFER_SIZE / sizeof(uint32_t)); i++) {
    symbolTable.SymbolTableBuffer.pSymbolTableBufferUINT32[i] = 0;
  }
  firstFreeSymbolTableIndex = 0;

  for (i = 0; i < (OBJECT_DATA_TABLE_BUFFER_SIZE / sizeof(uint32_t)); i++) {
    objectDataTable.ObjectDataTableBuffer.pObjectDataTableBufferUINT32[i] = 0;
  }
  firstFreeObjectDataTableIndex = 0;

  for (i = 0; i < TRC_CFG_MAX_ISR_NESTING; i++) {
    ISR_stack[i] = 0;
  }
  ISR_stack_index = -1;

#if defined(TRC_CFG_ENABLE_STACK_MONITOR) &&                                   \
    (TRC_CFG_ENABLE_STACK_MONITOR == 1) && (TRC_CFG_SCHEDULING_ONLY == 0)
  for (i = 0; i < TRC_CFG_STACK_MONITOR_MAX_TASKS; i++) {
    tasksInStackMonitor[i].tcb = 0;
    tasksInStackMonitor[i].uiPreviousLowMark = 0;
  }
#endif /* defined(TRC_CFG_ENABLE_STACK_MONITOR) &&                             \
          (TRC_CFG_ENABLE_STACK_MONITOR == 1) && (TRC_CFG_SCHEDULING_ONLY ==   \
          0) */

  RecorderInitialized = 1;
}

/******************************************************************************/
/*** INTERNAL FUNCTIONS *******************************************************/
/******************************************************************************/
/* Internal function for starting/stopping the recorder. */
static void prvSetRecorderEnabled(uint32_t isEnabled) {
  TRACE_ALLOC_CRITICAL_SECTION();

  if (RecorderEnabled == isEnabled) {
    return;
  }

  TRACE_ENTER_CRITICAL_SECTION();

  if (isEnabled) {
    TRC_STREAM_PORT_ON_TRACE_BEGIN();

#if (TRC_STREAM_PORT_USE_INTERNAL_BUFFER == 1)
    TRC_STREAM_PORT_INTERNAL_BUFFER_INIT();
#endif

    eventCounter = 0;
    ISR_stack_index = -1;
    prvTraceStoreHeader();
    prvTraceStoreSymbolTable();
    prvTraceStoreObjectDataTable();
    prvTraceStoreExtensionInfo();
    prvTraceStoreStartEvent();
    prvTraceStoreTSConfig();
  } else {
    TRC_STREAM_PORT_ON_TRACE_END();
  }

  RecorderEnabled = isEnabled;

  TRACE_EXIT_CRITICAL_SECTION();
}

static void prvTraceStoreStartEvent() {
  void *currentTask;

  TRACE_ALLOC_CRITICAL_SECTION();

  TRACE_ENTER_CRITICAL_SECTION();

  if (uiTraceSystemState == TRC_STATE_IN_STARTUP) {
    currentTask = (void *)HANDLE_NO_TASK;
  } else {
    currentTask = (void *)TRACE_GET_CURRENT_TASK();
  }

  eventCounter++;

  {
    TRC_STREAM_PORT_ALLOCATE_EVENT_BLOCKING(EventWithParam_3, pxEvent,
                                            sizeof(EventWithParam_3));
    if (pxEvent != NULL) {
      pxEvent->base.EventID = PSF_EVENT_TRACE_START | PARAM_COUNT(3);
      pxEvent->base.EventCount = (uint16_t)TRC_GET_EVENT_COUNT(eventCounter);
      pxEvent->base.TS = prvGetTimestamp32();
      pxEvent->param1 = (uint32_t)TRACE_GET_OS_TICKS();
      pxEvent->param2 = (uint32_t)currentTask;
      pxEvent->param3 = SessionCounter++;
      TRC_STREAM_PORT_COMMIT_EVENT_BLOCKING(pxEvent, sizeof(EventWithParam_3));
    }
  }

  TRACE_EXIT_CRITICAL_SECTION();
}

/* Store the Timestamp Config event */
static void prvTraceStoreTSConfig(void) {
  /* If not overridden using vTraceSetFrequency, use default value */
  if (timestampFrequency == 0) {
    timestampFrequency = TRC_HWTC_FREQ_HZ;
  }

  eventCounter++;

  {
#if (TRC_HWTC_TYPE == TRC_CUSTOM_TIMER_INCR ||                                 \
     TRC_HWTC_TYPE == TRC_CUSTOM_TIMER_DECR)

    TRC_STREAM_PORT_ALLOCATE_EVENT_BLOCKING(EventWithParam_5, event,
                                            sizeof(EventWithParam_5));
    if (event != NULL) {
      event->base.EventID = PSF_EVENT_TS_CONFIG | (uint16_t)PARAM_COUNT(5);
      event->base.EventCount = (uint16_t)TRC_GET_EVENT_COUNT(eventCounter);
      event->base.TS = prvGetTimestamp32();

      event->param1 = (uint32_t)timestampFrequency;
      event->param2 = (uint32_t)(TRACE_TICK_RATE_HZ);
      event->param3 = (uint32_t)(TRC_HWTC_TYPE);
      event->param4 = (uint32_t)(TRC_CFG_ISR_TAILCHAINING_THRESHOLD);
      event->param5 = (uint32_t)(TRC_HWTC_PERIOD);
      TRC_STREAM_PORT_COMMIT_EVENT_BLOCKING(event,
                                            (uint32_t)sizeof(EventWithParam_5));
    }
#else
    TRC_STREAM_PORT_ALLOCATE_EVENT_BLOCKING(EventWithParam_4, event,
                                            sizeof(EventWithParam_4));
    if (event != NULL) {
      event->base.EventID = PSF_EVENT_TS_CONFIG | (uint16_t)PARAM_COUNT(4);
      event->base.EventCount = (uint16_t)TRC_GET_EVENT_COUNT(eventCounter);
      event->base.TS = prvGetTimestamp32();

      event->param1 = (uint32_t)timestampFrequency;
      event->param2 = (uint32_t)(TRACE_TICK_RATE_HZ);
      event->param3 = (uint32_t)(TRC_HWTC_TYPE);
      event->param4 = (uint32_t)(TRC_CFG_ISR_TAILCHAINING_THRESHOLD);
      TRC_STREAM_PORT_COMMIT_EVENT_BLOCKING(event,
                                            (uint32_t)sizeof(EventWithParam_4));
    }
#endif
  }
}

/* Stores the symbol table on Start */
static void prvTraceStoreSymbolTable(void) {
  uint32_t i = 0;
  uint32_t j = 0;
  TRACE_ALLOC_CRITICAL_SECTION();

  TRACE_ENTER_CRITICAL_SECTION();

  {
    for (i = 0; i < (sizeof(SymbolTable) / sizeof(uint32_t));
         i += (SYMBOL_TABLE_SLOT_SIZE / sizeof(uint32_t))) {
      TRC_STREAM_PORT_ALLOCATE_EVENT_BLOCKING(uint32_t, data,
                                              SYMBOL_TABLE_SLOT_SIZE);

      for (j = 0; j < (SYMBOL_TABLE_SLOT_SIZE / sizeof(uint32_t)); j++) {
        data[j] = symbolTable.SymbolTableBuffer.pSymbolTableBufferUINT32[i + j];
      }

      TRC_STREAM_PORT_COMMIT_EVENT_BLOCKING(data, SYMBOL_TABLE_SLOT_SIZE);
    }
  }
  TRACE_EXIT_CRITICAL_SECTION();
}

/* Stores the object table on Start */
static void prvTraceStoreObjectDataTable(void) {
  uint32_t i = 0;
  uint32_t j = 0;
  TRACE_ALLOC_CRITICAL_SECTION();

  TRACE_ENTER_CRITICAL_SECTION();

  {
    for (i = 0; i < (sizeof(ObjectDataTable) / sizeof(uint32_t));
         i += (OBJECT_DATA_SLOT_SIZE / sizeof(uint32_t))) {
      TRC_STREAM_PORT_ALLOCATE_EVENT_BLOCKING(uint32_t, data,
                                              OBJECT_DATA_SLOT_SIZE);

      for (j = 0; j < (OBJECT_DATA_SLOT_SIZE / sizeof(uint32_t)); j++) {
        data[j] = objectDataTable.ObjectDataTableBuffer
                      .pObjectDataTableBufferUINT32[i + j];
      }

      TRC_STREAM_PORT_COMMIT_EVENT_BLOCKING(data, OBJECT_DATA_SLOT_SIZE);
    }
  }
  TRACE_EXIT_CRITICAL_SECTION();
}

/* Stores the header information on Start */
static void prvTraceStoreHeader(void) {
  int i;
  char *platform_cfg = TRC_PLATFORM_CFG;

  TRACE_ALLOC_CRITICAL_SECTION();

  TRACE_ENTER_CRITICAL_SECTION();

  {
    TRC_STREAM_PORT_ALLOCATE_EVENT_BLOCKING(PSFHeaderInfo, header,
                                            sizeof(PSFHeaderInfo));
    header->psf = PSFEndianessIdentifier;
    header->version = FormatVersion;
    header->platform = TRACE_KERNEL_VERSION;
    header->options = 0;
    for (i = 0; i < TRC_PLATFORM_CFG_LENGTH; i++) {
      header->platform_cfg[i] = platform_cfg[i];
      if (platform_cfg[i] == 0) {
        break;
      }
    }
    header->platform_cfg_patch = TRC_PLATFORM_CFG_PATCH;
    header->platform_cfg_minor = TRC_PLATFORM_CFG_MINOR;
    header->platform_cfg_major = TRC_PLATFORM_CFG_MAJOR;
    header->options = 2;
    header->numCores = TRC_CFG_PLATFORM_NUM_CORES;
    header->heapCounter = trcHeapCounter;
    header->heapMax = trcHeapMax;
    /* Lowest bit used for TRC_IRQ_PRIORITY_ORDER */
    header->options = header->options | (TRC_IRQ_PRIORITY_ORDER << 0);
    header->symbolSize = SYMBOL_TABLE_SLOT_SIZE;
    header->symbolCount = (TRC_CFG_SYMBOL_TABLE_SLOTS);
    header->objectDataSize = 8;
    header->objectDataCount = (TRC_CFG_OBJECT_DATA_SLOTS);
    TRC_STREAM_PORT_COMMIT_EVENT_BLOCKING(header, sizeof(PSFHeaderInfo));
  }
  TRACE_EXIT_CRITICAL_SECTION();
}

/* Stores the header information on Start */
static void prvTraceStoreExtensionInfo(void) {
  TRACE_ALLOC_CRITICAL_SECTION();

  TRACE_ENTER_CRITICAL_SECTION();

  {
    TRC_STREAM_PORT_ALLOCATE_EVENT_BLOCKING(PSFExtensionInfoType, extinfo,
                                            sizeof(PSFExtensionInfoType));
    memcpy(extinfo, &PSFExtensionInfo, sizeof(PSFExtensionInfoType));
    TRC_STREAM_PORT_COMMIT_EVENT_BLOCKING(extinfo,
                                          sizeof(PSFExtensionInfoType));
  }
  TRACE_EXIT_CRITICAL_SECTION();
}

/* Returns the error or warning, as a string, or NULL if none. */
static const char *prvTraceGetError(int errCode) {
  /* Note: the error messages are short, in order to fit in a User Event.
  Instead, the users can read more in the below comments.*/

  switch (errCode) {

  case PSF_WARNING_SYMBOL_TABLE_SLOTS:
    /* There was not enough symbol table slots for storing symbol names.
    The number of missing slots is counted by NoRoomForSymbol. Inspect this
    variable and increase TRC_CFG_SYMBOL_TABLE_SLOTS by at least that value. */

    return "Exceeded SYMBOL_TABLE_SLOTS (see prvTraceGetError)";

  case PSF_WARNING_SYMBOL_MAX_LENGTH:
    /* A symbol name exceeded TRC_CFG_SYMBOL_MAX_LENGTH in length.
    Make sure the symbol names are at most TRC_CFG_SYMBOL_MAX_LENGTH,
    or inspect LongestSymbolName and increase TRC_CFG_SYMBOL_MAX_LENGTH
    to at least this value. */

    return "Exceeded SYMBOL_MAX_LENGTH (see prvTraceGetError)";

  case PSF_WARNING_OBJECT_DATA_SLOTS:
    /* There was not enough symbol object table slots for storing object
    properties, such as task priorites. The number of missing slots is
    counted by NoRoomForObjectData. Inspect this variable and increase
    TRC_CFG_OBJECT_DATA_SLOTS by at least that value. */

    return "Exceeded OBJECT_DATA_SLOTS (see prvTraceGetError)";

  case PSF_WARNING_STRING_TOO_LONG:
    /* Some string argument was longer than the maximum payload size
    and has been truncated by "MaxBytesTruncated" bytes.

    This may happen for the following functions:
    - vTracePrint
    - vTracePrintF
    - vTraceStoreKernelObjectName
    - xTraceRegisterString
    - vTraceSetISRProperties

    A PSF event may store maximum 60 bytes payload, including data
    arguments and string characters. For User Events, also the User
    Event Channel (4 bytes) must be squeezed in, if a channel is
    specified (can be NULL). */

    return "String too long (see prvTraceGetError)";

  case PSF_WARNING_STREAM_PORT_READ:
    /* TRC_STREAM_PORT_READ_DATA is expected to return 0 when completed
    successfully. This means there is an error in the communication with
    host/Tracealyzer. */

    return "TRC_STREAM_PORT_READ_DATA returned error (!= 0).";

  case PSF_WARNING_STREAM_PORT_WRITE:
    /* TRC_STREAM_PORT_WRITE_DATA is expected to return 0 when completed
    successfully. This means there is an error in the communication with
    host/Tracealyzer. */

    return "TRC_STREAM_PORT_WRITE_DATA returned error (!= 0).";

  case PSF_WARNING_STACKMON_NO_SLOTS:
    /* TRC_CFG_STACK_MONITOR_MAX_TASKS is too small to monitor all tasks. */

    return "TRC_CFG_STACK_MONITOR_MAX_TASKS too small!";

  case PSF_WARNING_STREAM_PORT_INITIAL_BLOCKING:
    /* Blocking occurred during vTraceEnable. This happens if the trace buffer
    is smaller than the initial transmission (trace header, object table, and
    symbol table). */

    return "Blocking in vTraceEnable (see xTraceGetLastError)";

  case PSF_ERROR_EVENT_CODE_TOO_LARGE:
    /* The highest allowed event code is 4095, anything higher is an unexpected
    error. Please contact support@percepio.com for assistance.*/

    return "Invalid event code (see prvTraceGetError)";

  case PSF_ERROR_ISR_NESTING_OVERFLOW:
    /* Nesting of ISR trace calls exceeded the limit (TRC_CFG_MAX_ISR_NESTING).
    If this is unlikely, make sure that you call vTraceStoreISRExit in the end
    of all ISR handlers. Or increase TRC_CFG_MAX_ISR_NESTING. */

    return "Exceeded ISR nesting (see prvTraceGetError)";

  case PSF_ERROR_DWT_NOT_SUPPORTED:
    /* On ARM Cortex-M only - failed to initialize DWT Cycle Counter since not
    supported by this chip. DWT timestamping is selected automatically for ART
    Cortex-M3, M4 and higher, based on the __CORTEX_M macro normally set by
    ARM's CMSIS library, since typically available. You can however select
    SysTick timestamping instead by defining adding "#define
    TRC_CFG_ARM_CM_USE_SYSTICK".*/

    return "DWT not supported (see prvTraceGetError)";

  case PSF_ERROR_DWT_CYCCNT_NOT_SUPPORTED:
    /* On ARM Cortex-M only - failed to initialize DWT Cycle Counter since not
    supported by this chip. DWT timestamping is selected automatically for ART
    Cortex-M3, M4 and higher, based on the __CORTEX_M macro normally set by
    ARM's CMSIS library, since typically available. You can however select
    SysTick timestamping instead by defining adding "#define
    TRC_CFG_ARM_CM_USE_SYSTICK".*/

    return "DWT_CYCCNT not supported (see prvTraceGetError)";

  case PSF_ERROR_TZCTRLTASK_NOT_CREATED:
    /* vTraceEnable failed creating the trace control task (TzCtrl) - incorrect
    parameters (priority?) or insufficient heap size? */
    return "Could not create TzCtrl (see prvTraceGetError)";

  case PSF_ERROR_STREAM_PORT_WRITE:
    /* TRC_STREAM_PORT_WRITE_DATA is expected to return 0 when completed
    successfully. This means there is an error in the communication with
    host/Tracealyzer. */
    return "TRC_STREAM_PORT_WRITE_DATA returned error (!= 0).";
  }

  return NULL;
}

/* Gets the most recent tcb address */
uint32_t prvTraceGetCurrentTask(void) { return xCurrentTask; }

/* Sets the most recent tcb address */
void prvTraceSetCurrentTask(uint32_t tcb) { xCurrentTask = tcb; }

/* Begins an event with defined specified payload size. Must call
 * prvTraceEndStoreEvent() to finalize event creation. */
traceResult prvTraceBeginStoreEvent(uint32_t uiEventCode,
                                    uint32_t uiTotalPayloadSize) {
  uint32_t uiPayloadCount;

  if (RecorderEnabled == 0) {
    return TRACE_FAIL;
  }

  if (pvCurrentEvent != 0) {
    return TRACE_FAIL;
  }

  eventCounter++;

  uiCurrentEventPayloadSize = uiTotalPayloadSize;
  uiCurrentEventPayloadOffset = 0;
  uiPayloadCount = (uiTotalPayloadSize + (sizeof(uint32_t) - 1)) /
                   sizeof(uint32_t); /* 4-byte align */
  uiCurrentEventSize = sizeof(BaseEvent) + uiPayloadCount * sizeof(uint32_t);

  TRC_STREAM_PORT_ALLOCATE_EVENT(largestEventType, pxEvent, uiCurrentEventSize);
  if (pxEvent != 0) {
    pxEvent->base.EventID = (uint16_t)uiEventCode | PARAM_COUNT(uiPayloadCount);
    pxEvent->base.EventCount = (uint16_t)TRC_GET_EVENT_COUNT(eventCounter);

    pvCurrentEvent = pxEvent;
  } else {
    uiCurrentEventSize = 0;
    return TRACE_FAIL;
  }

  return TRACE_SUCCESS;
}

/* Ends the event that was begun by calling on prvTraceBeginStoreEvent() */
traceResult prvTraceEndStoreEvent() {
  if (RecorderEnabled == 0) {
    return TRACE_FAIL;
  }

  if (pvCurrentEvent == 0) {
    return TRACE_FAIL;
  }

  pvCurrentEvent->base.TS = prvGetTimestamp32();
  TRC_STREAM_PORT_COMMIT_EVENT(pvCurrentEvent, uiCurrentEventSize);

  uiCurrentEventPayloadSize = 0;
  uiCurrentEventPayloadOffset = 0;
  uiCurrentEventSize = 0;
  pvCurrentEvent = 0;

  return TRACE_SUCCESS;
}

/* Adds data of size uiSize as event payload */
traceResult prvTraceStoreEventPayload(void *pvData, uint32_t uiSize) {
  uint32_t i;

  if (pvCurrentEvent == 0) {
    return TRACE_FAIL;
  }

  if (uiCurrentEventPayloadOffset + uiSize > uiCurrentEventPayloadSize) {
    return TRACE_FAIL;
  }

  for (i = 0; i < uiSize; i++) {
    ((uint8_t *)pvCurrentEvent->data)[uiCurrentEventPayloadOffset] =
        ((uint8_t *)pvData)[i];
    uiCurrentEventPayloadOffset++;
  }

  return TRACE_SUCCESS;
}

/* Adds an uint32_t as event payload */
traceResult prvTraceStoreEventPayload32(uint32_t value) {
  if (pvCurrentEvent == 0) {
    return TRACE_FAIL;
  }

  if (uiCurrentEventPayloadOffset + sizeof(uint32_t) >
      uiCurrentEventPayloadSize) {
    return TRACE_FAIL;
  }

  /* Make sure we are writing at 32-bit aligned offset */
  if ((uiCurrentEventPayloadOffset & 3) != 0) {
    return TRACE_FAIL;
  }

  *(uint32_t *)&(
      ((uint8_t *)pvCurrentEvent->data)[uiCurrentEventPayloadOffset]) = value;
  uiCurrentEventPayloadOffset += sizeof(uint32_t);

  return TRACE_SUCCESS;
}

/* Adds an uint16_t as event payload */
traceResult prvTraceStoreEventPayload16(uint16_t value) {
  if (pvCurrentEvent == 0) {
    return TRACE_FAIL;
  }

  if (uiCurrentEventPayloadOffset + sizeof(uint16_t) >
      uiCurrentEventPayloadSize) {
    return TRACE_FAIL;
  }

  /* Make sure we are writing at 16-bit aligned offset */
  if ((uiCurrentEventPayloadOffset & 1) != 0) {
    return TRACE_FAIL;
  }

  *(uint16_t *)&(
      ((uint8_t *)pvCurrentEvent->data)[uiCurrentEventPayloadOffset]) = value;
  uiCurrentEventPayloadOffset += sizeof(uint16_t);

  return TRACE_SUCCESS;
}

/* Adds an uint8_t as event payload */
traceResult prvTraceStoreEventPayload8(uint8_t value) {
  if (pvCurrentEvent == 0) {
    return TRACE_FAIL;
  }

  if (uiCurrentEventPayloadOffset + sizeof(uint8_t) >
      uiCurrentEventPayloadSize) {
    return TRACE_FAIL;
  }

  ((uint8_t *)pvCurrentEvent->data)[uiCurrentEventPayloadOffset] = value;
  uiCurrentEventPayloadOffset += sizeof(uint8_t);

  return TRACE_SUCCESS;
}

/* Store an event with zero parameters (event ID only) */
void prvTraceStoreEvent0(uint16_t eventID) {
  TRACE_ALLOC_CRITICAL_SECTION();

  PSF_ASSERT_VOID(eventID < 4096, PSF_ERROR_EVENT_CODE_TOO_LARGE);

  TRACE_ENTER_CRITICAL_SECTION();

  if (RecorderEnabled) {
    eventCounter++;

    {
      TRC_STREAM_PORT_ALLOCATE_EVENT(BaseEvent, event, sizeof(BaseEvent));
      if (event != NULL) {
        event->EventID = eventID | PARAM_COUNT(0);
        event->EventCount = (uint16_t)eventCounter;
        event->TS = prvGetTimestamp32();
        TRC_STREAM_PORT_COMMIT_EVENT(event, sizeof(BaseEvent));
      }
    }
  }
  TRACE_EXIT_CRITICAL_SECTION();
}

/* Store an event with one 32-bit parameter (pointer address or an int) */
void prvTraceStoreEvent1(uint16_t eventID, uint32_t param1) {
  TRACE_ALLOC_CRITICAL_SECTION();

  PSF_ASSERT_VOID(eventID < 4096, PSF_ERROR_EVENT_CODE_TOO_LARGE);

  TRACE_ENTER_CRITICAL_SECTION();

  if (RecorderEnabled) {
    eventCounter++;

    {
      TRC_STREAM_PORT_ALLOCATE_EVENT(EventWithParam_1, event,
                                     sizeof(EventWithParam_1));
      if (event != NULL) {
        event->base.EventID = eventID | PARAM_COUNT(1);
        event->base.EventCount = (uint16_t)TRC_GET_EVENT_COUNT(eventCounter);
        event->base.TS = prvGetTimestamp32();
        event->param1 = (uint32_t)param1;
        TRC_STREAM_PORT_COMMIT_EVENT(event, sizeof(EventWithParam_1));
      }
    }
  }
  TRACE_EXIT_CRITICAL_SECTION();
}

/* Store an event with two 32-bit parameters */
void prvTraceStoreEvent2(uint16_t eventID, uint32_t param1, uint32_t param2) {
  TRACE_ALLOC_CRITICAL_SECTION();

  PSF_ASSERT_VOID(eventID < 4096, PSF_ERROR_EVENT_CODE_TOO_LARGE);

  TRACE_ENTER_CRITICAL_SECTION();

  if (RecorderEnabled) {
    eventCounter++;

    {
      TRC_STREAM_PORT_ALLOCATE_EVENT(EventWithParam_2, event,
                                     sizeof(EventWithParam_2));
      if (event != NULL) {
        event->base.EventID = eventID | PARAM_COUNT(2);
        event->base.EventCount = (uint16_t)TRC_GET_EVENT_COUNT(eventCounter);
        event->base.TS = prvGetTimestamp32();
        event->param1 = (uint32_t)param1;
        event->param2 = param2;
        TRC_STREAM_PORT_COMMIT_EVENT(event, sizeof(EventWithParam_2));
      }
    }
  }
  TRACE_EXIT_CRITICAL_SECTION();
}

/* Store an event with three 32-bit parameters */
void prvTraceStoreEvent3(uint16_t eventID, uint32_t param1, uint32_t param2,
                         uint32_t param3) {
  TRACE_ALLOC_CRITICAL_SECTION();

  PSF_ASSERT_VOID(eventID < 4096, PSF_ERROR_EVENT_CODE_TOO_LARGE);

  TRACE_ENTER_CRITICAL_SECTION();

  if (RecorderEnabled) {
    eventCounter++;

    {
      TRC_STREAM_PORT_ALLOCATE_EVENT(EventWithParam_3, event,
                                     sizeof(EventWithParam_3));
      if (event != NULL) {
        event->base.EventID = eventID | PARAM_COUNT(3);
        event->base.EventCount = (uint16_t)TRC_GET_EVENT_COUNT(eventCounter);
        event->base.TS = prvGetTimestamp32();
        event->param1 = (uint32_t)param1;
        event->param2 = param2;
        event->param3 = param3;
        TRC_STREAM_PORT_COMMIT_EVENT(event, sizeof(EventWithParam_3));
      }
    }
  }
  TRACE_EXIT_CRITICAL_SECTION();
}

/* Stores an event with <nParam> 32-bit integer parameters */
void prvTraceStoreEvent(int nParam, uint16_t eventID, ...) {
  va_list vl;
  int i;
  TRACE_ALLOC_CRITICAL_SECTION();

  PSF_ASSERT_VOID(eventID < 4096, PSF_ERROR_EVENT_CODE_TOO_LARGE);

  TRACE_ENTER_CRITICAL_SECTION();

  if (RecorderEnabled) {
    int eventSize = (int)sizeof(BaseEvent) + nParam * (int)sizeof(uint32_t);

    eventCounter++;

    {
      TRC_STREAM_PORT_ALLOCATE_DYNAMIC_EVENT(largestEventType, event,
                                             eventSize);
      if (event != NULL) {
        event->base.EventID = eventID | (uint16_t)PARAM_COUNT(nParam);
        event->base.EventCount = (uint16_t)TRC_GET_EVENT_COUNT(eventCounter);
        event->base.TS = prvGetTimestamp32();

        va_start(vl, eventID);
        for (i = 0; i < nParam; i++) {
          uint32_t *tmp = (uint32_t *)&(event->data[i]);
          *tmp = va_arg(vl, uint32_t);
        }
        va_end(vl);

        TRC_STREAM_PORT_COMMIT_EVENT(event, (uint32_t)eventSize);
      }
    }
  }
  TRACE_EXIT_CRITICAL_SECTION();
}

/* Stories an event with a string and <nParam> 32-bit integer parameters */
void prvTraceStoreStringEvent(int nArgs, uint16_t eventID, const char *str,
                              ...) {
  int len;
  va_list vl;

  for (len = 0; (str[len] != 0) && (len < 52); len++)
    ; /* empty loop */

  va_start(vl, str);
  prvTraceStoreStringEventHelper(nArgs, eventID, NULL, len, str, vl);
  va_end(vl);
}

/* Internal common function for storing string events */
static void prvTraceStoreStringEventHelper(int nArgs, uint16_t eventID,
                                           traceString userEvtChannel, int len,
                                           const char *str, va_list vl) {
  int nWords;
  int nStrWords;
  int i;
  int offset = 0;
  TRACE_ALLOC_CRITICAL_SECTION();

  /* The string length in multiples of 32 bit words (+1 for null character) */
  nStrWords = (len + 1 + 3) / 4;

  offset = nArgs * 4;

  /* The total number of 32-bit words needed for the whole payload */
  nWords = nStrWords + nArgs;

  if (nWords > 15) /* if attempting to store more than 60 byte (= max) */
  {
    /* Truncate event if too large. The	string characters are stored
    last, so usually only the string is truncated, unless there a lot
    of parameters... */

    /* Diagnostics ... */
    uint32_t bytesTruncated = (uint32_t)(nWords - 15) * 4;

    if (bytesTruncated > MaxBytesTruncated) {
      MaxBytesTruncated = bytesTruncated;
    }

    nWords = 15;
    len = 15 * 4 - offset;
  }

  TRACE_ENTER_CRITICAL_SECTION();

  if (RecorderEnabled) {
    int eventSize = (int)sizeof(BaseEvent) + nWords * (int)sizeof(uint32_t);

    eventCounter++;

    {
      TRC_STREAM_PORT_ALLOCATE_DYNAMIC_EVENT(largestEventType, event,
                                             eventSize);
      if (event != NULL) {
        uint32_t *data32;
        uint8_t *data8;
        event->base.EventID = (eventID) | (uint16_t)PARAM_COUNT(nWords);
        event->base.EventCount = (uint16_t)TRC_GET_EVENT_COUNT(eventCounter);
        event->base.TS = prvGetTimestamp32();

        /* 32-bit write-pointer for the data argument */
        data32 = (uint32_t *)&(event->data[0]);

        for (i = 0; i < nArgs; i++) {
          if ((userEvtChannel != NULL) && (i == 0)) {
            /* First, add the User Event Channel if not NULL */
            data32[i] = (uint32_t)userEvtChannel;
          } else {
            /* Add data arguments... */
            data32[i] = va_arg(vl, uint32_t);
          }
        }
        data8 = (uint8_t *)&(event->data[0]);
        for (i = 0; i < len; i++) {
          data8[offset + i] = str[i];
        }

        if (len < (15 * 4 - offset))
          data8[offset + len] =
              0; /* Only truncate if we don't fill up the buffer completely */
        TRC_STREAM_PORT_COMMIT_EVENT(event, (uint32_t)eventSize);
      }
    }
  }

  TRACE_EXIT_CRITICAL_SECTION();
}

/* Internal common function for storing string events without additional
 * arguments */
void prvTraceStoreSimpleStringEventHelper(uint16_t eventID,
                                          traceString userEvtChannel,
                                          const char *str) {
  int len;
  int nWords;
  int nStrWords;
  int i;
  int nArgs = 0;
  int offset = 0;
  TRACE_ALLOC_CRITICAL_SECTION();

  for (len = 0; (str[len] != 0) && (len < 52); len++)
    ; /* empty loop */

  /* The string length in multiples of 32 bit words (+1 for null character) */
  nStrWords = (len + 1 + 3) / 4;

  /* If a user event channel is specified, add in the list */
  if (userEvtChannel) {
    nArgs++;
    eventID++;
  }

  offset = nArgs * 4;

  /* The total number of 32-bit words needed for the whole payload */
  nWords = nStrWords + nArgs;

  if (nWords > 15) /* if attempting to store more than 60 byte (= max) */
  {
    /* Truncate event if too large. The	string characters are stored
    last, so usually only the string is truncated, unless there a lot
    of parameters... */

    /* Diagnostics ... */
    uint32_t bytesTruncated = (uint32_t)(nWords - 15) * 4;

    if (bytesTruncated > MaxBytesTruncated) {
      MaxBytesTruncated = bytesTruncated;
    }

    nWords = 15;
    len = 15 * 4 - offset;
  }

  TRACE_ENTER_CRITICAL_SECTION();

  if (RecorderEnabled) {
    int eventSize = (int)sizeof(BaseEvent) + nWords * (int)sizeof(uint32_t);

    eventCounter++;

    {
      TRC_STREAM_PORT_ALLOCATE_DYNAMIC_EVENT(largestEventType, event,
                                             eventSize);
      if (event != NULL) {
        uint32_t *data32;
        uint8_t *data8;
        event->base.EventID = (eventID) | (uint16_t)PARAM_COUNT(nWords);
        event->base.EventCount = (uint16_t)TRC_GET_EVENT_COUNT(eventCounter);
        event->base.TS = prvGetTimestamp32();

        /* 32-bit write-pointer for the data argument */
        data32 = (uint32_t *)&(event->data[0]);

        if (userEvtChannel != NULL) {
          /* First, add the User Event Channel if not NULL */
          data32[0] = (uint32_t)userEvtChannel;
        }

        data8 = (uint8_t *)&(event->data[0]);
        for (i = 0; i < len; i++) {
          data8[offset + i] = str[i];
        }

        if (len < (15 * 4 - offset))
          data8[offset + len] =
              0; /* Only truncate if we don't fill up the buffer completely */
        TRC_STREAM_PORT_COMMIT_EVENT(event, (uint32_t)eventSize);
      }
    }
  }

  TRACE_EXIT_CRITICAL_SECTION();
}

/* Saves a symbol name in the symbol table and returns the slot address */
void *prvTraceSaveSymbol(const char *name) {
  void *retVal = 0;
  TRACE_ALLOC_CRITICAL_SECTION();

  TRACE_ENTER_CRITICAL_SECTION();
  if (firstFreeSymbolTableIndex < SYMBOL_TABLE_BUFFER_SIZE) {
    /* The address to the available symbol table slot is the address we use */
    retVal = &symbolTable.SymbolTableBuffer
                  .pSymbolTableBufferUINT8[firstFreeSymbolTableIndex];
    prvTraceSaveObjectSymbol(retVal, name);
  }
  TRACE_EXIT_CRITICAL_SECTION();

  return retVal;
}

/* Saves a string in the symbol table for an object (task name etc.) */
void prvTraceSaveObjectSymbol(void *address, const char *name) {
  uint32_t i;
  uint8_t *ptrSymbol;
  TRACE_ALLOC_CRITICAL_SECTION();

  TRACE_ENTER_CRITICAL_SECTION();

  /* We do not look for previous entries -> changing a registered string is no
   * longer possible */
  if (firstFreeSymbolTableIndex < SYMBOL_TABLE_BUFFER_SIZE) {
    /* We access the symbol table via the union member pSymbolTableBufferUINT32
     * to avoid strict-aliasing issues */
    symbolTable.SymbolTableBuffer
        .pSymbolTableBufferUINT32[firstFreeSymbolTableIndex /
                                  sizeof(uint32_t)] = (uint32_t)address;

    /* We access the symbol table via the union member pSymbolTableBufferUINT8
     * to avoid strict-aliasing issues */
    ptrSymbol = &symbolTable.SymbolTableBuffer
                     .pSymbolTableBufferUINT8[firstFreeSymbolTableIndex +
                                              sizeof(uint32_t)];
    for (i = 0; i < (TRC_CFG_SYMBOL_MAX_LENGTH); i++) {
      ptrSymbol[i] = (uint8_t)name[i]; /* We do this first to ensure we also get
                                          the 0 termination, if there is one */

      if (name[i] == 0)
        break;
    }

    /* Check the length of "name", if longer than SYMBOL_MAX_LENGTH */
    while ((name[i] != 0) && i < 128) {
      i++;
    }

    /* Remember the longest symbol name, for diagnostic purposes */
    if (i > LongestSymbolName) {
      LongestSymbolName = i;
    }

    firstFreeSymbolTableIndex += SYMBOL_TABLE_SLOT_SIZE;
  } else {
    NoRoomForSymbol++;
  }

  TRACE_EXIT_CRITICAL_SECTION();
}

/* Deletes a symbol name (task name etc.) from symbol table */
void prvTraceDeleteSymbol(void *address) {
  uint32_t i, j;
  uint32_t *ptr, *lastEntryPtr;
  TRACE_ALLOC_CRITICAL_SECTION();

  TRACE_ENTER_CRITICAL_SECTION();

  for (i = 0; i < firstFreeSymbolTableIndex; i += SYMBOL_TABLE_SLOT_SIZE) {
    /* We access the symbol table via the union member pSymbolTableBufferUINT32
     * to avoid strict-aliasing issues */
    ptr = &symbolTable.SymbolTableBuffer
               .pSymbolTableBufferUINT32[i / sizeof(uint32_t)];
    if (*ptr == (uint32_t)address) {
      /* See if we have another entry in the table, and that this isn't already
       * the last entry */
      if (firstFreeSymbolTableIndex > SYMBOL_TABLE_SLOT_SIZE &&
          i != (firstFreeSymbolTableIndex - SYMBOL_TABLE_SLOT_SIZE)) {
        /* Another entry is available, get pointer to the last one */
        /* We access the symbol table via the union member
         * pSymbolTableBufferUINT32 to avoid strict-aliasing issues */
        lastEntryPtr =
            &symbolTable.SymbolTableBuffer
                 .pSymbolTableBufferUINT32[(firstFreeSymbolTableIndex -
                                            SYMBOL_TABLE_SLOT_SIZE) /
                                           sizeof(uint32_t)];

        /* Copy last entry to this position */
        for (j = 0; j < (SYMBOL_TABLE_SLOT_SIZE) / sizeof(uint32_t); j++) {
          ptr[j] = lastEntryPtr[j];
        }

        /* For good measure we also zero out the original position */
        *lastEntryPtr = 0;
      } else
        *ptr = 0; /* No other entry found, or this is the last entry */

      /* Lower index */
      firstFreeSymbolTableIndex -= SYMBOL_TABLE_SLOT_SIZE;

      break;
    }
  }

  TRACE_EXIT_CRITICAL_SECTION();
}

/* Saves an object data entry (current task priority) in object data table */
void prvTraceSaveObjectData(const void *address, uint32_t data) {
  uint32_t i;
  uint32_t foundSlot;
  uint32_t *ptr;
  TRACE_ALLOC_CRITICAL_SECTION();

  TRACE_ENTER_CRITICAL_SECTION();

  foundSlot = firstFreeObjectDataTableIndex;

  /* First look for previous entries using this address */
  for (i = 0; i < firstFreeObjectDataTableIndex; i += OBJECT_DATA_SLOT_SIZE) {
    /* We access the data table via the union member
     * pObjectDataTableBufferUINT32 to avoid strict-aliasing issues */
    ptr = &objectDataTable.ObjectDataTableBuffer
               .pObjectDataTableBufferUINT32[i / sizeof(uint32_t)];
    if (*ptr == (uint32_t)address) {
      foundSlot = i;
      break;
    }
  }

  if (foundSlot < OBJECT_DATA_TABLE_BUFFER_SIZE) {
    /* We access the data table via the union member
     * pObjectDataTableBufferUINT32 to avoid strict-aliasing issues */
    objectDataTable.ObjectDataTableBuffer
        .pObjectDataTableBufferUINT32[foundSlot / sizeof(uint32_t)] =
        (uint32_t)address;
    objectDataTable.ObjectDataTableBuffer
        .pObjectDataTableBufferUINT32[foundSlot / sizeof(uint32_t) + 1] = data;

    /* Is this the last entry in the object data table? */
    if (foundSlot == firstFreeObjectDataTableIndex) {
      firstFreeObjectDataTableIndex += OBJECT_DATA_SLOT_SIZE;
    }
  } else {
    NoRoomForObjectData++;
  }

  TRACE_EXIT_CRITICAL_SECTION();
}

/* Removes an object data entry (task base priority) from object data table */
void prvTraceDeleteObjectData(void *address) {
  uint32_t i, j;
  uint32_t *ptr, *lastEntryPtr;
  TRACE_ALLOC_CRITICAL_SECTION();

  TRACE_ENTER_CRITICAL_SECTION();

  for (i = 0; i < firstFreeObjectDataTableIndex; i += OBJECT_DATA_SLOT_SIZE) {
    /* We access the data table via the union member
     * pObjectDataTableBufferUINT32 to avoid strict-aliasing issues */
    ptr = &objectDataTable.ObjectDataTableBuffer
               .pObjectDataTableBufferUINT32[i / sizeof(uint32_t)];
    if (*ptr == (uint32_t)address) {
      /* See if we have another entry in the table, and that this isn't already
       * the last entry */
      if (firstFreeObjectDataTableIndex > OBJECT_DATA_SLOT_SIZE &&
          i != (firstFreeObjectDataTableIndex - OBJECT_DATA_SLOT_SIZE)) {
        /* Another entry is available, get pointer to the last one */
        /* We access the data table via the union member
         * pObjectDataTableBufferUINT32 to avoid strict-aliasing issues */
        lastEntryPtr =
            &objectDataTable.ObjectDataTableBuffer
                 .pObjectDataTableBufferUINT32[(firstFreeObjectDataTableIndex -
                                                OBJECT_DATA_SLOT_SIZE) /
                                               sizeof(uint32_t)];

        /* Copy last entry to this position */
        for (j = 0; j < (OBJECT_DATA_SLOT_SIZE) / sizeof(uint32_t); j++) {
          ptr[j] = lastEntryPtr[j];
        }

        /* For good measure we also zero out the original position */
        *lastEntryPtr = 0;
      } else
        *ptr = 0; /* No other entry found, or this is the last entry */

      /* Lower index */
      firstFreeObjectDataTableIndex -= OBJECT_DATA_SLOT_SIZE;

      break;
    }
  }

  TRACE_EXIT_CRITICAL_SECTION();
}

/* Checks if the provided command is a valid command */
int prvIsValidCommand(TracealyzerCommandType *cmd) {
  uint16_t checksum =
      (uint16_t)(0xFFFF - (cmd->cmdCode + cmd->param1 + cmd->param2 +
                           cmd->param3 + cmd->param4 + cmd->param5));

  if (cmd->checksumMSB != (unsigned char)(checksum >> 8))
    return 0;

  if (cmd->checksumLSB != (unsigned char)(checksum & 0xFF))
    return 0;

  if (cmd->cmdCode > CMD_LAST_COMMAND)
    return 0;

  return 1;
}

/* Executed the received command (Start or Stop) */
void prvProcessCommand(TracealyzerCommandType *cmd) {
  switch (cmd->cmdCode) {
  case CMD_SET_ACTIVE:
    prvSetRecorderEnabled(cmd->param1);
    break;
  default:
    break;
  }
}

/* Called on warnings, when the recording can continue. */
void prvTraceWarning(int errCode) {
  if (GET_ERROR_WARNING_FLAG(errCode) == 0) {
    /* Will never reach this point more than once per warning type, since we
     * verify if ErrorAndWarningFlags[errCode] has already been set */
    SET_ERROR_WARNING_FLAG(errCode);

    prvTraceStoreSimpleStringEventHelper(
        PSF_EVENT_USER_EVENT, trcWarningChannel, prvTraceGetError(errCode));
  }
}

/* Called on critical errors in the recorder. Stops the recorder! */
void prvTraceError(int errCode) {
  if (errorCode == PSF_ERROR_NONE) {
    /* Will never reach this point more than once, since we verify if errorCode
     * has already been set */
    errorCode = errCode;
    SET_ERROR_WARNING_FLAG(errorCode);

    prvTraceStoreSimpleStringEventHelper(
        PSF_EVENT_USER_EVENT, trcWarningChannel, prvTraceGetError(errorCode));
    prvTraceStoreSimpleStringEventHelper(PSF_EVENT_USER_EVENT,
                                         trcWarningChannel,
                                         "Recorder stopped in prvTraceError()");

    prvSetRecorderEnabled(0);
  }
}

/* If using DWT timestamping (default on ARM Cortex-M3, M4 and M7), make sure
 * the DWT unit is initialized. */
#ifndef TRC_CFG_ARM_CM_USE_SYSTICK
#if ((TRC_CFG_HARDWARE_PORT == TRC_HARDWARE_PORT_ARM_Cortex_M) &&              \
     (defined(__CORTEX_M) && (__CORTEX_M >= 0x03)))

void prvTraceInitCortexM() {
  /* Make sure the DWT registers are unlocked, in case the debugger doesn't do
   * this. */
  TRC_REG_ITM_LOCKACCESS = TRC_ITM_LOCKACCESS_UNLOCK;

  /* Make sure DWT is enabled is enabled, if supported */
  TRC_REG_DEMCR |= TRC_DEMCR_TRCENA;

  do {
    /* Verify that DWT is supported */
    if (TRC_REG_DEMCR == 0) {
      /* This function is called on Cortex-M3, M4 and M7 devices to initialize
      the DWT unit, assumed present. The DWT cycle counter is used for
      timestamping.

      If the below error is produced, the DWT unit does not seem to be
      available.

      In that case, define the macro TRC_CFG_ARM_CM_USE_SYSTICK in your build
      to use SysTick timestamping instead, or define your own timestamping by
      setting TRC_CFG_HARDWARE_PORT to TRC_HARDWARE_PORT_APPLICATION_DEFINED
      and make the necessary definitions, as explained in trcHardwarePort.h.*/

      prvTraceError(PSF_ERROR_DWT_NOT_SUPPORTED);
      break;
    }

    /* Verify that DWT_CYCCNT is supported */
    if (TRC_REG_DWT_CTRL & TRC_DWT_CTRL_NOCYCCNT) {
      /* This function is called on Cortex-M3, M4 and M7 devices to initialize
      the DWT unit, assumed present. The DWT cycle counter is used for
      timestamping.

      If the below error is produced, the cycle counter does not seem to be
      available.

      In that case, define the macro TRC_CFG_ARM_CM_USE_SYSTICK in your build
      to use SysTick timestamping instead, or define your own timestamping by
      setting TRC_CFG_HARDWARE_PORT to TRC_HARDWARE_PORT_APPLICATION_DEFINED
      and make the necessary definitions, as explained in trcHardwarePort.h.*/

      prvTraceError(PSF_ERROR_DWT_CYCCNT_NOT_SUPPORTED);
      break;
    }

    /* Reset the cycle counter */
    TRC_REG_DWT_CYCCNT = 0;

    /* Enable the cycle counter */
    TRC_REG_DWT_CTRL |= TRC_DWT_CTRL_CYCCNTENA;

  } while (0); /* breaks above jump here */
}
#endif
#endif

/* Performs timestamping using definitions in trcHardwarePort.h */
static uint32_t prvGetTimestamp32(void) {
#if ((TRC_HWTC_TYPE == TRC_FREE_RUNNING_32BIT_INCR) ||                         \
     (TRC_HWTC_TYPE == TRC_FREE_RUNNING_32BIT_DECR))
  return TRC_HWTC_COUNT;
#endif

#if ((TRC_HWTC_TYPE == TRC_CUSTOM_TIMER_INCR) ||                               \
     (TRC_HWTC_TYPE == TRC_CUSTOM_TIMER_DECR))
  return TRC_HWTC_COUNT;
#endif

#if ((TRC_HWTC_TYPE == TRC_OS_TIMER_INCR) ||                                   \
     (TRC_HWTC_TYPE == TRC_OS_TIMER_DECR))
  uint32_t ticks = TRACE_GET_OS_TICKS();
  return ((TRC_HWTC_COUNT)&0x00FFFFFFU) + ((ticks & 0x000000FFU) << 24);
#endif
}

#if defined(TRC_CFG_ENABLE_STACK_MONITOR) &&                                   \
    (TRC_CFG_ENABLE_STACK_MONITOR == 1) && (TRC_CFG_SCHEDULING_ONLY == 0)

void prvAddTaskToStackMonitor(void *task) {
  int i;
  int foundEmptySlot = 0;

  // find an empty slot
  for (i = 0; i < TRC_CFG_STACK_MONITOR_MAX_TASKS; i++) {
    if (tasksInStackMonitor[i].tcb == 0) {
      tasksInStackMonitor[i].tcb = task;
      tasksInStackMonitor[i].uiPreviousLowMark = 0xFFFFFFFF;
      foundEmptySlot = 1;
      break;
    }
  }

  if (foundEmptySlot == 0) {
    TaskStacksNotIncluded++;
  }
}

void prvRemoveTaskFromStackMonitor(void *task) {
  int i;

  for (i = 0; i < TRC_CFG_STACK_MONITOR_MAX_TASKS; i++) {
    if (tasksInStackMonitor[i].tcb == task) {
      tasksInStackMonitor[i].tcb = NULL;
      tasksInStackMonitor[i].uiPreviousLowMark = 0;
    }
  }
}

void prvReportStackUsage() {
  static int i = 0; /* Static index used to loop over the monitored tasks */
  int count = 0;    /* The number of generated reports */
  int initial =
      i; /* Used to make sure we break if we are back at the inital value */

  do {
    /* Check the current spot */
    if (tasksInStackMonitor[i].tcb != NULL) {
      /* Get the amount of unused stack */
      uint32_t unusedStackSpace =
          prvTraceGetStackHighWaterMark(tasksInStackMonitor[i].tcb);

      /* Store for later use */
      if (tasksInStackMonitor[i].uiPreviousLowMark > unusedStackSpace)
        tasksInStackMonitor[i].uiPreviousLowMark = unusedStackSpace;

      prvTraceStoreEvent2(PSF_EVENT_UNUSED_STACK,
                          (uint32_t)tasksInStackMonitor[i].tcb,
                          tasksInStackMonitor[i].uiPreviousLowMark);

      count++;
    }

    i = (i + 1) % TRC_CFG_STACK_MONITOR_MAX_TASKS; // Move i beyond this task
  } while (count < TRC_CFG_STACK_MONITOR_MAX_REPORTS && i != initial);
}
#endif /* defined(TRC_CFG_ENABLE_STACK_MONITOR) &&                             \
          (TRC_CFG_ENABLE_STACK_MONITOR == 1) && (TRC_CFG_SCHEDULING_ONLY ==   \
          0) */

#endif /*(TRC_USE_TRACEALYZER_RECORDER == 1)*/

#endif /*(TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING)*/
