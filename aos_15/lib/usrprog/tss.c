#include "tss.h"

void update_tss_esp(struct PCB* pthread)
{
    tss.esp0 = (uint32_t*) ((uint32_t)pthread + PAGE_SIZE);
}

static struct gdt_desc make_gdt_desc(uint32_t* desc_addr, uint32_t limit, uint8_t attr_low, uint8_t attr_high)
{
    struct gdt_desc desc;
    uint32_t desc_base        = (uint32_t) desc_addr;
    desc.limit_low_word       = limit & 0x0000ffff;
    desc.base_low_word        = desc_base & 0x0000ffff;
    desc.base_mid_byte        = ((desc_base & 0x00ff0000) >> 16);
    desc.attr_low_byte        = (uint8_t) (attr_low);
    desc.limit_high_attr_high = (((limit & 0x000f0000) >> 16) + (uint8_t) (attr_high));
    desc.base_high_byte       = desc_base >> 24;
    return desc;
}

void tss_init()
{
    put_str("tss_init start\n");
    uint32_t tss_size = sizeof(tss);
    memset(&tss, 0, tss_size);
    tss.ss0 = SELECTOR_K_STACK;
    tss.io_base = tss_size;               //没有IO位图 
    *((struct gdt_desc*)0xc0000900) = make_gdt_desc((uint32_t*)0, 0, 0, 0);
    *((struct gdt_desc*)0xc0000908) = make_gdt_desc((uint32_t*)0, 0xfffff, GDT_CODE_ATTR_LOW_DPL0, GDT_ATTR_HIGH);
    *((struct gdt_desc*)0xc0000910) = make_gdt_desc((uint32_t*)0, 0xfffff, GDT_DATA_ATTR_LOW_DPL0, GDT_ATTR_HIGH);
    *((struct gdt_desc*)0xc0000918) = make_gdt_desc((uint32_t*)0xc00b8000, 0x00000007, GDT_DATA_ATTR_LOW_DPL0, GDT_ATTR_HIGH);
    *((struct gdt_desc*)0xc0000920) = make_gdt_desc((uint32_t*)&tss, tss_size-1, TSS_ATTR_LOW, TSS_ATTR_HIGH);
    *((struct gdt_desc*)0xc0000928) = make_gdt_desc((uint32_t*)0, 0xfffff, GDT_CODE_ATTR_LOW_DPL3, GDT_ATTR_HIGH);
    *((struct gdt_desc*)0xc0000930) = make_gdt_desc((uint32_t*)0, 0xfffff, GDT_DATA_ATTR_LOW_DPL3, GDT_ATTR_HIGH);
    uint64_t gdt_operand = ((8 * 7 - 1) | ((uint64_t)(uint32_t)0xc0000900 << 16));
    asm volatile("lgdt %0" : : "m" (gdt_operand));
    asm volatile("ltr %w0" : : "r" (SELECTOR_TSS));
    put_str("tss_init and ltr done\n");
}