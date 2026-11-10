#include <stdio.h>
#include <stdint.h>
#include <hardware/regs/addressmap.h>
#include <pico/stdlib.h>
#include "../command.h"
#include <stdlib.h>
#include "../device/device.h"

uint32_t data_variable = 100;
uint32_t bss_variable;

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;


int main(void);


static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}

void mem_info(void)
{
    uintptr_t flash_start = XIP_BASE;
    uintptr_t flash_end   = XIP_BASE + PICO_FLASH_SIZE_BYTES;

    uintptr_t sram_start  = SRAM_BASE;
    uintptr_t sram_end    = SRAM_BASE + (264 * 1024);

    uintptr_t rom_start   = ROM_BASE;
    uintptr_t rom_end     = ROM_BASE + (16 * 1024);

    printf("%-12s %-10s %-10s %6s\n", "area", "start", "end", "size");

    row("flash", flash_start, flash_end);
    row("sram",  sram_start,  sram_end);
    row("rom",   rom_start,   rom_end);

    row("image",      (uintptr_t)&__flash_binary_start, (uintptr_t)&__flash_binary_end);
    row("free",       (uintptr_t)&__flash_binary_end,   flash_end);
    row("boot2",      (uintptr_t)&__boot2_start__,      (uintptr_t)&__boot2_end__);
    row("text",       (uintptr_t)&__boot2_end__,        (uintptr_t)&__etext);

    row("data flash", (uintptr_t)&__etext, 
                      (uintptr_t)&__etext + ((uintptr_t)&__data_end__ - (uintptr_t)&__data_start__));
                      
    row("data ram",   (uintptr_t)&__data_start__,       (uintptr_t)&__data_end__);

    row("bss",        (uintptr_t)&__bss_start__,        (uintptr_t)&__bss_end__);
    row("heap",       (uintptr_t)&__bss_end__,          (uintptr_t)&__HeapLimit);
    row("stack",      (uintptr_t)&__StackBottom,        (uintptr_t)&__StackTop);

    printf("\ntotal\n");

    
    uintptr_t flash_image_size = (uintptr_t)&__flash_binary_end - (uintptr_t)&__flash_binary_start;
    uintptr_t boot2_size       = (uintptr_t)&__boot2_end__ - (uintptr_t)&__boot2_start__;
    uintptr_t text_size        = (uintptr_t)&__etext - (uintptr_t)&__boot2_end__;
    uintptr_t data_size        = (uintptr_t)&__data_end__ - (uintptr_t)&__data_start__;
    uintptr_t bss_size         = (uintptr_t)&__bss_end__ - (uintptr_t)&__bss_start__;
    uintptr_t flash_free_size  = (XIP_BASE + PICO_FLASH_SIZE_BYTES) - (uintptr_t)&__flash_binary_end;
    uintptr_t ram_free_size    = (uintptr_t)&__HeapLimit - (uintptr_t)&__bss_end__;
    uintptr_t stack_size       = (uintptr_t)&__StackTop - (uintptr_t)&__StackBottom;
    
    
    printf("  flash image %8u = boot2 %u + text %u + data %u\n",
           (unsigned)flash_image_size, 
           (unsigned)boot2_size, 
           (unsigned)text_size, 
           (unsigned)data_size);

    printf("  flash free  %8u of %u\n", 
           (unsigned)flash_free_size, 
           (unsigned)PICO_FLASH_SIZE_BYTES);

    printf("  ram used    %8u = data %u + bss %u\n", 
           (unsigned)(data_size + bss_size), 
           (unsigned)data_size, 
           (unsigned)bss_size);

    printf("  ram free    %8u for heap and %u for stack\n", 
           (unsigned)ram_free_size, 
           (unsigned)stack_size);
}

void fw_info(void)
{
    data_variable += 1;
    bss_variable += 1;
    
    
    uint16_t *main_code = (uint16_t *)((uintptr_t)main & ~1u);
    uint16_t *fw_code = (uint16_t *)((uintptr_t)fw_info & ~1u);

    uint32_t stack_variable = 1946;
    uint32_t *heap_variable = malloc(sizeof(uint32_t));

    if (heap_variable != NULL)
    {
        *heap_variable = 1951;
    }


    printf("\n%-12s %-12s %s\n", "object", "address", "value");
    printf("%-12s 0x%08x   0x%04x \n", "main", (uintptr_t)main, *main_code);
    printf("%-12s 0x%08x   0x%04x \n", "fw_info", (uintptr_t)fw_info, *fw_code);

    printf("%-12s 0x%08x\n", "commands", (uintptr_t)&commands[0]);
    for (uint i = 0; i<command_count; i++)
    {
        uint16_t *cmd_code = (uint16_t *)((uintptr_t)commands[i].handler & ~1u);
        printf("%-12s 0x%08x   0x%04x \n", commands[i].name, (uintptr_t)commands[i].handler, 
        *cmd_code);
    }

    printf("%-12s 0x%08x   \"%s\"\n", "DEVICE_PROJECT", (uintptr_t)DEVICE_PROJECT, DEVICE_PROJECT);
    printf("%-12s 0x%08x   \"%s\"\n", "DEVICE_BOARD",   (uintptr_t)DEVICE_BOARD,   DEVICE_BOARD);
    printf("%-12s 0x%08x   %d\n", "data_variable", (uintptr_t)&data_variable, data_variable);
    printf("%-12s 0x%08x   %d\n", "bss_variable", (uintptr_t)&bss_variable, bss_variable);
    printf("%-12s 0x%08x   %u\n", "stack_variable", (uintptr_t)&stack_variable, stack_variable);
    printf("%-12s 0x%08x   %u\n", "heap_variable", (uintptr_t)heap_variable, *heap_variable);
    free(heap_variable);
    


}