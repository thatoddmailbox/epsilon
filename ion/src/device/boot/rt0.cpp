#include "isr.h"
#include <stdint.h>
#include <string.h>
#include <ion.h>
#include "../device.h"
#include "../timing.h"
#include "../console.h"

typedef void (*cxx_constructor)();

extern "C" {
  extern char _data_section_start_flash;
  extern char _data_section_start_ram;
  extern char _data_section_end_ram;
  extern char _bss_section_start_ram;
  extern char _bss_section_end_ram;
  extern cxx_constructor _init_array_start;
  extern cxx_constructor _init_array_end;
}

void abort() {
#if DEBUG
  while (1) {
  }
#else
  Ion::Device::coreReset();
#endif
}

/* By default, the compiler is free to inline any function call he wants. If the
 * compiler decides to inline some functions that make use of VFP registers, it
 * will need to push VFP them onto the stack in calling function's prologue.
 * Problem: in start()'s prologue, we would never had a chance to enable the FPU
 * since this function is the first thing called after reset.
 * We can safely assume that neither memcpy, memset, nor any Ion::Device::init*
 * method will use floating-point numbers, but ion_main very well can.
 * To make sure ion_main's potential usage of VFP registers doesn't bubble-up to
 * start(), we isolate it in its very own non-inlined function call. */
static void __attribute__((noinline)) non_inlined_ion_main() {
  return ion_main(0, nullptr);
}

void start() {
  // This is where execution starts after reset.
  // Many things are not initialized yet so the code here has to pay attention.

  /* Copy data section to RAM
   * The data section is R/W but its initialization value matters. It's stored
   * in Flash, but linked as if it were in RAM. Now's our opportunity to copy
   * it. Note that until then the data section (e.g. global variables) contains
   * garbage values and should not be used. */
  size_t dataSectionLength = (&_data_section_end_ram - &_data_section_start_ram);
  memcpy(&_data_section_start_ram, &_data_section_start_flash, dataSectionLength);

  /* Zero-out the bss section in RAM
   * Until we do, any uninitialized global variable will be unusable. */
  size_t bssSectionLength = (&_bss_section_end_ram - &_bss_section_start_ram);
  memset(&_bss_section_start_ram, 0, bssSectionLength);

  /* Initialize the FPU as early as possible.
   * For example, static C++ objects are very likely to manipulate float values */
  Ion::Device::initFPU();

#if 0
  Ion::Device::initMPU();
#endif

  /* Call static C++ object constructors
   * The C++ compiler creates an initialization function for each static object.
   * The linker then stores the address of each of those functions consecutively
   * between _init_array_start and _init_array_end. So to initialize all C++
   * static objects we just have to iterate between theses two addresses and
   * call the pointed function. */
#define SUPPORT_CPP_GLOBAL_CONSTRUCTORS 0
#if SUPPORT_CPP_GLOBAL_CONSTRUCTORS
  for (cxx_constructor * c = &_init_array_start; c<&_init_array_end; c++) {
    (*c)();
  }
#else
  /* In practice, static initialized objects are a terrible idea. Since the init
   * order is not specified, most often than not this yields the dreaded static
   * init order fiasco. How about bypassing the issue altogether? */
  if (&_init_array_start != &_init_array_end) {
    abort();
  }
#endif

  Ion::Device::init();

  non_inlined_ion_main();

  abort();
}

void __attribute__((interrupt)) isr_systick() {
  Ion::Timing::Device::MillisElapsed++;
}
