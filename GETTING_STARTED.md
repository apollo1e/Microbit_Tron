# μT-Kernel 3.0 for BBC micro:bit - Getting Started Guide

## What This Repository Achieves

This repository provides a complete **μT-Kernel 3.0 Real-Time Operating System (RTOS)** development environment specifically configured for **BBC micro:bit wireless communication projects**. 

### Key Achievements:
- ✅ **Real-time Operating System**: Full μT-Kernel 3.0 implementation with preemptive multitasking
- ✅ **Wireless Communication**: Custom 2.4GHz radio driver for micro:bit-to-micro:bit communication  
- ✅ **Educational Platform**: Sample applications demonstrating RTOS concepts (tasks, delays, message buffers)
- ✅ **Complete Build System**: Ready-to-use cross-compilation environment for ARM Cortex-M4
- ✅ **Multi-platform Support**: Adaptable to various ARM microcontrollers (STM32, RX, etc.)

### What You Can Build:
- **Sender/Receiver Applications**: Two micro:bits communicating wirelessly
- **Multi-task Applications**: Real-time systems with concurrent tasks
- **IoT Edge Devices**: Small-scale embedded systems with networking
- **Educational Projects**: Learning RTOS programming concepts

---

## Prerequisites

### Hardware Required:
- **2x BBC micro:bit v2** (recommended) or v1
- **2x USB cables** (micro-USB for micro:bit)
- **Computer** running macOS, Linux, or Windows

### Software Required:

#### 1. ARM Cross-Compiler Toolchain
```bash
# macOS (using Homebrew)
brew install --cask gcc-arm-embedded

# Ubuntu/Debian
sudo apt-get update
sudo apt-get install gcc-arm-none-eabi

# Windows
# Download from: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm
```

#### 2. Make Build System
```bash
# macOS (usually pre-installed)
xcode-select --install

# Ubuntu/Debian
sudo apt-get install build-essential

# Windows
# Install MinGW or use WSL
```

#### 3. Flashing Tool
```bash
# Install pyOCD for flashing micro:bit
pip3 install pyocd

# Or use drag-and-drop method (see Flashing section)
```

---

## Quick Start Guide

### Step 1: Clone the Repository
```bash
git clone <repository-url>
cd tron
```

### Step 2: Verify Toolchain Installation
```bash
arm-none-eabi-gcc --version
# Should output version information
```

### Step 3: Build the Sender Application
```bash
cd mtkernel_3/build_make
make clean
make
```

**Expected Output:**
```
Linker: sender.elf
YOUR PROJECT HAS BEEN COOKED SIR : sender.elf
Creating HEX: sender.hex
Done converted .elf to .hex sirrr: sender.hex
```

### Step 4: Build the Receiver Application
```bash
# Edit makefile to change target
sed -i 's/EXE_FILE := sender/EXE_FILE := receiver/' makefile
make clean
make
```

### Step 5: Flash to micro:bit Devices

#### Method 1: Drag-and-Drop (Easiest)
1. Connect micro:bit to computer via USB
2. micro:bit appears as USB drive (e.g., `MICROBIT`)
3. Copy `sender.hex` to first micro:bit
4. Copy `receiver.hex` to second micro:bit
5. micro:bit will automatically restart and run the program

#### Method 2: Using pyOCD
```bash
# Flash sender to first micro:bit
pyocd flash sender.hex --target nrf52833

# Flash receiver to second micro:bit  
pyocd flash receiver.hex --target nrf52833
```

### Step 6: Test Communication
1. **Power both micro:bits** (via USB or battery pack)
2. **Open serial monitor** to see output:
   ```bash
   # macOS/Linux
   screen /dev/tty.usbmodem* 115200
   
   # Or use any serial terminal at 115200 baud
   ```
3. **Expected behavior**:
   - **Sender**: Continuously sends "hi" messages every few seconds
   - **Receiver**: Displays received messages and heartbeat dots

---

## Understanding the Applications

### Current Applications Available:

#### 1. **Sender Application** (`app_sample2/app_main.c`)
- Creates a task that sends "hi" message every few seconds
- Uses μT-Kernel task delays (`tk_dly_tsk`)
- Demonstrates basic RTOS task creation and radio transmission

#### 2. **Receiver Application** (`app_sample/app_main.c`)  
- Listens for incoming radio packets
- Uses interrupt-driven radio reception
- Displays received messages via serial output
- Shows heartbeat dots when no messages received

#### 3. **Task Delay Example** (`dly_list1/app_main.c`)
- Educational example showing multiple tasks with different priorities
- Demonstrates task scheduling and timing
- Includes logging system to track task execution

---

## Customizing Applications

### Switching Between Applications

Edit `mtkernel_3/build_make/makefile`:

```makefile
# Change this line to select different applications:
APP = app_sample2    # Sender application
# APP = app_sample   # Receiver application  
# APP = dly_list1    # Task delay example
```

### Creating Your Own Application

1. **Create application directory**:
   ```bash
   mkdir mtkernel_3/my_app
   ```

2. **Create main file** (`mtkernel_3/my_app/app_main.c`):
   ```c
   #include <tk/tkernel.h>
   #include <tm/tmonitor.h>
   
   EXPORT INT usermain(void)
   {
       tm_printf("Hello from my custom app!\n");
       
       // Your application code here
       
       tk_slp_tsk(TMO_FEVR);  // Sleep forever
       return 0;
   }
   ```

3. **Create build file** (`mtkernel_3/my_app/subdir.mk`):
   ```makefile
   C_SRCS += \
   ../my_app/app_main.c
   
   OBJS += \
   mtkernel_3/my_app/app_main.o
   
   C_DEPS += \
   mtkernel_3/my_app/app_main.d
   
   mtkernel_3/my_app/%.o: ../my_app/%.c
   	@echo 'Building file: $<'
   	$(GCC) $(CFLAGS) $(INCPATH) -D$(TARGET) -c -o "$@" "$<"
   	@echo ' '
   ```

4. **Update makefile**:
   ```makefile
   APP = my_app
   ```

5. **Build and flash**:
   ```bash
   make clean && make
   ```

---

## Configuration Options

### System Configuration (`mtkernel_3/config/config.h`)

Key settings you can modify:

```c
#define CNF_MAX_TSKID    32    // Maximum number of tasks
#define CNF_MAX_SEMID    16    // Maximum number of semaphores  
#define CNF_TIMER_PERIOD 10    // System timer period (ms)
#define CNF_MAX_TSKPRI   32    // Task priority levels
```

### Radio Configuration
Radio settings are in the radio driver (`mtkernel_3/build_make/mtkernel_3/lib/radio/radio_driver.c`):

```c
#define MICROBIT_RADIO_BASE_ADDRESS  0x75626974  // Base address
#define MICROBIT_RADIO_MAX_PACKET    32          // Max packet size
```

---

## Troubleshooting

### Build Issues

**Problem**: `arm-none-eabi-gcc: command not found`
```bash
# Solution: Install ARM toolchain
brew install --cask gcc-arm-embedded  # macOS
sudo apt-get install gcc-arm-none-eabi  # Linux
```

**Problem**: `make: command not found`
```bash
# Solution: Install build tools
xcode-select --install  # macOS
sudo apt-get install build-essential  # Linux
```

**Problem**: Linker errors about missing files
```bash
# Solution: Clean and rebuild
make clean
make
```

### Flashing Issues

**Problem**: micro:bit not detected
- Check USB cable (some are power-only)
- Try different USB port
- Press reset button on micro:bit

**Problem**: `.hex` file doesn't work
- Ensure you're using the correct `.hex` file for your target
- Try the drag-and-drop method instead of pyOCD

### Runtime Issues

**Problem**: No serial output
- Check baud rate (should be 115200)
- Try different serial terminal software
- Ensure micro:bit is properly connected

**Problem**: Radio communication not working
- Ensure both micro:bits are running compatible firmware
- Check that one is sender and one is receiver
- Verify both micro:bits are powered on

---

## Advanced Usage

### Porting to Other Platforms

The system supports multiple platforms. To target a different board:

1. **Edit makefile** to change target:
   ```makefile
   TARGET := _IOTE_STM32L4_    # For STM32L4
   TARGET := _IOTE_RX231_      # For RX231
   TARGET := _IOTE_RZA2M_      # For RZA2M
   ```

2. **Include appropriate platform makefile**:
   ```makefile
   include iote_stm32l4.mk     # Instead of microbit.mk
   ```

### Understanding μT-Kernel API

Key system calls for application development:

```c
// Task Management
ID tk_cre_tsk(const T_CTSK *ctsk);    // Create task
ER tk_sta_tsk(ID tskid, INT stacd);   // Start task  
ER tk_dly_tsk(RELTIM dlytim);         // Delay task

// Synchronization
ID tk_cre_sem(const T_CSEM *csem);    // Create semaphore
ER tk_wai_sem(ID semid, INT cnt, TMO tmout);  // Wait semaphore

// Communication  
ID tk_cre_mbf(const T_CMBF *cmbf);    // Create message buffer
ER tk_snd_mbf(ID mbfid, const void *msg, INT msgsz, TMO tmout);  // Send message
```

### Debugging

Enable debug messages in `config.h`:
```c
#define USE_TASK_DBG_MSG     (1)    // Task debug messages
#define USE_EXCEPTION_DBG_MSG (1)   // Exception debug messages
```

---

## Project Structure

```
mtkernel_3/
├── app_sample/          # Receiver application
├── app_sample2/         # Sender application  
├── dly_list1/          # Task delay example
├── build_make/         # Build system and makefiles
├── config/             # System configuration
├── kernel/             # μT-Kernel core implementation
├── device/             # Device drivers (ADC, I2C, Serial)
├── lib/                # Libraries (radio, system libraries)
├── include/            # Header files
└── docs/               # Documentation (Japanese)
```

---

## Next Steps

1. **Try the examples**: Build and run sender/receiver applications
2. **Experiment with tasks**: Modify the task delay example


## Resources

- **μT-Kernel 3.0 Specification**: IEEE 2050-2018
- **TRON Forum**: https://www.tron.org
- **ARM Cortex-M Documentation**: ARM official documentation
- **BBC micro:bit Hardware**: https://microbit.org/

---

**Happy embedded programming with μT-Kernel 3.0!** 🚀
