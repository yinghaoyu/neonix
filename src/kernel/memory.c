#include <neonix/neonix.h>
#include <neonix/memory.h>
#include <neonix/types.h>
#include <neonix/debug.h>
#include <neonix/assert.h>
#include <neonix/string.h>
#include <neonix/stdlib.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define ZONE_VALID 1    // ards 可用内存区域
#define ZONE_RESERVED 2 // ards 不可用区域

#define IDX(addr) ((u32)addr >> 12) // 获取 addr 的页索引
#define DIDX(addr) (((u32)addr >> 22) & 0x3ff) // 获取 addr 的页目录索引
#define TIDX(addr) (((u32)addr >> 12) & 0x3ff) // 获取 addr 的页表索引
#define PAGE(idx) ((u32)idx << 12)             // 获取页索引 idx 对应的页开始的位置
#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)

// 内核页目录索引
#define KERNEL_PAGE_DIR 0x1000

// 内核页表索引
static u32 KERNEL_PAGE_TABLE[] =
{
  0x2000,
  0x3000,
};

// 二级页表, 一个页目录有4k页, 一页大小4k
#define KERNEL_MEMORY_SIZE (0x100000 * sizeof(KERNEL_PAGE_TABLE))

typedef struct ards_t
{
  u64 base; // 内存基地址
  u64 size; // 内存长度
  u32 type; // 类型
} _packed ards_t;

static u32 memory_base = 0; // 可用内存基地址，应该等于 1M
static u32 memory_size = 0; // 可用内存大小
static u32 total_pages = 0; // 所有内存页数
static u32 free_pages = 0;  // 空闲内存页数

#define used_pages (total_pages - free_pages) // 已用页数

void memory_init(u32 magic, u32 addr)
{
  u32 count;
  ards_t *ptr;

  // 如果是 neonix loader 进入的内核
  if (magic == (u32)NEONIX_MAGIC)
  {
    count = *(u32 *)addr;
    ptr = (ards_t *)(addr + 4);
    for (size_t i = 0; i < count; i++, ptr++)
    {
      LOGK("Memory base 0x%p size 0x%p type %d\n",
          (u32)ptr->base, (u32)ptr->size, (u32)ptr->type);  // ards_buffer中存储的内存信息块
      if (ptr->type == ZONE_VALID && ptr->size > memory_size)  // 这里会跳过第一个type为1的块
      {
        memory_base = (u32)ptr->base;
        memory_size = (u32)ptr->size;
      }
    }
  }
  else
  {
    panic("Memory init magic unknown 0x%p\n", magic);
  }

  LOGK("ARDS count %d\n", count);
  LOGK("Memory base 0x%p\n", (u32)memory_base);
  LOGK("Memory size 0x%p\n", (u32)memory_size);

  assert(memory_base == MEMORY_BASE); // 内存开始的位置为 1M
  assert((memory_size & 0xfff) == 0); // 要求按页对齐

  total_pages = IDX(memory_size) + IDX(MEMORY_BASE);
  free_pages = IDX(memory_size);

  LOGK("Total pages %d\n", total_pages);
  LOGK("Free pages %d\n", free_pages);

  if (memory_size < KERNEL_MEMORY_SIZE)
  {
    panic("System memory is %dM too small, at least %dM needed\n",
        memory_size / MEMORY_BASE, KERNEL_MEMORY_SIZE / MEMORY_BASE);
  }
}

static u32 start_page = 0;   // 可分配物理内存起始位置
static u8 *memory_map;       // 物理内存数组
static u32 memory_map_pages; // 物理内存数组占用的页数

void memory_map_init()
{
  // 初始化物理内存数组
  memory_map = (u8 *)memory_base;

  // 计算物理内存数组占用的页数
  memory_map_pages = div_round_up(total_pages, PAGE_SIZE);
  LOGK("Memory map page count %d\n", memory_map_pages);

  free_pages -= memory_map_pages;

  // 清空物理内存数组
  memset((void *)memory_map, 0, memory_map_pages * PAGE_SIZE);

  // 前 1M 的内存位置 以及 物理内存数组已占用的页，已被占用
  start_page = IDX(MEMORY_BASE) + memory_map_pages;
  for (size_t i = 0; i < start_page; i++)
  {
    memory_map[i] = 1;
  }

  LOGK("Total pages %d free pages %d\n", total_pages, free_pages);
}

// 分配一页物理内存
static u32 get_page()
{
  for (size_t i = start_page; i < total_pages; i++)
  {
    // 如果物理内存没有占用
    if (!memory_map[i])
    {
      memory_map[i] = 1;
      free_pages--;
      assert(free_pages >= 0);
      u32 page = ((u32)i) << 12;
      LOGK("GET page 0x%p\n", page);
      return page;
    }
  }
  panic("Out of Memory!!!");
}

// 释放一页物理内存
static void put_page(u32 addr)
{
  ASSERT_PAGE(addr);

  u32 idx = IDX(addr);

  // idx 大于 1M 并且 小于 总页面数
  assert(idx >= start_page && idx < total_pages);

  // 保证至少有一个引用
  assert(memory_map[idx] >= 1);

  // 物理引用减一
  memory_map[idx]--;

  // 若为 0，则空闲页加一
  if (!memory_map[idx])
  {
    free_pages++;
  }

  assert(free_pages > 0 && free_pages < total_pages);
  LOGK("PUT page 0x%p\n", addr);
}

// 得到 cr3 寄存器
u32 inline get_cr3()
{
  // 直接将 mov eax, cr3，返回值在 eax 中
  asm volatile("movl %cr3, %eax\n");
}

// 设置 cr3 寄存器，参数是页目录的地址
void set_cr3(u32 pde)
{
  ASSERT_PAGE(pde);
  asm volatile("movl %%eax, %%cr3\n" ::"a"(pde));
}

// 将 cr0 寄存器最高位 PG 置为 1，启用分页
static _inline void enable_page()
{
  // 0b1000_0000_0000_0000_0000_0000_0000_0000
  // 0x80000000
  asm volatile(
      "movl %cr0, %eax\n"
      "orl $0x80000000, %eax\n"
      "movl %eax, %cr0\n");
}

// 初始化页表项
static void entry_init(page_entry_t *entry, u32 index)
{
  *(u32 *)entry = 0;
  entry->present = 1;
  entry->write = 1;
  entry->user = 1;
  entry->index = index;
}

// 初始化内存映射
void mapping_init()
{
  page_entry_t *pde = (page_entry_t *)KERNEL_PAGE_DIR;
  memset(pde, 0, PAGE_SIZE);

  idx_t index = 0;

  for (idx_t didx = 0; didx < (sizeof(KERNEL_PAGE_TABLE) / 4); didx++)
  {
    page_entry_t *pte = (page_entry_t *)KERNEL_PAGE_TABLE[didx];
    memset(pte, 0, PAGE_SIZE);

    page_entry_t *dentry = &pde[didx];
    entry_init(dentry, IDX((u32)pte));

    for (idx_t tidx = 0; tidx < 1024; tidx++, index++)
    {
      // 第 0 页不映射，为造成空指针访问，缺页异常，便于排错
      if (index == 0)
        continue;

      page_entry_t *tentry = &pte[tidx];
      entry_init(tentry, index);
      memory_map[index] = 1; // 设置物理内存数组，该页被占用
    }
  }

  // 将最后一个页表指向页目录自己，方便修改
  page_entry_t *entry = &pde[1023];
  entry_init(entry, IDX(KERNEL_PAGE_DIR));

  // 设置 cr3 寄存器
  set_cr3((u32)pde);

  BMB;
  // 分页有效
  enable_page();
}

static page_entry_t *get_pde()
{
  return (page_entry_t *)(0xfffff000);
}

static page_entry_t *get_pte(u32 vaddr)
{
  return (page_entry_t *)(0xffc00000 | (DIDX(vaddr) << 12));
}

// 刷新虚拟地址 vaddr 的 快表 TLB
static void flush_tlb(u32 vaddr)
{
  // 这种刷新属于局部刷新, 如果重设 cr3 寄存器就是全局刷新
  asm volatile("invlpg (%0)" ::"r"(vaddr)
      : "memory");
}

void memory_test()
{
  BMB;

  // 将 20 M 0x1400000 内存映射到 64M 0x4000000 的位置, 这个位置实际是不存在的

  // 我们还需要一个页表，0x900000

  u32 vaddr = 0x4000000; // 线性地址几乎可以是任意的
  u32 paddr = 0x1400000; // 物理地址必须要确定存在
  u32 table = 0x900000;  // 页表也必须是物理地址

  page_entry_t *pde = get_pde();

  page_entry_t *dentry = &pde[DIDX(vaddr)];
  entry_init(dentry, IDX(table));

  page_entry_t *pte = get_pte(vaddr);
  page_entry_t *tentry = &pte[TIDX(vaddr)];

  entry_init(tentry, IDX(paddr));

  BMB;

  char *ptr = (char *)(0x4000000);
  ptr[0] = 'a';

  BMB;

  entry_init(tentry, IDX(0x1500000));
  flush_tlb(vaddr);

  BMB;

  ptr[2] = 'b';

  BMB;
}
